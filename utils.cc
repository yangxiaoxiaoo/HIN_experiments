#include "global.h"
using namespace std;

void check_path_wgt(vector<Path> paths, graph_t g, Query query){
    for(int i=0; i<paths.size(); i++){
    	float wgtSum = 0;
	vector<int> path = paths[i].nodeIds;
	cout << "\t"<< i <<": " << path[0] << " ";
	for(int i=1; i<path.size() && i<query.pattern.size(); i++){
		int prevNode = path[i-1];
		int curNode = path[i];
		for(int k=0; k<g.degree[curNode]; k++){
			int neigh = g.neighbors[g.nodes[curNode]+k];
			if(neigh == prevNode){
				cout <<"["<<g.wgts[g.nodes[curNode]+k].back() << " ("<<calcWgt(g.wgts[g.nodes[curNode]+k], query.time) << ")]" << " ";
				wgtSum += calcWgt(g.wgts[g.nodes[curNode]+k], query.time);
//				cout <<"["<<calcWgt(g.wgts[g.nodes[curNode]+k], query.time) << "]" << " ";
				break;
			}
		}
		cout << path[i] << " ";
	}
	cout << " => totalwgt: "<<wgtSum<<endl;
    }
    cout << "############################################################"<<endl;

}
int getCurrentRSS(){//in kilobytes
        long rss = 0L;
        FILE* fp = NULL;
        if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
                return (size_t)0L;              /* Can't open? */
        if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
        {
                fclose( fp );
                return (size_t)0L;              /* Can't read? */
        }
        fclose( fp );
        return (int) ((size_t)rss * (size_t)sysconf( _SC_PAGESIZE))/1024;
}
bool contain_loop(vector<int> &high, vector<int> &low){//check loop when merging the forward path and backward path in bi-directional search.
	for(int i=0; i<low.size()-1; i++)
		for(int j=0; j<high.size()-1; j++){//note the top node on the vectors are the same.
			if(low[i] == high[j])
				return true;
		}
	return false;
}
Path createPath(float wgt, std::vector<int> path){
        Path entity;
        entity.wgt = wgt;//weight of path from src to current node.
        entity.nodeIds = path;//node ids of current path. need to change if we need to output the path itself.
        return entity;
}


vector<unordered_set<int>> create_Prophet(const graph_t& g, Query query, double& timeUsed){
	struct timeval start, end;
        gettimeofday(&start, NULL);
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
	for(int i=query.pattern.size()-2; i>=midLayer-1; i--){
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
	for(int i=midLayer; i<query.pattern.size()-1; i++){
                unordered_set<int> lastLayer = layers[i-1];//upper layer.
                unordered_set<int> newLayer;
                for(unordered_set<int>::iterator it=lastLayer.begin(); it!=lastLayer.end(); it++){//expanding each node on last layer.
                        int node = *it;
                        for(int j=0; j<g.degree[node]; j++){//visit the neighbors of each node.
                                int neigh = g.neighbors[g.nodes[node]+j];
                                if(layers[i].count(neigh)>0)//Note: pattern checking not needed. g.typeMap[neigh] == query.pattern[i] &&
                                        newLayer.insert(neigh);
                        }
                }
                layers[i] = newLayer;
        }
        for(int i=midLayer-2; i>0; i--){
                unordered_set<int> belowLayer = layers[i+1];
                unordered_set<int> newLayer;
                for(unordered_set<int>::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
                        int node = *it;
                        for(int j=0; j<g.degree[node]; j++){
                                int neigh = g.neighbors[g.nodes[node]+j];
                                if(layers[i].count(neigh)>0)
                                        newLayer.insert(neigh);
                        }
                }
                layers[i] = newLayer;
        }
	gettimeofday(&end, NULL);
        timeUsed = (end.tv_sec + double(end.tv_usec)/1000000) - (start.tv_sec + double(start.tv_usec)/1000000);
	return layers;
}

//TODO
vector<unordered_map<int, float>> create_Prophet_Heuristics_generalized(const graph_t&g, GeneralizedQuery query, double& timeUsed){
	struct timeval start, end;
	gettimeofday(&start, NULL);
	int midLayer = query.pattern.size()/2 +1;//half forward and halp backward. And they meet and terminate at the same layer.
	vector<unordered_map<int, float> > layers;
	/*
	layers.reserve(query.pattern.size());
	unordered_map<int, float> top;
	top[query.src] = MAX_WEIGHT;
	layers.push_back(top);
	for(int i=1; i<midLayer; i++){
		unordered_map<int, float> lastLayer = layers[i-1];//upper layer.
		unordered_map<int, float> newLayer;
		for(unordered_map<int, float>::iterator it=lastLayer.begin(); it!=lastLayer.end(); it++){//expanding each node on last layer.
			int node = it->first;
			for(int j=0; j<g.degree[node]; j++){//visit the neighbors of each node.
				int neigh = g.neighbors[g.nodes[node]+j];
				if(g.typeMap[neigh] == query.pattern[i])
					newLayer[neigh] = MAX_WEIGHT;
			}
		}
		layers.push_back(newLayer);
	}//meet at midLayer-1
	unordered_map<int, float> bottom;
	for(int i=midLayer; i<query.pattern.size()-1; i++)
		layers.push_back(bottom);//empty.
	bottom[query.tgt] = 0;
	layers.push_back(bottom);
	for(int i=query.pattern.size()-2; i>=midLayer-1; i--){
		unordered_map<int, float> belowLayer = layers[i+1];
		unordered_map<int, float> newLayer;
		for(unordered_map<int, float>::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
			int node = it->first;
			float pathWgt = it->second;
			for(int j=0; j<g.degree[node]; j++){
				int neigh = g.neighbors[g.nodes[node]+j];
				float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
				if(g.typeMap[neigh]==query.pattern[i]){
					if(newLayer.count(neigh) == 0)
						newLayer[neigh] = pathWgt + wgt;
					else
						newLayer[neigh] = min(newLayer.at(neigh), pathWgt+wgt);
				}
			}
		}
		if(i == midLayer-1){//i==midLayer-1
			unordered_map<int, float> tmpMap;
			for(unordered_map<int, float>::iterator it=newLayer.begin(); it!=newLayer.end(); it++){
				int node = it->first;
				if(layers[i].count(node) > 0)
					tmpMap.insert(*it);
			}
			layers[i] = tmpMap;
		}
		else
			layers[i] = newLayer;

	}
	for(int i=midLayer; i<query.pattern.size()-1; i++){
                unordered_map<int, float> lastLayer = layers[i-1];//upper layer.
                unordered_map<int, float> newLayer;
                for(unordered_map<int, float>::iterator it=lastLayer.begin(); it!=lastLayer.end(); it++){//expanding each node on last layer.
                        int node = it->first;
                        for(int j=0; j<g.degree[node]; j++){//visit the neighbors of each node.
                                int neigh = g.neighbors[g.nodes[node]+j];
                                if(layers[i].count(neigh)>0) //Note: it seems that the pattern matching is not needed here.
                                        newLayer.insert(*(layers[i].find(neigh)));
                        }
                }
                layers[i] = newLayer;
        }
	for(int i=midLayer-2; i>0; i--){//i=0 not necessary.
                unordered_map<int, float> belowLayer = layers[i+1];
                unordered_map<int, float> newLayer;
                for(unordered_map<int, float>::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
                        int node = it->first;
			float pathWgt = it->second;
                        for(int j=0; j<g.degree[node]; j++){
                                int neigh = g.neighbors[g.nodes[node]+j];
				float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
                                if(layers[i].count(neigh)>0){//pattern checking not needed.
					if(newLayer.count(neigh) == 0)
                                                newLayer[neigh] = pathWgt + wgt;
                                        else
                                                newLayer[neigh] = min(newLayer.at(neigh), pathWgt+wgt);
					//newLayer[neigh] = min(layers[i].at(neigh), pathWgt+wgt);
					//layers[i].at(neigh) = newLayer.at(neigh);
				}
                        }
                }
		layers[i] = newLayer;
	}
	gettimeofday(&end, NULL);
	timeUsed = (end.tv_sec + double(end.tv_usec)/1000000) - (start.tv_sec + double(start.tv_usec)/1000000);
	*/
	return layers;
}

vector<unordered_map<int, float>> create_Prophet_Heuristics(const graph_t&g, Query query, double& timeUsed){
	struct timeval start, end;
	gettimeofday(&start, NULL);
	int midLayer = query.pattern.size()/2 +1;//half forward and halp backward. And they meet and terminate at the same layer.
	vector<unordered_map<int, float> > layers;
	layers.reserve(query.pattern.size());
	unordered_map<int, float> top;
	top[query.src] = MAX_WEIGHT;
	layers.push_back(top);
	for(int i=1; i<midLayer; i++){
		unordered_map<int, float> lastLayer = layers[i-1];//upper layer.
		unordered_map<int, float> newLayer;
		for(unordered_map<int, float>::iterator it=lastLayer.begin(); it!=lastLayer.end(); it++){//expanding each node on last layer.
			int node = it->first;
			for(int j=0; j<g.degree[node]; j++){//visit the neighbors of each node.
				int neigh = g.neighbors[g.nodes[node]+j];
				if(g.typeMap[neigh] == query.pattern[i])
					newLayer[neigh] = MAX_WEIGHT;
			}
		}
		layers.push_back(newLayer);
	}//meet at midLayer-1
	unordered_map<int, float> bottom;
	for(int i=midLayer; i<query.pattern.size()-1; i++)
		layers.push_back(bottom);//empty.
	bottom[query.tgt] = 0;
	layers.push_back(bottom);
	for(int i=query.pattern.size()-2; i>=midLayer-1; i--){
		unordered_map<int, float> belowLayer = layers[i+1];
		unordered_map<int, float> newLayer;
		for(unordered_map<int, float>::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
			int node = it->first;
			float pathWgt = it->second;
			for(int j=0; j<g.degree[node]; j++){
				int neigh = g.neighbors[g.nodes[node]+j];
				float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
				if(g.typeMap[neigh]==query.pattern[i]){
					if(newLayer.count(neigh) == 0)
						newLayer[neigh] = pathWgt + wgt;
					else
						newLayer[neigh] = min(newLayer.at(neigh), pathWgt+wgt);
				}
			}
		}
		if(i == midLayer-1){//i==midLayer-1
			unordered_map<int, float> tmpMap;
			for(unordered_map<int, float>::iterator it=newLayer.begin(); it!=newLayer.end(); it++){
				int node = it->first;
				if(layers[i].count(node) > 0)
					tmpMap.insert(*it);
			}
			layers[i] = tmpMap;
		}
		else
			layers[i] = newLayer;

	}
	for(int i=midLayer; i<query.pattern.size()-1; i++){
                unordered_map<int, float> lastLayer = layers[i-1];//upper layer.
                unordered_map<int, float> newLayer;
                for(unordered_map<int, float>::iterator it=lastLayer.begin(); it!=lastLayer.end(); it++){//expanding each node on last layer.
                        int node = it->first;
                        for(int j=0; j<g.degree[node]; j++){//visit the neighbors of each node.
                                int neigh = g.neighbors[g.nodes[node]+j];
                                if(layers[i].count(neigh)>0) //Note: it seems that the pattern matching is not needed here.
                                        newLayer.insert(*(layers[i].find(neigh)));
                        }
                }
                layers[i] = newLayer;
        }
	for(int i=midLayer-2; i>0; i--){//i=0 not necessary.
                unordered_map<int, float> belowLayer = layers[i+1];
                unordered_map<int, float> newLayer;
                for(unordered_map<int, float>::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
                        int node = it->first;
			float pathWgt = it->second;
                        for(int j=0; j<g.degree[node]; j++){
                                int neigh = g.neighbors[g.nodes[node]+j];
				float wgt = calcWgt(g.wgts[g.nodes[node]+j], query.time);
                                if(layers[i].count(neigh)>0){//pattern checking not needed.
					if(newLayer.count(neigh) == 0)
                                                newLayer[neigh] = pathWgt + wgt;
                                        else
                                                newLayer[neigh] = min(newLayer.at(neigh), pathWgt+wgt);
					//newLayer[neigh] = min(layers[i].at(neigh), pathWgt+wgt);
					//layers[i].at(neigh) = newLayer.at(neigh);
				}
                        }
                }
		layers[i] = newLayer;
	}
	gettimeofday(&end, NULL);
	timeUsed = (end.tv_sec + double(end.tv_usec)/1000000) - (start.tv_sec + double(start.tv_usec)/1000000);
	return layers;
}




vector<Path> pq2vec(priority_queue<Path, std::vector<Path>, comparator2> candidates){
	vector<Path> vec;
	while(!candidates.empty()){
                        vec.push_back(candidates.top());
                        candidates.pop();
                }
	vector<Path> reverse;
	for(int i=vec.size()-1; i>=0; i--)
		reverse.push_back(vec[i]);
	return reverse;

}

vector<GeneralizedQuery> decompo_Query_Tree(Query_tree QTree){ //only decompose the pattern, ititialize source and target as empty
    vector<GeneralizedQuery> decomposed_queries;
    cout<< "decomposing tree query..." << endl;

    //starting a new decomposed path.


    vector <int> non_touched_JT; //terminals and juctions that haven't been put into decomposed paths yet
    for(int i=0; i<(QTree.nodes_ordered.size()); i++){
            if(find(QTree.junction_index.begin(), QTree.junction_index.end(), i)
               || find(QTree.terminals_index.begin(), QTree.terminals_index.end(), i)){
                    non_touched_JT.push_back(QTree.nodes_ordered[i]);
               }
    }
    for(int i=0; i<(QTree.junction_index.size()); i++){
            //for each junction node, find the src and tgt and delete them from non_touched_JT
            GeneralizedQuery generalized_query;
            vector <int> nodes_pattern;

            int current_junction_pos = QTree.junction_index[i];
            int current_junction = QTree.nodes_ordered[current_junction_pos];
            int ref_pos = *find(non_touched_JT.begin(), non_touched_JT.end(), current_junction_pos);
            int src = non_touched_JT[ref_pos - 2];
            int tgt = non_touched_JT[ref_pos - 1];
            non_touched_JT.erase(non_touched_JT.begin()+ ref_pos - 2);
            non_touched_JT.erase(non_touched_JT.begin()+ ref_pos - 1);
            int src_index = *find(QTree.nodes_ordered.begin(), QTree.nodes_ordered.end(), src);
            int tgt_index = *find(QTree.nodes_ordered.begin(), QTree.nodes_ordered.end(), tgt);
            for(int j = src_index, j<=tgt_index, j++){
                nodes_pattern.push_back(QTree.patterns[j]);
                cout<<"added " <<QTree.nodes_ordered[j] << " to the current path, "<<"his pattern is "<<QTree.patterns[j]<<endl;
            }
            generalized_query.pattern = nodes_pattern;
            generalized_query.src = src;//for now, even if it is a junction, init it with one node. Do the decision later on what is it
            generalized_query.tgt = tgt;
            generalized_query.pos_junction = *find(nodes_pattern.begin(), nodes_pattern.end(), current_junction)
    }
    if (non_touched_JT.size()> 0){
        cout<<"warning: not all terminal/junction nodes are included!"<<endl;
    }

        //now it's time to insert the edge type into this decomposed query's pattern!
/*****dont do this for now.
        vector <int> edge_included;
        edge_included.push_back(nodes_pattern[0]);
        for(int i=1; i<nodes_pattern.size(); i++){
            Edge e = make_pair(nodes_pattern[i-1], nodes_pattern[i]);
            edge_included.push_back(QTree.Edges_types.find(e)-> second);
            edge_included.push_back(nodes_pattern[i]);
        }
        generalized_query.pattern = edge_included;
*/       //update the decomposed query pattern done.


        decomposed_queries.push_back(generalized_query);
        cout<< "added the above query to decomposed vector"<< endl;
   }

    return decomposed_queries;

}

/*todo
unordered_map<int, unordered_set> Retrieve_children(Query_tree QTree){
    int current_node;
    unordered_set children_set;



}

*/
