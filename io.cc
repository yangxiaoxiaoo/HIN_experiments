#include "global.h"
#include "algorithm"
using namespace std;


Query_tree readfromfile(char *fname){

    Query_tree bi_tree;


	std::ifstream infile(fname, std::ios::in);
	if (!infile) {
		std::cerr <<"can't open " << fname <<"\n";
		assert(false);
	}


	int maxlen = 100000; int cnt = 0; int chatfreq = 2000000;
	char buf[maxlen];

	//read unordered_map<int, int> map2leftcdr
	infile.getline(buf, maxlen);
	if (infile.eof()) cout<<"wrong query format!"<<endl;
	char* nums = strtok(buf, " ");
	int num = atoi(nums);
	char* pairs1;
	char* pairs2;
	for (int i= 0; i < num; i++){
		pairs1 = strtok(NULL, " ");
		pairs2 = strtok(NULL, " ");
		bi_tree.map2leftcdr[atoi(pairs1)] = atoi(pairs2);
	}

	//read unordered_map<int, int> map2rightcdr;
	infile.getline(buf, maxlen);
	if (infile.eof()) cout<<"wrong query format!"<<endl;
	nums = strtok(buf, " ");
	num = atoi(nums);
	for (int i= 0; i < num; i++){
		pairs1 = strtok(NULL, " ");
		pairs2 = strtok(NULL, " ");
		bi_tree.map2rightcdr[atoi(pairs1)] = atoi(pairs2);
	}

	//read unordered_map<int, int> map2parent;
	infile.getline(buf, maxlen);
	nums = strtok(buf, " ");
	num = atoi(nums);
	for (int i= 0; i < num; i++){
		pairs1 = strtok(NULL, " ");
		pairs2 = strtok(NULL, " ");
		bi_tree.map2parent[atoi(pairs1)] = atoi(pairs2);
	}

	//read 	vector<int> nodes_ordered;
	infile.getline(buf, maxlen);
	if (infile.eof()) cout<<"wrong query format!"<<endl;
	nums = strtok(buf, " ");
	num = atoi(nums);
	char* node;
	for (int i= 0; i < num; i++){
		node = strtok(NULL, " ");
		bi_tree.nodes_ordered.push_back(atoi(node));
	}

	//read vector<int> terminals_index;
	infile.getline(buf, maxlen);
	if (infile.eof()) cout<<"wrong query format!"<<endl;
	nums = strtok(buf, " ");
	num = atoi(nums);
	char* index;
	for (int i= 0; i < num; i++){
		index = strtok(NULL, " ");
		bi_tree.terminals_index.push_back(atoi(index));
	}


	//read vector<int> vector<int> terminals;
	infile.getline(buf, maxlen);
	if (infile.eof()) cout<<"wrong query format!"<<endl;
	nums = strtok(buf, " ");
	num = atoi(nums);

	for (int i= 0; i < num; i++){
		node = strtok(NULL, " ");
		bi_tree.terminals.push_back(atoi(node));
	}

	//vector<int> junction_index;
	infile.getline(buf, maxlen);
	if (infile.eof()) cout<<"wrong query format!"<<endl;
	nums = strtok(buf, " ");
	num = atoi(nums);
	for (int i= 0; i < num; i++){
		index = strtok(NULL, " ");
		bi_tree.junction_index.push_back(atoi(index));
	}

	//read vector<int>   vector<int> junctions;
	infile.getline(buf, maxlen);
	if (infile.eof()) cout<<"wrong query format!"<<endl;
	nums = strtok(buf, " ");
	num = atoi(nums);
	for (int i= 0; i < num; i++){
		node = strtok(NULL, " ");
		bi_tree.junctions.push_back(atoi(node));
	}

	//vector<int> patterns;
	infile.getline(buf, maxlen);
	if (infile.eof()) cout<<"wrong query format!"<<endl;
	nums = strtok(buf, " ");
	num = atoi(nums);
	for (int i= 0; i < num; i++){
		node = strtok(NULL, " ");
		bi_tree.patterns.push_back(atoi(node));
	}

	unordered_map<int, int> map2pattern;
	for (int i = 0; i < bi_tree.nodes_ordered.size(); i++){
		map2pattern[bi_tree.nodes_ordered[i]] = bi_tree.patterns[i];
	}
	bi_tree.map2patthern = map2pattern;




	return bi_tree;

}


//Loads the graph: index should be continuous. But edge list does not need to be sorted.
graph_t load_graph(char *fname, vector<pair<int, int>> &edge_list) {
	graph_t g;
	std::ifstream infile(fname, std::ios::in);
	if (!infile) {
		std::cerr <<"can't open " << fname <<"\n";
		assert(false);
	}

	int n1, n2;
	vector<pair<pair<int, int>,vector<float> > > edges;
	int maxlen = 100000; int cnt = 0; int chatfreq = 2000000;
	char buf[maxlen];
	int nodeNum = 0, edgeNum=0;//edgeNum does not matter in input file.
	int maxId = -1, minId = 100;
	while(1) {
		cnt += 1;
		if ((cnt % chatfreq) == 0)
		std::cerr <<"reading line " << cnt <<"\n";
		infile.getline(buf, maxlen);
		if (infile.eof()) break;
		if(cnt ==1){//first line is meta information on node/edge
			char* str1 = strtok(buf, " ");
			char* str2 = strtok(NULL, " ");
			nodeNum = atoi(str1);
//			edgeNum = atoi(str2);
			g.typeMap.reserve(nodeNum+1);
			std::vector<int> v0(nodeNum+1, 0);
			g.typeMap = v0;

		}
		if(cnt > 1){
			edgeNum += 1;
			char* sfrom = strtok(buf, " ");
			char* sto = strtok(NULL, " ");
			char* ntype1 = strtok(NULL, " ");
			char* ntype2 = strtok(NULL, " ");
			char* ntype3 = strtok(NULL, " ");

			memcpy(ntype1, &ntype1[1], strlen(ntype1)-2);
			ntype1[strlen(ntype1)-2] = '\0';
			memcpy(ntype2, &ntype2[1], strlen(ntype2)-2);
			ntype2[strlen(ntype2)-2] = '\0';
			vector<float> wgt;
			string s(strtok(NULL, "\n"));
			std::string delimiter = "_";
			size_t pos = 0;
			std::string token;
			while ((pos = s.find(delimiter)) != std::string::npos) {
			    token = s.substr(0, pos);
			    wgt.push_back(stof(token));
			    s.erase(0, pos + delimiter.length());
			}
			wgt.push_back(stof(s));//read the weight.
			if (sto==NULL){
				std::cerr <<"file " << fname <<" problem on line " << cnt <<"\n"; exit(-1);}

			n1 = atoi(sfrom);
			n2 = atoi(sto);
			int t1 = atoi(ntype1);
			int t2 = atoi(ntype2);
			//TEST passed oct 02
	//		cout<<"LOAD MAP TEST " <<t1 <<" "<<t2<<endl;
			//float wgt = atof(sweight);
			g.typeMap[n1] = t1;
			g.typeMap[n2] = t2;
			//TEST passes oct 02
		//	for (int i= 1; i < g.typeMap.size(); i++){
		//		cout<<g.typeMap[i]<<endl;
		//	}
			if(max(n1,n2) > maxId)
				maxId = max(n1,n2);
			if(min(n1,n2) < minId)
				minId = min(n1, n2);
			edges.push_back(make_pair(make_pair(n1,n2), wgt));
			edges.push_back(make_pair(make_pair(n2,n1), wgt));
			edge_list.push_back(make_pair(n1, n2));
		}
	}

  	std::sort(edges.begin(),edges.end());//edges contains each pair twice.
	vector<pair<pair<int,int>,vector<float> > >::iterator iter = --edges.end();
	g.n = (*iter).first.first+1;//the index of last node plus 1 is the number of nodes.
	cout << "Node Number:" <<g.n << endl;
	cout << "Edge Number:" <<edgeNum << endl;
	cout << "Max Number id: " << maxId << "; Min Number id: " << minId << endl;
	g.nodes.reserve(g.n);
	g.neighbors.reserve(edges.size());//edges have been double, so it is correct.
	g.wgts.reserve(edges.size());
	iter = edges.begin();
	g.nodes.push_back(0);
	g.neighbors.push_back((*iter).first.second);
	g.wgts.push_back((*iter).second);
	++iter;
	for(;iter != edges.end();++iter){
		int src = (*iter).first.first;
		int tgt = (*iter).first.second;
		vector<float> wgt = (*iter).second;
		if(src!=g.nodes.size()-1){//if src is not the last node.
			while(src != g.nodes.size()-1)//this is to fix the case that some node ids are not strictly continuous. e.g 1, 3, 4.
				g.nodes.push_back(g.neighbors.size());//point to the first node of neighborhood.
			g.neighbors.push_back(tgt);
			g.wgts.push_back(wgt);
		}
		else if(tgt!=g.neighbors[g.neighbors.size()-1]){//avoid repeating edges. But self-loop is allowed.
			g.neighbors.push_back(tgt);
			g.wgts.push_back(wgt);
		}
	}
	for(int i=0; i<g.n-1; i++){
		int deg = g.nodes[i+1]-g.nodes[i];
		g.degree.push_back(deg);
  	}
	g.degree.push_back(g.neighbors.size()-g.nodes[g.n-1]);
	return g;
}
void print2Screen(QueryResult qResult, double timeDiff){
	 if(qResult.paths.size()>0)
		cout << timeDiff << "\t" << qResult.paths[0].wgt << "\t" << qResult.mem << "\t" << qResult.totalPaths << endl;
	else
                cout << timeDiff << "\t" << -1 << "\t" << qResult.mem << "\t" << qResult.totalPaths << endl;
	cout << "#################################################"<< endl;
}
void print2File(QueryResult qResult, double timeDiff, ofstream& ofs){
	 if(qResult.paths.size()>0)
		ofs << timeDiff << "\t" << qResult.paths[0].wgt << "\t" << qResult.mem << "\t" << qResult.totalPaths << "\t";
	else
                ofs << timeDiff << "\t" << -1 << "\t" << qResult.mem << "\t" << qResult.totalPaths << "\t";
}

void print2FileTree(QueryResultTrees qResultT, double timeDiff, ofstream& ofs){
	 if(qResultT.trees.size()>0){
		ofs << timeDiff << "\t" << qResultT.trees[0].wgt << "\t" << qResultT.mem <<"\t" << qResultT.numTrees<< "\t" << qResultT.totalTrees << "\t";
	//	cout  <<i<<" th lightest tree has weight: "<< qResult1.trees[i].wgt << "\t" << qResult.mem << "\t" <<numtree << "\t" << qResult1.totalTrees << "\t" <<"time--"<<timeDiff1<<endl;
        }
	else
                ofs << timeDiff << "\t" << -1 << "\t" << qResultT.mem << "\t" << qResultT.totalTrees << "\t";
}

