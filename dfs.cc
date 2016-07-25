#include "global.h"
#include "proto.h"
#include "pruned_landmark_labeling.h"
using namespace std;

/*
	This file contains two methods based on DFS. One is plain iterative DFS. Another is DFS with distance oracle. 
*/
//Iterative DFS (variant) implementation. Without distance oracle. Recursive implementation is cleaner but not less efficient. 
//return the path and also the min-wgt. 

pair<vector<int>, float>  meta_path_iterative_dfs(const graph_t& g, Query query){
	int maxDepth = query.pattern.size()-1;
	float minWgt = MAX_WEIGHT;
	vector<int> result;
	pair<vector<int>, float> pResult;
	if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
		cout << query.src << " to " << query.tgt << endl;
		cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
		cout<< "src or tgt node does not follow pattern!" << endl;
		pResult =  make_pair(result, -1);
		return pResult;
	}
	stack<int> nodes;//stack for nodes that will be visited.
	stack<int> depths;//stack keeping track the depth of nodes in stack "nodes".
	stack<float> wgts;//stack keeping track the weight of path from src to the current node in stack "nodes".
	vector<int> paths;//vector for storing path.
	std::unordered_set<int> pathSet;//same elements with paths, but in hash set for quick query to avoid loops. 
	nodes.push(query.src);
	depths.push(0);
	wgts.push(0);
	int mem = 0;	
	while(!nodes.empty()){
		mem = max(mem, (int)nodes.size());
		int cur = nodes.top();
		nodes.pop();
		int dep = depths.top();
		depths.pop();
//		cout << dep << " "<<g.degree[cur]<< endl;
		float wgt = wgts.top();
		wgts.pop();
		while(paths.size() > dep ){//critical part. Going backward after reaching the end of one path. 
			int ele = paths.back();
			paths.pop_back();//poping out elements from the path.
			pathSet.erase(ele);
		}
		paths.push_back(cur);
		pathSet.insert(cur);
		if(dep == maxDepth){//when the the depth of current node reaches the end. 
			if(cur==query.tgt && wgt < minWgt){//simply check whether it is target node.
			   result = paths;
			   minWgt = wgt;
			}
		}
		else{
			for(int i=0; i<g.degree[cur]; i++){//another step forward from the current node. Expand the current nodes to its neighborhoods. 
				int next = g.neighbors[g.nodes[cur]+i];//id of neighbor node
				float w = calcWgt(g.wgts[g.nodes[cur]+i], query.time);//edge weight between the current node and neighbor node
				//cout<< g.wgts[g.nodes[cur]+i][0] << " "<<g.wgts[g.nodes[cur]+i].back() <<" "<<query.time;
				//cout << w << endl;
				//check if the neighbor node matches the pattern and have already in the path. 
				if(g.typeMap[next]==query.pattern[dep+1] && pathSet.count(next)==0 && wgt+w < minWgt){
					nodes.push(next);
					depths.push(dep+1);
					wgts.push(wgt+w);
					
				}
			}	
		}
		
	}
	result.push_back(mem);
	if(minWgt >= MAX_WEIGHT)
		minWgt = -1;
	return make_pair(result, minWgt);
}
//iterative DFS approach using exact distance oracle. 
pair<vector<int>, float>  meta_path_iterative_dfs_exact_oracle(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query){
	int maxDepth = query.pattern.size()-1;
	float minWgt = MAX_WEIGHT;
	vector<int> result;
	pair<vector<int>, float> pResult;
	if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
		cout<< "src or tgt node does not follow pattern!" << endl;
		pResult =  make_pair(result, -1);
		return pResult;
	}
	stack<int> nodes;
	stack<int> depths;
	stack<float> wgts;
	vector<int> paths;
	std::unordered_set<int> pathSet;
	nodes.push(query.src);
	depths.push(0);
	wgts.push(0);
	int mem = 0;
	while(!nodes.empty()){
		mem = max(mem, (int)nodes.size());
		int cur = nodes.top();
		nodes.pop();
		int dep = depths.top();
		depths.pop();
		float wgt = wgts.top();
		wgts.pop();
		while(paths.size() > dep ){//critical part.
			int ele = paths.back();
			paths.pop_back();
			pathSet.erase(ele);
		}
		paths.push_back(cur);
		pathSet.insert(cur);
		if(dep == maxDepth ){
			if(cur==query.tgt && wgt < minWgt){
			   result = paths;
			   minWgt = wgt;
			}
		}
		else{
			for(int i=0; i<g.degree[cur]; i++){
				int next = g.neighbors[g.nodes[cur]+i];
				float w = calcWgt(g.wgts[g.nodes[cur]+i], query.time);
				if(g.typeMap[next]==query.pattern[dep+1] && pathSet.count(next)==0 && wgt+w<minWgt){
					int tree_dist = pll.QueryDistance(query.tgt, next);//use exact distance oracle to query the shortest distance.
                                        int realDistInPattern = query.pattern.size()-dep-2;
                                        if(tree_dist > realDistInPattern) 
                                                continue;//prune the current neighbor node if it is too far away from the target node. 
					nodes.push(next);
					depths.push(dep+1);
					wgts.push(wgt+w);
					
				}
			}	
		}
		
	}
	result.push_back(mem);
	if(minWgt >= MAX_WEIGHT)
		minWgt = -1;
	return make_pair(result, minWgt);
}

