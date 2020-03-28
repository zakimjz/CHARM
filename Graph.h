#ifndef _GRAPH_H
#define _GRAPH_H

#include "assoc.h"
#include "Array.h"
#include "Itemset.h"

class GrItem{
private:
   int it;
   int itsup;
public:
   GrItem(int itt=0, int ittsup=0):it(itt), itsup(ittsup){};
   int& adj(){ return it; }
   int& sup(){ return itsup; }
   static int cmp_grit(const void *a, const void *b);
   friend ostream& operator << (ostream& fout, GrItem git);
};

class GrNode: public Array<GrItem *>{
private:
   int theItem;
   int theItemSup;
   long theSupSum;
   //Array<int> *theSup;
public:
   GrNode(int sz=0):Array<GrItem *>(sz), //theSup(Array<int>(sz)),
   theItem(-1), theItemSup(0), theSupSum(0){}
   int& item(){ return theItem; }
   int& sup(){ return theItemSup; }
   long & supsum(){ return theSupSum; }
   boolean find(int val);
   static int cmp_vertex(const void *a, const void *b);
};

class Graph: public Array<GrNode *>{
private:
public:
   static int numF1;
   Graph(int nv):Array<GrNode *>(nv){}
   ~Graph();
   void add_node(int item, int sup, int supsum=0);
   void add_adj(int vert, int nbr, int nbrsup);
   boolean connected(int vi, int vj);
   void sort();
   void print_iset(Itemset *it);
};

#endif //_GRAPH_H
