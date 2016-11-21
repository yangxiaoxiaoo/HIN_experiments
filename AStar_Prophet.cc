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
	subtree_vertex[id]=id_inpattern;
	entity.vertex2node = subtree_vertex;
	entity.nodeIdx = id;//current node id
	entity.curId_inpattern = id_inpattern;
	entity.wgt = wgt;//weight of path from root to current node.
	entity.key = key;
	entity.subtree = subtree;//node ids of current path -> now turned into subtree (visited part from root).
	return entity;
}

//Vertex current id is in the subtree expanded so far. And now we are inserting neighbor vertex. The boolean on_left captures whether ot not the neighbor is a left child
Instance_Tree Instance_Tree_Insert(Instance_Tree subtree, int curId, int neigh,float edgewgt,bool on_left){
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
//For testing purpose
/*	for(int ct=0; ct<layers.size(); ct++){
		cout << layers[ct].size() <<":\t" ;
		for(unordered_map<int, float>::iterator itr=layers[ct].begin(); itr != layers[ct].end(); itr++)
			cout << itr->first<<" ";
		cout << endl;
	}*/
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

/*
////////baseline2 utils
//return one result that is not in found_paths
QueryResult AStar_Prophet_pop(const graph_t& g, Query query, double& timeUsed, unordered_set<Path> found_paths){
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
                Path cur_path = createPath(curNode.wgt, path);
                auto p = found_paths.find(curNode);
                //if cur_path not in found_paths
                if (p==found_paths.end()){
                    qResult.paths.push_back();
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

*/

//A step forward in A* -- Three cases depending on whether the nodes are terminals or junctions
int Expand_current(const graph_t& g, Query_tree querytree, vector <int> pre_order_patterns, int curId, PQEntity_AStar_Tree curNode,
               Instance_Tree subtree, int& total,unordered_map<int, unordered_map<int, float>> node2layers, int curId_inpattern,
               std::priority_queue<PQEntity_AStar_Tree, std::vector<PQEntity_AStar_Tree>, comparator_AStar_Tree>& frontier, int& numTrees){
                   //each expanding operation may change: total, frontier, curId, curNode.  other variables wont change.
        bool on_left = true;


		if (find(querytree.junctions.begin(),querytree.junctions.end(), curId_inpattern) == querytree.junctions.end()){
            //curId is not a junction, find the only child
            if (find(querytree.terminals.begin(),querytree.terminals.end(), curId_inpattern) != querytree.terminals.end()){//--curId_inpattern is a terminal
                if (curId_inpattern == querytree.terminals.back()){ //it is the last terminal
                 //   cout<<"warning:--------reached the last terminal but still not finished"<<endl;
                //    cout<<"the size of subtree is "<<subtree.nodes.size()<<", current node id is"<<curId_inpattern<<endl;
                    return 0;
                }
                //if it is not the last terminal:

           //     if (curId_inpattern == 4) cout<<"------------------------debug 4------------"<<endl;
                int next_id_pattern = *(find(pre_order_patterns.begin(), pre_order_patterns.end(), curId_inpattern)+1); //curId_impattern's next one in pre_order_patterns, going to traverse him!
         //       cout << "next id I want to add is"<<next_id_pattern<<endl;
                int old_parent; //old parent is a VERTEX id

                for (auto it = subtree.nodes.begin(); it!= subtree.nodes.end(); ++it){//for node in subtree.nodes:
                    int node = *it;
                    if (curNode.vertex2node.find(node)->second == querytree.map2parent.find(next_id_pattern)->second){
          //          cout<<"test on "<<curNode.vertex2node.find(node)->second<<" == "<<querytree.map2parent.find(next_id_pattern)->second<<endl;

                        old_parent = node;
                    }
                }
                on_left = false; //right neighbor.
                for(int i=0; i<g.degree[old_parent]; i++){
                    int neigh = g.neighbors[g.nodes[old_parent]+i];//neighbor id.
                    unordered_map<int, float>::iterator found = node2layers[next_id_pattern].find(neigh);
                    if(found!= node2layers[next_id_pattern].end() && (find(querytree.terminals.begin(), querytree.terminals.end(), neigh) == querytree.terminals.end()  ||(find(querytree.terminals.begin(), querytree.terminals.end(), next_id_pattern) != querytree.terminals.end())  )){ //if neigh right type; and 1.next one is not terminal or 2.specified by nextid_inpattern terminal
                        //TEST print all neighs
                       // cout<<"??"<<found->first <<"-> " <<found->second<<endl;

                        float edgwgt = calcWgt(g.wgts[g.nodes[old_parent]+i], querytree.time);
                        if(find(subtree.nodes.begin(), subtree.nodes.end(), neigh) == subtree.nodes.end() &&edgwgt+curNode.wgt<MAX_WEIGHT ){//three cond: found neigh in this layer; neigh not already in subtree;
                            //append this neigh to the subtree at the right place
                            Instance_Tree new_subtree = Instance_Tree_Insert(subtree, old_parent, neigh, edgwgt, on_left);

                             //insert neigh to oldparent's right
                            float wgtnew = edgwgt + curNode.wgt;
                //            cout<<new_subtree.wgt<<"should equal to"<<wgtnew<<" is zero??"<<found->second<< endl;

                            frontier.push(createPQEntity_AStar_Tree(neigh, next_id_pattern, edgwgt+curNode.wgt, edgwgt+curNode.wgt+found->second, new_subtree, curNode.vertex2node));
                            numTrees ++;

                        }
                    }
                }

            }
            else{  //curId is not a terminal, not a junction, just on path
                int onlychild;
                if (querytree.map2leftcdr.find(curId_inpattern)!=querytree.map2leftcdr.end()){ //has a left child
                    onlychild = querytree.map2leftcdr[curId_inpattern];
                    on_left = true;
                }
                else {
                        if(querytree.map2rightcdr.find(curId_inpattern)!=querytree.map2rightcdr.end()){
                            //curId has a right child.
                            onlychild = querytree.map2rightcdr[curId_inpattern];
                            on_left = false;
                        }
                        else {
                   //             cout<<"warning, not a path!------------ADDED 0"<<endl;
                                return 0;
                        }
                }

                for(int i=0; i<g.degree[curId]; i++){
                    numTrees ++;
                    int neigh = g.neighbors[g.nodes[curId]+i];//neighbor id.
                    unordered_map<int, float>::iterator found = node2layers[onlychild].find(neigh);
                    float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], querytree.time);
                    if( found != node2layers[onlychild].end() && find(subtree.nodes.begin(), subtree.nodes.end(), neigh) == subtree.nodes.end() && edgwgt+curNode.wgt<MAX_WEIGHT  && (find(querytree.terminals.begin(), querytree.terminals.end(), neigh) == querytree.terminals.end()  ||(find(querytree.terminals.begin(), querytree.terminals.end(), onlychild) != querytree.terminals.end()) ) ){
                        //append this neigh to the subtree at the right place
                        Instance_Tree new_subtree = Instance_Tree_Insert(subtree, curId, neigh, edgwgt, on_left);
                                             //       cout<<new_subtree.nodes.size()<<subtree.nodes.size()<<endl;

                 //       cout<<" is zero??"<<found->second<< endl;

                        frontier.push(createPQEntity_AStar_Tree(neigh, onlychild, edgwgt+curNode.wgt, edgwgt+curNode.wgt+found->second, new_subtree, curNode.vertex2node));
                        total += 1;
                    }
                }
            }
		}
		else{ //curId is a junction, found in left child candidates and right child candidate.
                int leftchild, rightchild;
                leftchild = querytree.map2leftcdr[curId_inpattern];
                rightchild = querytree.map2rightcdr[curId_inpattern];
                //pushback with left child, update the right
                for(int i=0; i<g.degree[curId]; i++){
                    int neighleft = g.neighbors[g.nodes[curId]+i];//neighbor id 1.
                    unordered_map<int, float>::iterator foundleft = node2layers[leftchild].find(neighleft);
                    if (foundleft!= node2layers[leftchild].end()  && (find(querytree.terminals.begin(), querytree.terminals.end(), neighleft) == querytree.terminals.end()  ||(find(querytree.terminals.begin(), querytree.terminals.end(), leftchild) != querytree.terminals.end()) ) ){
                        float edgwgt_left = calcWgt(g.wgts[g.nodes[curId]+i], querytree.time);
                        if (find(subtree.nodes.begin(), subtree.nodes.end(), neighleft) == subtree.nodes.end()&& edgwgt_left+curNode.wgt<MAX_WEIGHT){
                            Instance_Tree new_subtree = Instance_Tree_Insert(subtree, curId, neighleft, edgwgt_left, true);
                                //                        cout<<new_subtree.nodes.size()<<subtree.nodes.size()<<endl;

                            float update_rightvalue = MAX_WEIGHT;
                            for(int j=0; j<g.degree[curId]; j++){ //update the rightvalue for each possible right child, find the lowest possible
                                numTrees ++;
                                if (j!=i){
                                    int neighright = g.neighbors[g.nodes[curId]+j];
                                    unordered_map<int, float>::iterator foundright = node2layers[rightchild].find(neighright);
                                    if (foundright!= node2layers[rightchild].end() && find(subtree.nodes.begin(), subtree.nodes.end(), neighright) == subtree.nodes.end()){
                                        float edgwgt_right = calcWgt(g.wgts[g.nodes[curId]+j], querytree.time);
                                        float bottomup = foundright->second;
                                       // cout<<"bottomup heuristic of rightnode is "<<bottomup<<endl;
                                        if ((edgwgt_right + foundright->second)< update_rightvalue){ //more promising, update.
                                            update_rightvalue = edgwgt_right + foundright->second;
                                           // cout<<"new right value uodated to: "<<update_rightvalue<<endl;
                                        }
                                    }
                                }
                            }
                   //         cout<<" is zero??"<<foundleft->first << foundleft->second<< endl;
                          //  if (foundleft->second != 0)
                            frontier.push(createPQEntity_AStar_Tree(neighleft, leftchild, edgwgt_left+curNode.wgt, edgwgt_left+curNode.wgt +foundleft->second+update_rightvalue, new_subtree, curNode.vertex2node));
                            total += 1;
                        }
                    }
                }
            }

    return curId;

}

///////////////////////HERES STARTS BASELINE UTILITIES///////////////////////


int Expand_current_exhaust(const graph_t& g, Query_tree querytree, vector <int> pre_order_patterns, int curId, PQEntity_AStar_Tree curNode,
               Instance_Tree subtree, int& total,unordered_map<int, unordered_map<int, float>> node2layers, int curId_inpattern,
               std::priority_queue<PQEntity_AStar_Tree, std::vector<PQEntity_AStar_Tree>, comparator_AStar_Tree>& frontier, int& numTrees){
                   //each expanding operation may change: total, frontier, curId, curNode.  other variables wont change.
        bool on_left = true;


		if (find(querytree.junctions.begin(),querytree.junctions.end(), curId_inpattern) == querytree.junctions.end()){
            //curId is not a junction, find the only child
            if (find(querytree.terminals.begin(),querytree.terminals.end(), curId_inpattern) != querytree.terminals.end()){//--curId_inpattern is a terminal
                if (curId_inpattern == querytree.terminals.back()){ //it is the last terminal
              //      cout<<"warning:--------reached the last terminal but still not finished"<<endl;
            //        cout<<"the size of subtree is "<<subtree.nodes.size()<<", current node id is"<<curId_inpattern<<endl;
                    return 0;
                }
                //if it is not the last terminal:

        //        if (curId_inpattern == 4) cout<<"------------------------debug 4------------"<<endl;
                int next_id_pattern = *(find(pre_order_patterns.begin(), pre_order_patterns.end(), curId_inpattern)+1); //curId_impattern's next one in pre_order_patterns, going to traverse him!
          //      cout << "next id I want to add is"<<next_id_pattern<<endl;
                int old_parent; //old parent is a VERTEX id

                for (auto it = subtree.nodes.begin(); it!= subtree.nodes.end(); ++it){//for node in subtree.nodes:
                    int node = *it;
                    if (curNode.vertex2node.find(node)->second == querytree.map2parent.find(next_id_pattern)->second){
          //          cout<<"test on "<<curNode.vertex2node.find(node)->second<<" == "<<querytree.map2parent.find(next_id_pattern)->second<<endl;

                        old_parent = node;
                    }
                }
                on_left = false; //right neighbor.
                for(int i=0; i<g.degree[old_parent]; i++){
                    int neigh = g.neighbors[g.nodes[old_parent]+i];//neighbor id.
                    unordered_map<int, float>::iterator found = node2layers[next_id_pattern].find(neigh);
                    if(found!= node2layers[next_id_pattern].end() && (find(querytree.terminals.begin(), querytree.terminals.end(), neigh) == querytree.terminals.end()  ||(find(querytree.terminals.begin(), querytree.terminals.end(), next_id_pattern) != querytree.terminals.end())  )){ //if neigh right type; and 1.next one is not terminal or 2.specified by nextid_inpattern terminal
                        //TEST print all neighs
                       // cout<<"??"<<found->first <<"-> " <<found->second<<endl;

                        float edgwgt = calcWgt(g.wgts[g.nodes[old_parent]+i], querytree.time);
                        if(find(subtree.nodes.begin(), subtree.nodes.end(), neigh) == subtree.nodes.end() &&edgwgt+curNode.wgt<MAX_WEIGHT ){//three cond: found neigh in this layer; neigh not already in subtree;
                            //append this neigh to the subtree at the right place
                            Instance_Tree new_subtree = Instance_Tree_Insert(subtree, old_parent, neigh, edgwgt, on_left);

                             //insert neigh to oldparent's right
                            float wgtnew = edgwgt + curNode.wgt;
            //                cout<<new_subtree.wgt<<"should equal to"<<wgtnew<<" is zero??"<<found->second<< endl;

                            frontier.push(createPQEntity_AStar_Tree(neigh, next_id_pattern, edgwgt+curNode.wgt, edgwgt+curNode.wgt, new_subtree, curNode.vertex2node));
                            numTrees ++;

                        }
                    }
                }

            }
            else{  //curId is not a terminal, not a junction, just on path
                int onlychild;
                if (querytree.map2leftcdr.find(curId_inpattern)!=querytree.map2leftcdr.end()){ //has a left child
                    onlychild = querytree.map2leftcdr[curId_inpattern];
                    on_left = true;
                }
                else {
                        if(querytree.map2rightcdr.find(curId_inpattern)!=querytree.map2rightcdr.end()){
                            //curId has a right child.
                            onlychild = querytree.map2rightcdr[curId_inpattern];
                            on_left = false;
                        }
                        else {
                           //     cout<<"warning, not a path!------------ADDED 0"<<endl;
                                return 0;
                        }
                }

                for(int i=0; i<g.degree[curId]; i++){
                    numTrees ++;
                    int neigh = g.neighbors[g.nodes[curId]+i];//neighbor id.
                    unordered_map<int, float>::iterator found = node2layers[onlychild].find(neigh);
                    float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], querytree.time);
                    if( found != node2layers[onlychild].end() && find(subtree.nodes.begin(), subtree.nodes.end(), neigh) == subtree.nodes.end() && edgwgt+curNode.wgt<MAX_WEIGHT  && (find(querytree.terminals.begin(), querytree.terminals.end(), neigh) == querytree.terminals.end()  ||(find(querytree.terminals.begin(), querytree.terminals.end(), onlychild) != querytree.terminals.end()) ) ){
                        //append this neigh to the subtree at the right place
                        Instance_Tree new_subtree = Instance_Tree_Insert(subtree, curId, neigh, edgwgt, on_left);
                                             //       cout<<new_subtree.nodes.size()<<subtree.nodes.size()<<endl;

                 //       cout<<" is zero??"<<found->second<< endl;

                        frontier.push(createPQEntity_AStar_Tree(neigh, onlychild, edgwgt+curNode.wgt, edgwgt+curNode.wgt, new_subtree, curNode.vertex2node));
                        total += 1;
                    }
                }
            }
		}
		else{ //curId is a junction, found in left child candidates and right child candidate.
                int leftchild, rightchild;
                leftchild = querytree.map2leftcdr[curId_inpattern];
                rightchild = querytree.map2rightcdr[curId_inpattern];
                //pushback with left child, update the right
                for(int i=0; i<g.degree[curId]; i++){
                    int neighleft = g.neighbors[g.nodes[curId]+i];//neighbor id 1.
                    unordered_map<int, float>::iterator foundleft = node2layers[leftchild].find(neighleft);
                    if (foundleft!= node2layers[leftchild].end()  && (find(querytree.terminals.begin(), querytree.terminals.end(), neighleft) == querytree.terminals.end()  ||(find(querytree.terminals.begin(), querytree.terminals.end(), leftchild) != querytree.terminals.end()) ) ){
                        float edgwgt_left = calcWgt(g.wgts[g.nodes[curId]+i], querytree.time);
                        if (find(subtree.nodes.begin(), subtree.nodes.end(), neighleft) == subtree.nodes.end()&& edgwgt_left+curNode.wgt<MAX_WEIGHT){
                            Instance_Tree new_subtree = Instance_Tree_Insert(subtree, curId, neighleft, edgwgt_left, true);
                                 //                       cout<<new_subtree.nodes.size()<<subtree.nodes.size()<<endl;

                            float update_rightvalue = MAX_WEIGHT;
                            for(int j=0; j<g.degree[curId]; j++){ //update the rightvalue for each possible right child, find the lowest possible
                                numTrees ++;
                                if (j!=i){
                                    int neighright = g.neighbors[g.nodes[curId]+j];
                                    unordered_map<int, float>::iterator foundright = node2layers[rightchild].find(neighright);
                                    if (foundright!= node2layers[rightchild].end() && find(subtree.nodes.begin(), subtree.nodes.end(), neighright) == subtree.nodes.end()){
                                        float edgwgt_right = calcWgt(g.wgts[g.nodes[curId]+j], querytree.time);
                                        float bottomup = foundright->second;
                                       // cout<<"bottomup heuristic of rightnode is "<<bottomup<<endl;
                                        if ((edgwgt_right + foundright->second)< update_rightvalue){ //more promising, update.
                                            update_rightvalue = edgwgt_right + foundright->second;
                                           // cout<<"new right value uodated to: "<<update_rightvalue<<endl;
                                        }
                                    }
                                }
                            }
                   //         cout<<" is zero??"<<foundleft->first << foundleft->second<< endl;
                          //  if (foundleft->second != 0)
                            frontier.push(createPQEntity_AStar_Tree(neighleft, leftchild, edgwgt_left+curNode.wgt, edgwgt_left+curNode.wgt, new_subtree, curNode.vertex2node));
                            total += 1;
                        }
                    }
                }
            }

    return curId;

}

//insert all candidates of this_node , append it next to check_connection_node in the specified way.
vector<Instance_Tree> Set_insert(const graph_t& g, Instance_Tree Old_tree, int this_node, int check_connection_node, bool insert_parent, bool insert_left, unordered_map<int, int> vertex2node,     unordered_map<int, unordered_map<int, float>> node2layers, int& numtrees){
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
    return new_trees;
}
//done

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
//        cout<<" this node is "<<this_node <<endl;
        //connected = false; //reset: dont assume connection for this node.
        for (auto itr = incomplete_tree.nodes.begin(); itr != incomplete_tree.nodes.end(); ++itr){
 //           cout<<" compare to "<< vertex2node[*itr]<<endl;
            if(this_node == vertex2node[*itr]){
                this_node_inmapped = true;
 //               cout <<"seen existing!"<<endl;
            }
        }
        //if ( this_node not in incomplete_tree.mapped_nodes){ //a unfixed node: is it connected?
        if (!this_node_inmapped){
//            cout<<" this node is new!!"<<this_node <<endl;
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
                   //     cout<<this_node<< "'s left child is "<<check_connection_node<<endl;
                        insert_left = true;
                        break; //inner for loop
                    }
                    //if check_connection_node is rightchild of this_node{
                    if (querytree.map2rightcdr.find(this_node)!= querytree.map2rightcdr.end()
                        &&querytree.map2rightcdr[this_node] == check_connection_node){
                  //      cout<<this_node<< "'s right child is "<<check_connection_node<<endl;
                        insert_left = false;
                        break;
                    }
                }
                else {
                    //if this_node is leftchild of check_connection_node{

                    if (querytree.map2leftcdr.find(check_connection_node)!= querytree.map2leftcdr.end()
                        &&querytree.map2leftcdr[check_connection_node] == this_node){
                   //     cout<<check_connection_node<< "'s left child is "<<this_node<<endl;
                        connected = true;
                        insert_parent = false;
                        insert_left = true;
                        break;
                    }
                    //else if this_node is rightchild of check_connection_node{
                    else if (querytree.map2rightcdr.find(check_connection_node)!= querytree.map2rightcdr.end()
                             &&querytree.map2rightcdr[check_connection_node] == this_node){
                   //     cout<<check_connection_node<< "'s right child is "<<this_node<<endl;
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
 //               cout<<"found connected, this node is "<<this_node<<"connected_to "<< check_connection_node<<endl;

                new_trees = Set_insert(g, incomplete_tree, this_node, check_connection_node, insert_parent, insert_left, vertex2node, node2layers, numtrees);
  //              cout<<"inserted num is "<<new_trees.size()<<endl;
                found = true;
                break;//outer loop break, do not check other this_node.
            }
            //if still none of them connected, cannot break, the outer loop is giving us a new this_node
        }
        if (found) break;

    }//here we have examined all this_node that is not there.

    return new_trees;
}
//done


//a comparison that takes a parameter and return an operator
struct compare_srtuct{
    compare_srtuct(vector<Instance_Tree> complete_trees) {this -> complete_trees = complete_trees; }
    bool operator() (int i, int j) {//both i and j are index that we want to compare weight on
        return (complete_trees[i].wgt < complete_trees[j].wgt);
    }
    vector<Instance_Tree> complete_trees;
};
//done

//take a list of matching trees and return top k lightest ones
vector<Instance_Tree> Top_k_weight(vector<Instance_Tree> complete_trees){
    vector<int> top_k_index;
    vector<Instance_Tree> Top_k_trees;
    float cur_weight;
    float largest_wgt;
    for(int i=0; i< complete_trees.size(); i++){
        if(top_k_index.size() >= TOP_K){ //already full: replace if see a better
            cur_weight = complete_trees[i].wgt;
            largest_wgt = complete_trees[top_k_index.back()].wgt; //largest weight is always the last one
            if (cur_weight < largest_wgt){ //less then the last element which is the largest
                top_k_index.pop_back();
                top_k_index.push_back(i);
                sort(top_k_index.begin(), top_k_index.end(), compare_srtuct(complete_trees)); //sort by the element weight
            }
        }
        else{//not full yet, just add!!
            cur_weight = complete_trees[i].wgt;
            top_k_index.push_back(i);
        }
    }
    //now we have the index, use that to pick trees
    if (top_k_index.size() == TOP_K){
 //       cout<<"select top k successful!"<<endl;
    }
//    else cout<<"************WARNING*************TOPK SELECTION from only "<<complete_trees.size()<<" candidates to " <<top_k_index.size()<<endl;
    for (int i=0; i<top_k_index.size();i++){
        Top_k_trees.push_back(complete_trees[top_k_index[i]]);
    }
    return Top_k_trees;
}
//done


//Take graph g and the query tree, change the vertices to node map, the mapback to the prophet mapping that discribes all that matches type
int typecheck_all(const graph_t& g, Query_tree querytree, unordered_map<int, int>& vertex2node, unordered_map<int, unordered_map<int, float>>& node2layers){
    int iterationnum = querytree.patterns.size()-1;
    float minWgt = MAX_WEIGHT;
    for(int i=0; i<querytree.terminals_index.size(); i++){
  //      cout<<"TEST OUTPUT "<<g.typeMap[querytree.nodes_ordered[querytree.terminals_index[i]]]<<" & "<<querytree.patterns[querytree.terminals_index[i]]<<endl;
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
    unordered_map<int, unordered_map<int, float>> node2layers;
    unordered_map<int, int> vertex2node;
	vector<Instance_Tree> complete_trees;
	vector<Instance_Tree> incompletetrees;
	vector<Instance_Tree> modified_trees;
	Instance_Tree modified_tree;
    typecheck_all(g, querytree,vertex2node, node2layers); //now these vertext to node and back mappings all give the right type
    cout<<vertex2node.size()<<","<<node2layers.size()<<"should be large num"<<endl;
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
	result.trees = Top_k_weight(complete_trees);
	result.numTrees = numtrees;
	result.mem = mem;
	result.totalTrees = totalTrees;
	return result;
}

/////////////////END OF BASELINE 1/////////////////




/*

//////////////////BASELINE 2////////////////////
//Baseline 2: decompose the tree into a longest path and feed paths to albert's algo
//from the longest path node to terminals, do sinple path matching.

//reuse from bruteforce: give me all complete trees.
vector<Instance_Tree> All_matching_trees(const graph_t& g, Query_tree querytree, double& timeUsed){
    unordered_map<int, unordered_map<int, float>> node2layers;
    unordered_map<int, int> vertex2node;
	vector<Instance_Tree> complete_trees;
	vector<Instance_Tree> incompletetrees;
	vector<Instance_Tree> modified_trees;
	Instance_Tree modified_tree;
    typecheck_all(g, querytree,vertex2node, node2layers); //now these vertext to node and back mappings all give the right type
    cout<<vertex2node.size()<<","<<node2layers.size()<<"should be large num"<<endl;
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

Query GetBackbone(const graph_t& g, Query_tree querytree){
    int src, tgt;
    vector<int> pattern_root2src;
    vector<int> pattern_root2tgt;
    Query backbone;
    int curnode = querytree.nodes_ordered.back();//start with root as a curnode.
    //traverse from root to the first terminal:
    while(true){
        pattern_root2src.push_back(g.typeMap[curnode])
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
        pattern_root2tgt.push_back(g.typeMap[curnode])
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
    for(int i=1; i<pattern_root2tgt.size(); i++){
        pattern_root2src.push_back(pattern_root2tgt[i]);
    }
    backbone.src = src;
    backbone.tgt = tgt;
    backbone.time = querytree.time;
    backbone.pattern = pattern_root2src;
    return backbone;
}
//done

//given a backbone instance, return the matching left_subtree as a usable query. all given nodes viewed as terminals
Query_tree left_from_backbone(Query_tree querytree, Query BackboneQuery, Path backboneinstance){

}

Query_tree right_from_backbone(Query_tree querytree, Query BackboneQuery, Path backboneinstance){

}

Instance_Tree Combine_tree(Query_tree left_query, Query_tree right_query, Query_tree query, Instance_Tree left_instance, Instance_Tree, right_instance){

}


QueryResultTrees Backbone_query(const graph_t& g, Query_tree querytree, double& timeUsed){
    Query longest_path;
    Path longest_path_instance;
    unordered_set<Path> found_paths;

    longest_path = GetBackbone(g, querytree);

    int numtrees, mem, totalTrees = 0; //TODO: increase them accordingly!

    float min_kseen_weight = MAX_WEIGHT;
    float newest_path_wgt = 0;

    vector<Instance_Tree> candidate_trees;
    vector<Instance_Tree> TOPk_trees;
    Query_tree left_querytree;
    Query_tree right_querytree;
    vector<Instance_Tree> left_instances;
    vector<Insrance_Tree> right_instances;
    Instance_Tree right_instance;
    Instance_Tree left_instance;
    Instance_Tree combined_tree;

    while(true){
        QueryResult first_path = AStar_Prophet_pop(g, longest_path, timeUsed, found_paths);
        if (first_path.paths.size()==0){
            break; //break condition 1/2: when no more matching path can be found.
        }
        longest_path_instance = first_path.paths[0];
        found_paths.insert(longest_path_instance);
        newest_path_wgt = longest_path_instance.wgt;
        min_kseen_weight = Top_k_weight(candidate_trees).back().wgt;
        if (newest_path_wgt > min_kseen_weight){
            break; //break condition 2/2: when this current backbone instance weight is larger then the largest of k-lightest ones, then it becomes useless to invest.
        }

        newest_path_wgt = longest_path_instance.wgt;
        left_querytree = left_from_backbone(querytree, longest_path, longest_path_instance);
        right_querytree = right_from_backbone(querytree, longest_path, longest_path_instance);
        left_instances = All_matching_trees(g, left_querytree, timeUsed);
        right_instances =  All_matching_trees(g, right_querytree, timeUsed);
        for (int i=0; i<left_instances.size(); i++){
            left_instance = left_instances[i];
            for (int j=0, j<right_instances.size(); i++){
                right_instance = right_instances[j];
                combined_tree = Combine_tree(left_querytree, right_querytree, left_instance, right_instance);
                candidate_trees.push_back(combined_tree);
            }
        }
    }

    //NOTICE: this simple TOP pop one by one may be slow, since we have to swipe k times, and check each step finishing a tree.
    //--if it turns out to be the case, consider recursive.
    TOPk_trees = Top_k_weight(candidate_trees);
    result.trees = TOPk_trees;
    result.numTrees = numTrees;
    result.mem = mem;
    result.totalTrees = totalTrees;
    return result;
}



///////////////////END OF BASELINE 2//////////////////////

*/







///////////////////BASELINE 1.5///////////////////
QueryResultTrees Bruteforce_modified(const graph_t& g, Query_tree querytree, double& timeUsed){
 int mem = 0, total = 1, numTrees = 0;
	int iterationnum = querytree.patterns.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResultTrees qResultTree;


        //check the struct invarient
        //Check if the terminals are of the type that match the pattern requirements (in order)
        for(int i=0; i<querytree.terminals_index.size(); i++){
   //         cout<<"TEST OUTPUT "<<g.typeMap[querytree.nodes_ordered[querytree.terminals_index[i]]]<<" & "<<querytree.patterns[querytree.terminals_index[i]]<<endl;




            if( (g.typeMap[querytree.nodes_ordered[querytree.terminals_index[i]]]!=querytree.patterns[querytree.terminals_index[i]])){
                //cout << query.src << " to " << query.tgt << endl;
                //cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
     //           cout<< "src or tgt node does not follow pattern!" << endl;
                return qResultTree;
                }
        }

    //Decompose the pattern (binary tree) into paths
    vector<GeneralizedQuery> decomposed_queries = decompo_Query_Tree(querytree);

    //Maps a node in pattern to vertices in input graph that potentially map to it
    //potentially implies type matches of paths to all terminals
    unordered_map<int, unordered_map<int, float>> node2layers; //candidate set with each's weight from lower level terminals. Will return this as tree prophet graph

    //Every time a vertex is in the node2layers of a node, record it here. Note that a vertex may be mapped to many different nodes in the pattern.
    unordered_map<int, int> vertex2node; //NOTE:  vertex2node is not 1-1 mapping

    //Intermediate data structures to record who is from left and who is from right to a junction node.
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
        layers = create_Prophet_Heuristics_generalized(g, current_query, timeUsed, current_junction_leftmap, current_junction_rightmap);//layers stores the legitimate nodes on each level. Each layer is a set.

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

 /*for testing
    int test1 = node2layers.size();
    int test2 = junction_leftmap.size();
    cout<<"all node to layers should be here: "<<test1<<endl;
    cout<<"all junctions should be here: "<<test2<<endl;
*/

//at this point, each node has a heuristic weight from leaves.

//TODO3: Astar--frontier motification. A* will do the propergate from top down
    //A priority queue in a vector container containing elements of type PQEntity_AStar_Tree
	std::priority_queue<PQEntity_AStar_Tree, std::vector<PQEntity_AStar_Tree>, comparator_AStar_Tree> frontier;
	PQEntity_AStar_Tree curNode;
	//Root is the last element of the post_order traversal of pattern tree
	int root = querytree.nodes_ordered.back();

	//for all vertices matching the root node, push them in priority queue
	for(unordered_map<int, float>::iterator it_root=node2layers[root].begin(); it_root!=node2layers[root].end(); it_root++){
        Instance_Tree tmptree;
        tmptree.nodes.insert(it_root->first);
        tmptree.wgt = 0;
        unordered_map<int, int> empty_vertex2node;
        frontier.push(createPQEntity_AStar_Tree(it_root->first, root, 0, 0, tmptree, empty_vertex2node));//frontier is the priority queue. replace src with root.
        numTrees ++;
	}



    //build a reference to decide which is the next curId_inpattern the neighbor should match to, on which side,
    //b pre-order based on a stack.
    stack <int> s;
    int curId_inpattern = root;
    vector <int> pre_order_patterns; //they are id of nodes, not type. NOTE
    //Traversing query pattern in pre-order
    while(true){
            if (pre_order_patterns.size()>= querytree.nodes_ordered.size()) break;
            while(true){
                pre_order_patterns.push_back(curId_inpattern); //first time at one node.
      //          cout<<"expended on current pattern is "<< curId_inpattern<< endl;
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
    //now, pre_order_patterns is recording the patterns.
    //for expansion, we need to know who is the next pattern node to match.

    //Top-down traversal in pre-order
	while(!frontier.empty()){//each time check one element in the PQ.
		mem = max(mem, (int)frontier.size());
		PQEntity_AStar_Tree curNode;
		curNode = frontier.top();
		frontier.pop();

		int curId = curNode.nodeIdx;
		int curId_inpattern = curNode.curId_inpattern;



		int done_countes = curNode.subtree.nodes.size()-1;//start from 0.  done_counts: refer to the old path depth
//		cout<<"key of PQ is"<<curNode.key<<", done counts = "<<done_countes<<endl;
		Instance_Tree subtree = curNode.subtree;
        if (curId == 4){
   //         cout<<"pop out frontier curId is"<<curId<<endl;
   //         cout<<done_countes<< "his inpattern node is"<<curId_inpattern<<endl;



		}

		if(done_countes==iterationnum){//reach the end of pattern.
            //test
           // break;
			if(curNode.wgt<MAX_WEIGHT){ //(%all terminal nodes are reached)--actually redundent.
				if(qResultTree.trees.size()<TOP_K){
                        subtree.wgt = curNode.wgt;
                        qResultTree.trees.push_back(subtree);}
                                if(qResultTree.trees.size()==TOP_K)
                                        break;
			}
			continue;
		}
		//expanding the neighbors of current node
		//Expand the most promising path with vertices mapping the next node in the path (if it exists, but can trace back otherwise)
		//It also updates the value of the right path when entering the junction for the first time. This update involves finding the best heuristic estimate, given that the junction vertex is fixed


        curId = Expand_current_exhaust(g, querytree, pre_order_patterns, curId, curNode, subtree,total,node2layers, curId_inpattern,frontier, numTrees);
        //each expanding operation may change: total, frontier, curId, curNode.
		}

	qResultTree.mem = mem;
	qResultTree.numTrees = numTrees;
        qResultTree.totalTrees = total;
        //Return k-lightest matching trees
        return qResultTree;
}

///////////////////MAIN ALGORITHM//////////////////////////

//Computes a prophet graph and runs A* to return k-ligh matching instances
QueryResultTrees AStar_Prophet_Tree(const graph_t& g, Query_tree querytree, double& timeUsed){
    int mem = 0, total = 1, numTrees = 0;
	int iterationnum = querytree.patterns.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResultTrees qResultTree;


        //check the struct invarient
        //Check if the terminals are of the type that match the pattern requirements (in order)
        for(int i=0; i<querytree.terminals_index.size(); i++){
            cout<<"TEST OUTPUT "<<g.typeMap[querytree.nodes_ordered[querytree.terminals_index[i]]]<<" & "<<querytree.patterns[querytree.terminals_index[i]]<<endl;




            if( (g.typeMap[querytree.nodes_ordered[querytree.terminals_index[i]]]!=querytree.patterns[querytree.terminals_index[i]])){
                //cout << query.src << " to " << query.tgt << endl;
                //cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResultTree;
                }
        }

    //Decompose the pattern (binary tree) into paths
    vector<GeneralizedQuery> decomposed_queries = decompo_Query_Tree(querytree);

    //Maps a node in pattern to vertices in input graph that potentially map to it
    //potentially implies type matches of paths to all terminals
    unordered_map<int, unordered_map<int, float>> node2layers; //candidate set with each's weight from lower level terminals. Will return this as tree prophet graph

    //Every time a vertex is in the node2layers of a node, record it here. Note that a vertex may be mapped to many different nodes in the pattern.
    unordered_map<int, int> vertex2node; //NOTE:  vertex2node is not 1-1 mapping

    //Intermediate data structures to record who is from left and who is from right to a junction node.
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
        layers = create_Prophet_Heuristics_generalized(g, current_query, timeUsed, current_junction_leftmap, current_junction_rightmap);//layers stores the legitimate nodes on each level. Each layer is a set.

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

 /*for testing
    int test1 = node2layers.size();
    int test2 = junction_leftmap.size();
    cout<<"all node to layers should be here: "<<test1<<endl;
    cout<<"all junctions should be here: "<<test2<<endl;
*/

//at this point, each node has a heuristic weight from leaves.

//TODO3: Astar--frontier motification. A* will do the propergate from top down
    //A priority queue in a vector container containing elements of type PQEntity_AStar_Tree
	std::priority_queue<PQEntity_AStar_Tree, std::vector<PQEntity_AStar_Tree>, comparator_AStar_Tree> frontier;
	PQEntity_AStar_Tree curNode;
	//Root is the last element of the post_order traversal of pattern tree
	int root = querytree.nodes_ordered.back();

	//for all vertices matching the root node, push them in priority queue
	for(unordered_map<int, float>::iterator it_root=node2layers[root].begin(); it_root!=node2layers[root].end(); it_root++){
        Instance_Tree tmptree;
        tmptree.nodes.insert(it_root->first);
        tmptree.wgt = 0;
        unordered_map<int, int> empty_vertex2node;
        frontier.push(createPQEntity_AStar_Tree(it_root->first, root, 0, it_root->second, tmptree, empty_vertex2node));//frontier is the priority queue. replace src with root.
        numTrees ++;
	}



    //build a reference to decide which is the next curId_inpattern the neighbor should match to, on which side,
    //b pre-order based on a stack.
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
    //now, pre_order_patterns is recording the patterns.
    //for expansion, we need to know who is the next pattern node to match.

    //Top-down traversal in pre-order
	while(!frontier.empty()){//each time check one element in the PQ.
		mem = max(mem, (int)frontier.size());
		PQEntity_AStar_Tree curNode;
		curNode = frontier.top();
		frontier.pop();

		int curId = curNode.nodeIdx;
		int curId_inpattern = curNode.curId_inpattern;



		int done_countes = curNode.subtree.nodes.size()-1;//start from 0.  done_counts: refer to the old path depth
//		cout<<"key of PQ is"<<curNode.key<<", done counts = "<<done_countes<<endl;
		Instance_Tree subtree = curNode.subtree;
        if (curId == 4){
            cout<<"pop out frontier curId is"<<curId<<endl;
            cout<<done_countes<< "his inpattern node is"<<curId_inpattern<<endl;



		}

		if(done_countes==iterationnum){//reach the end of pattern.
            //test
           // break;
			if(curNode.wgt<MAX_WEIGHT){ //(%all terminal nodes are reached)--actually redundent.
				if(qResultTree.trees.size()<TOP_K){
                        subtree.wgt = curNode.wgt;
                        qResultTree.trees.push_back(subtree);}
                                if(qResultTree.trees.size()==TOP_K)
                                        break;
			}
			continue;
		}
		//expanding the neighbors of current node
		//Expand the most promising path with vertices mapping the next node in the path (if it exists, but can trace back otherwise)
		//It also updates the value of the right path when entering the junction for the first time. This update involves finding the best heuristic estimate, given that the junction vertex is fixed


        curId = Expand_current(g, querytree, pre_order_patterns, curId, curNode, subtree,total,node2layers, curId_inpattern,frontier, numTrees);
        //each expanding operation may change: total, frontier, curId, curNode.
		}

	qResultTree.mem = mem;
	qResultTree.numTrees = numTrees;
        qResultTree.totalTrees = total;
        //Return k-lightest matching trees
        return qResultTree;
}

/***already implemented in layers. do not need anymore.
float getHeuristicValueTree(const graph_t&g, Query_tree query_tree, int depth, int curNode){
    //given a tree, calculate heuristic
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

*/

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
			//cout << depth << endl; cout << layers.size() << endl;
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
			//cout << depth << endl; cout << layers.size() << endl;
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
//Not much improvement.
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

/*
QueryResult AStar_Prophet_Explicit(const graph_t& g, Query query){
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
	double tp;
	vector<unordered_map<int,float> > layers = create_Prophet_Heuristics(g, query, tp);//layers stores the legitimate nodes on each level. Each layer is a set.

	vector<unordered_map<int, ProphetEntry> > prophetGraph;
	for(int i=0; i<=maxDepth; i++){
		unordered_map<int, ProphetEntry> oneLayer;
		prophetGraph.push_back(oneLayer);
	}
	for(unordered_map<int,float>::iterator it=layers[midLayer-1].begin(); it!=layers[midLayer-1].end(); it++){
		int node = it->first;
		ProphetEntry tmpEntry;
                prophetGraph[midLayer-1][node] = tmpEntry;
	}
	for(int i=midLayer; i<=query.pattern.size()-1; i++){//going down
                unordered_map<int,float> lastLayer = layers[i-1];//upper layer.
                unordered_map<int,float> newLayer;
                for(unordered_map<int,float>::iterator it=lastLayer.begin(); it!=lastLayer.end(); it++){//expanding each node on last layer.
                        int node = it->first;
                        for(int j=0; j<g.degree[node]; j++){//visit the neighbors of each node.
                                int neigh = g.neighbors[g.nodes[node]+j];
                                if(layers[i].count(neigh)>0){//Note: pattern checking not needed. g.typeMap[neigh] == query.pattern[i] &&
                               		float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
                                        newLayer.insert(*(layers[i].find(neigh)));
					prophetGraph[i-1].at(node).downwards.push_back(make_pair(neigh,wgt));
					if(prophetGraph[i].count(neigh) == 0){
						ProphetEntry tmpEntry;
						prophetGraph[i][neigh] = tmpEntry;
					}
				}
                        }
                }
                layers[i] = newLayer;
        }
	for(int i=midLayer-2; i>=0; i--){//going up
                unordered_map<int, float> belowLayer = layers[i+1];
                unordered_map<int, float> newLayer;
                for(unordered_map<int, float>::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
                        int node = it->first;
                        for(int j=0; j<g.degree[node]; j++){
                                int neigh = g.neighbors[g.nodes[node]+j];
				float pathWgt = it->second;
                                if(layers[i].count(neigh)>0){
                               		float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
					if(prophetGraph[i].count(neigh)==0){
						ProphetEntry tmpEntry;
						prophetGraph[i][neigh] = tmpEntry;
						newLayer[neigh] = pathWgt + wgt;
					}
					else
						 newLayer[neigh] = min(newLayer.at(neigh), pathWgt+wgt);
					prophetGraph[i].at(neigh).downwards.push_back(make_pair(node,wgt));
				}
                        }
                }
		layers[i] = newLayer;
	}

	/**********Start searching***********/

	/********************
	std::priority_queue<PQEntity_AStar, std::vector<PQEntity_AStar>, comparator_AStar> frontier;
	PQEntity_AStar curNode;
	vector<int> tmpPath;
	tmpPath.push_back(query.src);
	frontier.push(createPQEntity_AStar(query.src, 0, 0, tmpPath));//frontier is the priority queue.
	int mem = 0, total=1;
	while(!frontier.empty()){
		mem = max(mem, (int)frontier.size());
		PQEntity_AStar curNode;
		curNode = frontier.top();
		frontier.pop();
		int curId = curNode.nodeIdx;
		int depth = curNode.path.size()-1;//start from 0.
		vector<int> path = curNode.path;
		if(depth==maxDepth){//reach the end of pattern.
			if(curId == query.tgt){
				if(qResult.paths.size()<TOP_K)
                                        qResult.paths.push_back(createPath(curNode.wgt, path));
                                if(qResult.paths.size()==TOP_K)
                                        break;
			}
			continue;
		}
		//expanding the neighbors of current node.
		vector<pair<int,float> > neighVec = prophetGraph[depth][curId].downwards;
		for(int j=0; j<neighVec.size(); j++){
			int neigh = neighVec[j].first;//neighbor id.
			float edgwgt = neighVec[j].second;
			//if neighbor is not in the path and follow the pattern.
			if( find(path.begin(), path.end(), neigh) == path.end() && edgwgt+curNode.wgt<MAX_WEIGHT ){
				vector<int> newPath = path;
				newPath.push_back(neigh);
				frontier.push(createPQEntity_AStar(neigh, edgwgt+curNode.wgt, edgwgt+curNode.wgt+(layers[depth+1]).at(neigh), newPath));
				total += 1;
			}
		}

	}
	qResult.mem = mem;
        qResult.totalPaths = total;
        return qResult;
}

******************/
