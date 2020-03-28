#ifndef __ARRAY_H
#define __ARRAY_H
#include <iostream>
using namespace std;

const float grow = 2;
template <class Items>
class Array{
   friend ostream& operator << (ostream& fout, Array<Items>& ary);
private:
   int theSz;
   int totSz;
   Items *theAry;
public:
   Array(int sz, Items *ary);
   Array(int sz=2);
   Array(Array<Items> *ary);
   ~Array();

   void copy (Array<Items> *ary);
   void Realloc(int newlen);
   void compact(int nsz=-1);
   void add(Items it);

   void optadd(Items it){ theAry[theSz++] = it; }
   Items *& array(){ return theAry; }
   void reset(){ theSz = 0; }
   int& size(){ return theSz; }
   int& totsize(){ return totSz; }
   Items& operator [] (int index){ return theAry[index]; }
   
   int compare(Array<Items> *a2);

   int subset(Array<Items> * ar);
  //int subset(Array<int> * ar);

   int search(Items key) {
      int head=0;
      int tail=theSz-1,mid;
      
      while(head <= tail) {
	 mid=(head+tail);
         if(theAry[mid] == key)
             return mid;
         if(theAry[mid] < key)
            head=mid+1;
         else 
            tail=mid-1;
      }

      return -1;
   }   
};

#endif //__ARRAY_H










