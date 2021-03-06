import random
import networkx as nx
from networkx.algorithms import isomorphism


def gen_adj(n): #generate a list of template of size n

    #binary tree query has a degree between 1 and 3. used 1 can be 0, 1, 2, 3
    #decide a partial line randomly, given that the known part of that line contains used1 #1s.
    def random_select_col(linelen, used1):
        random_line = []
        for i in range(linelen):
            random_line.append(0)
        if used1 > 3: #already illegal
            return []
        if used1 == 3:
            return random_line
        else:
            new1 = random.randint(0, 3-used1) #number of 1 newly added in this part
            print "linelen is "+ str(linelen)
            print "planning to add number of 1s "+str(new1)
            if new1 > linelen:
                return []
            pos = random.sample(xrange(linelen), new1)
            for index in pos:
                random_line[index] = 1
        return random_line


    adjs = []
    while(True):
        if len(adjs) > 0:
            adjs = []
        #initialize adjs
        for i in range(n):
            line = []
            for j in range(n):
                line.append(0)
            adjs.append(line)
        for i in range(n): #decide each line
            used1 = sum(adjs[i][:i])
            unknown_half = random_select_col(n - i, used1)
            if unknown_half == []: #illegal case, exit with an empty matrix
                return [[]]
            for j in range(i + 1, n):  #j > i
                adjs[i][j] = unknown_half[j - i - 1] #append newly generated half behind
            for j in range(n):
                adjs[j][i] = adjs[i][j]
        #then decide that last line, root, has 2 1s.
        if sum(adjs[n-1]) == 2:
            break

    return adjs


def load_graph_struct(graphfile):
    node2type = dict()
    with open(graphfile) as f:
        lines = f.readlines()
    del lines[0]

    myList = []
    for line in lines:
        node_fro = int(line.split()[0])
        node_to = int(line.split()[1])
        type_fro = int(line.split()[2].strip('[').strip(']'))
        type_to = int(line.split()[3].strip('[').strip(']'))
        myList.append([node_fro, node_to])
        if node_fro not in node2type:
            node2type[node_fro] = type_fro
        if node_to not in node2type:
            node2type[node_to] = type_to



    g = nx.Graph()
    g.add_edges_from(myList)
    print len(g.nodes())
    return g, node2type


############### output a set of queries into files. Return 0
def grow_from_seed(N, repeat, adj, g, seedfile, v2type, sample, prefix, seedamount):

    def isterminal(curnode, adj):
        return sum(adj[curnode]) == 1  #terminals have degree 1


    def children(curnode, adj):
        children_list = []
        row = adj[curnode]
        for j in range(0, curnode): #children has a smaller index then curnode
            if row[j] == 1:
                children_list.append(j)
        return children_list

    seedcount = 0

    if len(adj[0]) == 0:
        return
    with open(seedfile,'r') as infile:
        for line in infile:
            seedcount += 1
            if seedcount > seedamount:
                break
            try:
                seed = int(line.split('\t')[0])
            except Exception as seederror:
                print "meta-line for seed readed..."
                return
            #for this seed, generate patterns
            n = len(adj)

            for sample_index in range(sample):
                map2left = []
                map2right = []
                map2parent = []
                terminalmap = dict()
                patterns = [0 for a in range(N)]
                junctions = []
                rootnode = n-1  #start from the root
                queue = [[rootnode, seed]]
                seen_child_count = dict()
                while len(queue)!= 0:
                    current= queue.pop(0)
                    print "current pair is"
                    print current
                    curnode = current[0]
                    curvertex = current[1]
                    patterns[curnode] = v2type[curvertex]
                    print "pattern added for node"
                    print curnode
                    if isterminal(curnode, adj):
                        terminalmap[curnode] = curvertex
                    else:
                        for node in children(curnode, adj):
                            if curnode not in seen_child_count:

                                seen_child_count[curnode] = 1 #havent seen it, assign to the right
                                #map2right[curnode] = node
                                map2right += [curnode, node]
                                #map2parent[node] = curnode
                                map2parent += [node, curnode]
                                nextvertex = random.choice(g.neighbors(curvertex))
                                queue.append([node, nextvertex])


                            else:
                                if seen_child_count[curnode] == 1: #have seen it once, assign to the left
                                    if curnode not in junctions:
                                        junctions.append(curnode)
                                    #map2left[curnode] = node
                                    map2left += [curnode, node]
                                    #map2parent[node] = curnode
                                    map2parent += [node, curnode]
                                    nextvertex = random.choice(g.neighbors(curvertex))
                                    queue.append([node, nextvertex])
                                    seen_child_count[curnode] += 1

                                else: #have seen it twice,
                                    print "Warning: seeing a 3rd children, check tree structure!"


                junctions.reverse()
                outfile = prefix + 'queries/N' +str(N)+"repeat"+str(repeat)+ str(seed)+'_seeded_'+str(sample_index) + '.query'
                output_tree(adj, patterns, terminalmap, map2right, map2left, map2parent, junctions, outfile)
                #map2left switched with map2right.




def output_tree(adj, patterns, terminalmap,  map2left, map2right, map2parent, junctions, outfile): #2D list, list, map, filename string
    print terminalmap

    nodes = []
    for i in range(len(adj)):
        nodes.append(i)
    #nodes 0 1 2 3 4


    junction_index = junctions[:]


    terminals = []
    terminal_index = []
    for key, value in sorted(terminalmap.iteritems()):
        print "key = "
        print key
        print "value = "
        print value
        terminal_index.append(key)
        terminals.append(value)


    #terminal replacement from template
    for key,value in terminalmap.iteritems():
        for i in range(len(nodes)):
            if nodes[i] == key:
                nodes[i] = value
                break

    #    nodes = 105 30 3 27 5
    terminal_index.sort()

    for key,value in terminalmap.iteritems():
        for i in range(len(map2parent)):
            if map2parent[i] == key:
                map2parent[i] = value
                break
        for i in range(len(map2left)):
            if map2left[i] == key:
                map2left[i] = value
                break
        for i in range(len(map2right)):
            if map2right[i] == key:
                map2right[i] = value
                break


   # for i in terminal_index:
   #     terminals.append(nodes[i])

    for node in junctions:
        for i in range(len(nodes)):
            if nodes[i] == node:
                junction_index.append(i)
                break

    junction_index.sort()

    if len(patterns) != len(nodes):
        print "tree sampling not formatted"
        return
    with open(outfile, 'a') as fout:



        #line 1: map to the left child
        fout.write(str(len(map2left)/2))
        for item in map2left:
            fout.write(' ' + str(item))
        fout.write('\n')

        #line 2: map to the right child
        fout.write(str(len(map2right)/2))
        for item in map2right:
            fout.write(' ' + str(item))
        fout.write('\n')

        #line 3 map to parent
        fout.write(str(len(map2parent)/2))
        for item in map2parent:
            fout.write(' ' + str(item))
        fout.write('\n')

        #line 4: nodes
        fout.write(str(len(nodes)))
        for item in nodes:
            fout.write(' ' + str(item))
        fout.write('\n')

        #line 5: terminal index
        fout.write(str(len(terminal_index)))
        for item in terminal_index:
            fout.write(' ' + str(item))
        fout.write('\n')

        #line 6: terminals
        fout.write(str(len(terminals)))
        for item in terminals:
            fout.write(' ' + str(item))
        fout.write('\n')

        #line 7: junction index
        fout.write(str(len(junction_index)))
        for item in junction_index:
            fout.write(' ' + str(item))
        fout.write('\n')

        #line 8: junctions
        fout.write(str(len(junctions)))
        for item in junctions:
            fout.write(' ' + str(item))
        fout.write('\n')

        #line 8: patterns
        fout.write(str(len(patterns)))
        for item in patterns:
            fout.write(' ' + str(item))
        fout.write('\n')

        fout.write(str(adj) + '\n')

def test():
    #test functionality implemented in this file
    adj = gen_adj(5)
    print adj
    g, dictype = load_graph_struct("./Enron/enron_graph.wgt.norm")
    print len(dictype)
    prefix = "./Enron/"
    seedfile = prefix + "seedranks.dat"
    grow_from_seed(5, 1, adj, g, seedfile, dictype, 1, prefix, 1)

def main():
    g, v2type = load_graph_struct("./Enron/enron_graph.wgt.norm")
    prefix = "./Enron/"
    seedfile = prefix + "seedranks.dat"
    for n in range(4, 6):
        #randomly sample one
        for repeat in range(5): #generate ramdonmly 100 times
            adj = gen_adj(n)
            grow_from_seed(n, repeat, adj, g, seedfile, v2type,20, prefix, seedamount=20)






import tree_gen

class new_tree:

    def loadgraph(self, path):
        return load_graph_struct(path)

    def terminal_list(self, tree):
        terminals = []
        for node in range(1, tree.number_of_nodes()+1):
            if tree.degree(node)== 1:
                terminals.append(node)
        return terminals

    def is_all_terminals_repeated(self, mapping, threshold, tree, vertex_appearance_tracker):
        #return true when all terminals has been mapped more then threshold times

        terminal_list = self.terminal_list(tree)

        for vertex, node in mapping.iteritems():
            if node in terminal_list:
                if vertex_appearance_tracker[(node, vertex)] < threshold:
                    return False
                #return false if any terminal nodes haven't appeared more then threshold times
        return True

    def sort_left_right(self, pair, post_order_nodes):
        if post_order_nodes.index(pair[0]) < post_order_nodes.index(pair[1]):
            return pair[0], pair[1]
        else:
            return pair[1], pair[0]

    def if_onechild_onleft(self, parent, child, post_order_nodes):
        if post_order_nodes.index(parent) < post_order_nodes.index(child):
            return False
        else:
            return True

    def template_output(self, mapping, tree, v2type, outfile):

        node2vertex = {}
        for k, v in mapping.iteritems():
            node2vertex[v] = k

        tree_inG = nx.relabel_nodes(tree, node2vertex) #use the ISO subgraph made of verteces

        root = 0
        for node in tree_inG.nodes():
            if tree_inG.degree(node) == 2:
                root = node
                break

       # assert root != 0
        nodes =list(nx.dfs_postorder_nodes(tree_inG, root))
        print nodes



        map2children = nx.bfs_successors(tree_inG, root) #will branch into left and right later
        map2parent = nx.bfs_predecessors(tree_inG, root)
        print "map to children is "
        print map2children
        print "map to parent is "
        print map2parent

        map2left = {}
        map2right = {}
        for key, value in map2children.iteritems():
            assert len(value) <= 2 #binary tree assertion
            if len(value) == 2:
                map2left[key], map2right[key] = self.sort_left_right(value, nodes)
            if len(value) == 1:
                is_left = self.if_onechild_onleft(key, value[0], nodes)
                if is_left:
                    map2left[key] = value[0]
                else:
                    map2right[key] = value[0]

        print "map to left is "
        print map2left
        print "map to right is "
        print map2right

        terminals = []
        terminal_index = []
        junctions = []
        junction_index = []
        for index in range(len(nodes)):
            if tree_inG.degree(nodes[index]) == 1:
                terminal_index.append(index)
                terminals.append(nodes[index])
            if tree_inG.degree(nodes[index]) == 3:
                junction_index.append(index)
                junctions.append(nodes[index])
        junction_index.append(len(nodes) -1) #append root as a special junction
        junctions.append(root)

        patterns = [v2type[x] for x in nodes]

        with open(outfile, 'w') as fout:
            #line 1: map to the left child
            fout.write(str(len(map2left)))
            for k, v in map2left.iteritems():
                fout.write(' ' + str(k) + ' ' + str(v))
            fout.write('\n')
            #line 2: map to the right child
            fout.write(str(len(map2right)))
            for k, v in map2right.iteritems():
                fout.write(' ' + str(k) + ' ' + str(v))
            fout.write('\n')
            #line 3 map to parent
            fout.write(str(len(map2parent)))
            for k, v in map2parent.iteritems():
                fout.write(' ' + str(k) + ' ' + str(v))
            fout.write('\n')
            #line 4: nodes
            fout.write(str(len(nodes)))
            for item in nodes:
                fout.write(' ' + str(item))
            fout.write('\n')
            #line 5: terminal index
            fout.write(str(len(terminal_index)))
            for item in terminal_index:
                fout.write(' ' + str(item))
            fout.write('\n')
            #line 6: terminals
            fout.write(str(len(terminals)))
            for item in terminals:
                fout.write(' ' + str(item))
            fout.write('\n')
            #line 7: junction index
            fout.write(str(len(junction_index)))
            for item in junction_index:
                fout.write(' ' + str(item))
            fout.write('\n')
            #line 8: junctions
            fout.write(str(len(junctions)))
            for item in junctions:
                fout.write(' ' + str(item))
            fout.write('\n')
            #line 8: patterns
            fout.write(str(len(patterns)))
            for item in patterns:
                fout.write(' ' + str(item))
            fout.write('\n')
            #############write a line of metadata


    def new_tree_match(self, G, v2type, tree, threshold, outfile_prefix, repeat):
        #use new tree matching algorithm to return tree template's matching subgraph in G
        #seed not fixed as root, but any 3-degree node in the tree
        #use those templete where all terminals have appeared more then threshold times

        GM = isomorphism.GraphMatcher(G, tree)

        vertex_appearance_tracker = dict()
        outputcount = 1

       # counter = 0
        for mapping in GM.subgraph_isomorphisms_iter():

            for vertex, node in mapping.iteritems():



                if (node, vertex) not in vertex_appearance_tracker:
                    vertex_appearance_tracker[(node, vertex)] = 1
                else:
                    vertex_appearance_tracker[(node, vertex)] += 1


            if self.is_all_terminals_repeated(mapping, threshold, tree, vertex_appearance_tracker):
                print mapping
                #write to the template files when all requrement satisfies
                self.template_output(mapping, tree, v2type, outfile_prefix + str(outputcount))
                outputcount += 1
                if outputcount > repeat:
                    break


    def test_new_tree(self, N):


        tree = tree_gen.gen_tree(N)
        #tree = tree_gen.gen_bin_tree(N)
        print tree.edges()

      #  g, v2type = load_graph_struct("./Enron/enron_graph.wgt.norm")
      #  self.new_tree_match(g, v2type, tree, threshold=3, outfile_prefix="./Enron/new_queries/N"+str(N)+'/', repeat=20)

        g, v2type = load_graph_struct("./DBLP/dblp_graph.new.wgt")
        self.new_tree_match(g, v2type, tree, threshold=4, outfile_prefix="./DBLP/new_queries/N"+str(N)+'/', repeat=20)

      #  g, v2type = load_graph_struct("./PhotoNet/graph_prank.graph_new.graph")
       # self.new_tree_match(g, v2type, tree, threshold=4, outfile_prefix="./PhotoNet/new_queries/N"+str(N)+'/', repeat=2000)
     #   g, v2type = load_graph_struct("./YelpPhoto/yelp_review_tip_photos.graph_new.graph")
     #   self.new_tree_match(g, v2type, tree, threshold=3, outfile_prefix="./YelpPhoto/new_queries/N"+str(N)+'/', repeat=2000)



    def templete1(self):
        graph = nx.Graph()
        graph.add_nodes_from([1, 2, 3, 4, 5])
        graph.add_edges_from([(1, 2), (2, 3), (2, 4), (1,5)])
        return graph

    def hard_templates(self):
        tree = self.templete1()
        adj = nx.adjacency_matrix(tree).todense()
        g, v2type = load_graph_struct("./DBLP/dblp_graph.new.wgt")
        prefix = "./Enron/"
        seedfile = prefix + "seedranks.dat"

        grow_from_seed(5, 5, adj, g, seedfile, v2type, prefix, seedamount=20)

if __name__ == '__main__':


    new_tree_test = new_tree()
    new_tree_test.hard_templates()

#    main()
