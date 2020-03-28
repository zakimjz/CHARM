#ifndef __MEMMAN_H
#define __MEMMAN_H

#include "assoc.h"
#include "Itemset.h"
#include "calcdb.h"

class Memman{
private:
public:
   static void read_from_disk(Itemset *iset, 
                              int alloc_flg=1); 
   static Itemset * read_from_disk(int it, Dbase_Ctrl_Blk *DCB);
};

#endif //__MEMMAN_H
