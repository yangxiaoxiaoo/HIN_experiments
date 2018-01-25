import experiment_batch
import random
import os

def write_to_file(n, infile):
    #write to file a graph where:
    # node 0 is connected to all (n, 2n) range nodes,
    #them connected to node 1,
    #node 1 connect to range(2n, 3n) nodes,
    #them connected to node 2, 2 to (3n , 4n), to 3.
    #Total nodes 3n+ 4, total edges 6n

    with open(infile + str(n), 'w') as fp:
        fp.write(str(4 * n + 4) + " "+ str(6*n) + '\n')  #add additional dangling nodes because of the edge vector allocation
        for i in range(n, 2*n):
          #  w_up = random.uniform(0, 10)
            fp.write(str(0) + ' ' + str(i) + ' [0] [1] e0 ' + '0.000000' + "\n")
            w_down = random.uniform(0, 10)
            fp.write(str(1) + ' ' + str(i) + ' [2] [1] e0 ' + str(w_down) + "\n")
        for i in range(2*n, 3*n):
            w_up = random.uniform(0, 10)
            fp.write(str(1) + ' '+ str(i) + ' [2] [3] e0 ' + str(w_up) + "\n")
            w_down = random.uniform(0, 10)
            fp.write(str(2) + ' '+ str(i) + ' [4] [3] e0 ' + str(w_down) + "\n")
        for i in range(3*n, 4*n):
            w_up = random.uniform(0, 10)
            fp.write(str(2) + ' '+ str(i) + ' [4] [5] e0 ' + str(w_up) + "\n")
            w_down = random.uniform(0, 10)
            fp.write(str(3) + ' '+ str(i) + ' [6] [5] e0 ' + str(w_down) + "\n")

def write_query(queryfile):
    with open(queryfile, 'w') as fp:
        fp.write("0 1 2 3 4 5 6\n")
        fp.write("0 3 978307200\n")




if __name__ == "__main__":
    # write_query("SynData/query")

    infile = "SynData/Graph"
    n = 100
    write_to_file(n, infile)
    queryfile = "SynData/query"
    write_query(queryfile)

    choice = 7 #AStar_prophet
    experiment_batch.run(["./pro-heaps-path", infile + str(n), queryfile, str(choice)], 700)

    choice = 1 #plain DFS
    experiment_batch.run(["./pro-heaps-path", infile + str(n), queryfile, str(choice)], 700)
