#ifndef __LISTS_H
#define __LISTS_H

#include<iostream>
using namespace std;

template <class Items>
class ListNodes {
private:
   ListNodes<Items> *theNext;
   Items theItem;
   int chb;

public:
   ListNodes(Items item, ListNodes<Items> *next);
   ~ListNodes(){}
   
   ListNodes<Items>* next (){ return theNext; }
   ListNodes<Items>*& lnext (){ return theNext; }
   Items& item(){ return theItem; }
   void set_item(Items it){ theItem = it;}
   void set_bit() { chb=1; }
   int chbit() { return chb;}
};

template <class Items>
class List {
   friend ostream& operator << (ostream& fout, List<Items>& lst);   
private:
   ListNodes<Items> *theHead;
   ListNodes<Items> *theLast;
   int theSize;
   
public: 
   List();
   ~List();
   void clear();
   ListNodes<Items> *head (){ return theHead; }
   ListNodes<Items> *last (){ return theLast; }
   int size (){ return theSize; }

   void set_head(ListNodes<Items> *hd){ theHead = hd;}
   void append (Items item);   
   void prepend(Items item);
   int addsort (Items item, int (*cmp) (const void *, const void *), int ascend=1);
   void remove(ListNodes<Items> *prev, ListNodes<Items> *val);
   Items frontremove();   
   //   friend class ostream;

};


#endif //__LISTS_H
