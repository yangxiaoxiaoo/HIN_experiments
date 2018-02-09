#include "global.h"
#include "proto.h"
#include "pruned_landmark_labeling.h"
using namespace std;


PQEntity_AStar createPQEntity_AStar(int id, float wgt, float key, std::vector<int> path){
	PQEntity_AStar entity;
	entity.nodeIdx = id;//node id
	entity.wgt = wgt;//weight of path from src to current node.
	entity.key = key;
	entity.path = path;//node ids of current path. need to change if we need to output the path itself.
	return entity;
}


PQEntity_AStar_Tree createPQEntity_AStar_Tree(int id, int id_inpattern, float wgt, float key, Instance_Tree subtree, unordered_map<int, int> subtree_vertex){

	PQEntity_AStar_Tree entity;
	//subtree_vertex[id_inpattern]= id;
	entity.node2vertex = subtree_vertex;
	entity.node2vertex[id_inpattern] = id;
	entity.nodeIdx = id;//current node id
	entity.curId_inpattern = id_inpattern;
	entity.wgt = wgt;//weight of subtree
	entity.key = key;
	entity.subtree = subtree;//node ids of current path -> now turned into subtree (visited part from root).
	return entity;
}

//Vertex current id is in the subtree expanded so far. And now we are inserting neighbor vertex. The boolean on_left captures whether ot not the neighbor is a left child
Instance_Tree Instance_Tree_Insert(Instance_Tree subtree, int curId, int neigh,float edgewgt,bool on_left){
    Instance_Tree result = subtree;
    if (on_left==true){
        result.map2leftcdr[curId] = neigh;
        result.map2parent[neigh] = curId;
        result.nodes.insert(neigh);
    }
    else{
        result.map2rightcdr[curId] = neigh;
        result.map2parent[neigh] = curId;
        result.nodes.insert(neigh);
    }

    result.wgt += edgewgt;
   return result;

}

Instance_Tree_rep Instance_Tree_Insert_rep(Instance_Tree_rep subtree, int curId, int neigh,float edgewgt,bool on_left){

    if (on_left==true){
        subtree.map2leftcdr[curId] = neigh;
        subtree.map2parent[neigh] = curId;
        subtree.nodes.insert(neigh);
    }
    else{
        subtree.map2rightcdr[curId] = neigh;
        subtree.map2parent[neigh] = curId;
        subtree.nodes.insert(neigh);
    }

    subtree.wgt += edgewgt;
   return subtree;

}


QueryResult AStar_Prophet(const graph_t& g, Query query, double& timeUsed){
	int maxDepth = query.pattern.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResult qResult;
        if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
                cout << query.src << " to " << query.tgt << endl;
                cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResult;
        }
	int midLayer = query.pattern.size()/2 +1;//half forward and halp backward. And they meet and terminate at the same layer.
	vector<unordered_map<int, float> > layers = create_Prophet_Heuristics(g, query, timeUsed);//layers stores the legitimate nodes on each level. Each layer is a set.

	std::priority_queue<PQEntity_AStar, std::vector<PQEntity_AStar>, comparator_AStar> frontier;
	PQEntity_AStar curNode;
	vector<int> tmpPath;
	tmpPath.push_back(query.src);
	frontier.push(createPQEntity_AStar(query.src, 0, 0, tmpPath));//frontier is the priority queue.
	int mem = 0, total = 1;
	while(!frontier.empty()){
		mem = max(mem, (int)frontier.size());
		PQEntity_AStar curNode;
		curNode = frontier.top();
		frontier.pop();
		int curId = curNode.nodeIdx;
		int depth = curNode.path.size()-1;//start from 0.
		vector<int> path = curNode.path;
		if(depth==maxDepth){//reach the end of pattern.
			if(curId == query.tgt && curNode.wgt<MAX_WEIGHT){
				if(qResult.paths.size()<TOP_K)
                                        qResult.paths.push_back(createPath(curNode.wgt, path));
                                if(qResult.paths.size()==TOP_K)
                                        break;
			}
			continue;
		}
		//expanding the neighbors of current node.
		for(int i=0; i<g.degree[curId]; i++){
			int neigh = g.neighbors[g.nodes[curId]+i];//neighbor id.
			//if neighbor is not in the path and follow the pattern.
			//cout << depth << endl; cout << layers.size() << endl;
			unordered_map<int, float>::iterator found = layers[depth+1].find(neigh);
			float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], query.time);
			if( found != layers[depth+1].end() && find(path.begin(), path.end(), neigh) == path.end() && edgwgt+curNode.wgt<MAX_WEIGHT){
				vector<int> newPath = path;
				newPath.push_back(neigh);
				frontier.push(createPQEntity_AStar(neigh, edgwgt+curNode.wgt, edgwgt+curNode.wgt+found->second, newPath));
				total += 1;
			}
		}

	}
	qResult.mem = mem;
        qResult.totalPaths = total;
        return qResult;
}





//WWW version of expand current PET. return true if it is front element, else not.
//Execute the expansion and insert non-front element to priority queue.
bool Expand_current_v2(const graph_t& g, Query_tree querytree, vector <int> pre_order_patterns, int& curId, PQEntity_AStar_Tree& curNode,
               Instance_Tree subtree, int& total,unordered_map<int, unordered_map<int, tuple<float, float>>> node2layers, int curId_inpattern,
               std::priority_queue<PQEntity_AStar_Tree, std::vector<PQEntity_AStar_Tree>, comparator_AStar_Tree>& frontier, int& numTrees, unordered_map<int,  unordered_map<int, unordered_map<int, float>>> & candidxleft, unordered_map<int,  unordered_map<int, unordered_map<int, float>>> & candidxright){
                   //each expanding operation may change: total, frontier, curId, curNode.  other variables wont change.
        bool on_left = true;
        float old_key = curNode.key; //use this to compare with the new key later to decide it front-optimization.
        bool front_found = false;
        curId = curNode.nodeIdx; //must update! otherwise wrong for front element.

        PQEntity_AStar_Tree Top_element;

		if (find(querytree.junctions.begin(),querytree.junctions.end(), curId_inpattern) == querytree.junctions.end()){
            //curId is not a junction
            if (find(querytree.terminals.begin(),querytree.terminals.end(), curId_inpattern) != querytree.terminals.end()){
                //--curId is a terminal
                if (curId_inpattern == querytree.terminals.back()){ //it is the last terminal
                    return 0;
                }

                int next_id_pattern = *(find(pre_order_patterns.begin(), pre_order_patterns.end(), curId_inpattern)+1); //curId_impattern's next one in pre_order_patterns, going to traverse him!
              //old parent is a VERTEX id
                int old_parent_inpattern = querytree.map2parent[next_id_pattern];
                int old_parent = curNode.node2vertex[old_parent_inpattern];
                cout<<"old parent is" <<old_parent<<endl;
                on_left = false; //right neighbor.
                unordered_map<int, float> child_wgt = candidxright[old_parent_inpattern][old_parent];
                for(auto it = child_wgt.begin(); it!= child_wgt.end(); it++){
                    int neigh = it->first;
                    float edgwgt = it->second;
                    subtree = curNode.subtree;
                    if(find(subtree.nodes.begin(), subtree.nodes.end(),neigh)== subtree.nodes.end()){
                        Instance_Tree new_subtree = Instance_Tree_Insert(subtree, old_parent, neigh, edgwgt, on_left);
                        float traversed_wgt = edgwgt+subtree.wgt;

                        unordered_map<int, tuple<float,float>>::iterator found = node2layers[next_id_pattern].find(neigh);
                        assert (found != node2layers[next_id_pattern].end() );
                        float leftvalue = get<0>(node2layers[old_parent_inpattern][old_parent]);
                        float rightvalue = get<1>(node2layers[old_parent_inpattern][old_parent]);

                        float key = old_key - rightvalue + (edgwgt + std::get<0>(found->second) + std::get<1>(found->second));



                        unordered_map<int, int> new_node2vertex = curNode.node2vertex;
                        new_node2vertex[next_id_pattern]= neigh;
                        if (!front_found && (key == old_key)){ //this is a front element! the first fond wont be inserted into PQ.

                                front_found = true;
                                Top_element = createPQEntity_AStar_Tree
                                              (neigh, next_id_pattern, traversed_wgt, key, new_subtree, new_node2vertex);
                            }
                        else{ //not a front element
                            frontier.push(createPQEntity_AStar_Tree
                                              (neigh, next_id_pattern, traversed_wgt, key, new_subtree, new_node2vertex));
                            total += 1;
                        }
                    }
                }
            }
            else{  //curId is not a terminal, not a junction, just on path
                int onlychild;
                unordered_map<int,  unordered_map<int, unordered_map<int, float>>> candidx;
                if (querytree.map2leftcdr.find(curId_inpattern)!=querytree.map2leftcdr.end()){ //has a left child
                    onlychild = querytree.map2leftcdr[curId_inpattern];
                    on_left = true;
                    candidx = candidxleft;
                }
                else {
                        if(querytree.map2rightcdr.find(curId_inpattern)!=querytree.map2rightcdr.end()){
                            onlychild = querytree.map2rightcdr[curId_inpattern];
                            on_left = false;
                            candidx = candidxright;
                        }
                        else {
                                return 0;
                        }
                }

                unordered_map<int, float> child_wgt = candidx[curId_inpattern][curId];
                for(auto it = child_wgt.begin(); it!= child_wgt.end(); it++){
                    int neigh = it->first;
                    float edgwgt = it->second;
                    subtree = curNode.subtree;
                    if(find(subtree.nodes.begin(), subtree.nodes.end(),neigh)== subtree.nodes.end()){
                        Instance_Tree new_subtree = Instance_Tree_Insert(subtree, curId, neigh, edgwgt, on_left);

                        float traversed_wgt = edgwgt+curNode.wgt;
                        unordered_map<int, tuple<float,float>>::iterator found = node2layers[onlychild].find(neigh);
                        float leftvalue = get<0>(node2layers[curId_inpattern][curId]);
                        float rightvalue = get<1>(node2layers[curId_inpattern][curId]);
                        float key;
                        if (on_left) key = old_key - leftvalue + (edgwgt + std::get<0>(found->second) + std::get<1>(found->second));
                        else key = old_key - rightvalue + (edgwgt + std::get<0>(found->second) + std::get<1>(found->second));

                        assert(key >= old_key - 0.001);
                        unordered_map<int, int> new_node2vertex = curNode.node2vertex;
                        new_node2vertex[onlychild]= neigh;

                        assert(key >= old_key);
                        if (!front_found && (key == old_key)){ //this is a front element! the first fond wont be inserted into PQ.

                                front_found = true;
                                Top_element = (createPQEntity_AStar_Tree
                                              (neigh, onlychild, traversed_wgt,key , new_subtree, new_node2vertex));

                            }
                        else{ //not a front element
                            frontier.push(createPQEntity_AStar_Tree
                                              (neigh, onlychild, traversed_wgt,key , new_subtree, new_node2vertex));
                            total += 1;
                        }
                    }
                }
            }
		}
		else{ //curId is a junction.
            //first time visiting a junction: found in left child candidates and right child candidate.
            int leftchild, rightchild;
            leftchild = querytree.map2leftcdr[curId_inpattern];
            rightchild = querytree.map2rightcdr[curId_inpattern];
            on_left = true;
            //pushback with left child, update the right
            unordered_map<int, float> child_wgt = candidxleft[curId_inpattern][curId];
                          for(auto it = child_wgt.begin(); it!= child_wgt.end(); it++){
                int neigh = it->first;
                float edgwgt = it->second;
                subtree = curNode.subtree;
                if(find(subtree.nodes.begin(), subtree.nodes.end(),neigh)== subtree.nodes.end()){
                    Instance_Tree new_subtree = Instance_Tree_Insert(subtree, curId, neigh, edgwgt, on_left);


                    float rightvalue = get<1>(node2layers[curId_inpattern][curId]);
                    float traversed_wgt = edgwgt+curNode.wgt;
                    //assert( new_subtree.wgt == traversed_wgt );
                    unordered_map<int, tuple<float,float>>::iterator found = node2layers[leftchild].find(neigh);
                    float leftvalue = get<0>(node2layers[curId_inpattern][curId]);
                    float key;
                    key = old_key - leftvalue + (edgwgt + std::get<0>(found->second) + std::get<1>(found->second)) ;

                    unordered_map<int, int> new_node2vertex = curNode.node2vertex;
                    new_node2vertex[leftchild]= neigh;
                    if (!front_found && (key == old_key)){ //this is a front element! the first fond wont be inserted into PQ.
                            front_found = true;
                            Top_element = (createPQEntity_AStar_Tree
                                          (neigh, leftchild, traversed_wgt,key , new_subtree, new_node2vertex));

                        }
                    else{ //not a front element
                        frontier.push(createPQEntity_AStar_Tree
                                          (neigh, leftchild, traversed_wgt,key , new_subtree, new_node2vertex));
                        total += 1;
                    }
                }
            }

        }
    if (front_found) curNode = Top_element;

    return front_found;

}




std::vector<PQEntity_AStar_Tree>  Expand_brute_v2(const graph_t& g, Query_tree querytree, vector <int> pre_order_patterns, int& curId, PQEntity_AStar_Tree& curNode,
               Instance_Tree subtree, int& total,unordered_map<int, unordered_map<int, tuple<float, float>>> node2layers, int curId_inpattern,
               int& numTrees, unordered_map<int,  unordered_map<int, unordered_map<int, float>>> & candidxleft, unordered_map<int,  unordered_map<int, unordered_map<int, float>>> & candidxright){
                   //each expanding operation may change: total, frontier, curId, curNode.  other variables wont change.
        std::vector<PQEntity_AStar_Tree> results;
        bool on_left = true;
        float old_key = curNode.key; //use this to compare with the new key later to decide it front-optimization.
        bool front_found = false;
        subtree = curNode.subtree;

		if (find(querytree.junctions.begin(),querytree.junctions.end(), curId_inpattern) == querytree.junctions.end()){
            //curId is not a junction
            if (find(querytree.terminals.begin(),querytree.terminals.end(), curId_inpattern) != querytree.terminals.end()){
                //--curId is a terminal
                if (curId_inpattern == querytree.terminals.back()){ //it is the last terminal
                    return results;
                }

                int next_id_pattern = *(find(pre_order_patterns.begin(), pre_order_patterns.end(), curId_inpattern)+1); //curId_impattern's next one in pre_order_patterns, going to traverse him!
              //old parent is a VERTEX id
                int old_parent_inpattern = querytree.map2parent[next_id_pattern];
                int old_parent = curNode.node2vertex[old_parent_inpattern];
                on_left = false; //right neighbor.
                unordered_map<int, float> child_wgt = candidxright[old_parent_inpattern][old_parent];
                for(auto it = child_wgt.begin(); it!= child_wgt.end(); it++){
                    int neigh = it->first;
                    float edgwgt = it->second;
                    if(find(subtree.nodes.begin(), subtree.nodes.end(),neigh)== subtree.nodes.end()){
                    //check if vertex has been used!
                        Instance_Tree new_subtree = Instance_Tree_Insert(subtree, old_parent, neigh, edgwgt, on_left);
                        float traversed_wgt = edgwgt+curNode.wgt;
                        new_subtree.wgt = traversed_wgt;
                        unordered_map<int, tuple<float,float>>::iterator found = node2layers[next_id_pattern].find(neigh);
                        float key = 0;
                        unordered_map<int, int> new_node2vertex = curNode.node2vertex;
                        new_node2vertex[next_id_pattern]= neigh;
                        results.push_back(createPQEntity_AStar_Tree
                                              (neigh, next_id_pattern, traversed_wgt, key, new_subtree, new_node2vertex));
                        total += 1;
                    }
                }
            }
            else{  //curId is not a terminal, not a junction, just on path
                int onlychild;
                unordered_map<int,  unordered_map<int, unordered_map<int, float>>> candidx;
                if (querytree.map2leftcdr.find(curId_inpattern)!=querytree.map2leftcdr.end()){ //has a left child
                    onlychild = querytree.map2leftcdr[curId_inpattern];
                    on_left = true;
                    candidx = candidxleft;
                }
                else {
                        if(querytree.map2rightcdr.find(curId_inpattern)==querytree.map2rightcdr.end()){
                            onlychild = querytree.map2rightcdr[curId_inpattern];
                            on_left = false;
                            candidx = candidxright;
                        }
                        else {
                                return results;
                        }
                }

                unordered_map<int, float> child_wgt = candidx[curId_inpattern][curId];
                for(auto it = child_wgt.begin(); it!= child_wgt.end(); it++){
                    int neigh = it->first;
                    float edgwgt = it->second;
                    if(find(subtree.nodes.begin(), subtree.nodes.end(),neigh)== subtree.nodes.end()){

                        Instance_Tree new_subtree = Instance_Tree_Insert(subtree, curId, neigh, edgwgt, on_left);
                        float traversed_wgt = edgwgt+curNode.wgt;
                        new_subtree.wgt = traversed_wgt;
                        unordered_map<int, tuple<float,float>>::iterator found = node2layers[onlychild].find(neigh);
                        float key = 0;
                        unordered_map<int, int> new_node2vertex = curNode.node2vertex;
                        new_node2vertex[onlychild]= neigh;
                        results.push_back(createPQEntity_AStar_Tree
                                              (neigh, onlychild, traversed_wgt,key , new_subtree, new_node2vertex));
                        total += 1;
                    }
                }
            }
		}
		else{ //curId is a junction.
            //first time visiting a junction: found in left child candidates and right child candidate.
            int leftchild, rightchild;
            leftchild = querytree.map2leftcdr[curId_inpattern];
            rightchild = querytree.map2rightcdr[curId_inpattern];
            on_left = true;
            //pushback with left child, update the right
            unordered_map<int, float> child_wgt = candidxleft[curId_inpattern][curId];
                          for(auto it = child_wgt.begin(); it!= child_wgt.end(); it++){
                int neigh = it->first;
                float edgwgt = it->second;
                if(find(subtree.nodes.begin(), subtree.nodes.end(),neigh)== subtree.nodes.end()){

                    Instance_Tree new_subtree = Instance_Tree_Insert(subtree, curId, neigh, edgwgt, on_left);
                    float rightvalue = get<1>(node2layers[curId_inpattern][curId]);
                    float traversed_wgt = edgwgt+curNode.wgt;
                    new_subtree.wgt = traversed_wgt;
                    unordered_map<int, tuple<float,float>>::iterator found = node2layers[leftchild].find(neigh);
                    float key = 0;
                    unordered_map<int, int> new_node2vertex = curNode.node2vertex;
                    new_node2vertex[leftchild]= neigh;
                    //cout<< key<<"replace" <<old_key<<endl;
                    //assert(key >= old_key);
                    results.push_back(createPQEntity_AStar_Tree
                                          (neigh, leftchild, traversed_wgt,key , new_subtree, new_node2vertex));
                    total += 1;
                }
            }

        }

    return results;

}




//insert all candidates of this_node , append it next to check_connection_node in the specified way.
vector<Instance_Tree> Set_insert(const graph_t& g, std::unordered_map<int, int> node2pattern, Instance_Tree Old_tree, int this_node, int check_connection_node, bool insert_parent, bool insert_left, unordered_map<int, int> vertex2node,     unordered_map<int, unordered_map<int, float>> node2layers, int& numtrees){
    int old_node; //the vertex from
    int newnode_candidate;
    unordered_set<int> nodes_insertion;
    vector<Instance_Tree> new_trees;
    int existing_vertex;
    for (auto itr = Old_tree.nodes.begin(); itr != Old_tree.nodes.end(); ++itr){
       // for  existing_vertex in Old_tree.nodes:
        existing_vertex = *itr;
        if (vertex2node[existing_vertex] == check_connection_node){
            old_node = existing_vertex;
            break;
        }
    }
   // for newnode_candidate in node2layers[this_node]:{ //rewrite in a way that calculate weight is easy...
    for(int i=0; i<g.degree[old_node]; i++){ //for all neighbors of old_node, if any is in node2layers[this_node], that one is a new candidate
        newnode_candidate = g.neighbors[g.nodes[old_node]+i];
        if (g.typeMap[newnode_candidate] == node2pattern[this_node] && find(Old_tree.nodes.begin(), Old_tree.nodes.end(),newnode_candidate)== Old_tree.nodes.end()){
            unordered_map<int, float>::iterator found = node2layers[this_node].find(newnode_candidate);
            numtrees += node2layers[this_node].size();
            //if newnode_candidate is in node2layers[this_node]
            if (found!=node2layers[this_node].end()){
                Instance_Tree new_tree;
                if (insert_parent){
                    new_tree.map2parent[old_node] = newnode_candidate;
                    if (insert_left){ //check_connection is left child of this_node
                        new_tree.map2leftcdr[newnode_candidate] = old_node;
                    }
                    else{
                        new_tree.map2rightcdr[newnode_candidate] = old_node;
                    }
                }
                else{ //this_node is a child of old node
                    new_tree.map2parent[newnode_candidate] = old_node;
                    if (insert_left){
                        new_tree.map2leftcdr[old_node] = newnode_candidate;
                    }
                    else{
                        new_tree.map2rightcdr[old_node] = newnode_candidate;
                    }
                }
                nodes_insertion = Old_tree.nodes; //.insert(newnode_candidate);
                nodes_insertion.insert(newnode_candidate);
                new_tree.nodes = nodes_insertion;
                new_tree.wgt = Old_tree.wgt + calcWgt(g.wgts[g.nodes[old_node]+i], 0);//no-time deterministic experiments in baseline, query time set as 0
                if (new_tree.nodes.size()>Old_tree.nodes.size())
                    new_trees.push_back(new_tree);
            }
        }
    }
    return new_trees;
}



//grow the current instanse tree by one node, and return a list of expanded trees
//expand one node that is not the same as anything in imcomplete_tree or complete_instances
//dont have to check type since prophet graph has already give us the vertex2node and node2vertex with type constraints.
vector<Instance_Tree> expend_withcheck(const graph_t& g, unordered_map<int, int> vertex2node, unordered_map<int, unordered_map<int, float>> node2layers, Query_tree querytree, Instance_Tree incomplete_tree, int& numtrees){
	vector<Instance_Tree> new_trees;
	Instance_Tree new_tree;
    bool connected = false; //fpr each check_connection_node
    bool found = false; //for each this_node
    bool insert_parent = false; //if this_node is parent of existing check_connection_node
    bool insert_left = false; //can mean new inserted node is on the left, or can mean cexisting node is inserted node's left child
    int curnode;

    for (int i = 0; i< querytree.nodes_ordered.size(); i++){ //enumerate all placeholder in pattern as this node
        int this_node = querytree.nodes_ordered[i];
        bool this_node_inmapped = false;
        for (auto itr = incomplete_tree.nodes.begin(); itr != incomplete_tree.nodes.end(); ++itr){
            if(this_node == vertex2node[*itr]){
                this_node_inmapped = true;
            }
        }
        //if ( this_node not in incomplete_tree.mapped_nodes){ //a unfixed node: is it connected?
        if (!this_node_inmapped){
            int check_connection_node;
            //for check_connection_node in incomplete_tree.mapped_nodes:
            for (auto itr = incomplete_tree.nodes.begin(); itr != incomplete_tree.nodes.end(); ++itr){
                check_connection_node = vertex2node[*itr];
                connected = false; //for every checked node, see if it is connected
                //if this_node is parent of check_connection_node{
                if (querytree.map2parent.find(check_connection_node)!= querytree.map2parent.end()
                    &&querytree.map2parent[check_connection_node] == this_node){
                    connected = true;
                    insert_parent = true;
                    //if check_connection_node is leftchild of this_node{
                    if (querytree.map2leftcdr.find(this_node)!= querytree.map2leftcdr.end()
                        &&querytree.map2leftcdr[this_node] == check_connection_node){
                        insert_left = true;
                        break; //inner for loop
                    }
                    //if check_connection_node is rightchild of this_node{
                    if (querytree.map2rightcdr.find(this_node)!= querytree.map2rightcdr.end()
                        &&querytree.map2rightcdr[this_node] == check_connection_node){
                        insert_left = false;
                        break;
                    }
                }
                else {
                    //if this_node is leftchild of check_connection_node{

                    if (querytree.map2leftcdr.find(check_connection_node)!= querytree.map2leftcdr.end()
                        &&querytree.map2leftcdr[check_connection_node] == this_node){
                        connected = true;
                        insert_parent = false;
                        insert_left = true;
                        break;
                    }
                    //else if this_node is rightchild of check_connection_node{
                    else if (querytree.map2rightcdr.find(check_connection_node)!= querytree.map2rightcdr.end()
                             &&querytree.map2rightcdr[check_connection_node] == this_node){
                        connected = true;
                        insert_parent = false;
                        insert_left = false;
                        break;
                    }
                }

                //now we have checked this_node. if connected is true, it is connected to some and should insert next to check_connection_nodes


                //if not connected, move to the next check_connection_node
            }//end of inner for loop. here we have looked at all check_connection_node that exists.
            if (connected){

                new_trees = Set_insert(g, querytree.map2patthern, incomplete_tree, this_node, check_connection_node, insert_parent, insert_left, vertex2node, node2layers, numtrees);
                found = true;
                break;//outer loop break, do not check other this_node.
            }
        }
        if (found) break;

    }//here we have examined all this_node that is not there.

    return new_trees;
}
//done

vector<Instance_Tree> expend_withoutcheck(const graph_t& g, unordered_map<int, int> vertex2node, unordered_map<int, unordered_map<int, float>> node2layers, Query_tree querytree, Instance_Tree incomplete_tree, int& numtrees){
	vector<Instance_Tree> new_trees;
	Instance_Tree new_tree;
    bool connected = false; //fpr each check_connection_node
    bool found = false; //for each this_node
    bool insert_parent = false; //if this_node is parent of existing check_connection_node
    bool insert_left = false; //can mean new inserted node is on the left, or can mean cexisting node is inserted node's left child
    int curnode;

    for (int i = 0; i< querytree.nodes_ordered.size(); i++){ //enumerate all placeholder in pattern as this node
        int this_node = querytree.nodes_ordered[i];
        bool this_node_inmapped = false; //always false without check!


        if (!this_node_inmapped){
            int check_connection_node;
            for (auto itr = incomplete_tree.nodes.begin(); itr != incomplete_tree.nodes.end(); ++itr){
                check_connection_node = vertex2node[*itr];
                connected = false;
                if (querytree.map2parent.find(check_connection_node)!= querytree.map2parent.end()
                    &&querytree.map2parent[check_connection_node] == this_node){
                    connected = true;
                    insert_parent = true;
                    if (querytree.map2leftcdr.find(this_node)!= querytree.map2leftcdr.end()
                        &&querytree.map2leftcdr[this_node] == check_connection_node){
                        insert_left = true;
                        break;
                    }
                    if (querytree.map2rightcdr.find(this_node)!= querytree.map2rightcdr.end()
                        &&querytree.map2rightcdr[this_node] == check_connection_node){
                        insert_left = false;
                        break;
                    }
                }
                else {

                    if (querytree.map2leftcdr.find(check_connection_node)!= querytree.map2leftcdr.end()
                        &&querytree.map2leftcdr[check_connection_node] == this_node){

                        connected = true;
                        insert_parent = false;
                        insert_left = true;
                        break;
                    }

                    else if (querytree.map2rightcdr.find(check_connection_node)!= querytree.map2rightcdr.end()
                             &&querytree.map2rightcdr[check_connection_node] == this_node){

                        connected = true;
                        insert_parent = false;
                        insert_left = false;
                        break;
                    }
                }
            }
            if (connected){

                new_trees = Set_insert(g, querytree.map2patthern, incomplete_tree, this_node, check_connection_node, insert_parent, insert_left, vertex2node, node2layers, numtrees);
                found = true;
                break;
            }

        }
        if (found) break;
    }
    return new_trees;
}

//a comparison that takes a parameter and return an operator
struct compare_srtuct{
    compare_srtuct(vector<PQEntity_AStar_Tree> complete_trees) {this -> complete_trees = complete_trees; }
    bool operator() (int i, int j) {//both i and j are index that we want to compare weight on
        return (complete_trees[i].wgt < complete_trees[j].wgt);
    }
    vector<PQEntity_AStar_Tree> complete_trees;
};

struct compare_entity{
    compare_entity(vector<PQEntity_AStar_Tree> complete_trees) {this -> complete_trees = complete_trees; }
    bool operator() (PQEntity_AStar_Tree tree_i, PQEntity_AStar_Tree tree_j) {//both i and j are index that we want to compare weight on
        return (tree_i.wgt < tree_j.wgt);
    }
    vector<PQEntity_AStar_Tree> complete_trees;
};


//sort a list of matching trees and return top k lightest ones. TODO: only return top k.
vector<PQEntity_AStar_Tree> Top_k_weight(vector<PQEntity_AStar_Tree> complete_trees){
    vector<PQEntity_AStar_Tree> Top_k_trees = complete_trees;
    sort(Top_k_trees.begin(), Top_k_trees.end(), compare_entity(Top_k_trees));
    return Top_k_trees;

}



//Take graph g and the query tree, change the vertices to node map, the mapback to the prophet mapping that discribes all that matches type
int typecheck_all(const graph_t& g, Query_tree querytree, unordered_map<int, int>& vertex2node, unordered_map<int, unordered_map<int, float>>& node2layers){
    int iterationnum = querytree.patterns.size()-1;
    float minWgt = MAX_WEIGHT;
    for(int i=0; i<querytree.terminals_index.size(); i++){
        if( (g.typeMap[querytree.nodes_ordered[querytree.terminals_index[i]]]!=querytree.patterns[querytree.terminals_index[i]])){
            cout<< "src or tgt node does not follow pattern!" << endl;
            return 1;
            }
    }
    vector<GeneralizedQuery> decomposed_queries = decompo_Query_Tree(querytree);
    unordered_map<int, float> junction_leftmap;
    unordered_map<int, float> junction_rightmap;

    //for each path query (obtained by decomposition), generate candidate prophet graph
    for (int i=0; i<decomposed_queries.size(); i++){//process by the post-order
        GeneralizedQuery current_query = decomposed_queries[i];
        //If in the path being processed now, either source or target is a junction, return the previous iteration's node2layers to initialize the values
        if (find(querytree.junctions.begin(), querytree.junctions.end(), current_query.srcs.begin()->first)!= querytree.junctions.end()){ //if src is a junction, get candidate layers from previous iteration
            current_query.srcs = node2layers[current_query.srcs.begin()->first];
        }
        else{
            if(find(querytree.junctions.begin(), querytree.junctions.end(), current_query.tgts.begin()->first)!= querytree.junctions.end()){ //tgt is a junction
                current_query.tgts = node2layers[current_query.tgts.begin()->first];
            }
        }
        vector<unordered_map<int, float> > layers;
        unordered_map<int, float> current_junction_leftmap;
        unordered_map<int, float> current_junction_rightmap;

        //Calling path query function from Albert (which has been generalized here to handle a set of potentially matching vertices as source or target)
        //It also computes the heuristic values in the process (value of a node2layer node key)
        double temptime = 0.0;
        layers = create_Prophet_Heuristics_generalized(g, current_query, temptime, current_junction_leftmap, current_junction_rightmap);//layers stores the legitimate nodes on each level. Each layer is a set.
        for (int i=0; i< layers.size(); i++){
            node2layers[current_query.nodes[i]] = layers[i];  //i-th node updated into the candidate set chart. using map--unordered map for now...
            for (auto it = layers[i].begin(); it!=layers[i].end(); it++){
                int vertex = it->first;
                vertex2node[vertex] = current_query.nodes[i];
            }
        }
        junction_leftmap.insert(current_junction_leftmap.begin(), current_junction_leftmap.end());
        junction_rightmap.insert(current_junction_rightmap.begin(), current_junction_rightmap.end());
    }
    return 0;
}
//done

//Baseline 1: based on prophet graph, list all candidates and
QueryResultTrees Bruteforce(const graph_t& g, Query_tree querytree, double& timeUsed){


    int mem = 0, totalTrees = 1, numTrees = 0;
    vector<PQEntity_AStar_Tree> complete_trees;
	vector<PQEntity_AStar_Tree> incompletetrees;
	vector<PQEntity_AStar_Tree> modified_trees;
	int iterationnum = querytree.patterns.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResultTrees result;

        for(int i=0; i<querytree.terminals_index.size(); i++){
            if((g.typeMap[querytree.nodes_ordered[querytree.terminals_index[i]]]!=querytree.patterns[querytree.terminals_index[i]])){
                cout<< "src or tgt node does not follow pattern!" << endl;
                return result;
            }
        }

    unordered_map<int,  unordered_map<int, unordered_map<int, float>>>  candidxleft;
    unordered_map<int,  unordered_map<int, unordered_map<int, float>>>  candidxright;
    //map a node in pattern into vertices in input graph that maps to its left children, if have any.

    unordered_map<int, unordered_map<int, tuple<float,float>>> node2vertices_hrtc = bottom_up_hrtc_compute(g, querytree, candidxleft, candidxright);
    //Maps a node in pattern to vertices in input graph that potentially map to it, each comes with a tuple of (left heuristic, right heuristic).
    cout<<candidxleft.size()<<endl;
    cout<<candidxright.size()<<endl;


	PQEntity_AStar_Tree curNode;
	//Root is the last element of the post_order traversal of pattern tree
	int root = querytree.nodes_ordered.back();

	//for all vertices matching the root node, push them in priority queue
	for(unordered_map<int, tuple<float, float>>::iterator vertex_hrtc=node2vertices_hrtc[root].begin(); vertex_hrtc!=node2vertices_hrtc[root].end(); vertex_hrtc++){
        Instance_Tree tmptree;
        tmptree.nodes.insert(vertex_hrtc->first);
        tmptree.wgt = 0;
        unordered_map<int, int> empty_node2vertex;
        empty_node2vertex[root] = vertex_hrtc->first;
        incompletetrees.push_back(createPQEntity_AStar_Tree(vertex_hrtc->first, root, 0, (std::get<0>(vertex_hrtc->second) + std::get<1>(vertex_hrtc->second)), tmptree, empty_node2vertex));//frontier is the priority queue. replace src with root.
        //get<1> gives the left heuristic value, get<2> gives the right heuristic value.
        numTrees ++;
	}

    //follow a pre-order to decide which is the next curId_inpattern the neighbor should match to, on which side, using this stack s.
    stack <int> s;
    int curId_inpattern = root;
    vector <int> pre_order_patterns; //they are id of nodes, not type. NOTE
    //Traversing query pattern in pre-order
    while(true){
            if (pre_order_patterns.size()>= querytree.nodes_ordered.size()) break;
            while(true){
                pre_order_patterns.push_back(curId_inpattern); //first time at one node.
                cout<<"expended on current pattern is "<< curId_inpattern<< endl;
                s.push(curId_inpattern);
                if (querytree.map2leftcdr.find(curId_inpattern)!= querytree.map2leftcdr.end()){//current ppattern node still have left child;
                        curId_inpattern = querytree.map2leftcdr[curId_inpattern];
                }
                else//have no left child;
                        break;
            }
            if (s.empty())
                break;
            else{
                while(true){
                    if (s.empty()) break; //reached the end of tree
                    curId_inpattern = s.top();
                    s.pop();
                    if (querytree.map2rightcdr.find(curId_inpattern)!= querytree.map2rightcdr.end()){
                        curId_inpattern = querytree.map2rightcdr[curId_inpattern];
                        break;
                    }
                }
            }
        }


    //Top-down traversal in pre-order


	while(!incompletetrees.empty()){

		if (incompletetrees.size() > mem) mem ++;

        curNode = incompletetrees.back();
        incompletetrees.pop_back();
        int curId = curNode.nodeIdx;
        int curId_inpattern = curNode.curId_inpattern;
        Instance_Tree subtree = curNode.subtree;

        modified_trees = Expand_brute_v2(g, querytree, pre_order_patterns, curId, curNode, subtree,totalTrees,node2vertices_hrtc, curId_inpattern, numTrees, candidxleft, candidxright);

        totalTrees += modified_trees.size();

        for (int i=0; i<modified_trees.size(); i++){
            PQEntity_AStar_Tree modified_tree = modified_trees[i];
            if(modified_tree.subtree.nodes.size() == querytree.nodes_ordered.size()){ //already complete after the growth
                complete_trees.push_back(modified_tree);
            }
            else{
                incompletetrees.push_back(modified_tree);
            }
        }
	}


	vector<PQEntity_AStar_Tree>TOPk_trees = Top_k_weight(complete_trees);
    vector<Instance_Tree> trees;
    for (int i = 0; i< TOPk_trees.size() && i < TOP_K; i++){
        trees.push_back(TOPk_trees[i].subtree);
    }

    result.trees = trees;

	result.mem = mem;
	result.totalTrees = totalTrees;
	return result;
}

/////////////////END OF BASELINE 1/////////////////






//////////////////BASELINE 2////////////////////
//Baseline 2: decompose the tree into a longest path and feed paths to albert's algo
//from the longest path node to terminals, do sinple path matching.

////////baseline2 utils
//return one result that is not in found_paths
QueryResult AStar_Prophet_pop(const graph_t& g, Query query, double& timeUsed, vector<Path> found_paths){
    Path cur_path;
	int maxDepth = query.pattern.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResult qResult;
        if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
                cout << query.src << " to " << query.tgt << endl;
                cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResult;
        }
	int midLayer = query.pattern.size()/2 +1;//half forward and halp backward. And they meet and terminate at the same layer.
    vector<unordered_map<int, float> > layers = create_Prophet_Heuristics(g, query, timeUsed);//layers stores the legitimate nodes on each level. Each layer is a set.

	std::priority_queue<PQEntity_AStar, std::vector<PQEntity_AStar>, comparator_AStar> frontier;
	PQEntity_AStar curNode;
	vector<int> tmpPath;
	tmpPath.push_back(query.src);
	frontier.push(createPQEntity_AStar(query.src, 0, 0, tmpPath));//frontier is the priority queue.
	int mem = 0, total = 1;
	while(!frontier.empty()){
		mem = max(mem, (int)frontier.size());
		PQEntity_AStar curNode;
		curNode = frontier.top();
		frontier.pop();
		int curId = curNode.nodeIdx;
		int depth = curNode.path.size()-1;//start from 0.
		vector<int> path = curNode.path;
		if(depth==maxDepth){//reach the end of pattern.
			if(curId == query.tgt && curNode.wgt<MAX_WEIGHT){
                    //just found one
                cur_path = createPath(curNode.wgt, path);  //RIGHT TILL HERE

                //if cur_path not in found_paths. hand-written find function cause no == operator in struct path.
                bool found = false;
                for (Path path :found_paths){
                    if (path.nodeIds == cur_path.nodeIds)
                            found = true;
                }
                if (!found){
                    qResult.paths.push_back(cur_path);
                    break;
                }
			}
			continue;
		}
		//expanding the neighbors of current node.
		for(int i=0; i<g.degree[curId]; i++){
			int neigh = g.neighbors[g.nodes[curId]+i];//neighbor id.
			//if neighbor is not in the path and follow the pattern.
			//cout << depth << endl; cout << layers.size() << endl;
			unordered_map<int, float>::iterator found = layers[depth+1].find(neigh);
			float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], query.time);
			if( found != layers[depth+1].end() && find(path.begin(), path.end(), neigh) == path.end() && edgwgt+curNode.wgt<MAX_WEIGHT){
				vector<int> newPath = path;
				newPath.push_back(neigh);
				frontier.push(createPQEntity_AStar(neigh, edgwgt+curNode.wgt, edgwgt+curNode.wgt+found->second, newPath));
				total += 1;
			}
		}

	}
	qResult.mem = mem;
        qResult.totalPaths = total;
        return qResult;
}

//reuse from bruteforce: give me all complete trees.
vector<Instance_Tree> All_matching_trees(const graph_t& g, Query_tree querytree, double& timeUsed){
    unordered_map<int, unordered_map<int, float>> node2layers;
    unordered_map<int, int> vertex2node;
	vector<Instance_Tree> complete_trees;
	vector<Instance_Tree> incompletetrees;
	vector<Instance_Tree> modified_trees;
	Instance_Tree modified_tree;
    typecheck_all(g, querytree,vertex2node, node2layers); //now these vertext to node and back mappings all give the right type

	QueryResultTrees result;
	int numtrees = 0;
	int mem = 0;
	int totalTrees = 0;
	Instance_Tree incomplete_tree; //the current incomplete tree that we want to expand
    incomplete_tree.nodes.insert(querytree.terminals[0]);
    incomplete_tree.wgt = 0; //initialize a single node tree with weight 0
	incompletetrees.push_back(incomplete_tree); //initialize with first terminal
	while(incompletetrees.size() != 0){
        //for those that can never complete?

		if (incompletetrees.size() > mem) mem ++;
		//mem is the maximum lenghth that incompletetrees has reached.
        //while there still is incomplete trees, pop out one instance and grow that one
		incomplete_tree = incompletetrees.back();
		incompletetrees.pop_back();
		modified_trees = expend_withcheck(g, vertex2node, node2layers, querytree, incomplete_tree, numtrees);
		totalTrees += modified_trees.size();

		//for modified_tree in modified_trees
		for (int i=0; i<modified_trees.size(); i++){
            modified_tree = modified_trees[i];
			if(modified_tree.nodes.size() == querytree.nodes_ordered.size()){ //already complete after the growth
				complete_trees.push_back(modified_tree);
				//in bruteforce, number of memory always equals to number of trees.
			}
            else{
                incompletetrees.push_back(modified_tree);
            }
		}
	}
	return complete_trees;
}

Query GetBackbone(const graph_t& g, Query_tree querytree, int &rootpos){
    int src, tgt;
    vector<int> pattern_root2src;
    vector<int> pattern_root2tgt;
    Query backbone;
    int curnode = querytree.nodes_ordered.back();//start with root as a curnode.
    //traverse from root to the first terminal:
    while(true){
        pattern_root2src.push_back(g.typeMap[curnode]);
        if (querytree.map2leftcdr.find(curnode)!= querytree.map2leftcdr.end()){//curnode has left child
            curnode = querytree.map2leftcdr[curnode];
        }
        else {
            if(querytree.map2rightcdr.find(curnode)!= querytree.map2rightcdr.end()){ //has no left child, look at if there is a right child.
                curnode = querytree.map2rightcdr[curnode];
            }
            else{ //first reaches a terminal!
                    src = curnode;
                    break;
            }
        }
    }
    //now we have got a pattern from root to src
    //traverse from root to the last terminal:
    curnode = querytree.nodes_ordered.back();
    while(true){
        pattern_root2tgt.push_back(g.typeMap[curnode]);
        if (querytree.map2rightcdr.find(curnode)!= querytree.map2rightcdr.end()){//curnode has left child
            curnode = querytree.map2rightcdr[curnode];
        }
        else {
            if(querytree.map2leftcdr.find(curnode)!= querytree.map2leftcdr.end()){ //has no left child, look at if there is a right child.
                curnode = querytree.map2leftcdr[curnode];
            }
            else{ //first reaches a terminal!
                    tgt = curnode;
                    break;
            }
        }
    }
    //combine the two pattern by reversing root2src, and combine with root2tgt
    reverse(pattern_root2src.begin(), pattern_root2src.end());
	rootpos = pattern_root2src.size()-1;
    for(int i=1; i<pattern_root2tgt.size(); i++){
        pattern_root2src.push_back(pattern_root2tgt[i]);
    }
    backbone.src = src;
    backbone.tgt = tgt;
    backbone.time = querytree.time;
    backbone.pattern = pattern_root2src;
    return backbone;
}


//a modified version of ALL_matching_trees, take a query_tree_fixed instead
//return all matching instances, used in backbone given baselines.
vector<Instance_Tree> All_matching_trees_fixed(const graph_t& g, Query_tree_fixed query_tree_fixed, double& timeUsed, int &numtrees, int &mem, int &totalTrees){


    unordered_map<int, unordered_map<int, float>> node2layers;
	Query_tree querytree;
	querytree.map2leftcdr = query_tree_fixed.map2leftcdr;
	querytree.map2rightcdr = query_tree_fixed.map2rightcdr;
	querytree.map2parent = query_tree_fixed.map2parent;
	querytree.map2patthern = query_tree_fixed.map2patthern;
	querytree.nodes_ordered = query_tree_fixed.nodes_ordered;
	querytree.terminals_index = query_tree_fixed.terminals_index;
	querytree.junction_index = query_tree_fixed.junction_index;
	querytree.junctions = query_tree_fixed.junctions;
	querytree.terminals = query_tree_fixed.terminals;
	querytree.patterns = query_tree_fixed.patterns;
	querytree.time = query_tree_fixed.time;


    unordered_map<int, int> vertex2node;
	vector<Instance_Tree> complete_trees;
	vector<Instance_Tree> incompletetrees;
	vector<Instance_Tree> modified_trees;

	if (querytree.nodes_ordered.size()==0){
        return complete_trees;
	}
	if (querytree.nodes_ordered.size() == query_tree_fixed.fixed_verteces.size()){
	    //corner case when left or right is already fixed
	    Instance_Tree one_tree;
        one_tree.map2leftcdr = query_tree_fixed.map2leftcdr;
        one_tree.map2parent = query_tree_fixed.map2parent;
        one_tree.map2rightcdr = query_tree_fixed.map2rightcdr;
        one_tree.nodes = unordered_set<int> (query_tree_fixed.nodes_ordered.begin(), query_tree_fixed.nodes_ordered.end());
        one_tree.wgt = 0; //backbone added in later.
        complete_trees.push_back(one_tree);
        return complete_trees;
	}

	Instance_Tree modified_tree;
    typecheck_all(g, querytree,vertex2node, node2layers); //now these vertext to node and back mappings all give the right type

	QueryResultTrees result;



	Instance_Tree incomplete_tree; //the current incomplete tree that we want to expand
    //incomplete_tree.nodes.insert(querytree.terminals[0]);
	incomplete_tree.nodes.insert(query_tree_fixed.fixed_verteces.begin(), query_tree_fixed.fixed_verteces.end());

    incomplete_tree.wgt = 0; //initialize a single node tree with weight 0.
	//NOTE: weight of the two query_tree_fixed are combined weight of non-backbone parts!
	incompletetrees.push_back(incomplete_tree); //initialize with first terminal
	while(incompletetrees.size() != 0){

		//mem is the maximum lenghth that incompletetrees has reached.
        //while there still is incomplete trees, pop out one instance and grow that one
		incomplete_tree = incompletetrees.back();
		incompletetrees.pop_back();
		modified_trees = expend_withcheck(g, vertex2node, node2layers, querytree, incomplete_tree, numtrees);
		totalTrees += modified_trees.size();
        if (modified_trees.size() > mem) mem = modified_trees.size();
		//for modified_tree in modified_trees
		for (int i=0; i<modified_trees.size(); i++){
            numtrees ++;
            modified_tree = modified_trees[i];
			if(modified_tree.nodes.size() == querytree.nodes_ordered.size()){ //already complete after the growth
				complete_trees.push_back(modified_tree);
				totalTrees ++;
				//in bruteforce, number of memory always equals to number of trees.
			}
            else{
                incompletetrees.push_back(modified_tree);
            }
		}
	}
	return complete_trees;
}


//given a backbone instance, return the matching left_subtree as a usable query. all given nodes viewed as terminals
Query_tree_fixed left_from_backbone(int rootpos, Query_tree querytree, Query BackboneQuery, Path backboneinstance){
	Query_tree_fixed* result_pos = new Query_tree_fixed;
	Query_tree_fixed result = *result_pos;
	int root = querytree.nodes_ordered.back();
	if (querytree.map2leftcdr.find(root)!= querytree.map2leftcdr.end()){ //root has a left child
		int leftchild = querytree.map2leftcdr[root];
		int index = find(querytree.nodes_ordered.begin(), querytree.nodes_ordered.end(),leftchild) - querytree.nodes_ordered.begin(); //index = nodes.findposition(leftchild)
		for (int i= 0; i<=index; i++){
			result.nodes_ordered.push_back(querytree.nodes_ordered[i]); //nodes before leftchild (including)
			result.patterns.push_back(querytree.patterns[i]); //patterns = querytree.patterns.cut at the leftchild
		}
		for (int i=0; i< querytree.junction_index.size(); i++){
			if(querytree.junction_index[i]<=index){
				result.junction_index.push_back(querytree.junction_index[i]);
			}
		}
		for (int i=0; i< querytree.terminals_index.size(); i++){
			if(querytree.terminals_index[i]<=index){
				result.terminals_index.push_back(querytree.terminals_index[i]);
			}
		}
		for(int i = 0; i<rootpos; i++) //add the backbone verteces src side before root
				result.fixed_verteces.push_back(backboneinstance.nodeIds[i]);
        if (result.nodes_ordered.size()>0){
            for (int i=0; i<result.nodes_ordered.size(); i++){
                result.map2patthern.insert(make_pair(result.nodes_ordered[i], result.patterns[i]));
            }
            for (int i=0; i<result.junction_index.size();i++){
                result.junctions.push_back(result.nodes_ordered[result.junction_index[i]]);
            }
            for (int i=0; i<result.terminals_index.size();i++){
                result.terminals.push_back(result.nodes_ordered[result.terminals_index[i]]);
            }


            //from left child traverse down add to parental list
            std::queue<int> left_traverse_Q;
            int curnode = leftchild;
            left_traverse_Q.push(curnode);
             //start from the left child
            while(!left_traverse_Q.empty()){

                curnode = left_traverse_Q.front();
                left_traverse_Q.pop();

                if (find(querytree.terminals.begin(), querytree.terminals.end(), curnode)==querytree.terminals.end()){ // curnode is not a terminal:
                    if (querytree.map2leftcdr.find(curnode)!=querytree.map2leftcdr.end()){//curnode has left child
                        result.map2leftcdr[curnode]=querytree.map2leftcdr[curnode];
                        result.map2parent[querytree.map2leftcdr[curnode]] = curnode;
                        left_traverse_Q.push(querytree.map2leftcdr[curnode]);
                    }
                    if (querytree.map2rightcdr.find(curnode)!=querytree.map2rightcdr.end()){//curnode has right child
                        result.map2rightcdr[curnode]= querytree.map2rightcdr[curnode];
                        result.map2parent[querytree.map2rightcdr[curnode]] = curnode;
                        left_traverse_Q.push(querytree.map2rightcdr[curnode]);
                    }
                } //do nothing if it is already a terminal
            }
		}
		//finished the traverse

	}
	else{ //has no left child, return empty query tree
		return result;
	}
}


Query_tree_fixed right_from_backbone(int rootpos, Query_tree querytree, Query BackboneQuery, Path backboneinstance){
	Query_tree_fixed result;
	int root = querytree.nodes_ordered.back();
	if (querytree.map2rightcdr.find(root)!= querytree.map2rightcdr.end()){ //root has a right child
        int leftchild = querytree.map2leftcdr[root];
		int rightchild = querytree.map2rightcdr[root];
		int index = find(querytree.nodes_ordered.begin(), querytree.nodes_ordered.end(),leftchild)-querytree.nodes_ordered.begin(); //index = nodes.findposition(leftchild!!NOT RIGHT)
		for (int i= index+1; i<=querytree.nodes_ordered.size()-2; i++){ //-1 is the root position, not including
			result.nodes_ordered.push_back(querytree.nodes_ordered[i]); //nodes after partition (including)
			result.patterns.push_back(querytree.patterns[i]); //patterns = query_tree.patterns.cut at the leftchild
		}
		for (int i=0; i< querytree.junction_index.size()-1; i++){ //-1 to exclude root
			if(querytree.junction_index[i]>index){
				result.junction_index.push_back(querytree.junction_index[i]-index-1);
			}
		}
		for (int i=0; i< querytree.terminals_index.size(); i++){
			if(querytree.terminals_index[i]>index){
				result.terminals_index.push_back(querytree.terminals_index[i]-index-1);
			}
		}
		for(int i = rootpos + 1; i<backboneinstance.nodeIds.size(); i++) //add the backbone verteces after root to tgt side
				result.fixed_verteces.push_back(backboneinstance.nodeIds[i]);
        if (result.nodes_ordered.size()>0){
            for (int i=0; i<result.nodes_ordered.size(); i++){
                result.map2patthern.insert(make_pair(result.nodes_ordered[i], result.patterns[i]));
            }
            for (int i=0; i<result.junction_index.size();i++){
                result.junctions.push_back(result.nodes_ordered[result.junction_index[i]]);
            }
            for (int i=0; i<result.terminals_index.size();i++){
                result.terminals.push_back(result.nodes_ordered[result.terminals_index[i]]);
            }

        //	from right child traverse down add to parental list
            std::queue<int> right_traverse_Q;
            int curnode = rightchild;
            right_traverse_Q.push(curnode); //start from the root's right child
            while(!right_traverse_Q.empty()){
                curnode = right_traverse_Q.front();
                right_traverse_Q.pop();

                if (find(querytree.terminals.begin(), querytree.terminals.end(), curnode)==querytree.terminals.end()){ // curnode is not a terminal:
                    if (querytree.map2leftcdr.find(curnode)!=querytree.map2leftcdr.end()){//curnode has left child
                        result.map2leftcdr[curnode]=querytree.map2leftcdr[curnode];
                        result.map2parent[querytree.map2leftcdr[curnode]] = curnode;
                        right_traverse_Q.push(querytree.map2leftcdr[curnode]);
                    }
                    if (querytree.map2rightcdr.find(curnode)!=querytree.map2rightcdr.end()){//curnode has right child
                        result.map2rightcdr[curnode]= querytree.map2rightcdr[curnode];
                        result.map2parent[querytree.map2rightcdr[curnode]] = curnode;
                        right_traverse_Q.push(querytree.map2rightcdr[curnode]);
                    }
                } //do nothing if it is already a terminal

            }
            //finished the traverse
        }

	}
	else{ //has no right child, return empty query tree
		return result;
	}
}

Instance_Tree Combine_tree(int rootpos, Path backboneinstance, Query_tree querytree, Instance_Tree left_instance, Instance_Tree right_instance){
//NOTE: weight of the two query_tree_fixed are combined weight of non-backbone parts!
	Instance_Tree result;
	int root = querytree.nodes_ordered.back();
	int index = rootpos;
	int root_vertex = backboneinstance.nodeIds[index];


	//if left instance is empty
	if (left_instance.nodes.size()==0){
        if (querytree.map2rightcdr.find(root)!= querytree.map2rightcdr.end()){//have a right instance, stick the left instance to result
            result = right_instance;
            result.map2rightcdr[root_vertex]=querytree.map2rightcdr[root];
            result.map2parent[querytree.map2rightcdr[root]]=root_vertex;
            result.nodes.insert(root_vertex);

        }
        result.wgt =  right_instance.wgt + backboneinstance.wgt;
        return result;

	}

	//if right instance is empty
	if (right_instance.nodes.size()==0){
        if (querytree.map2leftcdr.find(root)!= querytree.map2leftcdr.end()){//have a left instance, stick the left instance to result
            result = left_instance;
            result.map2leftcdr[root_vertex]=querytree.map2leftcdr[root];
            result.map2parent[querytree.map2leftcdr[root]]=root_vertex;
            result.nodes.insert(root_vertex);
        }
        result.wgt = left_instance.wgt + backboneinstance.wgt;
        return result;
	}


	//else case: bath left and right are not empty
	if (querytree.map2leftcdr.find(root)!= querytree.map2leftcdr.end()){//have a left instance, stick the left instance to result
		result = left_instance;
		result.map2leftcdr[root_vertex]=querytree.map2leftcdr[root];
		result.map2parent[querytree.map2leftcdr[root]]=root_vertex;
		result.nodes.insert(root_vertex);
	}
	if (querytree.map2rightcdr.find(root)!= querytree.map2rightcdr.end()){//have a right instance, stick the left instance to result
		result.map2leftcdr.insert(right_instance.map2leftcdr.begin(), right_instance.map2leftcdr.end());
		result.map2rightcdr.insert(right_instance.map2rightcdr.begin(), right_instance.map2rightcdr.end());
		result.map2parent.insert(right_instance.map2parent.begin(), right_instance.map2parent.end());
		result.nodes.insert(right_instance.nodes.begin(), right_instance.nodes.end());
	}
	result.wgt = left_instance.wgt + right_instance.wgt + backboneinstance.wgt;
	return result;
}


QueryResultTrees Backbone_query(const graph_t& g, Query_tree querytree, double& timeUsed){
    QueryResultTrees result;
    Query longest_path;
    Path longest_path_instance;
    vector<Path> found_paths;

	int rootpos = 0;
    longest_path = GetBackbone(g, querytree, rootpos);
	cout<<"the position of root on backbone is: "<<rootpos;


    int numtrees, mem, totalTrees = 0; //TODO: increase them accordingly!

    float min_kseen_weight = MAX_WEIGHT;
    float newest_path_wgt = 0;

    vector<PQEntity_AStar_Tree> candidate_trees;
    vector<PQEntity_AStar_Tree> TOPk_trees;
    Query_tree_fixed left_querytree;
    Query_tree_fixed right_querytree;
    vector<Instance_Tree> left_instances;
    vector<Instance_Tree> right_instances;
    Instance_Tree right_instance;
    Instance_Tree left_instance;
    Instance_Tree combined_tree;

    while(true){
        QueryResult first_path = AStar_Prophet_pop(g, longest_path, timeUsed, found_paths);
        if (first_path.paths.size()==0){
            break; //break condition 1/2: when no more matching path can be found.
        }
        longest_path_instance = first_path.paths.front(); //first element in a length 1 vector
        found_paths.push_back(longest_path_instance);
        newest_path_wgt = longest_path_instance.wgt;
        TOPk_trees = Top_k_weight(candidate_trees);
        if (TOPk_trees.size()>0)
            min_kseen_weight = TOPk_trees.back().wgt;
        if (newest_path_wgt > min_kseen_weight){
            break; //break condition 2/2: when this current backbone instance weight is larger then the largest of k-lightest ones, then it becomes useless to invest.
        }

        newest_path_wgt = longest_path_instance.wgt;
        left_querytree = left_from_backbone(rootpos, querytree, longest_path, longest_path_instance);  //modify the query into a querytree and a set of extra_terminals
        right_querytree = right_from_backbone(rootpos, querytree, longest_path, longest_path_instance);

        if (left_querytree.nodes_ordered.size()==0 or right_querytree.nodes_ordered.size()==0){
            if (left_querytree.nodes_ordered.size()==0){
                left_instance.nodes.clear();
                cout<< "right instances matching..."<<endl;
                right_instances =  All_matching_trees_fixed(g, right_querytree, timeUsed, numtrees, mem, totalTrees);
                cout<< "right instances count is: "<< right_instances.size()<<endl;
                for (int j=0;j<right_instances.size(); j++){
                    right_instance = right_instances[j];
                    combined_tree = Combine_tree(rootpos,longest_path_instance, querytree, left_instance, right_instance);
                    unordered_map<int, int> emptymap;
                    PQEntity_AStar_Tree combined_tree_entity = createPQEntity_AStar_Tree(0,0,combined_tree.wgt, combined_tree.wgt, combined_tree, emptymap);
                    candidate_trees.push_back(combined_tree_entity);
                }

            }

            if (right_querytree.nodes_ordered.size()==0){
                right_instance.nodes.clear();
                cout<< "left instances matching..."<<endl;
                left_instances = All_matching_trees_fixed(g, left_querytree, timeUsed, numtrees, mem, totalTrees);
                cout<< "left instances count is: "<< left_instances.size()<<endl;
                for (int i=0; i<left_instances.size(); i++){
                    left_instance = left_instances[i];
                    combined_tree = Combine_tree(rootpos,longest_path_instance, querytree, left_instance, right_instance);
                    unordered_map<int, int> emptymap;
                    PQEntity_AStar_Tree combined_tree_entity = createPQEntity_AStar_Tree(0,0,combined_tree.wgt, combined_tree.wgt, combined_tree, emptymap);
                    candidate_trees.push_back(combined_tree_entity);
                }
            }





        }
        //neither left nor right tree is empty.
        else{
            left_instances = All_matching_trees_fixed(g, left_querytree, timeUsed, numtrees, mem, totalTrees);
            right_instances =  All_matching_trees_fixed(g, right_querytree, timeUsed, numtrees, mem, totalTrees);



            for (int i=0; i<left_instances.size(); i++){
                left_instance = left_instances[i];
                for (int j=0;j<right_instances.size(); j++){
                    right_instance = right_instances[j];
                    combined_tree = Combine_tree(rootpos,longest_path_instance, querytree, left_instance, right_instance);
                    unordered_map<int, int> emptymap;
                    PQEntity_AStar_Tree combined_tree_entity = createPQEntity_AStar_Tree(0,0,combined_tree.wgt, combined_tree.wgt, combined_tree, emptymap);
                    candidate_trees.push_back(combined_tree_entity);
                }
            }
        }
    }

    //NOTICE: this simple TOP pop one by one may be slow, since we have to swipe k times, and check each step finishing a tree.
    //--if it turns out to be the case, consider recursive.


    int mem_last = 0;
    Instance_Tree temptree;
    for (int i= 0; i< candidate_trees.size(); i++){
        temptree = candidate_trees[i].subtree;
        mem += temptree.nodes.size();
    }

    result.mem = max(mem_last, mem);


    //result.mem = candidate_trees.size();

    TOPk_trees = Top_k_weight(candidate_trees);
    vector<Instance_Tree> trees;
    for (int i = 0; i< TOPk_trees.size() && i< TOP_K; i++){
        trees.push_back(TOPk_trees[i].subtree);
    }

    result.trees = trees;
    result.numTrees = numtrees;
   // result.mem = mem;
    result.totalTrees = totalTrees;
    return result;
}



///////////////////END OF BASELINE 2//////////////////////









///////////////////MAIN ALGORITHM//////////////////////////

//Computes a prophet graph and runs A* to return k-lightest matching instances.
//functions with v2 are new versions for WWW camera ready, and will replace their original after passing tests.
QueryResultTrees AStar_Prophet_Tree_v2(const graph_t& g, Query_tree querytree, double& timeUsed){
    int mem = 0, total = 1, numTrees = 0;
	int iterationnum = querytree.patterns.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResultTrees qResultTree;

        for(int i=0; i<querytree.terminals_index.size(); i++){
            if((g.typeMap[querytree.nodes_ordered[querytree.terminals_index[i]]]!=querytree.patterns[querytree.terminals_index[i]])){
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResultTree;
            }
        }

    //compute node2vertices_hrtc: a heuristic of left side and right side bottom down.
    //if a vertex has no left child, the left heuristic value will be 0.


    unordered_map<int,  unordered_map<int, unordered_map<int, float>>>  candidxleft;
    unordered_map<int,  unordered_map<int, unordered_map<int, float>>>  candidxright;
    //map a node in pattern into vertices in input graph that maps to its left children, if have any.

    unordered_map<int, unordered_map<int, tuple<float,float>>> node2vertices_hrtc = bottom_up_hrtc_compute(g, querytree, candidxleft, candidxright);
    //Maps a node in pattern to vertices in input graph that potentially map to it, each comes with a tuple of (left heuristic, right heuristic).
    cout<<candidxleft.size()<<endl;
    cout<<candidxright.size()<<endl;



    //A* top down: a priority queue in a vector container containing elements of type PQEntity_AStar_Tree
	std::priority_queue<PQEntity_AStar_Tree, std::vector<PQEntity_AStar_Tree>, comparator_AStar_Tree> frontier;
	PQEntity_AStar_Tree curNode;
	//Root is the last element of the post_order traversal of pattern tree
	int root = querytree.nodes_ordered.back();

	//for all vertices matching the root node, push them in priority queue
	for(unordered_map<int, tuple<float, float>>::iterator vertex_hrtc=node2vertices_hrtc[root].begin(); vertex_hrtc!=node2vertices_hrtc[root].end(); vertex_hrtc++){
        Instance_Tree tmptree;
        tmptree.nodes.insert(vertex_hrtc->first);
        tmptree.wgt = 0;
        unordered_map<int, int> empty_node2vertex;
        empty_node2vertex[root] = vertex_hrtc->first;
        frontier.push(createPQEntity_AStar_Tree(vertex_hrtc->first, root, 0, (std::get<0>(vertex_hrtc->second) + std::get<1>(vertex_hrtc->second)), tmptree, empty_node2vertex));//frontier is the priority queue. replace src with root.
        //get<1> gives the left heuristic value, get<2> gives the right heuristic value.
        numTrees ++;
	}

    //follow a pre-order to decide which is the next curId_inpattern the neighbor should match to, on which side, using this stack s.
    stack <int> s;
    int curId_inpattern = root;
    vector <int> pre_order_patterns; //they are id of nodes, not type. NOTE
    //Traversing query pattern in pre-order
    while(true){
            if (pre_order_patterns.size()>= querytree.nodes_ordered.size()) break;
            while(true){
                pre_order_patterns.push_back(curId_inpattern); //first time at one node.
                cout<<"expended on current pattern is "<< curId_inpattern<< endl;
                s.push(curId_inpattern);
                if (querytree.map2leftcdr.find(curId_inpattern)!= querytree.map2leftcdr.end()){//current ppattern node still have left child;
                        curId_inpattern = querytree.map2leftcdr[curId_inpattern];
                }
                else//have no left child;
                        break;
            }
            if (s.empty())
                break;
            else{
                while(true){
                    if (s.empty()) break; //reached the end of tree
                    curId_inpattern = s.top();
                    s.pop();
                    if (querytree.map2rightcdr.find(curId_inpattern)!= querytree.map2rightcdr.end()){
                        curId_inpattern = querytree.map2rightcdr[curId_inpattern];
                        break;
                    }
                }
            }
        }


    //Top-down traversal in pre-order
    bool front_element = false;


	while(!frontier.empty() || front_element){//each time check one element in the PQ. or, alternatively, there is a front_element out waiting to grow.
		mem = max(mem, (int)frontier.size());

		if (!front_element){
            curNode = frontier.top();
            cout<<curNode.key;
            frontier.pop();
		}
		//elsewise, front element is curNode, no need to further retrieve.

		int curId = curNode.nodeIdx;
		int curId_inpattern = curNode.curId_inpattern;



		int done_countes = curNode.subtree.nodes.size()-1;//start from 0.  done_counts: refer to the old path depth
		Instance_Tree subtree = curNode.subtree;

		if(done_countes==iterationnum){//reach the end of pattern.
            front_element = false;
			if(curNode.wgt<MAX_WEIGHT){ //(%all terminal nodes are reached)--actually redundent.
				if(qResultTree.trees.size()<TOP_K){
                    subtree.wgt = curNode.wgt;
                    qResultTree.trees.push_back(subtree);
                }
            if(qResultTree.trees.size()==TOP_K)
                break;
			}
			continue;
		}
		//expanding the neighbors of current node
		//Expand the most promising path with vertices mapping the next node in the path (if it exists, but can trace back otherwise)
		//It also updates the value of the right path when entering the junction for the first time. This update involves finding the best heuristic estimate, given that the junction vertex is fixed


        front_element = Expand_current_v2(g, querytree, pre_order_patterns, curId, curNode, subtree,total,node2vertices_hrtc, curId_inpattern,frontier, numTrees, candidxleft, candidxright);
        //each expanding operation may change: total, frontier, curId, curNode.
		}

	qResultTree.mem = mem;
	qResultTree.numTrees = numTrees;
    qResultTree.totalTrees = total;
    //Return k-lightest matching trees
    return qResultTree;
}


float getHeuristicValue(const graph_t&g, Query query, int depth, int curNode){
	unordered_map<int, float> oneLayer;
	oneLayer[curNode] = 0;
	for(int i=depth+1; i<=query.pattern.size()-1; i++){
		unordered_map<int, float> newLayer;
                for(unordered_map<int, float>::iterator it = oneLayer.begin(); it!=oneLayer.end(); it++){
                        int node = it->first;
                        float prewgt = it->second;
                        for(int j=0; j<g.degree[node]; j++){
                                int neigh = g.neighbors[g.nodes[node]+j];
                                float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
                                if(g.typeMap[neigh]==query.pattern[i] && prewgt+wgt<MAX_WEIGHT){
                                        if(newLayer.count(neigh)>0)
                                                newLayer[neigh] = min(newLayer.at(neigh), prewgt+wgt);
                                        else
                                                newLayer[neigh] = prewgt+wgt;
                                }
                        }
                }
                oneLayer = newLayer;
	}
	if(oneLayer.count(query.tgt)>0){
		return oneLayer.at(query.tgt);}
	else return -1;
}


QueryResult AStar_Original(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle){
	int maxDepth = query.pattern.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResult qResult;
        if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
                cout << query.src << " to " << query.tgt << endl;
                cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResult;
        }

	std::priority_queue<PQEntity_AStar, std::vector<PQEntity_AStar>, comparator_AStar> frontier;
	PQEntity_AStar curNode;
	vector<int> tmpPath;
	tmpPath.push_back(query.src);
	frontier.push(createPQEntity_AStar(query.src, 0, 0, tmpPath));//frontier is the priority queue.
	int mem = 0, total = 1;
	while(!frontier.empty()){
		mem = max(mem, (int)frontier.size());
		PQEntity_AStar curNode;
		curNode = frontier.top();
		frontier.pop();
		int curId = curNode.nodeIdx;
		int depth = curNode.path.size()-1;//start from 0.
		vector<int> path = curNode.path;
		if(depth==maxDepth){//reach the end of pattern.
			if(curId == query.tgt && curNode.wgt<MAX_WEIGHT){
				if(qResult.paths.size()<TOP_K)
                                        qResult.paths.push_back(createPath(curNode.wgt, path));
                                if(qResult.paths.size()==TOP_K)
                                        break;
			}
			continue;
		}
		//expanding the neighbors of current node.
		for(int i=0; i<g.degree[curId]; i++){
			int neigh = g.neighbors[g.nodes[curId]+i];//neighbor id.
			//if neighbor is not in the path and follow the pattern.
			if(g.typeMap[neigh]!=query.pattern[depth+1])
				continue;
			float heuValue = getHeuristicValue(g, query, depth+1, neigh);
			total += 1;
			if(heuValue < 0 )
				continue;
			float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], query.time);
			if( find(path.begin(), path.end(), neigh) == path.end() && edgwgt+curNode.wgt<MAX_WEIGHT){
				if(oracle == WITH_ORACLE){
                                  int tree_dist = pll.QueryDistance(query.tgt, neigh);//use exact distance oracle.
                                  int realDistInPattern = query.pattern.size()-depth-2;
                                  if(tree_dist > realDistInPattern)
                                        continue;
                                }
				vector<int> newPath = path;
				newPath.push_back(neigh);
				frontier.push(createPQEntity_AStar(neigh, edgwgt+curNode.wgt, edgwgt+curNode.wgt+heuValue, newPath));
			}
		}

	}
	qResult.mem = mem;
        qResult.totalPaths = total;
        return qResult;
}

QueryResult AStar_Original_OPT(const graph_t& g, Query query){
	int maxDepth = query.pattern.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResult qResult;
        if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
                cout << query.src << " to " << query.tgt << endl;
                cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResult;
        }
	vector<unordered_map<int, float>> heuristics; //first-int: node id. second-int: layer
	for(int i=0; i<=query.pattern.size()-1; i++){
		unordered_map<int, float> tmp;
		heuristics.push_back(tmp);
	}
	unordered_map<int, float> oneLayer;
	oneLayer[query.tgt] = 0;
	heuristics[query.pattern.size()-1] = (oneLayer);
	for(int i=query.pattern.size()-2; i>=1; i--){
		unordered_map<int, float> newLayer;
		for(unordered_map<int, float>::iterator it = oneLayer.begin(); it!=oneLayer.end(); it++){
			int node = it->first;
			float prewgt = it->second;
			for(int j=0; j<g.degree[node]; j++){
				int neigh = g.neighbors[g.nodes[node]+j];
				float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
				if(g.typeMap[neigh]==query.pattern[i] && prewgt+wgt<MAX_WEIGHT){
					if(newLayer.count(neigh)>0)
						newLayer[neigh] = min(newLayer.at(neigh), prewgt+wgt);
					else
						newLayer[neigh] = prewgt+wgt;
				}
			}
		}
		oneLayer = newLayer;
		heuristics[i] = newLayer;
	}

	std::priority_queue<PQEntity_AStar, std::vector<PQEntity_AStar>, comparator_AStar> frontier;
	PQEntity_AStar curNode;
	vector<int> tmpPath;
	tmpPath.push_back(query.src);
	frontier.push(createPQEntity_AStar(query.src, 0, 0, tmpPath));//frontier is the priority queue.
	int mem = 0, total = 1;
	while(!frontier.empty()){
		mem = max(mem, (int)frontier.size());
		PQEntity_AStar curNode;
		curNode = frontier.top();
		frontier.pop();
		int curId = curNode.nodeIdx;
		int depth = curNode.path.size()-1;//start from 0.
		vector<int> path = curNode.path;
		if(depth==maxDepth){//reach the end of pattern.
			if(curId == query.tgt && curNode.wgt<MAX_WEIGHT){
				if(qResult.paths.size()<TOP_K)
                                        qResult.paths.push_back(createPath(curNode.wgt, path));
                                if(qResult.paths.size()==TOP_K)
                                        break;
			}
			continue;
		}
		//expanding the neighbors of current node.
		for(int i=0; i<g.degree[curId]; i++){
			int neigh = g.neighbors[g.nodes[curId]+i];//neighbor id.
			//if neighbor is not in the path and follow the pattern.
			if(heuristics[depth+1].count(neigh) <=0)
				continue;
			float heuValue = heuristics[depth+1].at(neigh);
			float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], query.time);
			if( find(path.begin(), path.end(), neigh) == path.end() && edgwgt+curNode.wgt<MAX_WEIGHT){
				vector<int> newPath = path;
				newPath.push_back(neigh);
				frontier.push(createPQEntity_AStar(neigh, edgwgt+curNode.wgt, edgwgt+curNode.wgt+heuValue, newPath));
				total += 1;
			}
		}

	}
	qResult.mem = mem;
        qResult.totalPaths = total;
        return qResult;
}

QueryResult Bidirec_AStar_Prophet(const graph_t& g, Query query){
	int maxDepth = query.pattern.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResult qResult;
        if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
                cout << query.src << " to " << query.tgt << endl;
                cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResult;
        }
	int midLayer = query.pattern.size()/2 +1;//half forward and halp backward. And they meet and terminate at the same layer.
 	vector<unordered_map<int, pair<float, float> > > layers;//id, heuristic to target, heuristic to source.
	layers.reserve(query.pattern.size());
	unordered_map<int, pair<float,float> > top;
	top[query.src] = make_pair(MAX_WEIGHT, 0);
	layers.push_back(top);
	for(int i=1; i<=midLayer-1; i++){
		unordered_map<int, pair<float,float> > lastLayer = layers[i-1];//upper layer.
		unordered_map<int, pair<float,float> > newLayer;
		for(unordered_map<int, pair<float,float> >::iterator it=lastLayer.begin(); it!=lastLayer.end(); it++){//expanding each node on last layer.
			int node = it->first;
			float pathWgt = it->second.second;
			for(int j=0; j<g.degree[node]; j++){//visit the neighbors of each node.
				int neigh = g.neighbors[g.nodes[node]+j];
				float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
				if(g.typeMap[neigh] == query.pattern[i]){
					if(newLayer.count(neigh) == 0)
						newLayer[neigh] = make_pair(MAX_WEIGHT, pathWgt+wgt);
					else if(pathWgt+wgt < newLayer.at(neigh).second)
						newLayer.at(neigh).second = pathWgt + wgt;
				}
			}
		}
		layers.push_back(newLayer);
	}//meet at midLayer-1
	unordered_map<int, pair<float,float> > bottom;
	for(int i=midLayer; i<query.pattern.size()-1; i++)
		layers.push_back(bottom);//empty.
	bottom[query.tgt] = make_pair(0,MAX_WEIGHT);
	layers.push_back(bottom);
	for(int i=query.pattern.size()-2; i>=midLayer-1; i--){
		unordered_map<int, pair<float,float> > belowLayer = layers[i+1];
		unordered_map<int, pair<float,float> > newLayer;
		for(unordered_map<int, pair<float,float> >::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
			int node = it->first;
			float pathWgt = it->second.first;
			for(int j=0; j<g.degree[node]; j++){
				int neigh = g.neighbors[g.nodes[node]+j];
				float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
				if(g.typeMap[neigh]==query.pattern[i]){
					if(newLayer.count(neigh) == 0)
						newLayer[neigh] = make_pair(pathWgt + wgt, MAX_WEIGHT);
					else if(pathWgt+wgt < newLayer.at(neigh).first)
						newLayer.at(neigh).first = pathWgt+wgt;
				}
			}
		}
		if(i == midLayer-1){//i==midLayer-1
			unordered_map<int, pair<float,float> > tmpMap;
			for(unordered_map<int, pair<float,float> >::iterator it=newLayer.begin(); it!=newLayer.end(); it++){
				int node = it->first;
				if(layers[i].count(node) > 0){
					it->second.second = layers[i].at(node).second;
					tmpMap.insert(*it);
				}
			}
			layers[i] = tmpMap;
		}
		else
			layers[i] = newLayer;

	}
	for(int i=midLayer; i<=query.pattern.size()-1; i++){//going down. change second.
                unordered_map<int, pair<float,float> > lastLayer = layers[i-1];//upper layer.
                unordered_map<int, pair<float,float> > newLayer;
                for(unordered_map<int, pair<float,float> >::iterator it=lastLayer.begin(); it!=lastLayer.end(); it++){//expanding each node on last layer.
                        int node = it->first;
			float pathWgt = it->second.second;
                        for(int j=0; j<g.degree[node]; j++){//visit the neighbors of each node.
                                int neigh = g.neighbors[g.nodes[node]+j];
				float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
                                if(layers[i].count(neigh)>0){ //Note: it seems that the pattern matching is not needed here.
					if(newLayer.count(neigh) == 0)
	                                        newLayer[neigh] = make_pair(layers[i].at(neigh).first, pathWgt+wgt);
					else if(pathWgt+wgt < newLayer.at(neigh).second )
						newLayer.at(neigh).second = pathWgt+wgt;
				}
                        }
                }
                layers[i] = newLayer;
        }
	for(int i=midLayer-2; i>=0; i--){//i=0 not necessary.
                unordered_map<int, pair<float,float> > belowLayer = layers[i+1];
                unordered_map<int, pair<float,float> > newLayer;
                for(unordered_map<int, pair<float,float> >::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
                        int node = it->first;
			float pathWgt = it->second.first;
                        for(int j=0; j<g.degree[node]; j++){
                                int neigh = g.neighbors[g.nodes[node]+j];
				float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
                                if(layers[i].count(neigh)>0){//pattern checking not needed.
					if(newLayer.count(neigh) == 0)
                                                newLayer[neigh] = make_pair(pathWgt + wgt, layers[i].at(neigh).second);
                                        else if(pathWgt+wgt < newLayer.at(neigh).first)
                                                newLayer.at(neigh).first =  pathWgt+wgt;
				}
                        }
                }
		layers[i] = newLayer;
	}

	std::priority_queue<PQEntity_AStar, std::vector<PQEntity_AStar>, comparator_AStar> forward;
	std::priority_queue<PQEntity_AStar, std::vector<PQEntity_AStar>, comparator_AStar> backward;
	vector<unordered_map<int, vector<PQEntity_AStar> > > forward_settled;
	vector<unordered_map<int, vector<PQEntity_AStar> > > backward_settled;//<layer num, node id> as key.
	for(int i=0; i<=maxDepth; i++){
		unordered_map<int, vector<PQEntity_AStar> > tmp;
		forward_settled.push_back(tmp);
		backward_settled.push_back(tmp);
	}

	PQEntity_AStar curNode;
	vector<int> tmpPath;
	tmpPath.push_back(query.src);
	forward.push(createPQEntity_AStar(query.src, 0, 0, tmpPath));//frontier is the priority queue.
	tmpPath.pop_back();
	tmpPath.push_back(query.tgt);
	backward.push(createPQEntity_AStar(query.tgt, 0, 0, tmpPath));//frontier is the priority queue.
	int alter = 0;
	int mem = 0, total = 2;
	priority_queue<Path, std::vector<Path>, comparator2> candidates;
	while( (!forward.empty()) || (!backward.empty()) ){
		mem = max(mem, (int)(forward.size()+backward.size()));
		if(forward.top().key>=minWgt || backward.top().key >= minWgt){//BHPA termination condition.
			break;
		}
/*		if(forward.size()>backward.size()){
			alter = 1;
		}
		else alter = 0;*/
		if(alter%2==0 && !forward.empty()){
			PQEntity_AStar curNode = forward.top();
			forward.pop();
			int curId = curNode.nodeIdx;
			int depth = curNode.path.size()-1;//start from 0.
			vector<int> path = curNode.path;
			if(depth==maxDepth)
				continue;
			if(backward_settled[depth].count(curId) > 0){//reach the end of pattern.
				vector<PQEntity_AStar> backNodeVec = backward_settled[depth].at(curId);
				for(int idx=0; idx<backNodeVec.size(); idx++){
					PQEntity_AStar backNode = backNodeVec[idx];
					vector<int> backPath = backNode.path;
					float newWgt = curNode.wgt+backNode.wgt;
					if(newWgt < minWgt && contain_loop(backPath, path) == false){
						vector<int> tmpPath = path;
						for(int t=backPath.size()-2; t>=0; t--)
							tmpPath.push_back(backPath[t]);
						if(candidates.size()==TOP_K)
 			                               candidates.pop();
                        			candidates.push(createPath(newWgt, tmpPath));
			                        minWgt = candidates.top().wgt;
						break;
					}
				}
			}
			if(forward_settled[depth].count(curId) ==0){
				vector<PQEntity_AStar> tmp;
				forward_settled[depth][curId] = tmp;
			}
			forward_settled[depth].at(curId).push_back(curNode);
			//expanding the neighbors of current node.
			for(int i=0; i<g.degree[curId]; i++){
				int neigh = g.neighbors[g.nodes[curId]+i];//neighbor id.
				//if neighbor is not in the path and follow the pattern.
				unordered_map<int, pair<float,float> >::iterator found = layers[depth+1].find(neigh);
				float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], query.time);
				if( found != layers[depth+1].end()  && find(path.begin(), path.end(), neigh) == path.end()&& edgwgt+curNode.wgt<MAX_WEIGHT){//&& found->second.first >=0
					vector<int> newPath = path;
					newPath.push_back(neigh);
					forward.push(createPQEntity_AStar(neigh, edgwgt+curNode.wgt, edgwgt+curNode.wgt+found->second.first, newPath));
					total += 1;
				}
			}
		}
		else if(alter%2==1 && !backward.empty()){
			PQEntity_AStar curNode;
			curNode = backward.top();
			backward.pop();
			int curId = curNode.nodeIdx;
			int depth = query.pattern.size()-curNode.path.size();//start from 0.
			vector<int> path = curNode.path;
			if(depth==0)
				continue;
			if(forward_settled[depth].count(curId) > 0){//reach the end of pattern.
				vector<PQEntity_AStar> forwardNodeVec = forward_settled[depth].at(curId);
				for(int idx=0; idx<forwardNodeVec.size(); idx++){
					PQEntity_AStar forwardNode = forwardNodeVec[idx];
					vector<int> forwardPath = forwardNode.path;
					float newWgt = curNode.wgt+forwardNode.wgt;
					if(newWgt < minWgt && contain_loop(forwardPath, path) == false){
						for(int t=path.size()-2; t>=0; t--)
							forwardPath.push_back(path[t]);
						if(candidates.size()==TOP_K)
                                                       candidates.pop();
                                                candidates.push(createPath(newWgt, forwardPath));
                                                minWgt = candidates.top().wgt;
						break;
					}
				}
			}
			if(backward_settled[depth].count(curId) ==0){
				vector<PQEntity_AStar> tmp;
				backward_settled[depth][curId] = tmp;
			}
			backward_settled[depth].at(curId).push_back(curNode);

			//expanding the neighbors of current node.
			for(int i=0; i<g.degree[curId]; i++){//15203 [14] 33147 [7] 23410
				int neigh = g.neighbors[g.nodes[curId]+i];//neighbor id.
				float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], query.time);
				//if neighbor is not in the path and follow the pattern.
				unordered_map<int, pair<float,float> >::iterator found = layers[depth-1].find(neigh);
				if( found != layers[depth-1].end() && find(path.begin(), path.end(), neigh) == path.end() && edgwgt+curNode.wgt<MAX_WEIGHT){
					vector<int> newPath = path;
					newPath.push_back(neigh);
					backward.push(createPQEntity_AStar(neigh, edgwgt+curNode.wgt, edgwgt+curNode.wgt+found->second.first, newPath));
					total += 1;
				}
			}
		}
		alter += 1;

	}
	qResult.paths = pq2vec(candidates);
	qResult.mem = mem;
        qResult.totalPaths = total;
        return qResult;
}

