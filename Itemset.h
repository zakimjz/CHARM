#ifndef __ITEMSET_H
#define __ITEMSET_H

#include <stdlib.h>
#include <iostream>
#include "assoc.h"
#include "Array.h"
#include "defset.h"

class Itemset{
private:
   Array<int> *theItemset;
   Array<int> *theTidList;
   int theSupport;
   int theDiff; 
   int drop;
   int theHashval;
   boolean theMemflg;
   
   static Array<int> *TmpIset; //to store tmp result
   void iset_fill(Itemset *it1, Itemset *it2);
public:
   static int NumIntersect;
   static void alloc_tmpiset(int sz);
   static Itemset *intersect_kway(Itemset *it1, Array<Itemset *> *it,
                                  int noalloc=0, int copyit=0);
   static Itemset *intersect(Itemset *it1, Itemset *it2, int noalloc, 
			     int *subset);
   static Itemset *intersect2it(Itemset *it1, Itemset *it2, int *subset);
   static Itemset *diff2it(Itemset *it1, Itemset *it2, int *subset);
   static Itemset *diff(Itemset *it1, Itemset *it2, int noalloc, 
			  int *subset);
   static int cmp_iset(const void *a, const void *b);
   static int cmp_sup(const void *a, const void *b);
   static int cmp_drop(const void *a, const void *b);

   Itemset(int itsz=1, int tsz=0);
   Itemset(int itsz, Array<int> *tidary);
   ~Itemset();
   void copy(Array<int> *iset, Array<int> *tlist);
   
   int &hashval(){ return theHashval; }
   
   int & support(){
     return theSupport;
   }
   int & diff(){
     return theDiff;
   }
   //int &hval() { return hashval; }

   int isetsize(){
      if (theItemset) return theItemset->size();
      else return 0;
   }

   int tidsize(){
      if (theTidList) return theTidList->size();
      else return 0;
   }

   boolean& memflg(){ return theMemflg; }
   Array<int> *&iset(){ return theItemset; }
   int litem(){ return (*this)[isetsize()-1]; }
   Array<int> *&tidlist(){ return theTidList; } 
   int &tid(int idx){ return (*theTidList)[idx]; }
   int& operator [] (unsigned int index){ return (*theItemset)[index]; }
   void insert(int k);
   void setdrop(int d) { drop=d;}
   int& idrop() { return drop; }
   
   friend ostream& operator << (ostream& fout, Itemset& iset);
};

#endif //__ITEMSET_H
 
