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
	struct timeval start_before_load,start, time1, time2, time3, time4, startQuery, endQuery;
	gettimeofday(&start_before_load,NULL);

	PrunedLandmarkLabeling<> pll;
	vector<pair<int, int>> edge_list;

	graph_t G = load_graph(in_fname, edge_list);//loading the graph.
	cout << "# nodes: " << G.n << ".  # edges: " << G.neighbors.size()/2 << endl;
	gettimeofday(&start,NULL);
	Query_tree testQTree;
	Query_tree sampledTree;


	 //TEST CASE0 PASSED.
 // with instance maching
 /*

    testQTree.patterns = {0,1,1}; //this is node pattern. post-order 22211. edge has a label too.
	testQTree.nodes_ordered = {0,1,5}; //non-terminal nodes assigned different values for edge distinction
	testQTree.map2leftcdr[5]=0;
	testQTree.map2rightcdr[5]=1;
	testQTree.map2parent[0]=5;
	testQTree.map2parent[1]=5;
	testQTree.terminals_index = {0, 1};
	testQTree.junction_index = {2};

*/

//TEST CASE 1, 2: Random select a seed and grow a tree out from there.
int seed_node = 0;
int trans_seed = 0;
//TEST CASE1 - DBLP:
//std::vector<int> seed_candidate = {2151771, 2151721, 2151652, 2151623};

//TEST CASE2 - ENRON:
    std::vector<int> seed_candidate = {27537, 32657, 46259, 44703, 44828};

//TEST 3 - CLOUD:
//	std::vector<int> seed_candidate = {0, 1, 4, 7, 11,14};

    for(int i = 0; i<seed_candidate.size(); i++){
        seed_node = seed_candidate[i];
        cout<<"current seed is"<<seed_node<<endl;
        sampledTree = sampleFrom(G, seed_node);
        if (sampledTree.nodes_ordered[0]!= 9999){
			testQTree = sampledTree;
			trans_seed = seed_node;
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

	gettimeofday(&time1, NULL);
	//query the pattern
    QueryResultTrees qResult = AStar_Prophet_Tree(G,testQTree,pTime2); //pTime2 is only useful if the weight depends on recency.

    gettimeofday(&time2, NULL);
	int numtree = qResult.numTrees; //the search space: number of trees generated
	timeDiff = (time2.tv_sec + double(time2.tv_usec)/1000000) - (time1.tv_sec + double(time1.tv_usec)/1000000);

    if(qResult.trees.size()>0){
        for (int i=0; i<qResult.trees.size(); i++){
                cout  <<i<<" th lightest tree has weight: "<< qResult.trees[i].wgt << "\t" << qResult.mem << "\t" <<numtree << "\t" << qResult.totalTrees << "\t" <<"time--"<<timeDiff<<endl;
        }
    }

	else
        cout << -1 << "\t" << qResult.mem << "\t" << qResult.totalTrees << endl;
	cout << "#################################################"<< endl;


	/////////////////compare to: same sized path case.
	Query testQ = Transform_2line(G, testQTree, trans_seed);
	gettimeofday(&time3, NULL);
	QueryResult qResult_comp = AStar_Prophet(G,testQ,pTime2);
	gettimeofday(&time4, NULL);
	double timeDiff2 = (time4.tv_sec + double(time4.tv_usec)/1000000) - (time3.tv_sec + double(time3.tv_usec)/1000000);

	if(qResult_comp.paths.size()>0){
        for (int i=0; i<qResult_comp.paths.size(); i++){
                cout  <<i<<" th lightest line has weight: "<< qResult_comp.paths[i].wgt << "\t" << qResult_comp.mem << "\t" <<qResult_comp.numPaths << "\t" << qResult_comp.totalPaths << "\t" <<"time--"<<timeDiff2<<endl;
        }
    }

	else
        cout << -1 << "\t" << qResult_comp.mem << "\t" << qResult_comp.totalPaths << endl;
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
