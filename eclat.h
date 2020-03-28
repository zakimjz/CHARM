#ifndef __ECLAT_H
#define __ECLAT_H
#include<iostream>
#include "assoc.h"
#include "List.h"
#include "Itemset.h"
#include "Array.h"
#include "Graph.h"

#define BOTUP 0
#define HYBRID 1

extern boolean ECLAT_bfs;
extern boolean ECLAT_eqc;
extern int ECLAT_search;
extern Array<int> NumLargeItemset;
extern int maxiter;

extern Array<int> *MaxAry;

class Eqclass: public List<Itemset *>{
public:
   static Itemset *iset;
   static void alloc_tmpiset(int sz);
};
void find_large_bfs(Eqclass *cluster, int iter);
void process_hybrid_main(Eqclass *cluster, int iter);

extern boolean prune2it (int vi, int vj);
extern Eqclass *get_ext_eqclass(int i1, Array<int> *cls, int sort);
extern void process_rhybrid(Eqclass *cluster, int iter);

#endif //__ECLAT_H
