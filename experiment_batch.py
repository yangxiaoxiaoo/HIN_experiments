#best version
import sys, shlex, os
import subprocess
from threading import Timer


ordered_seed =True
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


def repeat_for_shape(shape_choose, number, infile, seedfile):
    seen_seeds = set()

    datasetname = infile.split('/')[1]
    seed = 0
    with open(seedfile,'r') as infileP:
        for line in infileP:
            if ordered_seed:
                seed += 1
            else:
                seed = line.split('\t')[0]
            if seed not in seen_seeds:
                print "NEW ROUND START---"
                outdir = "./"+datasetname+"outputs/output"+str(shape_choose)+"/"+str(seed)
                print outdir
              #  try:
                run(["./pro-heaps", infile, outdir, str(shape_choose), str(seed)], 200)
                    #when there are real top-k, the result would be valid, otherwise no!
                #except Exception as Calling_timeout:
                 #   print "WARNING--TIMEOUT"
                 #   pass
                seen_seeds.add(seed)
                if len(seen_seeds) >= number:
                    break

def repeat_baseline2(optimization, shape_choose, number, infile, skip):
#skip: if I want to skip those that has already 3 lines
    datasetname = infile.split('/')[1]
    seed = 0
    dir = "./"+optimization + datasetname+"outputs/output"+str(shape_choose)
    Processed = os.listdir(dir)
    for file in Processed:
        i = 0
        with open(os.path.join(dir,file)) as thisfile:
            for i, l in enumerate(thisfile):
                pass
        print "line num is "
        print i + 1
        if i+1 == 2  or (not skip):
            seed = file.split('.')[0]
            outdir = dir+"/"+str(seed)
            print outdir
            run(["./pro-heaps", infile, outdir, str(shape_choose), str(seed)], 400)




def main():

    opt = 'O3_' #When changing here, also need to change at Makefile

    shapelist = [1, 2, 6]
    for shape in shapelist:
        repeat_baseline2(opt, shape, 300, "./Enron/enron_graph.wgt.norm", skip=False)

    shapelist = [1, 2, 5, 6, 7]
    for shape in shapelist:

        repeat_baseline2(opt, shape, 300, "./DBLP/dblp_graph.new.wgt", skip=False)
        repeat_baseline2(opt, shape, 300, "./PhotoNet/graph_prank.graph_new.graph", skip=False)
        repeat_baseline2(opt, shape, 300, "./YelpPhoto/yelp_review_tip_photos.graph_new.graph", skip=False)


 #       repeat_for_shape(shape, 300, "./YelpPhoto/yelp_review_tip_photos.graph_new.graph", "./YelpPhoto/seedranks.dat")
  #      repeat_for_shape(shape, 300, "./PhotoNet/graph_prank.graph_new.graph", "./PhotoNet/seedranks.dat")
 #       repeat_for_shape(shape, 300, "./Enron/enron_graph.wgt.norm", "./Enron/seedranks.dat")


def auto_query_main():
    #after March submission, we explicitly store generated queries for mining purposes.

   # datasets = ['Enron', 'PhotoNet', 'DBLP', 'YelpPhoto']
    datasets = ['Enron']
    for dataset in datasets:
        query_dir =  dataset + '/queries'
        outdir = dataset + '/auto_outputs/'
        infile = "./Enron/enron_graph.wgt.norm"
        for queryfile in os.listdir(query_dir):
            outfile =outdir + queryfile
            run(["./pro-heaps", infile, os.path.join(query_dir,queryfile), outfile], 300)

if __name__ == "__main__":
    #main()

    #redesigned experiments after March submission
    auto_query_main()

