#include "global.h"
#include "proto.h"
#include "pruned_landmark_labeling.h"
using namespace std;

//variant of dijkstra algorithm
//return the ids in the path and the lightest weight of path. The order of ids in the path might not be correct because of set. Will change the data structure if needed.  
//variant of Dijkstra's approach with exact distance oracle. 
QueryResult dijkstra(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle){
	int maxDepth = query.pattern.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResult qResult;
        if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
                cout << query.src << " to " << query.tgt << endl;
                cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResult;
        }
	std::priority_queue<Path, std::vector<Path>, comparator> frontier;
	vector<int> tmpPath;
	tmpPath.push_back(query.src);
	frontier.push(createPath(0, tmpPath));//frontier is the priority queue. 
	int mem = 1, total=1;
	while(!frontier.empty()){
		mem = max(mem, (int)frontier.size());
		Path curNode;
		curNode = frontier.top();
		frontier.pop();
		int curId = curNode.nodeIds.back();
		int depth = curNode.nodeIds.size()-1;//start from 0. 
		vector<int> path = curNode.nodeIds;
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
			float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], query.time);
			//if neighbor is not in the path and follow the pattern.
			if(g.typeMap[neigh] == query.pattern[depth+1] && find(path.begin(), path.end(), neigh) == path.end() && edgwgt+curNode.wgt<MAX_WEIGHT){
				if(oracle == WITH_ORACLE){
				  int tree_dist = pll.QueryDistance(query.tgt, neigh);//use exact distance oracle.
                                  int realDistInPattern = query.pattern.size()-depth-2;
                                  if(tree_dist > realDistInPattern)
                                        continue;
				}
				vector<int> newPath = path;
				newPath.push_back(neigh);
				frontier.push(createPath(edgwgt+curNode.wgt,newPath));
				total += 1;
			}
		}
		
	}
	qResult.mem = mem;
	qResult.totalPaths = total;
	return qResult;
}


QueryResult dijkstra_with_Prophet(const graph_t& g, Query query){
	assert(query.pattern.size()>2);//otherwise the problem becomes trivial.
	int maxDepth = query.pattern.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResult qResult;
        if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
                cout << query.src << " to " << query.tgt << endl;
                cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResult;
        }
	//layers stores the legitimate nodes on each level. Each layer is a set.
	double tp;
	vector<unordered_set<int> > layers = create_Prophet(g, query, tp);

	std::priority_queue<Path, std::vector<Path>, comparator> frontier;
	vector<int> tmpPath;
	tmpPath.push_back(query.src);
	frontier.push(createPath(0, tmpPath));//frontier is the priority queue.
	int mem = 1, total=1; 
	while(!frontier.empty()){
		mem = max(mem, (int)frontier.size());
		Path curNode;
		curNode = frontier.top();
		frontier.pop();
		int curId = curNode.nodeIds.back();
		int depth = curNode.nodeIds.size()-1;//start from 0. 
		vector<int> path = curNode.nodeIds;
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
			float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], query.time);
			//if neighbor is not in the path and follow the pattern.
			if( layers[depth+1].count(neigh)>0 && find(path.begin(), path.end(), neigh) == path.end() && edgwgt+curNode.wgt<MAX_WEIGHT){
				vector<int> newPath = path;
				newPath.push_back(neigh);
				frontier.push(createPath(edgwgt+curNode.wgt, newPath));
				total += 1;
//				layers[depth+1].at(neigh) = -layers[depth+1].at(neigh); //Note: important. Avoid repeated visit to the node in the same layer.
				//a few more words on this. It seems we should not do this. Because the possibility of loops.  
			}
		}
	}
	qResult.mem = mem;
        qResult.totalPaths = total;
        return qResult;
}

//The implementation of this one is not completed. 
//But this is not going to work properly since it cannot deal with the top-K case.
QueryResult bidir_Dijkstra(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle){
	int maxDepth = query.pattern.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResult qResult;
        if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
                cout << query.src << " to " << query.tgt << endl;
                cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResult;
        }
	std::priority_queue<Path, std::vector<Path>, comparator> forward;
	std::priority_queue<Path, std::vector<Path>, comparator> backward;
	unordered_map<int, Path> forwardVisited;
	unordered_map<int, Path> backwardVisited;
	
	priority_queue<Path, std::vector<Path>, comparator2> candidates;
	vector<int> tmpPath;
	tmpPath.push_back(query.src);
	forward.push(createPath(0, tmpPath));//frontier is the priority queue. 
	tmpPath.pop_back();
	tmpPath.push_back(query.tgt);
	backward.push(createPath(0, tmpPath));
	int mem = 1, total=2, flag = 0;
	while(!forward.empty() || !backward.empty()){
		mem = max(mem, (int)(forward.size()+backward.size()));
		flag = 1-flag;
		Path curNode;
		if(flag == 1){
			curNode = forward.top();
			forward.pop();
			int curId = curNode.nodeIds.back();
			int depth = curNode.nodeIds.size()-1;//start from 0. 
			vector<int> path = curNode.nodeIds;
			forwardVisited[curId] = createPath(curNode.wgt, path);
			if(depth==maxDepth){//reach the end of pattern. 
				continue;
			}
			if(backwardVisited.count(curId)>0){
				int newWgt= curNode.wgt + backwardVisited[curId].wgt;
				path.pop_back();
				for(vector<int>::reverse_iterator rit=backwardVisited[curId].nodeIds.rbegin(); rit!=backwardVisited[curId].nodeIds.rend(); rit++){
					path.push_back(*rit);
				}
				if(path.size()==query.pattern.size())
					candidates.push(createPath(newWgt,path));
				break;}
			//expanding the neighbors of current node. 
			for(int i=0; i<g.degree[curId]; i++){
				int neigh = g.neighbors[g.nodes[curId]+i];//neighbor id. 
				float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], query.time);
				//if neighbor is not in the path and follow the pattern.
				if(g.typeMap[neigh] == query.pattern[depth+1] && forwardVisited.count(neigh)==0
					 && edgwgt+curNode.wgt<MAX_WEIGHT){
					if(oracle == WITH_ORACLE){
					  int tree_dist = pll.QueryDistance(query.tgt, neigh);//use exact distance oracle.
	                                  int realDistInPattern = query.pattern.size()-depth-2;
	                                  if(tree_dist > realDistInPattern)
	                                        continue;
					}
					vector<int> newPath = path;
					newPath.push_back(neigh);
					forward.push(createPath(edgwgt+curNode.wgt,newPath));
					total += 1;
				}
			}
		}else{
			curNode = backward.top();
			backward.pop();
			int curId = curNode.nodeIds.back();
			int depth = curNode.nodeIds.size()-1;//start from 0. 
			vector<int> path = curNode.nodeIds;
			backwardVisited[curId] = createPath(curNode.wgt, path);
			if(depth==maxDepth){//reach the end of pattern. 
				continue;
			}
			if(forwardVisited.count(curId)>0){
				int newWgt = curNode.wgt + forwardVisited[curId].wgt;
				vector<int> candidatePath = forwardVisited[curId].nodeIds;
				candidatePath.pop_back();
				for(vector<int>::reverse_iterator rit=path.rbegin(); rit!=path.rend(); rit++){
					candidatePath.push_back(*rit);
				}
				if(candidatePath.size() == query.pattern.size())
					candidates.push(createPath(newWgt, candidatePath));
				break;}
			//expanding the neighbors of current node. 
			for(int i=0; i<g.degree[curId]; i++){
				int neigh = g.neighbors[g.nodes[curId]+i];//neighbor id. 
				float edgwgt = calcWgt(g.wgts[g.nodes[curId]+i], query.time);
				//if neighbor is not in the path and follow the pattern.
				if(g.typeMap[neigh] == query.pattern[depth+1] && backwardVisited.count(neigh)==0
					 && edgwgt+curNode.wgt<MAX_WEIGHT){
					if(oracle == WITH_ORACLE){
					  int tree_dist = pll.QueryDistance(query.src, neigh);//use exact distance oracle.
	                                  int realDistInPattern = query.pattern.size()-depth-2;
	                                  if(tree_dist > realDistInPattern)
	                                        continue;
					}
					vector<int> newPath = path;
					newPath.push_back(neigh);
					backward.push(createPath(edgwgt+curNode.wgt,newPath));
					total += 1;
				}
			}
		}
		
	}
	if(!forward.empty() || !backward.empty()){
		for(auto it=forwardVisited.begin(); it!=forwardVisited.end(); it++){
			int curId = it->first; Path tmpPath = it->second; float wgt = tmpPath.wgt; int len = tmpPath.nodeIds.size();
			for(int i=0; wgt<minWgt && i<g.degree[curId]; i++){
				int neigh = g.neighbors[g.nodes[curId]+i];
				float edgeWgt = calcWgt(g.wgts[g.nodes[curId]+i], query.time);
				if(backwardVisited.count(neigh)>0 && wgt+edgeWgt+backwardVisited[neigh].wgt<minWgt && 
					len+backwardVisited.at(neigh).nodeIds.size()==query.pattern.size()){
//					cout << len <<" "<<backwardVisited.at(neigh).nodeIds.size()<<" == "<<query.pattern.size() << " @@ "<<neigh<<"   ##  ";
					vector<int> candidatePath = tmpPath.nodeIds;
					int newWgt = wgt+edgeWgt+backwardVisited[neigh].wgt;
					if(candidates.size()==TOP_K)
						candidates.pop();
					for(vector<int>::reverse_iterator rit=backwardVisited[neigh].nodeIds.rbegin(); rit!=backwardVisited[neigh].nodeIds.rend(); rit++){
//						cout << " " << *rit;
						candidatePath.push_back(*rit);
					}
//					cout << endl;
					candidates.push(createPath(newWgt,candidatePath));
					if(candidates.size()==TOP_K)
						minWgt = candidates.top().wgt;
					total += 1;
				}
			}
		}
	}
	qResult.paths = pq2vec(candidates);
	qResult.mem = mem;
	qResult.totalPaths = total;
	return qResult;
} 

