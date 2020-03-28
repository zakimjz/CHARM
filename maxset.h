#ifndef __MAXSET_H
#define __MAXSET_H

#include "assoc.h"
#include "Array.h"
#include "Itemset.h"

class maxset{
private:
   Array<Itemset *> *maxary;
   Array<int> **idxary;
   Array<int> **curary;
   int *pos;
   int idxarysz;
public:
   maxset(int maxsz=2, int idxsz=2);
   ~maxset();

   Array<int> *maxlist(int it){ return idxary[it]; }
   Itemset *maxit(int it){ return (*maxary)[it]; }
   boolean add(Itemset *iset);
   boolean add(Array<int> *iset);
   boolean subset(Array<int> *iset);
   friend ostream& operator << (ostream& fout, maxset& mset);
};

#endif //__MAXSET_H
