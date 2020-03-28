CC  = g++
#CFLAGS = -g -DSORT_ITEM -fguiding-decls
#CFLAGS = -O3 -Wno-non-template-friend
CFLAGS = -O3 -Wno-non-template-friend -Wno-deprecated 
#CFLAGS = -O4 -DDEC -tdefine_templates 
#CC = CC
#CFLAGS  = -O2 -DSGI -no_auto_include
HEADER  = assoc.h partition.h extl2.h Array.h Itemset.h Graph.h \
	memman.h List.h eclat.h Util.h  maxset.h \
	Count.h hashtable.h calcdb.h
OBJS	= extl2.o partition.o Array.o Itemset.o Graph.o \
	memman.o List.o Util.o maxset.o \
	Count.o hashtable.o calcdb.o
OBJ = eclat.o
LIBS = -lm -lc 
TARGET  = charm

default:	$(TARGET)

clean:
	rm -rf *~ *.o $(TARGET)

charm: assoc.cc $(OBJ) $(OBJS) $(HEADER) stats.h
	$(CC) $(CFLAGS) -o charm assoc.cc $(OBJ) $(OBJS) $(LIBS)
	strip charm

.SUFFIXES: .o .cc

.cc.o:
	$(CC) $(CFLAGS) -c $<       

# dependencies
# $(OBJS): $(HFILES) 

Itemset.o: Itemset.h defset.h
Array.o: Array.cc Array.h
Graph.o: Graph.cc Graph.h defset.h
extl2.o: extl2.cc extl2.h
partition.o: partition.cc partition.h
eclat.o: eclat.cc eclat.h defset.h
neclat.o: neclat.cc eclat.h defset.h
memman.o: memman.cc memman.h
List.o: List.cc List.h
Util.o: Util.cc Util.h
maxset.o: maxset.cc maxset.h
Count.o: Count.cc Count.h
hashtable.o: hashtable.cc hashtable.h
calcdb.o: calcdb.h
