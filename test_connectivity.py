__author__ = 'xiaofeng'
import auto_querygen
import sys

def connectivity_test(u, v):
    #test connectivity of nodes in graph for debugging C++ project
    def nodes_connected(G, u, v):
        return u in G.neighbors(v)

    g, v2type = auto_querygen.load_graph_struct("./Enron/enron_graph.wgt.norm")
    return nodes_connected(g, u, v)




if __name__ == '__main__':
#    test()
    #main()
    print connectivity_test(int(sys.argv[1]), int(sys.argv[2]))