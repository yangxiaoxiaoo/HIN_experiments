import sys
import os
import random

#######################################################
# You can change the params below:
seed = 42
node_types = {
    "PHYS_HOST" : (3,3),
    "VM" : (2,2),
    "ALARM" : (2,2),
    "SWITCH" : (1,1),
    "APP" : (2,2)
}
node_type_to_id = {
    "PHYS_HOST" : 1,
    "VM" : 2,
    "ALARM" : 3,
    "SWITCH" : 4,
    "APP" : 5
}
app_vm_usage = (1,5)
switch_connection_prob = 0.6
#######################################################

class Node:
    def __init__(self, node_id, node_type, adj_list):
        self.node_id = node_id
        self.node_type = node_type
        self.adj_list = adj_list

    def __str__(self):
        return "(" + str(self.node_id) + ":" + self.node_type + " [" + str(self.adj_list)  + "])"


def insert_node(graph, node_type):
    next_id = len(graph.keys())
    graph[next_id] = Node(next_id, node_type, [])
    return next_id


def connect(graph, node_1, node_2):
    graph[node_1].adj_list.append(node_2)
    graph[node_2].adj_list.append(node_1)
    # print "Connecting : " + graph[node_1].node_type + " (" + str(graph[node_1].node_id) + ") TO " + graph[node_2].node_type + " (" + str(graph[node_2].node_id) + ")"


def insert_connected_node(graph, anchor_id, node_type):
    next_id = len(graph.keys())
    graph[next_id] = Node(next_id, node_type, [anchor_id])
    connect(graph, next_id, anchor_id)
    return next_id

    
def construct_graph(description):
    graph = {}

    num_switch = random.randint(description["SWITCH"][0], description["SWITCH"][1])
    num_phys_host = random.randint(description["PHYS_HOST"][0], description["PHYS_HOST"][1])
    num_application = random.randint(description["APP"][0], description["APP"][1])

    phys_host_per_switch = num_phys_host / num_switch

    phys_host_ids = []
    switch_ids = []
    vm_ids = []

    for phys_host_num in xrange(num_phys_host):
        num_vm = random.randint(description["VM"][0], description["VM"][1])
        phys_host = insert_node(graph, "PHYS_HOST")
        phys_host_ids.append(phys_host)
        for vm_num in xrange(num_vm):
            vm = insert_connected_node(graph, phys_host, "VM")
            vm_ids.append(vm)
            num_alarm = random.randint(description["ALARM"][0], description["ALARM"][1])
            for alarm_num in xrange(num_alarm):
                alarm = insert_connected_node(graph, vm, "ALARM")


    for app_num in xrange(num_application):
        num_vm_hosts = random.randint(app_vm_usage[0], app_vm_usage[1])
        app = insert_node(graph, "APP")
        for i in xrange(num_vm_hosts):
            selected_vm = vm_ids[random.randint(0, len(vm_ids) - 1)]
            connect(graph, app, selected_vm)
                
    for sw in xrange(num_switch):
        next_switch = insert_node(graph, "SWITCH")
        switch_ids.append(next_switch)
        grab_phys_host_to_connect = phys_host_ids[sw * phys_host_per_switch: (sw+1) * phys_host_per_switch]
        # print "Connecting SWITCH: " + str(graph[next_switch].node_id) + " to PHYS_HOSTS " + str(sw * phys_host_per_switch) + ":"+ str((sw + 1)* phys_host_per_switch)
        for phys_host in grab_phys_host_to_connect:
            connect(graph, phys_host, next_switch)

    for i in xrange(len(switch_ids)):
        for j in xrange(i+1,len(switch_ids)):
            switch_1 = switch_ids[i]
            switch_2 = switch_ids[j]
            prob = random.random()
            if (prob >= switch_connection_prob):
                connect(graph, switch_1, switch_2)
    return graph
            

def edge_to_str(node_1, node_2, mask_1 = False, mask_2 = False):
    n1_id = str(node_1.node_id)
    n2_id = str(node_2.node_id)
    if (mask_1):
        n1_id = "?"
    if (mask_2):
        n2_id = "?"

    
    return n1_id + " " + n2_id + " [" + str(node_type_to_id[node_1.node_type]) + "] [" + str(node_type_to_id[node_2.node_type]) + "] e0 " + str(random.random())

def output_graph(graph, output_file):
    node_count = len(graph.keys())
    edge_count = 0
    for node_id in graph.keys():
        node = graph[node_id]
        for adj in node.adj_list:
            if adj > node.node_id:
                edge_count += 1

    output_file.write( str(node_count) + " " + str(edge_count) + os.linesep)
    for node_id in graph.keys():
        node = graph[node_id]
        assert(node.node_id == node_id)
        for adj in node.adj_list:
            if adj > node.node_id:
                edge_count += 1
                adj_node = graph[adj]
                assert(adj == adj_node.node_id)
                output_file.write(edge_to_str(node, adj_node) + os.linesep)


def star_pattern_search_aux(graph, root, bfs_types):
    if (len(bfs_types) == 0):
        return set(tuple([root]))

    arms = set()
    for neighbour_id in graph[root].adj_list:
        if graph[neighbour_id].node_type == bfs_types[0]:
            if len(bfs_types) > 1:
                for recursive_soln in star_pattern_search_aux(graph, neighbour_id, bfs_types[1:]):
                    arms.add( tuple([root] + list(recursive_soln) ))
            else:
                arms.add( tuple([root, neighbour_id]) )
    return arms

def star_pattern_search(graph, pattern, arm_count):
    result_set = []
    for node_id in graph.keys():
        node = graph[node_id]
        if (node.node_type == pattern[0]):
            root = node_id
            star_set = star_pattern_search_aux(graph, root, pattern[1:])
            if (len(star_set) >= arm_count):
                result_set.append(star_set)
    return result_set


def output_star_pattern(graph, star_set, num_arms, output_file):
    n = len(list(star_set)[0])
    nodes = (n-1) * num_arms + 1
    edges = nodes - 1
    output_file.write(str(nodes)+ " " + str(edges) + os.linesep)
    count = 0
    for star_tuple in star_set:
        if (count >= num_arms):
            break
        count += 1
        for i in xrange(1, len(star_tuple)):
            node_1 = graph[star_tuple[i-1]]
            node_2 = graph[star_tuple[i]]
            if (i == len(star_tuple) - 1):
                output_file.write(edge_to_str(node_1, node_2, mask_1 = True, mask_2 = False) + os.linesep)                
            else:
                output_file.write(edge_to_str(node_1, node_2, mask_1 = True, mask_2 = True) + os.linesep)


def output_star_pattern_list(graph, star_set, num_arms, output_file):
    for star in star_set:
        output_star_pattern(graph, star, num_arms, output_file)


if __name__ == "__main__":
    random.seed(seed)
    graph = construct_graph(node_types)
    with open("./output-graph.txt","w") as output_han:
        output_graph(graph, output_han)
    pattern_res_set = star_pattern_search(graph, ["SWITCH", "PHYS_HOST","VM","APP"], 3)
    with open("./output-patterns.txt", "w") as output_patterns:
        output_star_pattern_list(graph, pattern_res_set, 3, output_patterns)
