import networkx as nx

class UF:
    def __init__(self, nodes):
        self.par = {}
        for n in nodes:
            self.par[n] = -1

    def _root(self, x):
        if self.par[x] < 0:
            return x
        else:
            self.par[x] = self._root(self.par[x])
            return self.par[x]

    def union(self, x, y):
        x, y = self._root(x), self._root(y)
        if x == y:
            return
        if self.par[y] < self.par[x]:
            x, y = y, x

        self.par[x] += self.par[y]
        self.par[y] = x


import matplotlib.pyplot as plt


def hierarchy_pos(G, root, width=1., vert_gap=0.2, vert_loc=0, xcenter=0.5,
                  pos=None, parent=None):


    '''If there is a cycle that is reachable from root, then this will see infinite recursion.
   G: the graph
   root: the root node of current branch
   width: horizontal space allocated for this branch - avoids overlap with other branches
   vert_gap: gap between levels of hierarchy
   vert_loc: vertical location of root
   xcenter: horizontal location of root
   pos: a dict saying where all nodes go if they have been assigned
   parent: parent of this branch.'''
    if pos == None:
        pos = {root: (xcenter, vert_loc)}
    else:
        pos[root] = (xcenter, vert_loc)
    neighbors = G.neighbors(root)
    if parent != None:
        neighbors.remove(parent)
    if len(neighbors) != 0:
        dx = width / len(neighbors)
        nextx = xcenter - width / 2 - dx / 2
        for neighbor in neighbors:
            nextx += dx
            pos = hierarchy_pos(G, neighbor, width=dx, vert_gap=vert_gap,
                                vert_loc=vert_loc - vert_gap, xcenter=nextx, pos=pos,
                                parent=root)
    return pos

def gen_tree(N):
    """
    generate a networkx graph that is a tree with N nodes
    labeled {1, 2, 3, ..., N}
    :param N:
    :return:
    """
    edges = []
    for i in range(1, N + 1):
        for j in range(i + 1, N + 1):
            edges.append((i, j))

    import random
    random.shuffle(edges)
    nodes = range(1, N + 1)
    cc = UF(nodes)
    graph = nx.Graph()
    edges_remaining = N - 1

    for edge in edges:
        if edges_remaining == 0:
            return graph
        if cc._root(edge[0]) == cc._root(edge[1]):
            continue
        else:
            cc.union(edge[0], edge[1])
            graph.add_edge(edge[0], edge[1])
            edges_remaining -= 1
            #nx.draw_circular(graph)
            #plt.draw()
            #plt.show()

    assert edges_remaining == 0
    return graph


def test_gen(N):
    G = gen_tree(N)
    pos = hierarchy_pos(G, 1)
    nx.draw(G, pos=pos)
    plt.draw()
    plt.show()



