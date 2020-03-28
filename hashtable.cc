#include <algorithm>  
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <iostream>
#include "assoc.h"
#include "hashtable.h"
#include "Count.h"
#include "timetrack.h"

CountAry RCloCount;
TimeTracker tt;

HashItem::HashItem(Itemset *item, int hashsize) {
   ary=new Array<int>(item->iset()->size());
   ary->copy(item->iset());
   qsort(ary->array(), ary->size(),sizeof(int),cmpint);
   sup=item->support();
   unsigned long value=0;
   //value=(*item->tidlist())[sup-1];
   //if (use_diff && item->isetsize() > 1) value = item->hval();
   //else{
   
   value = item->hashval();

   //if (use_diff && use_hash){
   //   value = item->hashval();
   // }
   //else{
   //   for (int i=0; item->tidlist() && i < item->tidlist()->size()-1; i+=3){
   //      value += (*item->tidlist())[i];
   //  }
   //}

   //value = value + sup;
     //}
   //cout << "VAL " << value << " ISET: " << *item;
   hval=(int)(value%hashsize);
}

HashItem::HashItem(Itemset *item) {
   ary=new Array<int>(item->iset()->size());
   ary->copy(item->iset());
   qsort(ary->array(), ary->size(),sizeof(int),cmpint);
   sup=item->support();
   unsigned long value=0;
   value = item->hashval();
   //cout << "VAL " << value << " ISET: " << *item;
   hval=value;
}


int HashItem::cmp (const void *a, const void *b) {
   HashItem *ia = (HashItem *)a;
   HashItem *ib = (HashItem *)b;
   if (ia->support() > ib->support())  return 1;
   else if (ia->support() < ib->support()) return -1;
   else if (ia->array()->size() > ib->array()->size() ) return 1;
   else if( ia->array()->subset(ib->array()) ) return 0;
   else return 1;
}

bool HashItem::subset(HashItem *ia)
{
   int i,j;
   if (ary->size() > ia->ary->size()) return false;
   for (i=0,j=0; i < ary->size() && j < ia->ary->size();){
      if ((*ary)[i] < (*ia->ary)[j]) return false;
      else if ((*ary)[i] > (*ia->ary)[j]) j++;
      else{
         i++;
         j++;
      }
   }
   if (i < ary->size()) return false;
   return true;
}
 

int HashItem::cmp2 (const void *a, const void *b) {
   HashItem *ia = (HashItem *)a;
   HashItem *ib = (HashItem *)b;
   if (ia->support() > ib->support())  return 1;
   else if (ia->support() < ib->support()) return -1;
   else if (ia->array()->size() > ib->array()->size() ) return 1;
   else if( ia->subset(ib) ) return 0;
   else if (ia->array()->size() < ib->array()->size() ) return -1;
   else return 1;
}

HashTable::HashTable (int size)
{
   int i, root;
   if (size < 3)
   {
      size = 3;
   }
   // Smallest power of 2 >= x
   for (i = 4; i < size; i *= 2);
   theSize = i+1;
   root = (int) sqrt((double)theSize);
   i = 3;
   while (i <= root)
   {
      if ((theSize % i) == 0)     // n is composite:
      {
         i = 1;                // restart loop and
         theSize += 2;            // check next odd n
         root = (int) sqrt((double)theSize);
      }
      i += 2;
   }
   cout << "SIZE : "<< theSize << " " << size << endl;
   theCells = new Array< List<HashItem *> * >(theSize);   
   if (theCells == NULL){
      cout << "MEMORY EXCEEDED\n";
      exit(-1);
   }
   for (i=0; i < theSize; i++){
      (*theCells)[i] = NULL;
   }
   num_find=0;
   num_probe=0;
}


HashTable::~HashTable()
{
   delete [] theCells;
}



int HashTable::add (Itemset * item)
{
   num_find++;num_probe++; //??
   HashItem *Hitem= new HashItem(item, theSize);

   int pos;
   int i;
   pos = Hitem->HashVal();

   if ( (*theCells)[pos] == NULL){
       (*theCells)[pos]= new List<HashItem *>;
       (*theCells)[pos]->append(Hitem);
       return 1;
   }
   else {
       return (*theCells)[pos]->addsort(Hitem,HashItem::cmp,2);
   }
   //return pos;
}

void HashTable::clear()
{
   for (int i=0; i < theSize; i++){
      (*theCells)[i] = NULL;
   }
}
void HashTable::clear_cells()
{
   for (int i=0; i < theSize; i++){
      delete (*theCells)[i];
      (*theCells)[i] = NULL;
   }
}

ostream& operator << (ostream& outputStream, HashItem& Hitem){
   outputStream << *Hitem.array()<< " - " << Hitem.support() << endl;
}

ostream& operator << (ostream& outputStream, HashTable& hasht){
   outputStream << "HASH TABLE: Size = " <<hasht.theSize << "\n" ;
   outputStream.flush();
   for (int i=0; i < hasht.theSize; i++){
      outputStream << i << ": " << endl;
      ListNodes<HashItem *> *hd = (*hasht.theCells)[i]->head();
      for (; hd; hd = hd->next()) {
         outputStream << *hd->item();
         int k=hd->item()->array()->size();
         RCloCount.add(k);
      }
      //outputStream << *(*hasht.theCells)[i] << endl;
   }
    RCloCount.print();
   return  outputStream;
}

void HashTable::print() {
   //cout << "HASH TABLE: Size = " << theSize << "\n" ;
   for (int i=0; i < theSize; i++){
      //cout << i << ": " << endl;
      if(!(*theCells)[i]) continue;
      ListNodes<HashItem *> *hd = (*theCells)[i]->head();
      for (; hd; hd = hd->next()) {
         //cout << *hd->item();
         int k=hd->item()->array()->size();
         RCloCount.add(k);
      }
   }
   RCloCount.print();
    cout<< flush;
   return;
}

//try to add new closed set to hash table
bool cHashTable::add(Itemset *iset)
{
   HashItem *Hitem= new HashItem(iset);

   int hres = chtable.hash_funct()(Hitem->HashVal());
   cHTFind p = chtable.equal_range(hres);
   cHTable:: iterator hi = p.first;

   int ires;
   for (; hi!=p.second; hi++){
      ires = HashItem::cmp2(Hitem, (*hi).second);

      if (ires == 0){
         delete Hitem;
         return false;
      }
   }
   chtable.insert(cHTPair(hres, Hitem));
   return true;
}

