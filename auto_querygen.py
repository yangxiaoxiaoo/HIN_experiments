import random
import networkx as nx

def gen_adj(n): #generate a list of template of size n

    #binary tree query has a degree between 1 and 3. used 1 can be 0, 1, 2, 3
    #decide a partial line randomly, given that the known part of that line contains used1 #1s.
    def random_select_col(n, linelen, used1):
        random_line = []
        for i in range(linelen):
            random_line.append(0)
        if used1 == 3:
            return random_line
        else:
            new1 = random.randint(0, 3-used1) #number of 1 newly added in this part
            pos = random.sample(xrange(linelen), new1)
            for index in pos:
                random_line[index] = 1
        return random_line


    adjs = []
    #initialize adjs
    for i in range(n):
        line = []
        for j in range(n):
            line.append(0)
        adjs.append(line)


    for i in range(n): #decide each line
        used1 = sum(adjs[i][:i])
        unknown_half = random_select_col(n, n - i, used1)
        for j in range(i + 1, n):  #j > i
            adjs[i][j] = unknown_half[j - i - 1] #append newly generated half behind
        for j in range(n):
            adjs[j][i] = adjs[i][j]

    return adjs


def load_graph_struct(graphfile):
    with open(graphfile) as f:
        lines = f.readlines()
    del lines[0]

    myList = [[line.split()[0], line.split()[1]] for line in lines]
    g = nx.Graph()
    g.add_edges_from(myList)
    print len(g.nodes())
    return g


###############TODO
def grow_from_seed(adj, g, seedfile):
    with open(seedfile,'r') as infile:
        for line in infile:
            seed = line.split('\t')[0]
            #for this seed, generate patterns

            patterns = []
            terminalmap = dict()
            output_tree(adj, patterns, terminalmap, outfile)




def output_tree(adj, patterns, terminalmap, outfile): #2D list, list, map, filename string

#decide those based on adj
    nodes = [] #1 2 3 4 5
    map2parent = [] #1 3 2 3 3 5 4 5
    map2left = [] #5 3 3 1
    map2right = [] #5 4 3 2
    junctions = [] #3 5

    junction_index = []
    terminal_index = []
    terminals = []

    #terminal replacement from template
    for key,value in terminalmap:
        for i in range(len(nodes)):
            if nodes[i] == key:
                nodes[i] = value
                break
            terminal_index.append(i)
    #    nodes = 105 30 3 27 5
    terminal_index.sort()

    for key,value in terminalmap:
        for i in range(len(map2parent)):
            if nodes[i] == key:
                nodes[i] = value
                break
        for i in range(len(map2left)):
            if nodes[i] == key:
                nodes[i] = value
                break
        for i in range(len(map2right)):
            if nodes[i] == key:
                nodes[i] = value
                break


    for i in terminal_index:
        terminals.append(nodes[i])

    for node in junctions:
        for i in range(len(nodes)):
            if nodes[i] == node:
                junction_index.append(i)

    junction_index.sort()

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

        #line 8: terminals
        fout.write(str(len(junctions)))
        for item in junctions:
            fout.write(' ' + str(item))
        fout.write('\n')

        #line 8: patterns
        fout.write(str(len(patterns)))
        for item in patterns:
            fout.write(' ' + str(item))
        fout.write('\n')

def test():
    print gen_adj(7)
    load_graph_struct("./Enron/enron_graph.wgt.norm")


def main():
    g = load_graph_struct("./Enron/enron_graph.wgt.norm")
    for n in range(4, 10):
        adjs = gen_adj(n)
        for adj in adjs:
            grow_from_seed(adj, g, seedfile)


if __name__ == '__main__':
    test()
  #  main()