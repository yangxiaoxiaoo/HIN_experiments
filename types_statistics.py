from collections import Counter

#read from HIN file the frequncy of each type of edges
#HIN graph: This is the HIN graph in edgelist format
# Each line (except the The first line) represents an edge with format
# "node_id node_id [node_label] [node_label] edge_label".
# For the node labels, "[0]" represents the Question,
# "[1]" represents the Answer,
# "[2]" represents Tag,
# "[3]" represents User.
# Labels of edges can be inferred directly from the vertex labels.
# the graph contains 21,579,657 nodes and 53,325,635 edges.
def count_edge_types():
    list_of_edges = list()
    fin = "data/SOF_HIN.edgelist"
    with open(fin, "r") as fp:
        for line in fp:
            try:
                edgetype = line.split()[4]
                list_of_edges.append(edgetype)
            except Exception:
                #first line has only two entry
                pass
    counter = Counter(list_of_edges)
    print counter
    return counter

#map an edgetype back to a list of edges
def map_edgetype_to_list():



if __name__ == "__main__":
    count_edge_types()
