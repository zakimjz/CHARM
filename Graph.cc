#include <stdlib.h>
#include <limits.h>
#include "Graph.h"
#include "Util.h"


int cmpint (const void * it1, const void * it2)
{
   int i1 = *(int *) it1;
   int i2 = *(int *) it2;
   if (i1 > i2) return 1;
   else if (i1 < i2) return -1;
   else return 0;
}

int GrItem::cmp_grit(const void *a, const void *b)
{
   int fa = (*(GrItem **)a)->adj();
   int fb = (*(GrItem **)b)->adj();

   int res;
   if (fa < fb) res = -1;
   else if (fa > fb) res = 1;
   else res = 0;
   return res;
}

ostream& operator << (ostream& fout, GrItem git)
{
   fout << git.adj();
   return fout;
}


int GrNode::cmp_vertex(const void *a, const void *b)
{
   long fa = (*F2Graph)[*((int *)a)]->supsum();
   long fb = (*F2Graph)[*((int *)b)]->supsum();
   
   int res;
   if (fa < fb) res = -1;
   else if (fa > fb) res = 1;
   else res = 0;
   
   if (sort_ascend == TRUE) return res;
   else return (-1*res);
}

boolean GrNode::find(int val)
{
   int ret;
   GrItem grit(val);
   return Util<GrItem *>::Bsearch(0, size()-1, array(),
                                  &grit, GrItem::cmp_grit, ret);
}

Graph::~Graph()
{
   for (int i=0; i < size(); i++){
      if ((*this)[i]) delete (*this)[i];
   }
}
 

void Graph::add_node(int item, int sup, int supsum)
{
   GrNode *grnode = new GrNode(0);
   grnode->item() = item;
   grnode->sup() = sup;
   grnode->supsum() = supsum;
   this->add(grnode);
}

void Graph::add_adj(int vert, int nbr, int nbrsup)
{
   GrItem *grit = new GrItem;
   grit->adj() = nbr;
   grit->sup() = nbrsup;
   
   if ((*this)[vert]->array() == NULL){
      (*this)[vert]->Realloc(2);
   }
   (*this)[vert]->add(grit);
}

boolean Graph::connected(int vi, int vj)
{
   if (vi < vj){
      if ((*this)[vi]->array() == NULL) return FALSE;
      else return (*this)[vi]->find(vj);
   }
   else if (vi > vj){
      if ((*this)[vj]->array() == NULL) return FALSE;
      else return (*this)[vj]->find(vi);
   }
   else return FALSE;
}


void Graph::sort()
{
   int i,j,idx;
   //cout << "INT_MAX " << INT_MAX << " " << LONG_MAX << endl;
   int nullval = (sort_ascend == TRUE) ? INT_MAX:0;

   //for (i=0; i < size(); i++){
   //   if ((*this)[i]->supsum() == 0)
   //      (*this)[i]->supsum() = nullval;
   //}

   int *posary = new int[size()];
   for (i=0; i < size(); i++) posary[i] = i;

   //cout << "POSARY " << size() << " : ";
   //for (i=0; i < size(); i++) cout << " " << posary[i];
   //cout << endl << flush;
   
   //qsort(this->array(), size(), sizeof(GrNode *), GrNode::cmp_vertex);
   qsort(posary, size(), sizeof(int), GrNode::cmp_vertex);

   //for (i=0; i < size(); i++)
   //   if ((*this)[i]->supsum() == nullval)
   //      break;
   //if (i < size()) this->compact(i);

   //cout << "POSARY " << size() << " : ";
   //for (i=0; i < size(); i++) cout << " " << posary[i];
   //cout << endl << flush;

   int *invary = new int[size()];
   for (i=0; i < size(); i++) invary[posary[i]] = i;
   
   Graph *sgr = new Graph(size());
   GrNode *grn;
   for (i=0; i < size(); i++){
      grn = (*this)[posary[i]];
      sgr->add_node(grn->item(), grn->sup(), grn->supsum());
      
   }
   
   
   for (i=0; i < size(); i++){
      grn = (*this)[i];   
      for (j=0; j < grn->size(); j++){
         idx = (*grn)[j]->adj();
         if (invary[idx] < invary[i]){
            sgr->add_adj(invary[idx], invary[i], (*grn)[j]->sup());
         }
         else if (invary[idx] > invary[i]){
            sgr->add_adj(invary[i], invary[idx], (*grn)[j]->sup());
         }
      }
   }

   for (i=0; i < size(); i++){
      if ((*sgr)[i]->size() > 0)
         qsort((*sgr)[i]->array(), (*sgr)[i]->size(), sizeof(GrItem *),
               GrItem::cmp_grit);
   }

   GrNode **tmp = array();
   array() = sgr->array();
   sgr->array() = NULL;
   delete tmp;
}


void Graph::print_iset(Itemset *it)
{
   int i;
   int *ary = new int[it->isetsize()];
   for (i=0; i < it->isetsize(); i++) ary[i] = (*this)[(*it)[i]]->item();
   qsort(ary,it->isetsize(),sizeof(int),cmpint);
   
   for (i=0; i < it->isetsize(); i++)
      cout << ary[i] << " ";
   cout << "-1 " <<  it->support() << endl;
}

