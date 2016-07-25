CC = g++
OPTFLAGS = -std=c++0x -w -g -O3 
INCLUDES = -I.
#refer to global.h to see the choice of weights assignment
WGT1 = -DSTATIC_WGT 
WGT2 = -DDBLP_WGT 
WGT3 = -DSOF_WGT
WGT4 = -DENRON_WGT
CFLAGS = $(OPTFLAGS) $(INCLUDES) $(WGT4) 
OBJS = io.o dfm.o bfm.o dijkstra.o AStar_Prophet.o utils.o findPath_main.o

%.o: %.cc *.h
	$(CC) $(CFLAGS) -c $<

pro-heaps: $(OBJS) 	
	$(CC) $(CFLAGS) -c findPath_main.cc
	$(CC) $(CFLAGS) -o $@ $(OBJS)
	chmod 755 $@
	rm -f *.o
clean:
	rm -f *.o	
