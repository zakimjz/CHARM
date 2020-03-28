# CHARM Algorithm

Charm mines all the frequent closed itemsets as described in [2002-charm].
If you want to generate the entire lattice with super/sub-concept links
then also look at [2005-charm:tkde] https://github.com/zakimjz/CHARM-L

[2002-charm] Mohammed J. Zaki and Ching-Jui Hsiao. CHARM: an efficient algorithm for closed itemset mining. In 2nd SIAM International Conference on Data Mining. April 2002.

[2005-charm:tkde] Mohammed J. Zaki and Ching-Jui Hsiao. Efficient algorithms for mining closed itemsets and their lattice structure. IEEE Transactions on Knowledge and Data Engineering, 17(4):462–478, April 2005. doi:10.1109/69.846291.

You will also need access to the utils files:
https://github.com/zakimjz/tposedb


# HOW TO

1) Generate a data file using the IBM data generator program, gen. 
   OR
  Start with an ascii file (see chess.ascii example file in this directory)
  
The format of the ascii/binary file should be

\<cid\> \<tid\> \<numitem\> \<item list\>

        CID TID #ITEMS LIST_OF_ITEMS
        1   1   4       0 1 4 6
        2   2   3       4 7 9
  
2) If ascii file, first convert to binary using makebin

        makebin chess.ascii chess.data

Binary file MUST have .data extension   

3) Get configuration by running getconf 
(gen automatically generates conf file, so this step can be skipped)
   
       getconf -i chess -o chess -a


Before running the rest you should now have the following files

        chess.data
        chess.conf

4)  run: exttpose -i XXX -o XXX -l -s LMINSUP -a 0

        example: exttpose -i chess -o chess -l -s 0.2 -a 0

        or  exttpose -i chess -o chess -l -s 0 -a 0 
                        (this allows any minsup to be used later)

note: this produces the files XXX.tpose, and XXX.idx

The XXX.tpose file is the DB in vertical format, and
XXX.idx is an index file specifying where the tid-list for each item
begins.

You can specify a value of LMINSUP to be the same as the one you will use to
run charm below, in which case you will have to rerun exttpose each time you
use a new lower MINSUP. Alternatively, you can use a small value for LMINSUP,
and it will continue to work for all values of MINSUP >= LMINSUP when you
run elcat.


The time for inverting is stored in summary.out. The format is:

    TPOSE DB_FILENAME X NUMITEMS TOTAL_TIME

(see note one TOTAL_TIME below)               

You should now have the following files:

        chess.data
        chess.conf
        chess.tpose
        chess.idx

3) run charm

        charm -i XXX -e 1 -d -l -s <MINSUP>

        other flags
         -o output the patterns found
                output format: itemset - sup (tidset)
         -d uses diffsets instead of tidsets (from length 3 onwards)

         -l uses diffsets for pass 2 as well
                (this should NOT be used for sparse datasets, since tidset
                 size of pass 2 is smaller than diffset size for
                 sparse sets.)
         -H 1 for exact closed sets (takes longer time)
                the default is to mine a superset of the closed sets
                the use of -H 1 eliminates any non-closed sets using a
                    hashing technique

To run charm directly on horizontal DB

         charm -i XXX -h -d -l -s <MINSUP>

          -h option converts from a horizontal DB XXX.data to an
             in-memory vertical DB. Thus this version should only be
             used with small DB that can fit in memory. IF you need to
             run large DB then I have other scripts that first create
             a disk-based vertical DB and then charm runs on that
             DB. I think the current version is sufficient for most
             experiments, but if you will do performance tests for
             large DB then please ask for the other scripts.

          If you use -h option there is no need to run exttpose

MINSUP is in fractions, i.e., specify 0.5 if you want 50% minsup or
0.01 if you want 1% support.
 
note that the summary of the run is stored in the summary.out
file. The format of this file is as follows:

CHARM (other options) DB_FILENAME MINSUP NUMTRANS_IN_DB ACTUAL_SUPPORT
      [ ITER_i |Ci| |Fi| timeForIter_i avg_tidset/diffset_size ] 
      [TOT total_cands tot_freq tot_elapsed_time] 
      NumberofIntersections XXX XXX XXX XXX 
      tottime maxiters user_time sys_time

Note3: -e 1 option is a flag indicating charm to compute the support
of 2-itemsets from scratch. The number 1 says there is only one DB
partition that will be inverted entirely in main memory. If the
original DB is large then this inversion will obviously take too much
time. So in this case I recommend dividing the DB into chunks of size
roughly 5MB (assuming there is 32MB available to the process). The
exttpose program is equiped to handle this case. If you specify a <-p
NUMPART> flag to exttpose it will divide the DB into NUMPART
chunks. Now you can run charm with -e NUMPART option. You must do this
if the DB is large otherwise the timings for charm will be
skewed. Generally, the more the partitions the better the running time
for charm. For example:

        exttpose -i XXX -o XXX -l -a 0 -s LMINSUP -p 10
        charm -i XXX -s MINSUP -e 10
 

In summary run

        charm -i chess -e 1 -s 0.8 -o for the current project.



