#include "global.h"
#include "proto.h"
#include "pruned_landmark_labeling.h"
using namespace std;

/*
This file defines the following functions. 
QueryResult BFM(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle);
QueryResult bidirectional_BFM(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle);
QueryResult bidirectional_prune_bfs(const graph_t& g, Query query);//implicit prophet graph
QueryResult bidirectional_prune_bfs_prophet(const graph_t& g, Query query);//explicit prophet graph
*/


//Iterative BFM and BFM with exact distance oracle.
QueryResult BFM(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle){
	int maxDepth = query.pattern.size()-1;
        float minWgt = MAX_WEIGHT;
        QueryResult qResult;
        if( (g.typeMap[query.src]!=query.pattern[0]) || (g.typeMap[query.tgt]!=query.pattern[maxDepth]) ){
                cout << query.src << " to " << query.tgt << endl;
                cout << g.typeMap[query.src] << " and " << g.typeMap[query.tgt] << endl;
                cout<< "src or tgt node does not follow pattern!" << endl;
                return qResult;
        }
	vector<vector<int> > frontier;//frontier of BFS. keep track of the path from src to the current node.
	vector<float> frtwgt; //weight associated with paths in frontier. 
	vector<int> tmpPath;
	frtwgt.push_back(0);
	tmpPath.push_back(query.src);
	frontier.push_back(tmpPath);
	int numPaths = 0;
	int mem = 1, total=1;
	priority_queue<Path, std::vector<Path>, comparator2> candidates;
	for(int i=1; i<query.pattern.size(); i++){//expand by one layer forward. 
		mem = max(mem, (int)frontier.size());
		vector<vector<int> > tmpFrt;
		vector<float> tmpwgt;
		int count = 0;//for the convinience of frtwgt.
		for(vector<vector<int> >::iterator it=frontier.begin(); it!=frontier.end(); it++){//for each node on the frontier.
			vector<int> path = *it;
			float prewgt = frtwgt[count];//weight associtated with path.
			int curNode = path.back();
			for(int j=0; j<g.degree[curNode]; j++){//expand the neighbors of the frontier nodes. 
                                int neigh = g.neighbors[g.nodes[curNode]+j];
				if(find(path.begin(), path.end(), neigh)!=path.end())
					continue;
				float wgt = calcWgt(g.wgts[g.nodes[curNode]+j], query.time);
				if(i==query.pattern.size()-1){//for last frontier.
					if(neigh == query.tgt && prewgt+wgt<MAX_WEIGHT){
						numPaths += 1;
						total += 1;//not very correct. DFM is more accurate. 
						if(prewgt+wgt < minWgt){
							if(candidates.size()==TOP_K)
				                                candidates.pop();
							path.push_back(neigh);
				                        candidates.push(createPath(prewgt+wgt, path));
                           				minWgt = candidates.top().wgt;
							path.pop_back();}
					}
				}
				else if(g.typeMap[neigh]==query.pattern[i] && prewgt+wgt<MAX_WEIGHT){//match the patterni & no loop.
					if(oracle == WITH_ORACLE){
  					  int tree_dist = pll.QueryDistance(query.tgt, neigh);//use exact distance oracle .
	                               	  int realDistInPattern = query.pattern.size()-i-1;
 	                                  if(tree_dist > realDistInPattern || find(path.begin(), path.end(), neigh) != path.end())
         	                               continue;
					}
					path.push_back(neigh);
					tmpFrt.push_back(path);
					tmpwgt.push_back(prewgt + wgt);
					total += 1;
					path.pop_back();
				}	
			}
			count += 1;
		}
		frontier = tmpFrt;//generate new frontier. 
		frtwgt = tmpwgt;
	}
	qResult.paths = pq2vec(candidates);
	qResult.numPaths = numPaths;
	qResult.mem = mem; 
        qResult.totalPaths = total;                           
        return qResult;
}

//Bidirectional BFM and with/without distance oracle. 
QueryResult bidirectional_BFM(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle){
	//please use the distance oracle here to quickly check two nodes are impossible to have paths following the pattern.
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
	int mem=1, total=2;
	priority_queue<Path, std::vector<Path>, comparator2> candidates; 
	int midLayer = query.pattern.size()/2 +1; 
	vector<vector<int> > forward;//forward frontier
	vector<float> fwgt;
	vector<vector<int> > backward;//backward frontier
	vector<float> bwgt;
	vector<int> tmpPath;
	fwgt.push_back(0);
	bwgt.push_back(0);
	tmpPath.push_back(query.src);
	forward.push_back(tmpPath);
	tmpPath.clear();
	tmpPath.push_back(query.tgt);
	backward.push_back(tmpPath);
	int mx = 0;
	for(int i=1; i<midLayer; i++){
		mx = max(mx, (int)forward.size());
		vector<vector<int> > tmpForward;
		vector<float> tmpwgt;
		int count = 0;
		for(vector<vector<int> >::iterator it=forward.begin(); it!=forward.end(); it++){
			vector<int> path = *it;
			float prewgt = fwgt[count];
			int curNode = path.back();
			for(int j=0; j<g.degree[curNode]; j++){
                                int neigh = g.neighbors[g.nodes[curNode]+j];
                                float wgt = calcWgt(g.wgts[g.nodes[curNode]+j], query.time);
				if(g.typeMap[neigh]==query.pattern[i] && prewgt+wgt<MAX_WEIGHT){
                                        if(find(path.begin(), path.end(), neigh) != path.end())
                                                continue;
					if(oracle == WITH_ORACLE){
					  int tree_dist = pll.QueryDistance(query.tgt, neigh);//use exact distance oracle .
                                	  int realDistInPattern = query.pattern.size()-i-1;
 	                                  if(tree_dist > realDistInPattern)
         	                               continue;
					}
					path.push_back(neigh);
					tmpForward.push_back(path);
					tmpwgt.push_back(prewgt + wgt);
					path.pop_back();
					total += 1;
				}	
			}
			count += 1;
		}
		forward = tmpForward;
		fwgt = tmpwgt;
	}//meet at midLayer-1 layer.
	for(int i=query.pattern.size()-2; i>=midLayer-1; i--){
		mem = max(mem, (int)backward.size());
		vector<vector<int> > tmpBackward;
		vector<float> tmpwgt;
		int count = 0;
		for(vector<vector<int> >::iterator it=backward.begin(); it!=backward.end(); it++){
			vector<int> path = *it;
			float prewgt = bwgt[count]; 
			int curNode = path.back();
			for(int j=0; j<g.degree[curNode]; j++){
                                int neigh = g.neighbors[g.nodes[curNode]+j];
                                float wgt = calcWgt(g.wgts[g.nodes[curNode]+j], query.time);
				if(g.typeMap[neigh] == query.pattern[i] && prewgt+wgt<MAX_WEIGHT){//node pattern matching not necessary. But edge pattern needed if there is. 
                                        if(find(path.begin(), path.end(), neigh) != path.end())
                                                continue;
				 	if(oracle == WITH_ORACLE){
					  int tree_dist = pll.QueryDistance(query.src, neigh);//use exact distance oracle .
                                          int realDistInPattern = i;
                                          if(tree_dist > realDistInPattern)
                                               continue;
					}
					path.push_back(neigh);
					tmpBackward.push_back(path);
					tmpwgt.push_back(prewgt+wgt);
					path.pop_back();
					total += 1;
				}	
			}
			count += 1;	
		}
		backward = tmpBackward;
		bwgt = tmpwgt;
	}
	mem = max(mx, (int)forward.size()+mem);//either reach max in forward or backward. 
	unordered_map<int, vector<vector<int> > > forwardMap;	
	unordered_map<int, vector<float> > fwgtMap;	
	unordered_map<int, vector<vector<int> > > backwardMap;
	unordered_map<int, vector<float> > bwgtMap;
	vector<int> keyVec;
	for(int i=0; i<forward.size(); i++){
		int key = forward[i].back();
		if(forwardMap.find(key) == forwardMap.end()){
			keyVec.push_back(key);
			vector<vector<int> > tmpVec;
			tmpVec.push_back(forward[i]);
			forwardMap[key] = tmpVec;
			vector<float> wgtVec;
			wgtVec.push_back(fwgt[i]);
			fwgtMap[key] = wgtVec;
		}
		else{
			forwardMap.find(key)->second.push_back(forward[i]);
			fwgtMap.find(key)->second.push_back(fwgt[i]);
		}
		
	}	
	for(int i=0; i<backward.size(); i++){
		int key = backward[i].back();
		if(backwardMap.find(key) == backwardMap.end()){
			vector<vector<int> > tmpVec;
			tmpVec.push_back(backward[i]);
			backwardMap[key] = tmpVec;
			vector<float> wgtVec;
			wgtVec.push_back(bwgt[i]);
			bwgtMap[key] = wgtVec;
		}
		else{
			backwardMap.find(key)->second.push_back(backward[i]);
			bwgtMap.find(key)->second.push_back(bwgt[i]);
		}
		
	}
	int numPaths = 0;
	for(int i=0; i<keyVec.size(); i++){
		unordered_map<int, vector<vector<int> > >::iterator it = backwardMap.find(keyVec[i]);
		if(it == backwardMap.end())
			continue;
		vector<vector<int> > forwardPaths = forwardMap.find(keyVec[i])->second;
		vector<vector<int> > backwardPaths = it->second;
		vector<float> wgt1 = fwgtMap.find(keyVec[i])->second;
		vector<float> wgt2 = bwgtMap.find(keyVec[i])->second;
		for(int j=0; j<forwardPaths.size(); j++){
			for(int k=0; k<backwardPaths.size(); k++){
				if(!contain_loop(forwardPaths[j], backwardPaths[k])){
					numPaths += 1;
					if(wgt1[j]+wgt2[k] < minWgt){
						if(candidates.size()==TOP_K)
			                                candidates.pop();
						vector<int> result = forwardPaths[j];
						for(int t=backwardPaths[k].size()-2; t>=0; t--)
							result.push_back(backwardPaths[k][t]);
                        			candidates.push(createPath(wgt1[j]+wgt2[k], result));
                           			minWgt = candidates.top().wgt;
					}	
				}
			}
		}
		
	}
	qResult.paths = pq2vec(candidates);
	qResult.numPaths = numPaths;
        qResult.mem = mem;
        qResult.totalPaths = total;
        return qResult;
}

/**********************************************************/
//Use bidirectional BFS first to prune unrelated nodes and edges. Then use bidirectional BFS to search for the paths. 
//Implicit Prophet Graph. 
QueryResult bidirectional_BFM_Prophet_IM(const graph_t& g, Query query, double& timeUsed){
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
	int midLayer = query.pattern.size()/2 +1;//half forward and halp backward. And they meet and terminate at the same layer. 
	vector<unordered_set<int> > layers = create_Prophet(g, query, timeUsed);//layers stores the legitimate nodes on each level. Each layer is a set. 

	vector<vector<int> > forward;//for forward BFS
	vector<float> fwgt;//weights of forward paths.
	vector<vector<int> > backward;//for backward DFS
	vector<float> bwgt;//weights of backward paths. 
	vector<int> tmpPath;
	fwgt.push_back(0);
	bwgt.push_back(0);
	tmpPath.push_back(query.src);
	forward.push_back(tmpPath);
	tmpPath.clear();
	tmpPath.push_back(query.tgt);
	backward.push_back(tmpPath);
	int mem = 1, total = 2;
	int mx = 0;
	priority_queue<Path, std::vector<Path>, comparator2> candidates;
	for(int i=1; i<midLayer; i++){
		mx = max(mx, (int)forward.size());
		vector<vector<int> > tmpForward;
		vector<float> tmpwgt;
		int count = 0;
		for(vector<vector<int> >::iterator it=forward.begin(); it!=forward.end(); it++){
			vector<int> path = *it;
			float prewgt = fwgt[count];
			int curNode = path.back();
			for(int j=0; j<g.degree[curNode]; j++){
                                int neigh = g.neighbors[g.nodes[curNode]+j];
                                float wgt = calcWgt(g.wgts[g.nodes[curNode]+j], query.time);
				//node pattern matching not necessary. But edge pattern needed if it exists.
				if(layers[i].count(neigh) == 1 && find(path.begin(), path.end(), neigh) == path.end() &&prewgt+wgt<MAX_WEIGHT){
					path.push_back(neigh);
					tmpForward.push_back(path);
					tmpwgt.push_back(prewgt + wgt);
					path.pop_back();
					total += 1;
				}	
			}
			count += 1;
		}
		forward = tmpForward;
		fwgt = tmpwgt;
	}//meet at midLayer-1 layer. 
	for(int i=query.pattern.size()-2; i>=midLayer-1; i--){
		mem = max((int)backward.size(), mem);
		vector<vector<int> > tmpBackward;
		vector<float> tmpwgt;
		int count = 0;
		for(vector<vector<int> >::iterator it=backward.begin(); it!=backward.end(); it++){
			vector<int> path = *it;
			float prewgt = bwgt[count]; 
			int curNode = path.back();
			for(int j=0; j<g.degree[curNode]; j++){
                                int neigh = g.neighbors[g.nodes[curNode]+j];
                                float wgt = calcWgt(g.wgts[g.nodes[curNode]+j], query.time);
				//node pattern matching not necessary. But edge pattern needed if there is. 
				if(layers[i].count(neigh) == 1 && find(path.begin(), path.end(), neigh) == path.end() && prewgt+wgt<MAX_WEIGHT){
					path.push_back(neigh);
					tmpBackward.push_back(path);
					tmpwgt.push_back(prewgt+wgt);
					path.pop_back();
					total += 1;
				}	
			}
			count += 1;	
		}
		backward = tmpBackward;
		bwgt = tmpwgt;
	}
	mem = max(mx, (int)forward.size()+mem);
	//the next thing is to concatenate the forward and backward path.
	//to alleviate computation, store the forward and backward paths in hashmap, using the last node in the path as the key. 
	//then the paths can be easily group by the common intermediate node. 
	unordered_map<int, vector<vector<int> > > forwardMap;	
	unordered_map<int, vector<float> > fwgtMap;	
	unordered_map<int, vector<vector<int> > > backwardMap;
	unordered_map<int, vector<float> > bwgtMap;
	vector<int> keyVec;//store all the possible itermediate nodes that forward/backward meet on. 
	for(int i=0; i<forward.size(); i++){
		int key = forward[i].back(); 
		if(forwardMap.find(key) == forwardMap.end()){
			keyVec.push_back(key);
			vector<vector<int> > tmpVec;
			tmpVec.push_back(forward[i]);
			forwardMap[key] = tmpVec;
			vector<float> wgtVec;
			wgtVec.push_back(fwgt[i]);
			fwgtMap[key] = wgtVec;
		}
		else{
			forwardMap.find(key)->second.push_back(forward[i]);
			fwgtMap.find(key)->second.push_back(fwgt[i]);
		}
		
	}	
	for(int i=0; i<backward.size(); i++){
		int key = backward[i].back();
		if(backwardMap.find(key) == backwardMap.end()){
			vector<vector<int> > tmpVec;
			tmpVec.push_back(backward[i]);
			backwardMap[key] = tmpVec;
			vector<float> wgtVec;
			wgtVec.push_back(bwgt[i]);
			bwgtMap[key] = wgtVec;
		}
		else{
			backwardMap.find(key)->second.push_back(backward[i]);
			bwgtMap.find(key)->second.push_back(bwgt[i]);
		}
		
	}
	int maxProd = 0;
	int numPaths = 0;
	for(int i=0; i<keyVec.size(); i++){//this loop is for concatenation. 
		unordered_map<int, vector<vector<int> > >::iterator it = backwardMap.find(keyVec[i]);//keyVec is from forwardMap.
		if(it == backwardMap.end())//Check if backwardMap has the node.
			continue;
		//now find out one common node in the intermediate layer. 
		vector<vector<int> > forwardPaths = forwardMap.find(keyVec[i])->second;
		vector<vector<int> > backwardPaths = it->second; 
	
		maxProd = max(maxProd, (int)(forwardPaths.size()* (backwardPaths.size())));

		vector<float> wgt1 = fwgtMap.find(keyVec[i])->second;
		vector<float> wgt2 = bwgtMap.find(keyVec[i])->second;
		for(int j=0; j<forwardPaths.size(); j++){
			for(int k=0; k<backwardPaths.size(); k++){
				if(!contain_loop(forwardPaths[j], backwardPaths[k])){//check if there exists loops
					numPaths += 1;
					if(wgt1[j]+wgt2[k] < minWgt){
						if(candidates.size()==TOP_K)
                                                        candidates.pop();
                                                vector<int> result = forwardPaths[j];
                                                for(int t=backwardPaths[k].size()-2; t>=0; t--)
                                                        result.push_back(backwardPaths[k][t]);
                                                candidates.push(createPath(wgt1[j]+wgt2[k], result));
                                                minWgt = candidates.top().wgt;
					}	
				}
			}
		}
		
	}
	qResult.paths = pq2vec(candidates);
	qResult.numPaths = numPaths;
        qResult.mem = mem;
        qResult.totalPaths = total;
        return qResult;
}

//Use bidirectional BFS first to prune unrelated nodes and edges. Then use bidirectional BFS to search for the paths. 
//Explicit Prophet Graph. 
QueryResult bidirectional_BFM_Prophet_EX(const graph_t& g, Query query){
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
	int midLayer = query.pattern.size()/2 +1;//half forward and halp backward. And they meet and terminate at the same layer. 
	vector<unordered_set<int> > layers;//layers stores the legitimate nodes on each level. Each layer is a set. 
	layers.reserve(query.pattern.size());
	unordered_set<int> top;
	top.insert(query.src);
	layers.push_back(top);
	for(int i=1; i<midLayer; i++){
		unordered_set<int> lastLayer = layers[i-1];//upper layer.
		unordered_set<int> newLayer;
		for(unordered_set<int>::iterator it=lastLayer.begin(); it!=lastLayer.end(); it++){//expanding each node on last layer. 
			int node = *it;
			for(int j=0; j<g.degree[node]; j++){//visit the neighbors of each node. 
				int neigh = g.neighbors[g.nodes[node]+j];
				if(g.typeMap[neigh] == query.pattern[i])
					newLayer.insert(neigh);
			}
		}
		layers.push_back(newLayer);	
	}//meet at midLayer-1
	unordered_set<int> bottom;
	for(int i=midLayer; i<query.pattern.size()-1; i++)
		layers.push_back(bottom);//empty.
	bottom.insert(query.tgt);	
	layers.push_back(bottom);
	vector<unordered_map<int, ProphetEntry> > prophetGraph;
	for(int i=0; i<=maxDepth; i++){
		unordered_map<int, ProphetEntry> oneLayer;
		prophetGraph.push_back(oneLayer);
	}
	for(int i=query.pattern.size()-2; i>=midLayer-1; i--){//going up
		unordered_set<int> belowLayer = layers[i+1];
		unordered_set<int> newLayer;
		for(unordered_set<int>::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
			int node = *it;
			for(int j=0; j<g.degree[node]; j++){
				int neigh = g.neighbors[g.nodes[node]+j];
				if(g.typeMap[neigh]==query.pattern[i]) 
					newLayer.insert(neigh);
			}
		}
		if(i == midLayer-1){//i==midLayer-1
			unordered_set<int> tmpSet;
			for(unordered_set<int>::iterator it=newLayer.begin(); it!=newLayer.end(); it++){
				int node = *it;
				if(layers[i].count(node) > 0)
					tmpSet.insert(node);
			}
			layers[i] = tmpSet;
		}
		else
			layers[i] = newLayer;
			
	}
	for(unordered_set<int>::iterator it=layers[midLayer-1].begin(); it!=layers[midLayer-1].end(); it++){
		int node = *it;
		ProphetEntry tmpEntry;
                prophetGraph[midLayer-1][node] = tmpEntry;
	}
	for(int i=midLayer; i<=query.pattern.size()-1; i++){//going down
                unordered_set<int> lastLayer = layers[i-1];//upper layer.
                unordered_set<int> newLayer;
                for(unordered_set<int>::iterator it=lastLayer.begin(); it!=lastLayer.end(); it++){//expanding each node on last layer. 
                        int node = *it;
                        for(int j=0; j<g.degree[node]; j++){//visit the neighbors of each node. 
                                int neigh = g.neighbors[g.nodes[node]+j];
                                if(layers[i].count(neigh)>0){//Note: pattern checking not needed. g.typeMap[neigh] == query.pattern[i] && 
                               		float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
                                        newLayer.insert(neigh);
					prophetGraph[i-1].at(node).downwards.push_back(make_pair(neigh,wgt));
					if(prophetGraph[i].count(neigh) == 0){
						ProphetEntry tmpEntry;
						prophetGraph[i][neigh] = tmpEntry;
					}
					prophetGraph[i].at(neigh).upwards.push_back(make_pair(node,wgt));
				}
                        }
                }
                layers[i] = newLayer;
        }
	for(int i=midLayer-2; i>=0; i--){//going up
                unordered_set<int> belowLayer = layers[i+1];
                unordered_set<int> newLayer;
                for(unordered_set<int>::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
                        int node = *it;
                        for(int j=0; j<g.degree[node]; j++){
                                int neigh = g.neighbors[g.nodes[node]+j];
                                if(layers[i].count(neigh)>0){
                               		float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
                                        newLayer.insert(neigh);
					prophetGraph[i+1].at(node).upwards.push_back(make_pair(neigh,wgt));
					if(prophetGraph[i].count(neigh)==0){
						ProphetEntry tmpEntry;
						prophetGraph[i][neigh] = tmpEntry;
					}
					prophetGraph[i].at(neigh).downwards.push_back(make_pair(node,wgt));
				}
                        }
                }
		layers[i] = newLayer;
	}
	//20737 [16] 37694 [2] 4 [2] 36298 [2] 9753 [3] 35330 [1] 110 [5] 25879 [1] 17919

	vector<vector<int> > forward;//for forward BFS
	vector<float> fwgt;//weights of forward paths.
	vector<vector<int> > backward;//for backward DFS
	vector<float> bwgt;//weights of backward paths. 
	vector<int> tmpPath;
	fwgt.push_back(0);
	bwgt.push_back(0);
	tmpPath.push_back(query.src);
	forward.push_back(tmpPath);
	tmpPath.clear();
	tmpPath.push_back(query.tgt);
	backward.push_back(tmpPath);
	priority_queue<Path, std::vector<Path>, comparator2> candidates;
	int mem=1, total=2;
	int mx=0;
	for(int i=1; i<midLayer; i++){//going down
		mx = max(mx, (int)forward.size());
		vector<vector<int> > tmpForward;
		vector<float> tmpwgt;
		int count = 0;
		for(vector<vector<int> >::iterator it=forward.begin(); it!=forward.end(); it++){
			vector<int> path = *it;
			float prewgt = fwgt[count];
			int curNode = path.back();
			vector<pair<int,float> > neighVec = prophetGraph[i-1][curNode].downwards;
			for(int j=0; j<neighVec.size(); j++){
                                int neigh = neighVec[j].first;
                        	float wgt = neighVec[j].second;
				//node pattern matching not necessary. But edge pattern needed if it exists.
				if(find(path.begin(),path.end(),neigh) == path.end() && prewgt+wgt<MAX_WEIGHT){
					path.push_back(neigh);
					tmpForward.push_back(path);
					tmpwgt.push_back(prewgt + wgt);
					path.pop_back();
					total += 1;
				}
			}
			count += 1;
		}
		forward = tmpForward;
		fwgt = tmpwgt;
	}//meet at midLayer-1 layer. 
	for(int i=query.pattern.size()-2; i>=midLayer-1; i--){//going up
		mem = max((int)backward.size(), mem);
		vector<vector<int> > tmpBackward;
		vector<float> tmpwgt;
		int count = 0;
		for(vector<vector<int> >::iterator it=backward.begin(); it!=backward.end(); it++){
			vector<int> path = *it;
			float prewgt = bwgt[count]; 
			int curNode = path.back();
			vector<pair<int,float> > neighVec = prophetGraph[i+1][curNode].upwards;
			for(int j=0; j<neighVec.size(); j++){
                                int neigh = neighVec[j].first;
                	        float wgt = neighVec[j].second;
				//node pattern matching not necessary. But edge pattern needed if there is. 
				if(find(path.begin(),path.end(),neigh) == path.end() && prewgt+wgt<MAX_WEIGHT){
					path.push_back(neigh);
					tmpBackward.push_back(path);
					tmpwgt.push_back(prewgt+wgt);
					path.pop_back();
					total += 1;
				}
			}
			count += 1;	
		}
		backward = tmpBackward;
		bwgt = tmpwgt;
	}
	mem = max(mx, (int)forward.size()+mem);
	//the next thing is to concatenate the forward and backward path.
	//to alleviate computation, store the forward and backward paths in hashmap, using the last node in the path as the key. 
	//then the paths can be easily group by the common intermediate node. 
	unordered_map<int, vector<vector<int> > > forwardMap;	
	unordered_map<int, vector<float> > fwgtMap;	
	unordered_map<int, vector<vector<int> > > backwardMap;
	unordered_map<int, vector<float> > bwgtMap;
	vector<int> keyVec;//store all the possible itermediate nodes that forward/backward meet on. 
	for(int i=0; i<forward.size(); i++){
		int key = forward[i].back(); 
		if(forwardMap.find(key) == forwardMap.end()){
			keyVec.push_back(key);
			vector<vector<int> > tmpVec;
			tmpVec.push_back(forward[i]);
			forwardMap[key] = tmpVec;
			vector<float> wgtVec;
			wgtVec.push_back(fwgt[i]);
			fwgtMap[key] = wgtVec;
		}
		else{
			forwardMap.find(key)->second.push_back(forward[i]);
			fwgtMap.find(key)->second.push_back(fwgt[i]);
		}
		
	}	
	for(int i=0; i<backward.size(); i++){
		int key = backward[i].back();
		if(backwardMap.find(key) == backwardMap.end()){
			vector<vector<int> > tmpVec;
			tmpVec.push_back(backward[i]);
			backwardMap[key] = tmpVec;
			vector<float> wgtVec;
			wgtVec.push_back(bwgt[i]);
			bwgtMap[key] = wgtVec;
		}
		else{
			backwardMap.find(key)->second.push_back(backward[i]);
			bwgtMap.find(key)->second.push_back(bwgt[i]);
		}
		
	}
	int maxProd = 0;
	int numPaths = 0;
	for(int i=0; i<keyVec.size(); i++){//this loop is for concatenation. 
		unordered_map<int, vector<vector<int> > >::iterator it = backwardMap.find(keyVec[i]);//keyVec is from forwardMap.
		if(it == backwardMap.end())//Check if backwardMap has the node.
			continue;
		//now find out one common node in the intermediate layer. 
		vector<vector<int> > forwardPaths = forwardMap.find(keyVec[i])->second;
		vector<vector<int> > backwardPaths = it->second; 
	
		maxProd = max(maxProd, (int)(forwardPaths.size()* (backwardPaths.size())));

		vector<float> wgt1 = fwgtMap.find(keyVec[i])->second;
		vector<float> wgt2 = bwgtMap.find(keyVec[i])->second;
		for(int j=0; j<forwardPaths.size(); j++){
			for(int k=0; k<backwardPaths.size(); k++){
				if(!contain_loop(forwardPaths[j], backwardPaths[k])){//check if there exists loops
					numPaths += 1;
					if(wgt1[j]+wgt2[k] < minWgt){
						if(candidates.size()==TOP_K)
                                                        candidates.pop();
                                                vector<int> result = forwardPaths[j];
                                                for(int t=backwardPaths[k].size()-2; t>=0; t--)
                                                        result.push_back(backwardPaths[k][t]);
                                                candidates.push(createPath(wgt1[j]+wgt2[k], result));
                                                minWgt = candidates.top().wgt;
					}	
				}
			}
		}
		
	}
	qResult.paths = pq2vec(candidates);
	qResult.numPaths = numPaths;
        qResult.mem = mem;
        qResult.totalPaths = total;
        return qResult;
}

