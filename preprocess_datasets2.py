#best version -- modified for Yelp
import os
import time
import sys

#INPUT_GRAPH = "./PhotoNet/graph_prank.graph"
#OUTPUT_GRAPH = "./PhotoNet/graph_prank.graph_new.graph"
#SEED_RANKS = "./PhotoNet/seedranks.dat"

#INPUT_GRAPH = "./YelpPhoto/yelp_review_tip_photos.graph"
#OUTPUT_GRAPH = "./YelpPhoto/yelp_review_tip_photos.graph_new.graph"
#SEED_RANKS = "./YelpPhoto/seedranks.dat"


INPUT_GRAPH = "./Enron/enron_graph.wgt.norm"
SEED_RANKS = "./Enron/seedranks.dat"
Seedonly = True


Yelp = True

def read_and_convert(infile):
    node2degree = dict()
    node2id = dict()
    linenum = 0
    id = 0
    with open(infile, 'r') as fp:
        for line in fp:
            if len(line.split(' ')) < 3:
                continue
            linenum +=1
            if (linenum % 1000000 == 0):
                sys.stdout.write(".")
                sys.stdout.flush()
            tokens = line.split(' ')
            # Column 0 is id of start node
            node_fro = tokens[0]
            # NOTE in Yelp Column 1 is *type* of start node
            # NOTE in Yelp Column 2 is id of destination
            if Yelp:
                node_to = tokens[2]
            else:
                node_to = tokens[1]
            if node_fro in node2degree:
                node2degree[node_fro] +=1
            else: #first time seeing any node: add it to an id
                node2degree[node_fro] = 1
                node2id[node_fro] = id
                id +=1

            if node_to in node2degree:
                node2degree[node_to] +=1
            else:
                node2degree[node_to] = 1
                node2id[node_to] = id
                id+= 1


    print ". " + os.linesep + "[DONE]"
    return node2degree, linenum, node2id


#sort node2degree for seeding and output a list into file
def sort_print_degree(node2degree, outfile, node2id):
    with open(outfile, 'w') as fp:
        for key, value in sorted(node2degree.iteritems(), key=lambda (k, v): (v, k), reverse=True):
            fp.write(str(node2id[key]) + "\t" +str(value) + "\n")

#if the graph is already a sorted loadable format
def sort_print_seedonly(node2degree, outfile):
    with open(outfile, 'w') as fp:
        for key, value in sorted(node2degree.iteritems(), key=lambda (k, v): (v, k), reverse=True):
            fp.write(str(key) + "\t" +str(value) + "\n")


def write_new_format(infile, outfile, nodenum, edgenum, node2id):
    with open(outfile, 'w') as fp:
        fp.write(str(nodenum)+" "+str(edgenum) + os.linesep)  #if see error: this punctuation may be a tab instead
    with open(outfile, 'a') as fp:
        with open(infile, 'r') as fin:
            for line in fin:
                tokens = line.split(' ')
                node_fro = tokens[0]
                if Yelp:
                    node_to = tokens[2]
                else:
                    node_to = tokens[1]
                id_fro = node2id[node_fro]
                id_to = node2id[node_to]

                #NOTE yelp has different lines. need to change this part
                if Yelp:
                    normed_weight = float(tokens[5])/100000000
                    line_new = str(id_fro) + ' ' + str(id_to) + ' ' + tokens[1]+ ' ' + tokens[3]+' ' + tokens[4]+' ' + str(normed_weight)+"\n"
                else:
                    line_new = str(id_fro) + ' ' + str(id_to) + ' ' + tokens[2]+ ' ' + tokens[3]+' ' + tokens[4]+' ' + tokens[5]
                fp.write(line_new)


def main():

    if Seedonly:
        node2degree, edgenum, node2id = read_and_convert(INPUT_GRAPH)
        sort_print_seedonly(node2degree, SEED_RANKS)
    else:
        print "Reading input graph."
        time_start = time.time()
        node2degree, edgenum, node2id = read_and_convert(INPUT_GRAPH)
        time_end = time.time()
        print "Time elapsed: " + str((time_end - time_start))
        nodenum = len(node2degree)
        print "Writing new format..."
        time_start = time.time()
        write_new_format(INPUT_GRAPH, OUTPUT_GRAPH, nodenum, edgenum, node2id)
        time_end = time.time()
        print "Time elapsed: " + str((time_end - time_start))
        print "Writing degrees..."
        time_start = time.time()
        sort_print_degree(node2degree, SEED_RANKS, node2id)
        time_end = time.time()
        print "Time elapsed: " + str((time_end - time_start))




if __name__ == "__main__":
    main()


