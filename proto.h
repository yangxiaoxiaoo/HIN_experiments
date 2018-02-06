#include "global.h"
#include "pruned_landmark_labeling.h"

/*
	utils.cc
*/
std::vector<Shape> Auto_shapes(const graph_t& g, int seed_node, int shape);
std::vector<Query_tree> Auto_sampleFrom(const graph_t& g, int seed_node, Shape thisshape);
Query_tree binaryfy(Non_bi_tree tree);
std::vector<int> reverse_postorder(int root, std::unordered_map<int, int> map2leftcdr,
	std::unordered_map<int, int> map2rightcdr);
Query_tree sampleFrom(const graph_t& g, int seed_node, int shape);
Query Transform_2line(const graph_t& g, Query_tree testQTree, int transseed);
int reset_lighest_test(graph_t& g, Instance_Tree Test_T);
void check_path_wgt(std::vector<Path> paths, graph_t g, Query query);
int getCurrentRSS();//in kilobytes
bool contain_loop(std::vector<int> &high, std::vector<int> &low);
std::vector<std::unordered_set<int>> create_Prophet(const graph_t& g, Query query, double& timeUsed);
std::vector<std::unordered_map<int, float>> create_Prophet_Heuristics(const graph_t&g, Query query, double& timeUsed);
std::vector<std::unordered_map<int, float>>create_Prophet_Heuristics_generalized(const graph_t&g, GeneralizedQuery query, double& timeUsed, std::unordered_map<int, float>& leftvalue, std::unordered_map<int, float>& rightvalue );
Path createPath(float wgt, std::vector<int> path);
std::vector<Path> pq2vec(std::priority_queue<Path, std::vector<Path>, comparator2> candidates);
std::vector<GeneralizedQuery> decompo_Query_Tree(Query_tree QTree);
std::unordered_map<int, std::unordered_set<int>> Retrieve_children(Query_tree QTree);
std::unordered_map<int, std::unordered_map<int, std::tuple<float,float>>> bottom_up_hrtc_compute
(const graph_t& g, Query_tree querytree, std::unordered_map<int,  std::unordered_map<int, std::unordered_map<int, float>>> & candidxleft, std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, float>>> & candidxright);

/*
	io.cc
*/
Query_tree readfromfile(char *fname);
graph_t load_graph(char *fname, std::vector<std::pair<int, int>> &edge_list);
void print2Screen(QueryResult qResult, double timeDiff);
void print2File(QueryResult qResult, double timeDiff, std::ofstream& ofs);
void print2FileTree(QueryResultTrees qResultTree, double timeDiff, std::ofstream& ofs);
/*
	dfm.cc
*/
QueryResult DFM(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle);

/*
	bfm.cc
*/
QueryResult BFM(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle);
QueryResult bidirectional_BFM(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle);
QueryResult bidirectional_BFM_Prophet_IM(const graph_t& g, Query query, double& timeUsed);//implicit prophet graph
QueryResult bidirectional_BFM_Prophet_EX(const graph_t& g, Query query);//explicit prophet graph
/*
	dijkstra.cc
*/
QueryResult dijkstra(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle);
QueryResult dijkstra_with_Prophet(const graph_t& g, Query query);
QueryResult bidir_Dijkstra(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle);
/*
	AStar_Prophet.cc
*/
QueryResult AStar_Prophet(const graph_t& g, Query query, double& timeUsed);
QueryResultTrees AStar_Prophet_Tree(const graph_t& g, Query_tree query, double& timeUsed);
QueryResultTrees AStar_Prophet_Tree_v2(const graph_t& g, Query_tree query, double& timeUsed);
Instance_Tree Instance_Tree_Insert(Instance_Tree subtree, int curId, int neigh, float edgewgt, bool on_left);
QueryResult Bidirec_AStar_Prophet(const graph_t& g, Query query);
QueryResult AStar_Prophet_Explicit(const graph_t& g, Query query);
QueryResult AStar_Original(const graph_t& g, PrunedLandmarkLabeling<> &pll, Query query, bool oracle);
QueryResult AStar_Original_OPT(const graph_t& g, Query query);
float getHeuristicValue(const graph_t& g, Query query, int depth, int curNode);
float getHeuristicValueTree(const graph_t& g, Query_tree query_tree, int depth, int curNode);
int Expand_current(const graph_t& g, Query_tree querytree, std::vector <int> pre_order_patterns, int curId,
                    PQEntity_AStar_Tree curNode,Instance_Tree subtree, int& total,std::unordered_map<int, std::unordered_map<int, float>> node2layers,
                    int curId_inpattern, std::priority_queue<PQEntity_AStar_Tree, std::vector<PQEntity_AStar_Tree>, comparator_AStar_Tree>& frontier, int& numTrees);

bool Expand_current_v2(const graph_t& g, Query_tree querytree, std::vector <int> pre_order_patterns, int& curId,
                    PQEntity_AStar_Tree& curNode,Instance_Tree subtree, int& total,std::unordered_map<int, std::unordered_map<int, std::tuple<float, float>>> node2layers,
                    int curId_inpattern, std::priority_queue<PQEntity_AStar_Tree, std::vector<PQEntity_AStar_Tree>, comparator_AStar_Tree>& frontier, int& numTrees,
                    std::unordered_map<int,  std::unordered_map<int, std::unordered_map<int, float>>> & candidxleft, std::unordered_map<int,  std::unordered_map<int, std::unordered_map<int, float>>> & candidxright);
std::vector<PQEntity_AStar_Tree>  Expand_brute_v2(const graph_t& g, Query_tree querytree, std::vector <int> pre_order_patterns, int& curId,
                    PQEntity_AStar_Tree& curNode,Instance_Tree subtree, int& total,std::unordered_map<int, std::unordered_map<int, std::tuple<float, float>>> node2layers,
                    int curId_inpattern, int& numTrees,
                    std::unordered_map<int,  std::unordered_map<int, std::unordered_map<int, float>>> & candidxleft, std::unordered_map<int,  std::unordered_map<int, std::unordered_map<int, float>>> & candidxright);

int Expand_current_exhaust(const graph_t& g, Query_tree querytree, std::vector <int> pre_order_patterns, int curId,
                    PQEntity_AStar_Tree curNode,Instance_Tree subtree, int& total,std::unordered_map<int, std::unordered_map<int, float>> node2layers,
                    int curId_inpattern, std::priority_queue<PQEntity_AStar_Tree, std::vector<PQEntity_AStar_Tree>, comparator_AStar_Tree>& frontier, int& numTrees);

std::vector<Instance_Tree> Set_insert(const graph_t& g, Instance_Tree Old_tree, int this_node, int check_connection_node, bool insert_parent, bool insert_left, std::unordered_map<int, int> vertex2node, int&numtrees);
std::vector<Instance_Tree> expend_withcheck(const graph_t& g, std::unordered_map<int, int> vertex2node, Query_tree querytree, Instance_Tree incomplete_tree, int& numtrees);
std::vector<PQEntity_AStar_Tree> Top_k_weight(std::vector<PQEntity_AStar_Tree> complete_trees);
int typecheck_all(const graph_t& g, Query_tree querytree, std::unordered_map<int, int>& vertex2node, std::unordered_map<int, std::unordered_map<int, float>>& node2layers);
QueryResultTrees Bruteforce(const graph_t& g, Query_tree querytree, double& timeUsed);
QueryResultTrees Bruteforce_modified (const graph_t& g, Query_tree querytree, double& timeUsed);
//baseline2
Query GetBackbone(const graph_t& g, Query_tree querytree, int &rootpos);
std::vector<Instance_Tree> All_matching_trees_fixed(const graph_t& g, Query_tree_fixed query_tree_fixed, double& timeUsed, int &numtrees, int &mem, int &totalTrees);
Query_tree_fixed left_from_backbone(int rootpos, Query_tree querytree, Query BackboneQuery, Path backboneinstance);
Query_tree_fixed right_from_backbone(int rootpos, Query_tree querytree, Query BackboneQuery, Path backboneinstance);
Instance_Tree Combine_tree(int rootpos, Path backboneinstance, Query_tree query, Instance_Tree left_instance, Instance_Tree right_instance);
QueryResultTrees Backbone_query(const graph_t& g, Query_tree querytree, double& timeUsed);
QueryResult AStar_Prophet_pop(const graph_t& g, Query query, double& timeUsed,  std::vector<Path> found_paths);

