#include "global.h"
#include <cassert>
using namespace std;



vector<int> reverse_postorder(int root, unordered_map<int, int> map2leftcdr,
	unordered_map<int, int> map2rightcdr){
	vector<int> result;
	result.push_back(root);
    while ((map2leftcdr.find(root)!=map2leftcdr.end()) || map2rightcdr.find(root)!=map2rightcdr.end()){
    //while there is a left child or a right child of root
        if  (map2rightcdr.find(root)!=map2rightcdr.end()){
            vector<int> right_sub = reverse_postorder(map2rightcdr[root], map2leftcdr, map2rightcdr);
            result.insert(result.end(), right_sub.begin(), right_sub.end());
        }

        if  (map2leftcdr.find(root)!=map2leftcdr.end()){
            vector<int> left_sub = reverse_postorder(map2leftcdr[root], map2leftcdr, map2rightcdr);
            result.insert(result.end(), left_sub.begin(), left_sub.end());
        }
    }
    return result;
}

/*
Non_bi_tree_instance non_bi_instance(Instance_Tree binary_instance){

}

*/


Query_tree binaryfy(Non_bi_tree tree){

    /*
    std::unordered_map<int, vector int > map2chr;
    std::unordered_map<int, int> map2parent;
	std::unordered_map<int, int> map2patthern;

/////////////CONVERT TO//////////////////////

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

	*/

    Query_tree bi_tree;

    unordered_map<int, int> map2leftcdr;
	unordered_map<int, int> map2rightcdr;
	unordered_map<int, int> map2parent;
	unordered_map<int, int> map2pattern;

	vector<int> nodes_ordered; //unknown set as 0.
    vector<int> terminals_index; //the position of n terminals in the ordered nodes
    vector<int> junction_index; //the position of (n-1) junction nodes
    vector<int> junctions;
    vector<int> terminals;
	vector<int> patterns;//the type of nodes



    //STEP1: prepare variables
    map2pattern = tree.map2pattern;

	int new_node_id = 0;
	for (auto it : tree.map2chr){
	    if (it.first > new_node_id){
            new_node_id = it.first;
	    }
	}
	new_node_id ++;
	//new_node_id is now a safe id that is not any used label

	int root;
    unordered_set<int> terminalset;
	unordered_set<int> junctionset;
	//decide what are the terminals and root:
	for (auto it: tree.map2pattern){
        if (tree.map2chr.find(it.first)==tree.map2chr.end()){ //any node that does not have chr is a terminal
            terminalset.insert(it.first);
        }
        if (tree.map2parent.find(it.first)== tree.map2parent.end())
            root = it.first;
	}


    //build a binary tree structure mapping, and decide what are the junction nodes
    for (auto it : tree.map2chr){
        int node = it.first;
        int left_child, right_child;
        while (it.second.size()>= 2){ //more then 2 children: insert dummy node
                left_child = it.second.back();
                it.second.pop_back();
                right_child = it.second.back();
                it.second.pop_back();
                int dummy_node = new_node_id;
                new_node_id ++; //update new_node_id after using it as new label, to make sure it is always safe
                map2leftcdr[dummy_node] = left_child;
                map2rightcdr[dummy_node] = right_child;
                map2parent[left_child] = dummy_node;
                map2parent[right_child] = dummy_node;
                it.second.push_back(dummy_node);  //delete two children, add dummy node as a children
        }
        if (it.second.size()< 2){ //always true after the while loop.
            if (it.second.size()== 2){
                left_child = it.second[0];
                right_child = it.second[1];
                map2leftcdr[node] = left_child;
                map2rightcdr[node] = right_child;
                map2parent[left_child] = node;
                map2parent[right_child] = node;
                junctionset.insert(node);
            }
            if (it.second.size() == 1){//path node
                left_child = it.second[0];
                map2leftcdr[node] = left_child;
                map2parent[left_child] = node;
            }
            if (it.second.size() == 0){
                int error_inst;
                cout << "warning!! list of children is empty, invalid tree input."<<endl;
                cin >> error_inst;
            }
        }
    }

    //STEP2: traverse from root down in reverse-post-order
    nodes_ordered = reverse_postorder(root, map2leftcdr, map2rightcdr);
    reverse(nodes_ordered.begin(), nodes_ordered.end());

    //STEP3: by this pose-order, count index of terminals and junctions and patterns
    int i;
    for (i = 1; i<nodes_ordered.size(); i++){
        patterns.push_back(map2pattern[nodes_ordered[i]]);
        if (terminalset.find(nodes_ordered[i])!=terminalset.end()){
            terminals_index.push_back(i);
            terminals.push_back(nodes_ordered[i]);
        }
        if (junctionset.find(nodes_ordered[i])!=junctionset.end()){
            junction_index.push_back(i);
            junctions.push_back(nodes_ordered[i]);
        }
    }
    bi_tree.junctions = junctions;
    bi_tree.junction_index = junction_index;
    bi_tree.map2leftcdr = map2leftcdr;
    bi_tree.map2parent = map2parent;
    bi_tree.map2patthern = map2pattern;
    bi_tree.map2rightcdr = map2rightcdr;
    bi_tree.nodes_ordered = nodes_ordered;
    bi_tree.patterns = patterns;
    bi_tree.terminals = terminals;
    bi_tree.terminals_index = terminals_index;
    bi_tree.time = tree.time;

    return bi_tree;

}



//Unfinished
vector<Shape> Auto_shapes(const graph_t& g, int seed_node, int shape){
//automatically generate shapes. a shape is described by an adjacency matrix.
    vector<Shape> shapelist;
    for(int nodenum = 4; nodenum < 10 ;nodenum ++){

    }

    return shapelist;
}


//Unfinished
vector<Query_tree> Auto_sampleFrom(const graph_t& g, int seed_node, Shape thisshape){
//automatically generate a list of terminal sets with a given shape. No guarantee on existence of such subgraph
//reach out from seed to the neighbors BFS way
    vector<Query_tree> QTreelist;
    Query_tree QTree;
    //BFS from seed.

    int curnode = thisshape.seed_index;

    //for each curnode, use the child_count to tell if this is a terminal--if #child == 0, then it is.
    //if degree>3, then this shape is not valid, return empty vector.
    bool is_terminal = false;
    int child_count = 0;
    for (int i=0; i<thisshape.nodenum; i++){



        if (thisshape.adj[curnode][i]){
            if (QTree.map2parent[curnode] == i){ //this new found i is parent: do nothing.
                continue;
            }

            if (child_count == 0){ //necer added before, add as left child
                QTree.map2leftcdr[curnode] = i;
                QTree.map2parent[i] = curnode;
                child_count +=1;
            }
            if (child_count == 1){ //added once, add now as a right child
                QTree.map2rightcdr[curnode] = i;
                QTree.map2parent[i] = curnode;
                child_count += 1;
            }
            if (child_count == 2){ //new found after having already two child. Illegal case
                return QTreelist; //Return empty list.

            }
        }
    }
    if (child_count == 0){
        is_terminal = true; //curnode is a terminal.
    }
}







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
                return QTree;
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
                return QTree;
            }
        }
    }

     if (shape == 3){ //shape3: intersection of two l=4 path with a l=5 path
        int n1,n2, n3, n4, n5, n6, n7, n8, n9, n10, n11;
        vector<int>added_neighs;
        if (seed_node >= g.degree.size()){//seed_node not in graph
            return QTree;
        }
        added_neighs.push_back(seed_node);
        if (g.degree[seed_node] >= 2){
            n1 = g.neighbors[g.nodes[seed_node]];
            n5 = g.neighbors[g.nodes[seed_node]+1];
            added_neighs.push_back(n1);
            added_neighs.push_back(n5);
            if (g.degree[n1]>= 2 && g.degree[n5]>=3){
                n2 = g.neighbors[g.nodes[n1]];
                if(g.degree[n2]>=2 && find(added_neighs.begin(), added_neighs.end(), n2) == added_neighs.end()){ //n2 hasn't been added
                    added_neighs.push_back(n2);
                    n3 = g.neighbors[g.nodes[n2]];
                    if(g.degree[n3]>=2 && find(added_neighs.begin(), added_neighs.end(), n3) == added_neighs.end()){
                        added_neighs.push_back(n3);
                        n4 = g.neighbors[g.nodes[n3]];
                        if (find(added_neighs.begin(), added_neighs.end(), n4) == added_neighs.end()){
                            added_neighs.push_back(n4);
                        }
                        else return QTree;
                    }
                    else return QTree;
                }
                else return QTree;

                n6 = g.neighbors[g.nodes[n5]];
                n9 = g.neighbors[g.nodes[n5]+1];
                if (g.degree[n6]>=2 && g.degree[n9]>=2 && find(added_neighs.begin(), added_neighs.end(), n6) == added_neighs.end()
                    && find(added_neighs.begin(), added_neighs.end(),n9) == added_neighs.end()){
                    added_neighs.push_back(n6);
                    added_neighs.push_back(n9);
                    n7 = g.neighbors[g.nodes[n6]];
                    n10 = g.neighbors[g.nodes[n9]];
                    if (g.degree[n7]>=2 && g.degree[n10]>=2 && find(added_neighs.begin(), added_neighs.end(), n7) == added_neighs.end()
                        && find(added_neighs.begin(), added_neighs.end(),n10) == added_neighs.end()){
                        added_neighs.push_back(n7);
                        added_neighs.push_back(n10);
                        n8 = g.neighbors[g.nodes[n7]];
                        n11 = g.neighbors[g.nodes[n10]];
                        if (find(added_neighs.begin(), added_neighs.end(), n8) == added_neighs.end()
                        && find(added_neighs.begin(), added_neighs.end(),n11) == added_neighs.end()){
                            QTree.nodes_ordered = {n4, n3, n2, n1, n8, n7, n6, n11, n10, n9, n5, seed_node};
                            QTree.map2leftcdr[seed_node] = n1;
                            QTree.map2leftcdr[n1] = n2;
                            QTree.map2leftcdr[n2] = n3;
                            QTree.map2leftcdr[n3] = n4;
                            QTree.map2leftcdr[n5] = n6;
                            QTree.map2leftcdr[n6] = n7;
                            QTree.map2leftcdr[n7] = n8;
                            QTree.map2rightcdr[seed_node] = n5;
                            QTree.map2rightcdr[n5] = n9;
                            QTree.map2rightcdr[n9] = n10;
                            QTree.map2rightcdr[n10] = n11;
                            QTree.map2parent[n4] = n3;
                            QTree.map2parent[n3] = n2;
                            QTree.map2parent[n2] = n1;
                            QTree.map2parent[n1] = seed_node;
                            QTree.map2parent[n8] = n7;
                            QTree.map2parent[n7] = n6;
                            QTree.map2parent[n6] = n5;
                            QTree.map2parent[n5] = seed_node;
                            QTree.map2parent[n11] = n10;
                            QTree.map2parent[n10] = n9;
                            QTree.map2parent[n9] = n5;
                            QTree.terminals_index = {0, 4, 7};
                            QTree.junction_index = {10, 11};
                            QTree.patterns = {g.typeMap[n4], g.typeMap[n3], g.typeMap[n2],g.typeMap[n1],
                                g.typeMap[n8], g.typeMap[n7], g.typeMap[n6],g.typeMap[n11],
                                g.typeMap[n10], g.typeMap[n9], g.typeMap[n5],g.typeMap[seed_node]};
                            return QTree;

                        }
                        else return QTree;
                    }
                    else return QTree;
                }
                else return QTree;
            }
            else return QTree;
        }
        else return QTree;
    }
    //end of shape 3
    if (shape == 4){ //shape3 without 4, 8, 11
        int n1,n2, n3, n5, n6, n7, n9, n10;
        vector<int>added_neighs;
        if (seed_node >= g.degree.size()){//seed_node not in graph
            return QTree;
        }
        added_neighs.push_back(seed_node);
        if (g.degree[seed_node] >= 2){
            n1 = g.neighbors[g.nodes[seed_node]];
            n5 = g.neighbors[g.nodes[seed_node]+1];
            added_neighs.push_back(n1);
            added_neighs.push_back(n5);
            if (g.degree[n1]>= 2 && g.degree[n5]>=3){
                n2 = g.neighbors[g.nodes[n1]+1];
                if(g.degree[n2]>=2 && find(added_neighs.begin(), added_neighs.end(), n2) == added_neighs.end()){ //n2 hasn't been added
                    added_neighs.push_back(n2);
                    n3 = g.neighbors[g.nodes[n2]];
                    if(find(added_neighs.begin(), added_neighs.end(), n3) == added_neighs.end()){
                        added_neighs.push_back(n3);
                    }
                    else return QTree;
                }
                else return QTree;

                n6 = g.neighbors[g.nodes[n5]+1];
                n9 = g.neighbors[g.nodes[n5]+2];
                if (g.degree[n6]>=2 && g.degree[n9]>=2 && find(added_neighs.begin(), added_neighs.end(), n6) == added_neighs.end()
                    && find(added_neighs.begin(), added_neighs.end(),n9) == added_neighs.end()){
                    added_neighs.push_back(n6);
                    added_neighs.push_back(n9);
                    n7 = g.neighbors[g.nodes[n6]+1];
                    n10 = g.neighbors[g.nodes[n9]+1];
                    if ( find(added_neighs.begin(), added_neighs.end(), n7) == added_neighs.end()
                        && find(added_neighs.begin(), added_neighs.end(),n10) == added_neighs.end()){
                        added_neighs.push_back(n7);
                        added_neighs.push_back(n10);

                        QTree.nodes_ordered = {n3, n2, n1,  n7, n6,n10, n9, n5, seed_node};
                        QTree.map2leftcdr[seed_node] = n1;
                        QTree.map2leftcdr[n1] = n2;
                        QTree.map2leftcdr[n2] = n3;
                        QTree.map2leftcdr[n5] = n6;
                        QTree.map2leftcdr[n6] = n7;
                        QTree.map2rightcdr[seed_node] = n5;
                        QTree.map2rightcdr[n5] = n9;
                        QTree.map2rightcdr[n9] = n10;
                        QTree.map2parent[n3] = n2;
                        QTree.map2parent[n2] = n1;
                        QTree.map2parent[n1] = seed_node;
                        QTree.map2parent[n7] = n6;
                        QTree.map2parent[n6] = n5;
                        QTree.map2parent[n5] = seed_node;
                        QTree.map2parent[n10] = n9;
                        QTree.map2parent[n9] = n5;
                        QTree.terminals_index = {0, 3, 5};
                        QTree.junction_index = {7, 8};
                        QTree.patterns = {g.typeMap[n3], g.typeMap[n2],g.typeMap[n1],
                             g.typeMap[n7], g.typeMap[n6],
                            g.typeMap[n10], g.typeMap[n9], g.typeMap[n5],g.typeMap[seed_node]};
                            return QTree;
                    }
                    else return QTree;
                }
                else return QTree;
            }
            else return QTree;
        }
        else return QTree;
    }
    //end of shape 4

    if (shape == 5){ //a line
        int n1,n2, n3, n5, n9;
        vector<int>added_neighs;
        if (seed_node >= g.degree.size()){//seed_node not in graph
            return QTree;
        }
        added_neighs.push_back(seed_node);
        if (g.degree[seed_node] >= 2){
            n1 = g.neighbors[g.nodes[seed_node]];
            n5 = g.neighbors[g.nodes[seed_node]+1];
            cout<<"n1="<<n1<<" n5 ="<<n5<<endl;

            added_neighs.push_back(n1);
            added_neighs.push_back(n5);
            if (g.degree[n1]>= 2 && g.degree[n5]>=2){
                n2 = g.neighbors[g.nodes[n1]+1];
                cout<<"n2="<<n2<<", degree = "<<g.degree[n2]<<endl;

                if(g.degree[n2]>=2 && find(added_neighs.begin(), added_neighs.end(), n2) == added_neighs.end()){ //n2 hasn't been added
                    added_neighs.push_back(n2);
                    n3 = g.neighbors[g.nodes[n2]+1];
                    cout<<"n3= "<<n3<<endl;

                    if(find(added_neighs.begin(), added_neighs.end(), n3) == added_neighs.end()){
                        added_neighs.push_back(n3);
                    }
                    else return QTree;
                }
                else return QTree;


                n9 = g.neighbors[g.nodes[n5]+1];
                if(find(added_neighs.begin(), added_neighs.end(),n9) == added_neighs.end()){


                        cout<<" n9= "<<n9<<endl;


                        QTree.nodes_ordered = {n3, n2, n1,  n9, n5, seed_node};
                        QTree.map2leftcdr[seed_node] = n1;
                        QTree.map2leftcdr[n1] = n2;
                        QTree.map2leftcdr[n2] = n3;


                        QTree.map2rightcdr[seed_node] = n5;
                        QTree.map2rightcdr[n5] = n9;

                        QTree.map2parent[n3] = n2;
                        QTree.map2parent[n2] = n1;
                        QTree.map2parent[n1] = seed_node;


                        QTree.map2parent[n5] = seed_node;

                        QTree.map2parent[n9] = n5;
                        QTree.terminals_index = {0, 3};
                        QTree.junction_index = {5};
                        QTree.patterns = {g.typeMap[n3], g.typeMap[n2],g.typeMap[n1],
                             g.typeMap[n9], g.typeMap[n5],g.typeMap[seed_node]};
                            return QTree;


                    }
                    else{
                            return QTree;
                        }

            }
            else return QTree;
        }
        else return QTree;
    }
    //end of shape 5

     if (shape == 6){ //shape4 without 7, 10, 2,3
        int n1,n5, n6, n9;
        vector<int>added_neighs;
        cout<<"seednode is"<<seed_node<<endl;
        if (seed_node >= g.degree.size()){//seed_node not in graph
            return QTree;
        }
        added_neighs.push_back(seed_node);
        if (g.degree[seed_node] >= 2){
            n1 = g.neighbors[g.nodes[seed_node]];
            n5 = g.neighbors[g.nodes[seed_node]+1];
            cout<<"n1="<<n1<<" n5 ="<<n5<<endl;
            added_neighs.push_back(n1);
            added_neighs.push_back(n5);
            if (g.degree[n5]>=3){

                n6 = g.neighbors[g.nodes[n5]];
                n9 = g.neighbors[g.nodes[n5]+1];
                cout<<"n6= "<<n6<<" n9= "<<n9<<endl;

                if ( find(added_neighs.begin(), added_neighs.end(), n6) == added_neighs.end()
                    && find(added_neighs.begin(), added_neighs.end(),n9) == added_neighs.end()){

                    QTree.nodes_ordered = { n1,  n6, n9, n5, seed_node};
                    QTree.map2leftcdr[seed_node] = n1;

                    QTree.map2leftcdr[n5] = n6;

                    QTree.map2rightcdr[seed_node] = n5;
                    QTree.map2rightcdr[n5] = n9;


                    QTree.map2parent[n1] = seed_node;

                    QTree.map2parent[n6] = n5;
                    QTree.map2parent[n5] = seed_node;

                    QTree.map2parent[n9] = n5;
                    QTree.terminals_index = {0, 1, 2};
                    QTree.junction_index = {3, 4};
                    QTree.patterns = {g.typeMap[n1],
                         g.typeMap[n6],
                         g.typeMap[n9], g.typeMap[n5],g.typeMap[seed_node]};
                        return QTree;
                }
                else {
                    n6 = g.neighbors[g.nodes[n5]+1];
                    n9 = g.neighbors[g.nodes[n5]+2];
                    cout<<"n6= "<<n6<<" n9= "<<n9<<endl;

                    if ( find(added_neighs.begin(), added_neighs.end(), n6) == added_neighs.end()
                    && find(added_neighs.begin(), added_neighs.end(),n9) == added_neighs.end()){

                    QTree.nodes_ordered = { n1, n6, n9, n5, seed_node};
                    QTree.map2leftcdr[seed_node] = n1;

                    QTree.map2leftcdr[n5] = n6;

                    QTree.map2rightcdr[seed_node] = n5;
                    QTree.map2rightcdr[n5] = n9;


                    QTree.map2parent[n1] = seed_node;

                    QTree.map2parent[n6] = n5;
                    QTree.map2parent[n5] = seed_node;

                    QTree.map2parent[n9] = n5;
                    QTree.terminals_index = {0, 1, 2};
                    QTree.junction_index = {3, 4};
                    QTree.patterns = {g.typeMap[n1],
                         g.typeMap[n6],
                         g.typeMap[n9], g.typeMap[n5],g.typeMap[seed_node]};
                        return QTree;
                    }


                    else return QTree;
                }

            }
            else return QTree;
        }
        else return QTree;
    }
    //end of shape 7

        if (shape == 7){ //shape4 without 7, 10, 3
        int n1,n2,n5, n6, n9;
        vector<int>added_neighs;
        if (seed_node >= g.degree.size()){//seed_node not in graph
            return QTree;
        }
        added_neighs.push_back(seed_node);
        if (g.degree[seed_node] >= 2){
            n1 = g.neighbors[g.nodes[seed_node]];
            n5 = g.neighbors[g.nodes[seed_node]+1];
            cout<<"n1="<<n1<<" n5 ="<<n5<<endl;

            added_neighs.push_back(n1);
            added_neighs.push_back(n5);
            if (g.degree[n1]>= 2 && g.degree[n5]>=3){
                n2 = g.neighbors[g.nodes[n1]];
                cout<<"n2="<<n2<<", degree = "<<g.degree[n2]<<endl;

                if(find(added_neighs.begin(), added_neighs.end(), n2) == added_neighs.end()){ //n2 hasn't been added
                    added_neighs.push_back(n2);

                    }
                    else return QTree;
                }
                else return QTree;

                n6 = g.neighbors[g.nodes[n5]];
                n9 = g.neighbors[g.nodes[n5]+1];
                if (!(g.degree[n6]>=2 && g.degree[n9]>=2 && find(added_neighs.begin(), added_neighs.end(), n6) == added_neighs.end()
                    && find(added_neighs.begin(), added_neighs.end(),n9) == added_neighs.end())){

                        n6 = g.neighbors[g.nodes[n5]+1];
                        n9 = g.neighbors[g.nodes[n5]+2];
                        cout<<"n6= "<<n6<<" n9= "<<n9<<endl;
                        if (!(g.degree[n6]>=2 && g.degree[n9]>=2 && find(added_neighs.begin(), added_neighs.end(), n6) == added_neighs.end()
                            && find(added_neighs.begin(), added_neighs.end(),n9) == added_neighs.end())){
                            return QTree;
                            }
                        else{ //now found is good
                            added_neighs.push_back(n6);
                            added_neighs.push_back(n9);

                            QTree.nodes_ordered = {n2, n1,  n6, n9, n5, seed_node};
                            QTree.map2leftcdr[seed_node] = n1;
                            QTree.map2leftcdr[n1] = n2;

                            QTree.map2leftcdr[n5] = n6;

                            QTree.map2rightcdr[seed_node] = n5;
                            QTree.map2rightcdr[n5] = n9;

                            QTree.map2parent[n2] = n1;
                            QTree.map2parent[n1] = seed_node;

                            QTree.map2parent[n6] = n5;
                            QTree.map2parent[n5] = seed_node;

                            QTree.map2parent[n9] = n5;
                            QTree.terminals_index = {0, 2, 3};
                            QTree.junction_index = {4, 5};
                            QTree.patterns = {g.typeMap[n2],g.typeMap[n1],
                                 g.typeMap[n6],
                                 g.typeMap[n9], g.typeMap[n5],g.typeMap[seed_node]};
                                return QTree;

                        }
                    } //old found is good
                    else{
                        QTree.nodes_ordered = {n2, n1,  n6, n9, n5, seed_node};
                            QTree.map2leftcdr[seed_node] = n1;
                            QTree.map2leftcdr[n1] = n2;

                            QTree.map2leftcdr[n5] = n6;

                            QTree.map2rightcdr[seed_node] = n5;
                            QTree.map2rightcdr[n5] = n9;

                            QTree.map2parent[n2] = n1;
                            QTree.map2parent[n1] = seed_node;

                            QTree.map2parent[n6] = n5;
                            QTree.map2parent[n5] = seed_node;

                            QTree.map2parent[n9] = n5;
                            QTree.terminals_index = {0, 2, 3};
                            QTree.junction_index = {4, 5};
                            QTree.patterns = {g.typeMap[n2],g.typeMap[n1],
                                 g.typeMap[n6],
                                 g.typeMap[n9], g.typeMap[n5],g.typeMap[seed_node]};
                                return QTree;
                        }

            }
            else return QTree;
        }

    //end of shape 7


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
    if (QTree.nodes_ordered.size()<2){
        return decomposed_queries;
    }
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
                if (next_node==current_node){
                    break;
                }
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
                if (next_node==current_node){
                    break;
                }
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
        assert(non_touched_JT.size() == 1);

    return decomposed_queries;

}

unordered_map<int, unordered_map<int, tuple<float,float>>> bottom_up_hrtc_compute(const graph_t& g, Query_tree querytree){

    unordered_map<int, unordered_map<int, tuple<float,float>>> hrtcs;  //TODO: include the full CandIdx here, which include a max of all
    //algorithm 1. return (node -> {vertex1-><hrtc_left1, hrtic_right1, v_left1, v_right1>; vertex2 -> <left2, right2, v_left2, v_right2>; ...}).
    cout <<"computing heuristic value bottom-up..."<<endl;

    for (int i= 0; i< querytree.nodes_ordered.size(); i++){
        int node = querytree.nodes_ordered[i];
        unordered_map<int, tuple<float,float>> candidates;
        if (find(querytree.terminals_index.begin(), querytree.terminals_index.end(), i)!= querytree.terminals_index.end()){
            //terminal nodes come first in a postorder traversal
            tuple<float, float> terminal_hrtc (0.0, 0.0);
            candidates = {{querytree.nodes_ordered[i], terminal_hrtc}};
            hrtcs[node] = candidates;
        }
        else{
            //non-terminal node, will have at least one child
            if (querytree.map2leftcdr.find(node)!= querytree.map2leftcdr.end() && querytree.map2rightcdr.find(node)!= querytree.map2rightcdr.end()){
                //node has both left child and right child
                int leftchild = querytree.map2leftcdr[node];
                int best_left_candidate = 0;
                unordered_set<int> left_connected_cands;
                unordered_set<int> right_connected_cands;
                for(auto it = hrtcs[leftchild].begin(); it != hrtcs[leftchild].end(); ++it){
                    int left_candidate = it -> first;
                    float left_subtree_wgt = get<0>(it->second) + get<1>(it->second);

                    for(int j = 0; j < g.degree[left_candidate]; j++){
                        int node_candidate = g.neighbors[g.nodes[left_candidate]+j];
                        if (g.typeMap[node_candidate] == querytree.map2patthern[node]){
                            left_connected_cands = left_connected_cands.insert(node_candidate);
                            float best_left_value = MAX_WEIGHT;
                            float edge_wgt = calcWgt(g.wgts[g.nodes[left_candidate]+j], querytree.time);
                            if ((edge_wgt + left_subtree_wgt) < best_left_value){
                                best_left_value = (edge_wgt + left_subtree_wgt);
                                get<0>(hrtcs[node][node_candidate]) = best_left_value;
                            }
                        }
                    }
                }
                int rightchild = querytree.map2rightcdr[node];
                int best_right_candidate = 0;
                for(auto it = hrtcs[rightchild].begin(); it != hrtcs[rightchild].end(); ++it){
                    int right_candidate = it -> first;
                    float right_subtree_wgt = get<0>(it->second) + get<1>(it->second);

                    for(int j = 0; j < g.degree[right_candidate]; j++){
                        int node_candidate = g.neighbors[g.nodes[right_candidate]+j];
                        if (g.typeMap[node_candidate] == querytree.map2patthern[node]){
                            right_candidate_cands = right_candidate_cands.insert(node_candidate);
                            float best_right_value = MAX_WEIGHT;
                            float edge_wgt = calcWgt(g.wgts[g.nodes[right_candidate]+j], querytree.time);
                            if ((edge_wgt + right_subtree_wgt) < best_right_value){
                                best_right_value = (edge_wgt + right_subtree_wgt);
                                get<1>(hrtcs[node][node_candidate]) = best_right_value;
                            }
                        }
                    }
                }
                //join on each node candidate..only keep those that are connected to both sides.
                for(auto it = hrtcs[node].begin(); it != hrtcs[node].end();){
                    int node_candidate = it-> first;
                    if ( left_connected_cands.find(node_candidate) == left_connected_cands.end()
                        || right_connected_cands.find(node_candidate) == right_connected_cands.end()){
                            it = hrtcs[node].erase(it);
                    }
                    else{
                         it ++;
                    }
                }
            }
            else{
                //node has only one child, set the other field to 0.
                if(querytree.map2rightcdr.find(node)!= querytree.map2rightcdr.end()) {
                    //node has a right child
                    int rightchild = querytree.map2rightcdr[node];
                    int best_right_candidate = 0;
                    // float best_right_value = MAX_WEIGHT;
                    for(auto it = hrtcs[rightchild].begin(); it != hrtcs[rightchild].end(); ++it){
                        int right_candidate = it -> first;
                        float right_subtree_wgt = get<0>(it->second) + get<1>(it->second);

                        for(int j = 0; j < g.degree[right_candidate]; j++){
                            int node_candidate = g.neighbors[g.nodes[right_candidate]+j];
                            if (g.typeMap[node_candidate] == querytree.map2patthern[node]){
                                float best_right_value = MAX_WEIGHT;
                                float edge_wgt = calcWgt(g.wgts[g.nodes[right_candidate]+j], querytree.time);
                                if ((edge_wgt + right_subtree_wgt) < best_right_value){
                                    best_right_value = (edge_wgt + right_subtree_wgt);

                                    get<0>(hrtcs[node][node_candidate]) = 0;
                                    get<1>(hrtcs[node][node_candidate]) = best_right_value;
                                }
                            }
                        }
                    }
                }
                else{
                    //node has a left child
                    int leftchild = querytree.map2leftcdr[node];
                    int best_left_candidate = 0;
                    // float best_left_value = MAX_WEIGHT;
                    for(auto it = hrtcs[leftchild].begin(); it != hrtcs[leftchild].end(); ++it){
                        int left_candidate = it -> first;
                        float left_subtree_wgt = get<0>(it->second) + get<1>(it->second);

                        for(int j = 0; j < g.degree[left_candidate]; j++){
                            int node_candidate = g.neighbors[g.nodes[left_candidate]+j];
                            if (g.typeMap[node_candidate] == querytree.map2patthern[node]){
                                float best_left_value = MAX_WEIGHT;
                                float edge_wgt = calcWgt(g.wgts[g.nodes[left_candidate]+j], querytree.time);
                                if ((edge_wgt + left_subtree_wgt) < best_left_value){
                                    best_left_value = (edge_wgt + left_subtree_wgt);

                                    get<0>(hrtcs[node][node_candidate]) = best_left_value;
                                    get<1>(hrtcs[node][node_candidate]) = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }



    return hrtcs;
}



