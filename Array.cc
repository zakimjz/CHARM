#include <malloc.h>
#include <stdlib.h>

#include "assoc.h"
#include "Array.h"
#include "Graph.h"
#include "Util.h"
#include "Itemset.h"
#include "List.h"
#include "hashtable.h"

#ifdef SGI
/////////Template Instantiation Stuff////////////
#pragma instantiate Array<int>
#pragma instantiate Array<Array<int>*>
#pragma instantiate Array<GrNode *>
#pragma instantiate Array<GrItem *>
//#pragma instantiate Array<Array<Array<int>*> *>
//#pragma instantiate Array<Itemset *>
////////////////////////////////////////////////
#endif
#ifdef __GNUC__
/////////Template Instantiation Stuff////////////
//template class Array<int>;
//template class Array<Array<int>*>;
//template class Array<Array<Array<int>*>*>;
//template class Array<GrNode *>;
//template class Array<GrItem *>;
//template class Array<Itemset *>;
////////////////////////////////////////////////
#endif

template <class Items>
void Array<Items>::Realloc (int newlen)
{
   totSz = Util<Items>::Realloc (newlen, sizeof(Items), theAry);
}
 
template <class Items>
Array<Items>::Array(int sz)
{
   totSz = sz;
   theSz=0;
   theAry = NULL;
   if (sz > 0) Util<Items>::Realloc(totSz,sizeof(Items), theAry);
}


template <class Items>
Array<Items>::Array(int sz, Items *ary)
{
   totSz = sz;
   theSz= 0;
   theAry = ary;
}


template <class Items>
Array<Items>::Array(Array<Items> *ary)
{
   totSz = ary->theSz;
   theSz=ary->theSz;
   theAry = NULL;
   if (theSz > 0){
      Util<Items>::Realloc(totSz,sizeof(Items), theAry);
      for (int i=0; i < theSz; i++)
         theAry[i] = ary->theAry[i];
   }
}

template <class Items>
Array<Items>::~Array()
{
   if (theAry) free(theAry);
}

template <class Items>
void Array<Items>::copy (Array<Items> *ary)
{
  if (totSz < ary->theSz) Realloc(ary->theSz);
  theSz=ary->theSz;
  for (int i=0; i < theSz; i++) theAry[i] = ary->theAry[i]; 
}


template <class Items>
void Array<Items>::add(Items it)
{
   if (theSz+1 > totSz){
      Realloc((int) (grow*totSz));
   }
   theAry[theSz++] = it;
}
 
template <class Items>
void Array<Items>::compact(int nsz)
{
   if (nsz == -1)
      Realloc((unsigned int)theSz);
   else{
      Realloc((unsigned int)nsz);
      theSz = nsz;
   }
}

template <class Items>
int Array<Items>::compare(Array<Items>* ar2)
{
   return 0;
}

template <>
int Array<int>::compare(Array<int>* ar2)
{
   int len;
   if (size() <= ar2->size()) len = size();
   else len = ar2->size();
   for(int i=0; i < len; i++){
      if ((theAry)[i] > (ar2->theAry)[i]) return 1;
      else if ((theAry)[i] < (ar2->theAry)[i]) return -1;
   }
   if (size() < ar2->size()) return -1;
   else if (size() > ar2->size()) return 1;
   else return 0;
}

template <class Items>
int Array<Items>::subset(Array<Items>* ar2)
{
   return 0;
}

//return 0 if none is a subseq of other, 1 is this is subseq of ar, and -1 otw.
template <>
int Array<int>::subset(Array<int>* ar)
{
   int i,j;
   int sz1, sz2;
   Array<int> *ar1, *ar2;
   int retval;
   
   if (size() <= ar->size()){
      sz1 = size();
      sz2 = ar->size();
      ar1 = this;
      ar2 = ar;
      retval = 1;
   }
   else{
      sz1 = ar->size();
      sz2 = size();
      ar1 = ar;
      ar2 = this;
      retval = -1;
   }
   int start = 0;
   for(i=0; i < sz1; i++){
      for(j=start; j < sz2; j++){
         if ((ar1->theAry)[i] == (ar2->theAry)[j]){
            start = j+1;
            break;
         }
      }
      if (j >= ar2->size()) return 0;
   }
   return retval;
}


ostream& operator << (ostream& fout, Array<int>& ary){
   for (int i=0; i < ary.theSz; i++)
      fout << ary.theAry[i] << " ";
   return fout;
}

ostream& operator << (ostream& fout, Array<GrItem *>& ary){
   for (int i=0; i < ary.theSz; i++)
      fout << *ary.theAry[i] << " ";
   return fout;
}

ostream& operator << (ostream& fout, Array<GrNode *>& ary){
   for (int i=0; i < ary.theSz; i++)
      fout << *ary.theAry[i] << endl;
   return fout;
}

ostream& operator << (ostream& fout, Array<Array<int> *>& ary){
   for (int i=0; i < ary.theSz; i++)
      if (ary.theAry[i])
         fout << *ary.theAry[i] << ", ";
   fout << endl;
   return fout;
}

template<class I>
ostream& operator << (ostream& fout, Array<I>& ary){
   return fout;
}

#ifdef __GNUC__
/////////Template Instantiation Stuff////////////
template class Array<int>;
template class Array<Array<int>*>;
template class Array<Array<Array<int>*>*>;
template class Array<GrNode *>;
template class Array<GrItem *>;
template class Array<Itemset *>;
template class Array< List<HashItem *>*>;
class iterstat;
template class Array<iterstat *>;
////////////////////////////////////////////////
#endif
