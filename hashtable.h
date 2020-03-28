#ifndef __HASHTABLE_H
#define __HASHTABLE_H


#include <ext/hash_map> 

#include  "Itemset.h"
#include  "List.h"

using namespace __gnu_cxx;

#define HASHNS __gnu_cxx
#define HASHSIZE 100000

class HashItem;
class HashTable {
private:
   unsigned int theSize;
   int num_find;
   int num_probe;
   Array< List<HashItem *> * > *theCells;
public:
   HashTable (int sz=HASHSIZE);
   ~HashTable();
   
   int add (Itemset *);
   void clear(); //simply clear cells without deleting contents
   void clear_cells(); //deletes cell contents
   inline int size()
   {
      return theSize;
   }
   void print();
   friend ostream& operator << (ostream&, HashTable&);
};

class HashItem {
private:
   Array<int> *ary;
   int sup;
   int hval;
public:
   HashItem(Itemset *item, int hashsize);
   HashItem(Itemset *item);
   int HashVal() { return hval;}
   int support() {return sup;}
   Array<int>* array() { return ary; }

   bool subset(HashItem *ia);
   static int cmp (const void *a, const void *b);
   static int cmp2 (const void *a, const void *b);
   friend ostream& operator << (ostream& fout, HashItem& lst);
};


#define cHASHSIZE 100

typedef HASHNS::hash_multimap<int, HashItem *, HASHNS::hash<int>, equal_to<int> > cHTable;
typedef pair<cHTable::iterator, cHTable::iterator> cHTFind;
typedef cHTable::value_type cHTPair;

class cHashTable{
public:
   cHTable chtable;
   cHashTable(int sz=cHASHSIZE): chtable(sz){}

   bool add(Itemset *iset);

   void print_hashstats(){
      cout << "HASHSTATS " << chtable.size() << " "
           << chtable.bucket_count() << endl;
   }
};
 
#endif //__HASHTABLE_H
