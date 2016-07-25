run "make" to compile the code and generate exe file.

Example of running the code:
./pro-heaps ./Enron/enron_graph.wgt.norm test_enron.query 10


1) Input graph: e.g. ./Enron/enron_graph.wgt.norm
       format similar to http://web.cse.ohio-state.edu/~liangji/StackOverFlow.html. 
       But there are weights or attributes related to weight at the end of each line depending on the types of weights.
       Weights related to recency cannot be assigned before running therefore there are related attribute values stored at the end of each line.

2) query: two lines for each query. First line is the pattern. Second line contains two targetd entities and another variable specifying the timestamp for query (i.e. when is the query being executed. It is only useful when weights are defined based on recency). Timestamp is usually in unix epoch (sometime might be the one divided by 3600 etc.).   

3) Result:
	0: 11807 [-9.50062e+08 (46.7013)] 31500 [-9.66591e+08 (19.3727)] 14454  => totalwgt: 66.074
	1: 11807 [-9.50062e+08 (46.7013)] 31898 [-9.66591e+08 (19.3727)] 14454  => totalwgt: 66.074
	2: 11807 [-9.50062e+08 (46.7013)] 34081 [-9.66591e+08 (19.3727)] 14454  => totalwgt: 66.074

      # 0/1/2 before colons are the indices of the path (0 means the lightest path). Number like 11807 represents the idx of node.
        Number like 46.7013 in the bracket is the weight of edge. 
