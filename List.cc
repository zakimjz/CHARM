#ifdef __GNUC__
//#pragma implementation "List.h"
#endif
#include <stdlib.h>
#include <iostream>
#include "List.h"
#include "Itemset.h"
#include "hashtable.h"

#ifdef SGI
////////Template Stuff////////////////
#pragma instantiate List<int>::~List
#pragma instantiate List<Itemset *>
#include "eclat.h"
#pragma instantiate List<Eqclass *>
#pragma instantiate List<int>
#pragma instantiate List<Array <int> *>
//////////////////////////////////////
#endif

template <class Items>
ListNodes<Items>::ListNodes(Items item, ListNodes<Items> *next)
{
   theItem = item;
   theNext = next;
   chb=0;
}


template <class Items>
List<Items>::List(): theHead(NULL), theLast(NULL), theSize(0){}


template <class Items>
List<Items>::~List(){
   ListNodes<Items> *node = theHead;
   ListNodes<Items> *tmp;
   while (node){
      tmp = node;
      node = node->next();
      if (tmp->item()) delete tmp->item();
      delete tmp;
   }
}

//for int type, we can't delete the tmp->item()
template <>
List<int>::~List(){
   ListNodes<int> *node = theHead;
   ListNodes<int> *tmp;
   while (node){
      tmp = node;
      node = node->next();
      delete tmp;
   }
}

template <class Items>
void List<Items>::clear()
{
   ListNodes<Items> *node = theHead;
   ListNodes<Items> *tmp;
   while (node){
      tmp = node;
      node = node->next();
      delete tmp;
   }
   theHead = NULL;
}

template <class Items>
void List<Items>::append (Items item){
   theSize++;
   ListNodes<Items> *node = new ListNodes<Items> (item, 0);
   if (node == NULL){
      cout << "MEMORY EXCEEDED\n";
      exit(-1);
   }
   
   if (theHead == NULL){
      theHead = node;
      theLast = node;
   }
   else{
      theLast->lnext() = node;
      theLast = node;
   }
}

template <class Items>
void List<Items>::prepend (Items item){
   theSize++;
   ListNodes<Items> *node = new ListNodes<Items> (item, 0);
   if (node == NULL){
      cout << "MEMORY EXCEEDED\n";
      exit(-1);
   }
   
   if (theHead == 0){
      theHead = node;
      theLast = node;
   }
   else{
      node->lnext() = theHead;
      theHead = node;
   }
}

template <class Items>
Items List<Items>::frontremove()
{
   if (theHead == NULL){
      return (Items)0;
   }
   else{
      ListNodes<Items> *tmp = theHead;
      Items tmpit = tmp->item();
         
      if (theHead == theLast){
         theHead = theLast = NULL;
      }
      else theHead = theHead->next();
      theSize--;
      delete tmp;
      return tmpit;
   }
}

ostream& operator << (ostream& fout, List<int>& lst)
{
   ListNodes<int> *hd = lst.theHead;
   for (; hd; hd = hd->next())
      fout << hd->item() << " ";
   return fout;
}

ostream& operator << (ostream& fout, List<Itemset *>& lst)
{
   ListNodes<Itemset *> *hd = lst.theHead;
   for (; hd; hd = hd->next())
      fout << *hd->item();
   return fout;
}

ostream& operator << (ostream& fout, List<HashItem *>& lst)
{
   ListNodes<HashItem *> *hd = lst.theHead;
   for (; hd; hd = hd->next())
      fout << *hd->item();
   return fout;
}

template<class Items>
ostream& operator << (ostream& fout, List<Items>& lst)
{
   return fout;
}

template<class Items>
int List<Items>::addsort (Items item, int(*cmp)(const void *, const void *), int ascend)
{
   int res;
   ListNodes<Items> *node;
   ListNodes<Items> *temp = theHead;

   //cout << "IN ADDSORT " << endl << flush;
   //printf("theSize %d\b", theSize);
   theSize++;
   node = new ListNodes<Items>(item, 0);
   
   if (theHead == 0){
      theHead = node;
      theLast = node;
   }
   else{
      res = cmp((void *)item,(void *)theHead->item());
      if(ascend==2 && res==0) return 0;
      if ((ascend && res < 0) || (!ascend && res > 0)){
         node->lnext() = theHead;
         theHead = node;
      }
      else{
         while (temp->next()){
            res = cmp((void *)item,(void *)temp->next()->item());
            if(ascend==2 && res==0) return 0;
            if ((ascend && res < 0) || (!ascend && res > 0)){
               node->lnext() = temp->next();
               temp->lnext() = node;
               return 1;
            }
            temp = temp->next();
         }
         theLast->lnext() = node;
         theLast = node;
      }
   }
}


template<class Items>
void List<Items>::remove(ListNodes<Items> *prev, ListNodes<Items> *val)
{
   if (prev == NULL) theHead = val->next();
   else prev->lnext() = val->next();
   if (theLast == val) theLast = prev;
   theSize--;
}


#ifdef __GNUC__
////////Template Stuff////////////////
#include "eclat.h"
template class List<int>;
template class List<Itemset *>;
template class List<Eqclass *>;
template class List<Array <int> *>;
template class List<HashItem *>;
//////////////////////////////////////
#endif
