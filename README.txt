run "make" to compile the code and generate exe file.

Example of running the code:
./pro-heaps datafile queryfile outputfile


1) Inputfile: e.g. ./Enron/enron_graph.wgt.norm
       format similar to http://web.cse.ohio-state.edu/~liangji/StackOverFlow.html. 
       But there are weights or attributes related to weight at the end of each line depending on the types of weights.
       Weights related to recency cannot be assigned before running therefore there are related attribute values stored at the end of each line.

2) queryfile: 9 lines each file for a binary tree query. For example:
3 2 3 3 13809 6 193478
1 2 6
4 13809 3 3 2 193478 6 6 2
5 13809 3 193478 6 2
2 0 2
2 13809 193478
1 4
1 2
5 0 1 0 2 0  

line 1 starts with number 3, meaning that there are 3 mappings to left child, the rest being pairs of parent, left child.
line 2 starts with number 1, meaning that there is 1 mapping to right child, the rest being parent, right child.
line 3 records redundent verification mapping from node to parents in the similar format. It should be in consistent with the above two lines.
line 4 denotes a post-order of all nodes, starting from a count on how many nodes.
line 5 denotes which indexes in the above post-order list are indexes to terminal nodes in increasing order, starting with a count on list length.
line 6 denotes such terminal nodes starting with count. Should be consistent with line 4 and 5.
line 7 denotes which indexes in the post-order list are to junction nodes(nodes with both left child and right child), in increasing order starting with count. 
line 8 denotes such junction nodes starting with count. Should be consistent with line 4 and 7.
line 9 denotes the type of each node in the post-order nodes list, stating with a count.

3) The code will then perform KARPET, Baseline1, Baseline2 on the given query, and return top-k ansers with weight and query time in the output file.
