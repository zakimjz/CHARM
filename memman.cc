#include "memman.h"
#include "assoc.h"
#include "partition.h"
#include "Graph.h"
extern unsigned long int total_scan;

void Memman::read_from_disk(Itemset *iset, int alloc_flg)
{
   if (iset->memflg()) return;

   int tidsum = 0;
   
   int it = (*F2Graph)[(*iset)[0]]->item();
   int supsz = partition_get_idxsup(it);
   total_scan+=supsz;

   if (alloc_flg){
      if (iset->tidlist()) iset->tidlist()->Realloc(supsz);
      else iset->tidlist() = new Array<int>(supsz);
   }
   
   partition_read_item(iset->tidlist()->array(), it);

   iset->tidlist()->size() = supsz;
   iset->memflg() = TRUE;

   for (int i=0; i < iset->tidlist()->size(); i++)
      tidsum += (*iset->tidlist())[i];
   

   if (diff_input){
      iset->diff() = supsz;
      iset->support() = DBASE_NUM_TRANS - supsz;      
      iset->hashval() = tidsum;
   }
   else{
      iset->support() = supsz;
      iset->diff() = 0;
      iset->hashval() = tidsum;
   }
   
}

Itemset * Memman::read_from_disk(int it, Dbase_Ctrl_Blk *DCB)
{
   int rit = (*F2Graph)[it]->item();
   int supsz;
   int tidsum = 0;
   
   if (use_horizontal) supsz = DCB->tidlists[it]->size();
   else supsz = partition_get_idxsup(rit);
   Itemset *iset;
   
   if (use_horizontal) iset = new Itemset(1, 0);
   else iset = new Itemset(1,supsz);
   
   
   total_scan+=supsz;

   if (use_horizontal){
     iset->tidlist() = DCB->tidlists[it];
   }
   else{   
      partition_read_item(iset->tidlist()->array(), rit);
   }

   iset->iset()->optadd(it);
   iset->tidlist()->size() = supsz;
   iset->memflg() = TRUE;

   for (int i=0; i < iset->tidlist()->size(); i++)
      tidsum += (*iset->tidlist())[i];
   
   if (diff_input){
      iset->diff() = supsz;
      iset->support() = DBASE_NUM_TRANS - supsz;      
      iset->hashval() = tidsum;
   }
   else{
      iset->support() = supsz;
      iset->diff() = 0;
      iset->hashval() = tidsum;
   }

   return iset;
}



