#include "global.h"
using namespace std;


Query_tree sampleFrom(const graph_t& g, int seed_node, int shape){
//sample a height 3 binary tree using seed_node as a root
//if there exist terminals that satisfy, return query_tree; else return empty.
    Query_tree QTree;
    QTree.nodes_ordered = {9999};
    if (shape == 1){ //shape1: 5 nodes tree
        int neigh1,neigh2, neigh3, neigh4;
        if (seed_node >= g.degree.size()){//seed_node not in graph
            return QTree;
        }
        if (g.degree[seed_node] >= 2){
            neigh1 = g.neighbors[g.nodes[seed_node]];
            neigh2 = g.neighbors[g.nodes[seed_node]+1];
            if (g.degree[neigh1]>= 3){
                neigh3 = g.neighbors[g.nodes[neigh1]];
                if (neigh3 == seed_node){
                    neigh3 = g.neighbors[g.nodes[neigh1]+1];
                    neigh4 = g.neighbors[g.nodes[neigh1]+2];
                }
                else{ //neigh3 hasn't appeared before
                    neigh4 = g.neighbors[g.nodes[neigh1]+1];
                    if (neigh4 == seed_node){
                        neigh4 = g.neighbors[g.nodes[neigh1]+2];
                    }
                }
                //the only case QTree sampling was correct.
                //output a query using root -> 0, neigh1 -> 1, found 3 and 4 terminals

                QTree.nodes_ordered = {neigh3, neigh4 ,neigh1,neigh2,seed_node}; //SEED and neigh1 anonymous, 2/3/4 as terminals
                QTree.map2leftcdr[seed_node]=neigh1;
                QTree.map2leftcdr[neigh1]=neigh3;
                QTree.map2rightcdr[seed_node]=neigh2;
                QTree.map2rightcdr[neigh1]=neigh4;
                QTree.map2parent[neigh3]=neigh1;
                QTree.map2parent[neigh4]=neigh1;
                QTree.map2parent[neigh1]=seed_node;
                QTree.map2parent[neigh2]=seed_node;
                QTree.terminals_index = {0, 1, 3};
                QTree.junction_index = {2, 4};
                QTree.patterns = {g.typeMap[neigh3], g.typeMap[neigh4], g.typeMap[neigh1],g.typeMap[neigh2],g.typeMap[seed_node]};

            }
        }
    }
    if (shape == 2){ //shape2: complete tree of height 2
        int neigh1,neigh2, neigh3, neigh4, neigh5, neigh6;
        vector<int>added_neighs;
        if (seed_node >= g.degree.size()){//seed_node not in graph
            return QTree;
        }
        added_neighs.push_back(seed_node);
        if (g.degree[seed_node] >= 2){
            neigh1 = g.neighbors[g.nodes[seed_node]];
            neigh2 = g.neighbors[g.nodes[seed_node]+1];
            added_neighs.push_back(neigh1);
            added_neighs.push_back(neigh2);

            if (g.degree[neigh1]>= 3 && (g.degree[neigh2]>= 3)){
                neigh3 = g.neighbors[g.nodes[neigh1]];
                if (neigh3 == seed_node){

                    neigh3 = g.neighbors[g.nodes[neigh1]+1];
                    neigh4 = g.neighbors[g.nodes[neigh1]+2];
                    if (find(added_neighs.begin(), added_neighs.end(), neigh3) == added_neighs.end() && find(added_neighs.begin(), added_neighs.end(),neigh4) == added_neighs.end()){
                        added_neighs.push_back(neigh3);
                        added_neighs.push_back(neigh4);
                    }
                    else{
                        return QTree; //do not grow this branch--any way sampling is not our main contribution.
                    }
                }


                else{
                    added_neighs.push_back(neigh3);
                    neigh4 = g.neighbors[g.nodes[neigh1]+1];
                    if (find(added_neighs.begin(), added_neighs.end(),neigh4) != added_neighs.end()){
                        neigh4 = g.neighbors[g.nodes[neigh1]+2];
                        if (find(added_neighs.begin(),added_neighs.end(),neigh4) != added_neighs.end()){
                            return QTree;
                        }
                        else added_neighs.push_back(neigh4);
                    }
                }

                neigh5 = g.neighbors[g.nodes[neigh2]];
                if (find(added_neighs.begin(), added_neighs.end(),neigh5) != added_neighs.end()){
                    neigh5 = g.neighbors[g.nodes[neigh2]+1];
                    neigh6 = g.neighbors[g.nodes[neigh2]+2];
                    if (find(added_neighs.begin(), added_neighs.end(),neigh5) == added_neighs.end() && find(added_neighs.begin(), added_neighs.end(),neigh6) == added_neighs.end()){
                        added_neighs.push_back(neigh5);
                        added_neighs.push_back(neigh6);
                    }
                    else{
                        return QTree; //do not grow this branch--any way sampling is not our main contribution.
                    }
                }
                else{
                    added_neighs.push_back(neigh5);
                    if (find(added_neighs.begin(), added_neighs.end(),neigh6) != added_neighs.end()){
                        neigh6 = g.neighbors[g.nodes[neigh2]+2];
                        if (find(added_neighs.begin(), added_neighs.end(),neigh6) != added_neighs.end()){
                            return QTree;
                        }
                        else added_neighs.push_back(neigh6);
                    }
                }
                //the only case QTree sampling was correct.
                //output a query using root -> 0, neigh1 -> 1, neigh2-> 2, found 3,4,5,6 terminals

                QTree.nodes_ordered = {neigh3, neigh4 ,neigh1,neigh5, neigh6, neigh2,seed_node}; //SEED and neigh1 anonymous, 2/3/4 as terminals
                QTree.map2leftcdr[seed_node]=neigh1;
                QTree.map2leftcdr[neigh1]=neigh3;
                QTree.map2leftcdr[neigh2]=neigh5;
                QTree.map2rightcdr[seed_node]=neigh2;
                QTree.map2rightcdr[neigh1]=neigh4;
                QTree.map2rightcdr[neigh2]=neigh6;
                QTree.map2parent[neigh3]=neigh1;
                QTree.map2parent[neigh4]=neigh1;
                QTree.map2parent[neigh5]=neigh2;
                QTree.map2parent[neigh6]=neigh2;
                QTree.map2parent[neigh1]=seed_node;
                QTree.map2parent[neigh2]=seed_node;
                QTree.terminals_index = {0, 1, 3, 4};
                QTree.junction_index = {2, 5, 6};
                QTree.patterns = {g.typeMap[neigh3], g.typeMap[neigh4], g.typeMap[neigh1],g.typeMap[neigh5],g.typeMap[neigh6],g.typeMap[neigh2],g.typeMap[seed_node]};

            }
        }
    }
    return QTree;
}

Query Transform_2line(const graph_t& g, Query_tree testQTree, int transseed){
//only works for a seed sampled tree.
    Query TransQuery;
    int len = testQTree.nodes_ordered.size();
    int src = testQTree.terminals[0];
    int tgt;
    //first traverse from the terminal to root
    std::vector<int> nodes;
    std::vector<int> pattern;
    int curNode=src;
    nodes.push_back(src);
    int root = testQTree.nodes_ordered.back();
    while(curNode != root){
        curNode = testQTree.map2parent[curNode];
        nodes.push_back(curNode);
    //last push is root.
    }
    nodes.pop_back(); //replacing abstract root with real used seed.
    nodes.push_back(transseed);
    curNode = transseed;

    if (nodes.size()==len) { //already a path
        tgt = nodes.back();
            }
    else{//add more node to extend to the same size
        while(nodes.size()<len){
            //first iter: curNode==root+1;
            //end iter: nodes.size is enough
            int old_node = curNode;
            for(int i=0; i<g.degree[curNode]; i++){
                    int neigh = g.neighbors[g.nodes[curNode]+i];
                    if (find(nodes.begin(), nodes.end(), neigh)==nodes.end()){ //neigh not in nodes:
                        curNode = neigh;
                        nodes.push_back(curNode);
                        break; //only need one like this
                    }

            }
            if(curNode == old_node){//after iterating all neighbors, if still no good neigh,
                cout<<"***************Error!! Tree to path extention failed!"<<endl;
                break;
            }

        }

    }
    //test if finished correctly by size:
    if (nodes.size() == len){

        for(int i = 0; i<nodes.size();i++){
            if (find(testQTree.nodes_ordered.begin(), testQTree.nodes_ordered.end(), nodes[i]) != testQTree.nodes_ordered.end()){ //already in qtree, can be a unknown node not in g
                int type;
                int index = find(testQTree.nodes_ordered.begin(), testQTree.nodes_ordered.end(), nodes[i]) - testQTree.nodes_ordered.begin();
                type = testQTree.patterns[index];
                pattern.push_back(type);
            }
            else //expanded from root, is a node in g. ___given that root was seeded a SEED in G!! only works for a seed sampled tree.
            pattern.push_back(g.typeMap[nodes[i]]);
        }
        TransQuery.src = src;
        TransQuery.tgt = nodes.back();
        TransQuery.time = testQTree.time;
        TransQuery.pattern = pattern;

    }
    else{
        cout<<"***Error!! Tree to path extention failed!"<<nodes.size()<< " != "<< len<<endl;
    }
    return TransQuery;


}


int reset_lighest_test(graph_t& g, Instance_Tree Test_T){
//reset Test_T all weight 0.001 in graph g, for testing purpose
    return 0;
}

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

//Done1: assign weight accumulation to tree nodes
vector<std::unordered_map<int, float>> create_Prophet_Heuristics_generalized(const graph_t&g, GeneralizedQuery Gquery, double& timeUsed, unordered_map<int, float>& leftvalue, unordered_map<int, float>& rightvalue){
	struct timeval start, end;
	gettimeofday(&start, NULL);
	//int midLayer = Gquery.pattern.size()/2 +1;
	int midLayer = Gquery.pos_junction + 1; //src to junction forward and junction to tgt backward. And they meet and terminate at the same layer junction.
	vector<unordered_map<int, float> > layers;

	layers.reserve(Gquery.pattern.size());
	unordered_map<int, float> top;
	//terminal nodes has already been assigned to 0 by default! No need to initialize again.

	for (auto it = Gquery.srcs.begin(); it != Gquery.srcs.end(); ++it)
        top[it->first] = it->second;
	layers.push_back(top);
	for(int i=1; i<midLayer; i++){
		unordered_map<int, float> lastLayer = layers[i-1];//upper layer.
		unordered_map<int, float> newLayer;
		for(unordered_map<int, float>::iterator it=lastLayer.begin(); it!=lastLayer.end(); it++){//expanding each node on last layer.
			int node = it->first;
			for(int j=0; j<g.degree[node]; j++){//visit the neighbors of each node.
				int neigh = g.neighbors[g.nodes[node]+j];
				int observe_type = g.typeMap[neigh];
	//			cout<<"current node is "<<node<<" ,neighbor node name is "<<neigh<< "this neighbor type is "<<observe_type<<endl;
				if(g.typeMap[neigh] == Gquery.pattern[i])
					newLayer[neigh] = MAX_WEIGHT;
			}
		}
		layers.push_back(newLayer);
	}//meet at midLayer-1
	unordered_map<int, float> bottom;
	for(int i=midLayer; i<Gquery.pattern.size()-1; i++)
		layers.push_back(bottom);//empty.
	for (auto it = Gquery.tgts.begin(); it != Gquery.tgts.end(); ++it)
        bottom[it->first] = it->second; //bottom[query.tgt] = 0;
	layers.push_back(bottom);
	for(int i=Gquery.pattern.size()-2; i>=midLayer-1; i--){ //keep propergate from the tgt side till midlayer -1
		unordered_map<int, float> belowLayer = layers[i+1];
		unordered_map<int, float> newLayer;
		for(unordered_map<int, float>::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
			int node = it->first;
			float pathWgt = it->second;
			for(int j=0; j<g.degree[node]; j++){
				int neigh = g.neighbors[g.nodes[node]+j];
				float wgt = calcWgt(g.wgts[g.nodes[node]+j], Gquery.time);
				if(g.typeMap[neigh]==Gquery.pattern[i]){
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
	for(int i=midLayer; i<Gquery.pattern.size()-1; i++){
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
//	for(int i=midLayer-2; i>0; i--){
    for(int i=1; i<midLayer -1 ; i++){
            //A*heuristic modification:: change direction of this part, propergate from left side, till midlayer-2, later add at meet point midlayer -1
                unordered_map<int, float> belowLayer = layers[i-1];
                unordered_map<int, float> newLayer;
                for(unordered_map<int, float>::iterator it=belowLayer.begin(); it!=belowLayer.end(); it++){
                        int node = it->first;
			float pathWgt = it->second;
                        for(int j=0; j<g.degree[node]; j++){
                                int neigh = g.neighbors[g.nodes[node]+j];
				float wgt = calcWgt(g.wgts[g.nodes[node]+j], Gquery.time);
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


	//meet in the junction node --i =midlayer-1
	int junction = midLayer -1;
	unordered_map<int, float> left_belowLayer = layers[junction-1];
	unordered_map<int, float> right_belowLayer = layers[junction+1];
	unordered_map<int, float> meetLayer;
	//unordered_map<int, float> leftvalue;
	//unordered_map<int, float> rightvalue;



	for(unordered_map<int, float>::iterator it=left_belowLayer.begin(); it!=left_belowLayer.end(); it++){
                        int node = it->first;
                        float pathWgt = it->second;

                        for(int j=0; j<g.degree[node]; j++){
                                int neigh = g.neighbors[g.nodes[node]+j];
                                float wgt = calcWgt(g.wgts[g.nodes[node]+j], Gquery.time);
                                if(layers[junction].count(neigh)>0){//pattern checking not needed.
					                    if(leftvalue.count(neigh) == 0)
                                                leftvalue[neigh] = pathWgt + wgt;
                                        else
                                                 leftvalue[neigh]= min(leftvalue.at(neigh), pathWgt+wgt);
                                }
                        }
	}

    for(unordered_map<int, float>::iterator it=right_belowLayer.begin(); it!=right_belowLayer.end(); it++){
                        int node = it->first;
                        float pathWgt = it->second;

                        for(int j=0; j<g.degree[node]; j++){
                                int neigh = g.neighbors[g.nodes[node]+j];
                                float wgt = calcWgt(g.wgts[g.nodes[node]+j], Gquery.time);
                                if(layers[junction].count(neigh)>0){//pattern checking not needed.
					                    if(rightvalue.count(neigh) == 0)
                                                rightvalue[neigh] = pathWgt + wgt;
                                        else
                                                 rightvalue[neigh]= min(rightvalue.at(neigh), pathWgt+wgt);
                                }
                        }
    }


    for (unordered_map<int, float>::iterator it=leftvalue.begin(); it!=leftvalue.end(); it++){
        //leftvalue.size = rightvalue.size = number of candidate in the middle layer. add for each of them.
        int node = it-> first;
        meetLayer[node] = leftvalue[node] + rightvalue[node];

    }



	layers[junction] = meetLayer;






	gettimeofday(&end, NULL);
	timeUsed = (end.tv_sec + double(end.tv_usec)/1000000) - (start.tv_sec + double(start.tv_usec)/1000000);


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

//Decomposes the tree pattern into path patterns and also computes the order in which the path patterns should be processed
vector<GeneralizedQuery> decompo_Query_Tree(Query_tree QTree){ //only decompose the pattern, ititialize source and target as empty
    vector<GeneralizedQuery> decomposed_queries;

    cout<< "decomposing tree query..." << endl;

    //starting a new decomposed path.
    //Non_touched_JT stores the terminals and junctions in the post-order way. In a pre-order traversal, for a junction (not a terminal) at location i in the vector, the source and the target of the corresponding path is at location i-1 and i-2
    vector <int> non_touched_JT; //terminals and juctions that haven't been put into decomposed paths yet
    for(int i=0; i<(QTree.nodes_ordered.size()); i++){
            if((find(QTree.junction_index.begin(), QTree.junction_index.end(), i)!= QTree.junction_index.end())
               || (find(QTree.terminals_index.begin(), QTree.terminals_index.end(), i) != QTree.terminals_index.end())){
                    non_touched_JT.push_back(QTree.nodes_ordered[i]);
               }
    }
    for(int i=0; i<(QTree.junction_index.size()); i++){
            //for each junction node, find the src and tgt and delete them from non_touched_JT
            GeneralizedQuery generalized_query;
            vector <int> nodes_pattern;
            vector <int> nodes_temp; //for finding the position of junction
            int current_junction_pos = QTree.junction_index[i];
            int current_junction = QTree.nodes_ordered[current_junction_pos];
            int ref_pos = find(non_touched_JT.begin(), non_touched_JT.end(), current_junction) - non_touched_JT.begin();

            int src = non_touched_JT[ref_pos - 2];
            int tgt = non_touched_JT[ref_pos - 1];
            int current_node;
            int next_node;

            //from src to junction, then to tgt
            //src to junction---up
            for(current_node = src; current_node!= current_junction;){
                nodes_pattern.push_back(QTree.map2patthern[current_node]);
                nodes_temp.push_back(current_node);
                cout<<"added " <<current_node << " to the current path, "<<"his pattern is "<<QTree.map2patthern[current_node]<<endl;
                next_node = QTree.map2parent[current_node];
                current_node = next_node;
            }
            //junction to tgt---down
            for(current_node = current_junction; current_node!= tgt;){
                nodes_pattern.push_back(QTree.map2patthern[current_node]);
                nodes_temp.push_back(current_node);
                cout<<"added " <<current_node << " to the current path, "<<"his pattern is "<<QTree.map2patthern[current_node]<<endl;
                if (QTree.map2rightcdr.find(current_node) == QTree.map2rightcdr.end()){
                        next_node = QTree.map2leftcdr[current_node];
                    }
                else
                        next_node= QTree.map2rightcdr[current_node]; //path assumption: either have one right child or have one left node
                current_node = next_node;
            }


            nodes_pattern.push_back(QTree.map2patthern[tgt]);
            nodes_temp.push_back(tgt);
            cout<<"added " <<tgt << " to the current path, "<<"his pattern is "<<QTree.map2patthern[tgt]<<endl;

            non_touched_JT.erase(std::remove(non_touched_JT.begin(), non_touched_JT.end(), src),non_touched_JT.end());
            non_touched_JT.erase(std::remove(non_touched_JT.begin(), non_touched_JT.end(), tgt),non_touched_JT.end());

            generalized_query.pattern = nodes_pattern;
            generalized_query.srcs[src]=0.0;//for now, even if it is a junction, init it with one node. Do the decision later on what is it
            generalized_query.tgts[tgt]=0.0;
            generalized_query.pos_junction = find(nodes_temp.begin(), nodes_temp.end(), current_junction)-nodes_temp.begin();
            generalized_query.nodes = nodes_temp;
            decomposed_queries.push_back(generalized_query);
            cout<< "added the above query to decomposed vector, the remaining JT size is "<<non_touched_JT.size()<< endl;
    }

        cout<<non_touched_JT.size()<<"==make sure is 1" <<endl;

    return decomposed_queries;

}

/*****dont do this for now.        //now it's time to insert the edge type into this decomposed query's pattern!

        vector <int> edge_included;
        edge_included.push_back(nodes_pattern[0]);
        for(int i=1; i<nodes_pattern.size(); i++){
            Edge e = make_pair(nodes_pattern[i-1], nodes_pattern[i]);
            edge_included.push_back(QTree.Edges_types.find(e)-> second);
            edge_included.push_back(nodes_pattern[i]);
        }
        generalized_query.pattern = edge_included;
*/




