#include <iostream>
#include <limits.h>

#include "eclat.h"
#include "maxset.h"
#include "Graph.h"


maxset::maxset(int maxsz, int idxsz){
   int i;
   maxary = new Array<Itemset *>(maxsz);
   idxarysz = idxsz;
   idxary = new Array<int> *[idxsz];
   for (i=0; i < idxsz; i++)
      idxary[i] = NULL;
   curary = new Array<int> *[idxsz];
   for(i=0; i < idxsz; i++){
      curary[i] = NULL;
   }
   pos = new int[idxsz];
}
maxset::~maxset()
{
   int i;
   delete maxary;
   for (i=0; i < idxarysz; i++)
      if (idxary[i]) delete idxary[i];
   delete [] idxary;
   delete [] curary;
   delete [] pos;
}

boolean maxset::add(Itemset *iset){
   int i;
   //if (subset(iset->iset()) == TRUE) return FALSE;
   //cout << "ADD " << *iset;
   
   Itemset *it = new Itemset(iset->isetsize(),0);
   it->copy(iset->iset(), NULL);
   maxary->add(it);
   int idx = maxary->size()-1;
   for (i=0; i < iset->isetsize(); i++){
      if (idxary[(*iset)[i]] == NULL){
         idxary[(*iset)[i]] = new Array<int>(2);
      }
      idxary[(*iset)[i]]->add(idx);
   }
   return TRUE;
}

boolean maxset::add(Array<int> *iset){
   int i;
   if (subset(iset) == TRUE) return FALSE;
   //cout << "ADD " << *iset;
   
   Itemset *it = new Itemset(iset->size(),0);
   it->copy(iset, NULL);
   maxary->add(it);
   int idx = maxary->size()-1;
   for (i=0; i < iset->size(); i++){
      if (idxary[(*iset)[i]] == NULL){
         idxary[(*iset)[i]] = new Array<int>(2);
      }
      idxary[(*iset)[i]]->add(idx);
   }
   return TRUE;
}

boolean maxset::subset(Array<int> *iset){
   int i, j;
   //cout << *iset << endl << flush;
   int minidx = 0;
   int minval = INT_MAX;
   for (i=0; i < iset->size(); i++){
      if (idxary[(*iset)[i]] == NULL) return FALSE;
      //if (minval > idxary[(*iset)[i]]->size()){
      //minval = idxary[(*iset)[i]]->size();
      //minidx = (*iset)[i];
      //}
      curary[i] = idxary[(*iset)[i]];
   }
   //curary[0] = idxary[minidx];
   //for (i=0,j=1; i < iset->size(); i++){
   //   if (minidx != (*iset)[i])
   //      curary[j++] = idxary[(*iset)[i]];
   //}
   
   int k = iset->size();
   
   for (i=0; i < k; i++) pos[i] = 0;
   
   int cnt, lcnt;
   while (1){
      for (i=0; i < k; i++){
         if (pos[i] >= curary[i]->size()) return FALSE;
      }
      lcnt= cnt = 0;
      for (i=1; i < k; i++){
         if ((*curary[0])[pos[0]] > (*curary[i])[pos[i]]){
            pos[i]++;
            lcnt++;
         }
         else if ((*curary[0])[pos[0]] == (*curary[i])[pos[i]]) cnt++;
      }
      if (lcnt == 0){
         if (cnt == k-1) return TRUE;
         else pos[0]++;
      }
   }
   
   return FALSE;
}

ostream& operator << (ostream& fout, maxset& mset){
   int i;
   
   for (i=0; i < mset.maxary->size(); i++)
      fout << i << ": " << *(*mset.maxary)[i] << endl;
   
   for (i=0; i < mset.idxarysz; i++){
      fout << i << " " << (*F2Graph)[i]->item() << " -- ";
      if (mset.idxary[i]) fout << *mset.idxary[i];
      fout << endl;
   }
   return fout;
}



