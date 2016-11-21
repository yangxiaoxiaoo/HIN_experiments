import sys, shlex
import subprocess
from threading import Timer

# given a chosen shape, repeat number times the seeding and query
#each time randomly select a seed.


def run(cmd, timeout_sec):
  proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
    stderr=subprocess.PIPE)
  kill_proc = lambda p: p.kill()
  timer = Timer(timeout_sec, kill_proc, [proc])
  try:
    timer.start()
    stdout,stderr = proc.communicate()
  finally:
    timer.cancel()


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
                    print "NEW ROUND START---"
                    outdir = "./output"+str(shape_choose)+"/"+str(seed)
                    print outdir
                  #  try:
                    run(["./pro-heaps", infile, outdir, str(shape_choose), str(seed)], 20)
                        #when there are real top-k, the result would be valid, otherwise no!
                    #except Exception as Calling_timeout:
                     #   print "WARNING--TIMEOUT"
                     #   pass
                    seen_seeds.add(seed)
                    if len(seen_seeds) >= number:
                        break

#def read_time_into_onefile():
#    output =


def main():
   # repeat_for_shape(1, 100, "./Enron/enron_graph.wgt.norm")
   # repeat_for_shape(4, 2000, "./DBLP/dblp_graph.new.wgt")
    repeat_for_shape(7, 1000, "./Enron/enron_graph.wgt.norm")



if __name__ == "__main__":
    main()

