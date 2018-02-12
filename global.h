#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#include <unordered_set>
#include "math.h"
#include <iostream>
#include <fstream>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <stack>
#include <queue>
#include <ios>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <time.h>
#include <tuple>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <map>
#define MAX_WEIGHT 100000000
#define TOP_K 400
#define WITH_ORACLE true
#define WITHOUT_ORACLE false
#define SAFE_LIFT 1000000

typedef struct Query{
	int src;
	int tgt;
	float time;
	std::vector<int> pattern; //only types of nodes here
}Query;

typedef struct GeneralizedQuery{ //generalized, reusable query allowing src and tgt to be sets
	std::unordered_map<int, float> srcs;
	std::unordered_map<int, float> tgts;
	int pos_junction; //used for the next iteration indexing. which level is junction node -------------specified by the post-order traverse
	//TODO: modify query decomposition to assign this value!
	float time;
	std::vector<int> pattern; //only types of nodes here
	std::vector<int> nodes;
}GeneralizedQuery;

typedef std::pair<int, int> Edge;


typedef struct Shape{
    int shape_index; //just a name index of what shape it is for convenience
    int seed_index; //which node is seed
    int adj[100][100];
    int nodenum;
    }Shape;
//the first nodenum row and column in adj is the adjacency matrix of this shape



/*
template <>
struct std::hash<std::pair<int, int> > {
public:
        size_t operator()(std::pair<int, int> x) const throw() {
             size_t h = x;//something with x
             return h;
        }
};
*/



typedef struct Non_bi_tree{
    std::unordered_map<int, std::vector<int>> map2chr;
    std::unordered_map<int, int> map2parent;
	std::unordered_map<int, int> map2pattern;
	float time;
}Non_bi_tree;


typedef struct Query_tree{

	//given a tree query input. At this point, we assume all queries are trees
	//we have already decomposed the tree into post-order list of nodes
	std::unordered_map<int, int> map2leftcdr;
	std::unordered_map<int, int> map2rightcdr;
	std::unordered_map<int, int> map2parent;
	std::unordered_map<int, int> map2patthern;

	std::vector<int> nodes_ordered; //unknown set as 0.
    std::vector<int> terminals_index; //the position of n terminals in the ordered nodes
    std::vector<int> junction_index; //the position of (n-1) junction nodes
    std::vector<int> junctions;
    std::vector<int> terminals;
	std::vector<int> patterns;//the type of nodes

    float time;
//	std::map<Edge, int> Edges_types; //map a given edge to the type


	//expanded patterns is post order too.
	//patterns[0] ~ patterns[juction_index[0]] is the path pattern before junction[0].
	//patterns[junction[i-1]+1] ~ patterns[juction[i]] is the path pattern from junction[i-1] to junction[i]


	//  assert(junction_index.size()+ 1 == terminals_index.size());
	//	assert(patterns.size() == junction_index.size());

}Query_tree;


class Tree_query{
    public: Query_tree querytree;

};



typedef struct Query_tree_fixed{

	//given a tree query input. At this point, we assume all queries are trees
	//we have already decomposed the tree into post-order list of nodes
	std::unordered_map<int, int> map2leftcdr;
	std::unordered_map<int, int> map2rightcdr;
	std::unordered_map<int, int> map2parent;
	std::unordered_map<int, int> map2patthern;

	std::vector<int> nodes_ordered; //unknown set as 0.
    std::vector<int> terminals_index; //the position of n terminals in the ordered nodes
    std::vector<int> junction_index; //the position of (n-1) junction nodes
    std::vector<int> junctions;
    std::vector<int> terminals;
	std::vector<int> patterns;//the type of nodes
	std::vector<int> fixed_verteces;


    float time;

}Query_tree_fixed;


typedef struct Path{
	std::vector<int> nodeIds;
	float wgt;
}Path;

typedef struct Instance_Tree{

    std::unordered_map<int, int> map2leftcdr;
	std::unordered_map<int, int> map2rightcdr;
	std::unordered_map<int, int> map2parent;

	std::unordered_set<int> nodes;
    //std::unordered_map<int> junctions;

	float wgt;

}Instance_Tree;

typedef struct Instance_Tree_rep{
    //when I see a seen node, call it a different name and insert as unseen.
    std::unordered_map<int, int> map2leftcdr;
	std::unordered_map<int, int> map2rightcdr;
	std::unordered_map<int, int> map2parent;

	std::unordered_set<int> nodes;
	float wgt;
	int repeat;

}Instance_Tree_rep;







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


//adaptation for binary trees
typedef struct PQEntity_AStar_Tree{
        int nodeIdx;
        std::unordered_map<int, int> node2vertex;
        int curId_inpattern;
        float wgt;
	float key;
	//bool junction;
       // std::vector<int> subtree;
       Instance_Tree subtree;
}PQEntity_AStar_Tree;

//comparator for the priority queue in dijkstra algorithm.
struct comparator_AStar_Tree{
	bool operator()(PQEntity_AStar_Tree p1, PQEntity_AStar_Tree p2){
		return p1.key > p2.key;
	}
};


//comparator for Path.
struct comparator{//put smallest weight on the top. used by Dijkstra
        bool operator()(Path p1, Path p2){
                return p1.wgt > p2.wgt;
        }
};

struct comparator2{//put largest weight on the top. used by dfs.
        bool operator()(Path p1, Path p2){
                return p1.wgt < p2.wgt;
        }
};

typedef struct QueryResult{
	std::vector<Path> paths;
	int numPaths;//num of legitimate paths.
	int mem;//num of paths in memory at most.
	int totalPaths;//sum up the number of paths for each nodes.
		       //quantify the searching space. Basically num of paths generated.
}QueryResult;

typedef struct QueryResultTrees{
	std::vector<Instance_Tree> trees;
	int numTrees;//num of candidate instances.
	int mem;//num of candidates in memory at most.
	int totalTrees;//
		       //quantify the searching space. Basically num of paths generated.
}QueryResultTrees;

typedef struct ProphetEntry{
	std::vector<std::pair<int, float> > upwards;
	std::vector<std::pair<int, float> > downwards;
//	float est;//estimation or heuristics.
}ProphetEntry;

inline float calcWgt(std::vector<float> edgeVal, float queryTime){
/*Weight is static*/

    float weight = edgeVal[0];
//    std::cout<< "weight calculated: " << weight;
    if ( weight < 0.00001) return 10000;
    else return weight;
	//weight is static.

#ifdef STATIC_WGT
    cout<< "weight calculated: " << edgeVal[0];
	return edgeVal[0];//weight is static.
#endif
/*For DBLP*/
#ifdef DBLP_WGT
	if(edgeVal[1]>queryTime)	return MAX_WEIGHT;
	else return edgeVal[0]*(queryTime-edgeVal[1]+0.1)/4.0;
#endif

/*For StackOverFlow*/
//weights are based on the popularity of questions, answers and users.
#ifdef SOF_WGT_POP
	if(edgeVal[1]>queryTime)	return MAX_WEIGHT;
	else return (10/edgeVal[0]);//one year.(original unit is hour)
#endif

//weight are based on the recency
#ifdef SOF_WGT_TIME
	if(edgeVal[1]>queryTime)	return MAX_WEIGHT;
	else return (queryTime-edgeVal[1]+0.1)/(8640.0);//one year.(original unit is hour)
#endif

//combining the two
#ifdef SOF_WGT
	if(edgeVal[1]>queryTime)	return MAX_WEIGHT;
	else return (10/edgeVal[0])*(queryTime-edgeVal[1]+0.1)/(8640.0);//one year.(original unit is hour)
#endif


/*For Enron. All edge weights are normalized to be 0-100.*/
#ifdef ENRON_WGT
	if(edgeVal[0]>0)	return edgeVal[0];
	if(-edgeVal[0]>queryTime) return MAX_WEIGHT;
	else	return (queryTime+edgeVal[0]+0.1)/(604800.0);//1 week. (original unit is second)
#endif
}
//	else return ((967765860+edgeVal[0]+10)/21057300.0)*100.0;

class graph_t
{
public:
	int n;
  //The vector node contains iterators to the vector neighbors, pointing to the first neighbor of that node. There are num_nodes entries in the node vector and num_edges entries in the neighbors vector.
	std::vector<int> neighbors;// neighbors as one-d array .
	std::vector<std::vector<float> > wgts;//weightes on edges.
	std::vector<int> nodes;//incremental neighbors index.  //XF: what is the difference between neighbors and nodes?
	std::vector<int> typeMap;
	std::vector<int> degree;//instead of doing query everytime, store degree in a vector.
};


#endif //GLOBAL_HEADER
