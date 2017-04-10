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