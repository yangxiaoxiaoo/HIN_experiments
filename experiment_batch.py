import sys
import subprocess

# given a chosen shape, repeat number times the seeding and query
#each time randomly select a seed.
def repeat_for_shape(shape_choose, number, infile):
    seen_seeds = set()

    skip_first = 1
    with open(infile,'r') as infileP:
        for line in infileP:
            if skip_first == 1:
                skip_first = 0
            else:
                seed = line.split(' ')[0]
                if seed not in seen_seeds:
                    subprocess.check_call("./pro-heaps", infile, "./output/" +infile +str(seed), shape_choose, seed) # TODO: modify so that it support seed and shapechoose
                    seen_seeds.add(seed)
                    if len(seen_seeds) >= number:
                        break

def read_time_into_onefile():
    output =


def main():
    repeat_for_shape(1, 100, "./Enron/enron_graph.wgt.norm")
    repeat_for_shape(1, 100, "./DBLP/dblp_graph.new.wgt")

if __name__ == "__main__":
    main()