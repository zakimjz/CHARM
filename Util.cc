#ifdef __GNUC__
//#pragma implementation "Util.h"
#endif
#include "Util.h"
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "Array.h"
#include "Graph.h"
#include "Itemset.h"
#include "hashtable.h"
#include "List.h"
class HashItem;
#ifdef SGI
/////////Template Instantiation Stuff////////////
#pragma instantiate Util<int>
#pragma instantiate Util<GrItem *>
#pragma instantiate Util<GrNode *>
#pragma instantiate Util<Array<int> *>
#pragma instantiate Util<Array<Array<int> * > * >
#pragma instantiate Util<Itemset *>
////////////////////////////////////////////////
#endif


int Choose(int n, int k)
{
   int i;
   int val = 1;
 
   if (k >= 0 && k <= n){
      for (i=n; i > n-k; i--)
         val *= i;
      for (i=2; i <= k; i++)
         val /= i;
   }
   
   return val;
}

template <class Items>
boolean Util<Items>::Bsearch(int min, int max, Items *itary, Items it,
                      CMPFUNC cfunc, int& ret)
{
   int mid = (max+min)/2;
   if (max < min) return FALSE; 
   
   //cout << "MAX " << min  << " " << mid << " " << max << endl << flush;
   int cres = cfunc((void *)&it, (void *) &itary[mid]);
   if (cres == 0){
      if (ret) ret = mid;
      return TRUE;
   }
   else if (cres < 0) return Bsearch(min, mid-1, itary, it, cfunc, ret);
   else return Bsearch(mid+1, max, itary, it, cfunc, ret);   
}

template <class Items>
int Util<Items>::Realloc (int newlen, int elsz, Items*&ary)
{
   ary = (Items *)::realloc (ary, newlen*elsz);
   if (ary == NULL && newlen != 0){
      //cout << "NEWLEN = " << newlen << endl;     
      perror("REALLOC");
      exit(-1);
   }
   return newlen;
}

#ifdef __GNUC__
/////////Template Instantiation Stuff////////////
template class Util<int>;
template class Util<GrItem *>;
template class Util<GrNode *>;
template class Util<Array<int> *>;
template class Util<Array<Array<int> *>*>;
template class Util<Itemset *>;
template class Util<List<HashItem*>*>;
template class Util<Array< List<HashItem*> *>* >;
class iterstat;
template class Util<iterstat *>;
////////////////////////////////////////////////
#endif
