#include "global.h"
#include "proto.h"
#include "pruned_landmark_labeling.h"
#include <unordered_set>
using namespace std;

//unit test main, usage: ./pro-heaps ./Enron/enron_graph.wgt.norm
//query is specified in test main().




int main (int argc, char **argv){

    char *in_fname = argv[1];
	double timeDiff = 0.0;
	double pTime1 = 0, pTime2 = 0;//time for establishing prophet graph.
	struct timeval start_before_load,start, time1, time2, startQuery, endQuery;
	gettimeofday(&start_before_load,NULL);

	PrunedLandmarkLabeling<> pll;
	vector<pair<int, int>> edge_list;

	graph_t G = load_graph(in_fname, edge_list);//loading the graph.
	cout << "# nodes: " << G.n << ".  # edges: " << G.neighbors.size()/2 << endl;
	gettimeofday(&start,NULL);
	Query_tree testQTree;
	Query_tree sampledTree;

//TEST CASE1: no instance maching
/*
	testQTree.patterns = {2,2,2,3,1}; //this is node pattern. post-order 22211. edge has a label too.
	testQTree.nodes_ordered = {10698, 10388, 11807, 1, 2}; //non-terminal nodes assigned different values for edge distinction
	testQTree.map2leftcdr[2]=10698;
	testQTree.map2leftcdr[1]=10388;
	testQTree.map2rightcdr[2]=1;
	testQTree.map2rightcdr[1]=11807;
	testQTree.map2parent[10698]=2;
	testQTree.map2parent[1]=2;
    testQTree.map2parent[10388]=1;
	testQTree.map2parent[11807]=1;
	testQTree.terminals_index = {0, 1, 2};
	testQTree.junction_index = {3, 4};
	for (int i=0; i<testQTree.nodes_ordered.size(); i++){
        testQTree.map2patthern.insert(make_pair(testQTree.nodes_ordered[i], testQTree.patterns[i]));
	}
	for (int i=0; i<testQTree.junction_index.size();i++){
        testQTree.junctions.push_back(testQTree.nodes_ordered[testQTree.junction_index[i]]);
	}
    QueryResultTrees qResult = AStar_Prophet_Tree(G,testQTree,pTime2);

 */

 //TEST CASE2: with instance maching
    testQTree.patterns = {2,3,3,2,2,3}; //this is node pattern. post-order 22211. edge has a label too.
	testQTree.nodes_ordered = {11807, 1 ,116,2,10698,3}; //non-terminal nodes assigned different values for edge distinction
	testQTree.map2leftcdr[1]=11807;
	testQTree.map2leftcdr[2]=1;
	testQTree.map2leftcdr[3]=2;
	testQTree.map2rightcdr[2]=116;
	testQTree.map2rightcdr[3]=10698;
	testQTree.map2parent[10698]=3;
	testQTree.map2parent[2]=3;
    testQTree.map2parent[1]=2;
	testQTree.map2parent[116]=2;
	testQTree.map2parent[11807]=1;
	testQTree.terminals_index = {0, 2, 4};
	testQTree.junction_index = {3, 5};

    int seed_node;
    //DBLP:
    //std::vector<int> seed_candidate = {2151771, 2151721, 2151652, 2151623};
    //ENRON:
    std::vector<int> seed_candidate = {27537, 32657, 46259, 44703, 44828};

    for(int i = 0; i<seed_candidate.size(); i++){
        seed_node = seed_candidate[i];
        cout<<"current seed is"<<seed_node<<endl;
        sampledTree = sampleFrom(G, seed_node);
        if (sampledTree.nodes_ordered[0]!= 9999){
			testQTree = sampledTree;
			std::cout<<"sampling success: found a query tree!"<<endl;
			for (int i=0; i< testQTree.nodes_ordered.size();i++){
				cout<<testQTree.nodes_ordered[i]<<' ';
			}
			break;
        }
    }



	for (int i=0; i<testQTree.nodes_ordered.size(); i++){
        testQTree.map2patthern.insert(make_pair(testQTree.nodes_ordered[i], testQTree.patterns[i]));
	}
	for (int i=0; i<testQTree.junction_index.size();i++){
        testQTree.junctions.push_back(testQTree.nodes_ordered[testQTree.junction_index[i]]);
	}
	cout<<testQTree.junctions.size()<<endl;
    for (int i=0; i<testQTree.terminals_index.size();i++){
        testQTree.terminals.push_back(testQTree.nodes_ordered[testQTree.terminals_index[i]]);
	}
//	testQTree.Edges_types = { {make_pair(10698, 2), 1}, {make_pair(2, 1), 2}, {make_pair(1, 10388), 1}, {make_pair(1, 11807), 1} };
    QueryResultTrees qResult = AStar_Prophet_Tree(G,testQTree,pTime2);


    if(qResult.trees.size()>0){
        for (int i=0; i<qResult.trees.size(); i++){
                cout  <<i<<" th lightest tree has weight: "<< qResult.trees[i].wgt << "\t" << qResult.mem << "\t" << qResult.totalTrees << endl;
        }
    }

	else
        cout << -1 << "\t" << qResult.mem << "\t" << qResult.totalTrees << endl;
	cout << "#################################################"<< endl;

    return 0;
}

/*********
int main (int argc, char **argv) {

	char *in_fname = argv[1];
	double timeDiff = 0.0;
	double pTime1 = 0, pTime2 = 0;//time for establishing prophet graph.
	struct timeval start_before_load,start, time1, time2, startQuery, endQuery;
	gettimeofday(&start_before_load,NULL);

	PrunedLandmarkLabeling<> pll;
	vector<pair<int, int>> edge_list;

	graph_t G = load_graph(in_fname, edge_list);//loading the graph.
	cout << "# nodes: " << G.n << ".  # edges: " << G.neighbors.size()/2 << endl;
	gettimeofday(&start,NULL);
	double time_load = (start.tv_sec + double(start.tv_usec)/1000000) - (start_before_load.tv_sec + double(start_before_load.tv_usec)/1000000);//seconds..
	cout<<"Time to load the graph = "<<time_load<<endl;

	string lineStr;
	ifstream qfile(argv[2]);
	string outputFile(argv[2]);
	int choice = atoi(argv[3]);//define the choice
	string choiceStr(argv[3]);
//	std::ostringstream ss;
	int numOfSkip = 0;
	if(argc >= 5)
		numOfSkip = atoi(argv[4]);
	ostringstream tmpss;
	tmpss << numOfSkip;
	string tmpstr = outputFile +"."+choiceStr+"-"+tmpss.str()+".result.txt";
	if( (choice>=5 && choice<=8)||(choice==15) ){
		cout << "**** It is going to be ORACLE methods.***" << endl;
		//build the index for the distance oracle.
		cout << "Start building the oracle..." << endl;
		gettimeofday(&start_before_load,NULL);
		pll.ConstructIndex(edge_list);
		gettimeofday(&start,NULL);
		double time_index = (start.tv_sec + double(start.tv_usec)/1000000) - (start_before_load.tv_sec + double(start_before_load.tv_usec)/1000000);
		cout << "Time to build the exact distance oracle index = "<< time_index << endl;
	}
	ofstream ofs0 (tmpstr.c_str(), std::ofstream::out);//creating output stream.
	cout << "Executing queries ..." << endl;
	int queryNum = 10000;
	int countQuery = 0;
	int numPath = 0;
	while(getline(qfile, lineStr) && countQuery<queryNum)	{//for each query.
//		cout << lineStr << endl;
		countQuery ++;
		if(countQuery <= numOfSkip){
			getline(qfile, lineStr);
			continue;}
		gettimeofday(&startQuery, NULL);
		Query testQ;
	        vector<int> pattern;
		istringstream iss(lineStr);
		for(int num; iss>>num;)
			pattern.push_back(num);
		testQ.time = 2015;//default
		if(getline(qfile, lineStr)){
			iss.clear();
			iss.str(lineStr);
			iss  >> testQ.src >> testQ.tgt >> testQ.time;
		}
		else break;
		testQ.pattern = pattern;
		QueryResult qResult;
		vector<int> resultVec;
		//NEXT: run all the methods.
		gettimeofday(&time1, NULL);


		switch(choice){
			case 1:
				//dfs method.
//				cout << "1) Plain iterative DFS...." << endl;
				qResult = DFM(G, pll, testQ, WITHOUT_ORACLE);
				break;
			case 2:
				//Plain iterative BFS
//				cout << "2) Plain iterative BFS...." << endl;
				qResult = BFM(G, pll, testQ, WITHOUT_ORACLE);
				break;
			case 3:
//				cout << "3) Bi-directional bfs (without distanc oracle)...." << endl;
				qResult = bidirectional_BFM(G, pll, testQ, WITHOUT_ORACLE);
				break;
			case 4:
				//dijkstra's algorithm
//				cout << "4) Dijkstra's algorithm...." << endl;
				qResult = dijkstra(G, pll, testQ, WITHOUT_ORACLE);
				break;
			case 5:
//				cout << "5) DFS with exact distance oracle...." << endl;
				qResult = DFM(G, pll, testQ, WITH_ORACLE);
				break;
			case 6:
//				cout << "6) BFS with exact distance oracle...." << endl;
				qResult = BFM(G, pll, testQ, WITH_ORACLE);
				break;
			case 7:
//				cout << "7) Bi-directional bfs with exact distance oracle...." << endl;
				qResult = bidirectional_BFM(G, pll,testQ, WITH_ORACLE);
				break;
			case 8:
//				cout << "8) Dijkstra with exact distance oracle...." << endl;
                		qResult = dijkstra(G, pll, testQ, WITH_ORACLE);
				break;
			case 9:
//				cout << "9) Bi-directional bfs on prophet graph...." << endl;
		                qResult = bidirectional_BFM_Prophet_IM(G, testQ, pTime1);
				break;
			case 10:
//                              This is the method in the WWW paper (PRO-HEAPS).
//				cout << "10) A Star search with Prophet set...." << endl;
				qResult = AStar_Prophet(G,testQ,pTime2);
				check_path_wgt(qResult.paths, G, testQ);

				break;
			case 11:
//				cout << "11) Dijkstra algorithm with Prophet ..." << endl;
				qResult = dijkstra_with_Prophet(G,testQ);
				break;
			case 12:
//				cout << "12) Bi-directional explicit prophet...." << endl;
				qResult = bidirectional_BFM_Prophet_EX(G, testQ);
				break;
			default:
				cout << "Wrong choice." << endl;
		}


		gettimeofday(&time2, NULL);
		numPath = qResult.numPaths;
		timeDiff = (time2.tv_sec + double(time2.tv_usec)/1000000) - (time1.tv_sec + double(time1.tv_usec)/1000000);
		print2File(qResult, timeDiff, ofs0) ;
		ofs0 <<numPath;
		if(pTime1 != 0)
			ofs0<< "\t" << pTime1 << endl;
		else if(pTime2 !=0 )
			ofs0<< "\t" << pTime2 << endl;
		else
			ofs0 << endl;
		ofs0.flush();

/*		gettimeofday(&endQuery, NULL);
		timeDiff = (endQuery.tv_sec + double(endQuery.tv_usec)/1000000) - (startQuery.tv_sec + double(startQuery.tv_usec)/1000000);
		if (timeDiff > 3600*12)
			queryNum = 4;
		else if(timeDiff > 3600*4)
			queryNum = 12;
		else if(timeDiff > 3600)
			queryNum = 48;
		else if(timeDiff*1000 > 3600*48)
			queryNum = 100;*/
/**********
	}//end of reading query file.
	ofs0.close();

	return 0;
}

***********/
