#include "global.h"
#include "proto.h"
#include "pruned_landmark_labeling.h"
using namespace std;
/*
	This file contains two methods based on DFS. One is plain iterative DFS. Another is DFS with distance oracle. 
*/
//Iterative DFS (variant) implementation. Without distance oracle. Recursive implementation is cleaner but less efficient. 
QueryResult DFM(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle){
	int maxDepth = query.pattern.size()-1;
	float minWgt = MAX_WEIGHT;
	QueryResult qResult;
	if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
		cout << query.src << " to " << query.tgt << endl;
		cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
		cout<< "src or tgt node does not follow pattern!" << endl;
		return qResult;
	}
	priority_queue<Path, std::vector<Path>, comparator2> candidates;
	vector<int> pointers;//pointer to the position visited last.
	for(int i=0; i<query.pattern.size(); i++)
		pointers.push_back(-1);
	vector<int> nodes;//stack for nodes in the path.
	vector<float> wgts;//stack keeping track the weight of path from src to the current node in stack "nodes".
	unordered_set<int> pathSet;
	nodes.push_back(query.src);
	pathSet.insert(query.src);
	pointers[0] = 0;
	wgts.push_back(0);
	int mem = 1, total=1;	
	while(!nodes.empty()){
		int cur = nodes.back();
		float wgt = wgts.back();
		int dep = nodes.size()-1;
		if(dep == maxDepth){//when the the depth of current node reaches the end. 
			if(cur==query.tgt && wgt < minWgt){//simply check whether it is target node.
			   if(candidates.size()==TOP_K)
				candidates.pop();
			   candidates.push(createPath(wgt, nodes));
			   if(candidates.size()==TOP_K)
				   minWgt = candidates.top().wgt;
			}
			nodes.pop_back();wgts.pop_back();
			pathSet.erase(cur);
			continue;
		}
		bool find=false;
		for(int i=pointers[dep+1]+1; i<g.degree[cur] && !find; i++){
			int next = g.neighbors[g.nodes[cur]+i];//id of neighbor node
			float w = calcWgt(g.wgts[g.nodes[cur]+i], query.time);//edge weight between the current node and neighbor node
			//check if the neighbor node matches the pattern and have already in the path.
			if(g.typeMap[next]==query.pattern[dep+1] && pathSet.count(next)==0 && wgt+w < minWgt){
				if(oracle == WITH_ORACLE){
					int tree_dist = pll.QueryDistance(query.tgt, next);//use exact distance oracle to query the shortest distance.
	                                int realDistInPattern = query.pattern.size()-dep-2;
	                                if(tree_dist > realDistInPattern)
	                                    continue;//prune the current neighbor node if it is too far away from the target node.
				}
				total += 1;
				nodes.push_back(next);
				wgts.push_back(wgt+w);
				pathSet.insert(next);
				find = true;
				pointers[dep+1] = i;
			} 
		}
		if(find==false){//did not find the element of next layer. pop out cur. 
			nodes.pop_back();wgts.pop_back();
			pathSet.erase(cur);
			pointers[dep+1] = -1;//set to default. 
	
		}
		
	}
	qResult.paths = pq2vec(candidates);
	qResult.mem = mem;
	qResult.totalPaths = total;
	return qResult;
}

