#include "global.h"
#include "proto.h"
#include "pruned_landmark_labeling.h"
using namespace std;

typedef struct PQEntity_AStar{
        int nodeIdx;
        float wgt;
	float key;
        std::vector<int> path;
}PQEntity_AStar;

//comparator for the priority queue in dijkstra algorithm.
struct comparator_AStar{
	bool operator()(PQEntity_AStar p1, PQEntity_AStar p2){
		return p1.key > p2.key;
	}
};

PQEntity_AStar createPQEntity_AStar(int id, float wgt, float key, std::vector<int> path){
	PQEntity_AStar entity;
	entity.nodeIdx = id;//node id
	entity.wgt = wgt;//weight of path from src to current node.
	entity.key = key;
	entity.path = path;//node ids of current path. need to change if we need to output the path itself.
	return entity;
}

//adaptation for binary trees
typedef struct PQEntity_AStar_Tree{
        int nodeIdx;
        float wgt;
	float key;
	bool junction;
       // std::vector<int> subtree;
       Instance_Tree subtree;
}PQEntity_AStar_Tree;

//comparator for the priority queue in dijkstra algorithm.
struct comparator_AStar_Tree{
	bool operator()(PQEntity_AStar_Tree p1, PQEntity_AStar_Tree p2){
		return p1.key > p2.key;
	}
};

PQEntity_AStar_Tree createPQEntity_AStar_Tree(int id, float wgt, float key, Instance_Tree subtree){
	PQEntity_AStar_Tree entity;
	entity.nodeIdx = id;//current node id
	entity.wgt = wgt;//weight of path from root to current node.
	entity.key = key;
	entity.subtree = subtree;//node ids of current path -> now turned into subtree (visited part from root).
	return entity;
}

Instance_Tree Instance_Tree_Insert(Instance_Tree subtree, int curId, int neigh, bool on_left){
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


void Expand_current(const graph_t& g, Query_tree querytree, vector <int> pre_order_patterns, int& curId, PQEntity_AStar& curNode,
               Instance_Tree subtree, int& total,unordered_map<int, unordered_map<int, float>> node2layers, unordered_map<int, int> vertex2node,
               std::priority_queue<PQEntity_AStar, std::vector<PQEntity_AStar>, comparator_AStar>& frontier){
                   //each expanding operation may change: total, frontier, curId, curNode.  other variables wont change.
        bool on_left = true;
        int curId_inpattern = vertex2node[curId];
		if (find(querytree.junctions.begin(),querytree.junctions.end(), curId_inpattern) == querytree.junctions.end()){
            //curId is not a junction, find the only child
            if (--curId_inpattern is a terminal){          //todo
                int next_id_pattern = pre_order_patterns.find(curId_inpattern).next;  ///todo
                int old_parent = next_id_pattern.maptoparent; ///map back to the vertex using subtree. can map to parent node first, then from parent node to subtree;
                ///or just extend the defination of subtree to include this; old_parent is a VERTEX
                on_left = false;
                for(int i=0; i<g.degree[old_parent]; i++){
                    int neigh = g.neighbors[g.nodes[old_parent]+i];//neighbor id.
                    unordered_map<int, float>::iterator found = node2layers[next_id_pattern].find(neigh);
                    float edgwgt = calcWgt(g.wgts[g.nodes[next_id_pattern]+i], querytree.time);
                    if( found != node2layers[next_id_pattern].end() && find(subtree.nodes.begin(), subtree.nodes.end(), neigh) == subtree.nodes.end() && edgwgt+curNode.wgt<MAX_WEIGHT){
                        //append this neigh to the subtree at the right place
                        Instance_Tree new_subtree = Instance_Tree_Insert(subtree, curId, neigh, on_left);
                        frontier.push(createPQEntity_AStar_Tree(neigh, edgwgt+curNode.wgt, edgwgt+curNode.wgt+found->second, new_subtree));
                        total += 1;
                    }
                }

            }
            int onlychild;
		    if (querytree.map2leftcdr.find(curId_inpattern)!=querytree.map2leftcdr.end()){ //todo
                onlychild = querytree.map2leftcdr[curId_inpattern];
                on_left = true;
		    }
		    else{  //curId has a right child.
                onlychild = querytree.map2rightcdr[curId_inpattern];
                on_left = false;
		    }
            for(int i=0; i<g.degree[curId]; i++){
                int neigh = g.neighbors[g.nodes[curId]+i];//neighbor id.
                unordered_map<int, float>::iterator found = node2layers[onlychild].find(neigh);
                float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], querytree.time);
                if( found != node2layers[onlychild].end() && find(subtree.nodes.begin(), subtree.nodes.end(), neigh) == subtree.nodes.end() && edgwgt+curNode.wgt<MAX_WEIGHT){
                    //append this neigh to the subtree at the right place
                    Instance_Tree new_subtree = Instance_Tree_Insert(subtree, curId, neigh, on_left);
                    frontier.push(createPQEntity_AStar_Tree(neigh, edgwgt+curNode.wgt, edgwgt+curNode.wgt+found->second, new_subtree));
                    total += 1;
                }
            }
		}
		else{ //curId is a junction, found in left child candidates and right child candidate.
                int leftchild, rightchild;
                leftchild = querytree.map2leftcdr[curId_inpattern];
                rightchild = querytree.map2rightcdr[curId_inpattern];
                ///todo: pushback with left child, update the right.

                for(int i=0; i<g.degree[curId]; i++){
                    int neighleft = g.neighbors[g.nodes[curId]+i];//neighbor id 1.
                    for(int j=0; j<g.degree[curId]; j++){
                        if (j!= i){ //select two seperate neighbors as candidate children.
                            int neighright = g.neighbors[g.nodes[curId]+j];//neighbor id 2.
                            unordered_map<int, float>::iterator foundleft = node2layers[leftchild].find(neighleft);
                            unordered_map<int, float>::iterator foundright = node2layers[rightchild].find(neighright);
                            float edgwgt_left = calcWgt(g.wgts[g.nodes[curId]+i], querytree.time);
                            float edgwgt_right = calcWgt(g.wgts[g.nodes[curId]+j], querytree.time);
                            if( (foundleft != node2layers[leftchild].end() && find(subtree.nodes.begin(), subtree.nodes.end(), neighleft) == subtree.nodes.end())&&
                               (foundleft != node2layers[leftchild].end() && find(subtree.nodes.begin(), subtree.nodes.end(), neighleft) == subtree.nodes.end())
                               //this pair of (left, right) choice is not included in already traversed subtree.
                               && edgwgt_left + edgwgt_right + curNode.wgt < MAX_WEIGHT){
                                //append this left neigh to the subtree at the right place
                                Instance_Tree new_subtree1 = Instance_Tree_Insert(subtree, neighleft, true);
                                Instance_Tree new_subtree = Instance_Tree_Insert(new_subtree1, neighright, false);
                                frontier.push(createPQEntity_AStar_Tree(neigh, edgwgt_left+edgwgt_right+curNode.wgt, edgwgt_left+edgwgt_right+curNode.wgt+foundleft->second+foundright->second, new_subtree));
                                total += 1;
                            }
                        }
                    }
                }
		}
}



QueryResultTrees AStar_Prophet_Tree(const graph_t& g, Query_tree querytree, double& timeUsed){
	int iterationnum = querytree.patterns.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResultTrees qResultTree;

        //check the struct invarient
        for(int i=0; i<querytree.patterns.size(); i++){
            if( (g.typeMap[querytree.nodes_ordered[querytree.terminals_index[i]]]!=querytree.patterns[querytree.terminals_index[i]])){
                //cout << query.src << " to " << query.tgt << endl;
                //cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResultTree;
                }
        }

    vector<GeneralizedQuery> decomposed_queries = decompo_Query_Tree(querytree);
    unordered_map<int, unordered_map<int, float>> node2layers; //candidate set with each's weight from lower level terminals. Will return this as tree prophet graph
    unordered_map<int, int> vertex2node;
    unordered_map<int, float> junction_leftmap;
    unordered_map<int, float> junction_rightmap;

    for (int i=0; i<decomposed_queries.size(); i++){//process by the post-order
        GeneralizedQuery current_query = decomposed_queries[i];
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

        layers = create_Prophet_Heuristics_generalized(g, current_query, timeUsed, current_junction_leftmap, current_junction_rightmap);//layers stores the legitimate nodes on each level. Each layer is a set.
        for (int i=0; i< layers.size(); i++){
            node2layers[current_query.nodes[i]] = layers[i];  //i-th node updated into the candidate set chart. using map--unordered map for now...
            for (int j=0; j<layers[i].size(); j++){
                int vertex = layers[i][j];
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

	std::priority_queue<PQEntity_AStar_Tree, std::vector<PQEntity_AStar_Tree>, comparator_AStar_Tree> frontier;
	PQEntity_AStar_Tree curNode;
	vector<int> tmptree;
	int root = querytree.nodes_ordered.back();
	for(unordered_map<int, float>::iterator it_root=node2layers[root].begin(); it_root!=node2layers[root].end(); it_root++){
        Instance_Tree root;
        root.nodes.insert(it_root->first);
        root.wgt = 0;
        tmptree.push_back(root);
        frontier.push(createPQEntity_AStar_Tree(it_root->first, 0, it_root->second, tmptree));//frontier is the priority queue. replace src with root.
	}
	int mem = 0, total = 1;


    //build a reference to decide which is the next curId_inpattern the neighbor should match to, on which side,
    //b pre-order based on a stack.
    stack <int> s;
    int curId_inpattern = root;
    vector <int> pre_order_patterns;
    while(true){
            while(true){
                pre_order_patterns.push_back(curId_inpattern); //first time at one node.
                cout<<"expended on current pattern is "<< curId_inpattern<< endl;
                s.push(curId_inpattern);
                if (querytree.map2leftcdr.find(curId_inpattern)!= querytree.map2leftcdr.end()){//current ppattern node still have left child;
                        curId_inpattern = querytree.map2leftcdr(curId_inpattern);
                }
                else://have no left child;
                        break;
            }
            if s.empty():
                    break;
            curId_inpattern = s.pop();
            if (querytree.map2rightcdr.find(curId_inpattern)!= querytree.map2rightcdr.end()){
                curId_inpattern = querytree.map2rightcdr(curId_inpattern);
            }
        }
    //now, pre_order_patterns is recording the patterns.
    //for expansion, we need to know who is the next pattern node to match.


	while(!frontier.empty()){//each time check one element in the PQ.
		mem = max(mem, (int)frontier.size());
		PQEntity_AStar_Tree curNode;
		curNode = frontier.top();
		frontier.pop();
		int curId = curNode.nodeIdx;
		int done_countes = curNode.subtree.nodes.size()-1;//start from 0.  done_counts: refer to the old path depth
		Instance_Tree subtree = curNode.subtree;
		if(done_countes==iterationnum){//reach the end of pattern.
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
        Expand_current(g, querytree, pre_order_patterns, curId, curNode, subtree,total,node2layers,vertex2node,frontier);
		}

	qResultTree.mem = mem;
        qResultTree.totalTrees = total;
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
