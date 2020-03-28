#include "Itemset.h"
#include "Graph.h"

Array<int> *Itemset::TmpIset = NULL;
int Itemset::NumIntersect = 0;

Itemset::Itemset(int itsz, int tsz){
   theItemset = NULL;
   if (itsz > 0) theItemset = new Array<int>(itsz);
   theTidList = NULL;
   if (tsz > 0) theTidList = new Array<int>(tsz);
   theMemflg = FALSE;
   theSupport = 0;
   theHashval = 0;
   theDiff = 0;
   drop=0;
   //hashval = 0;
}

Itemset::Itemset(int itsz, Array<int> *tidary){
   theItemset = NULL;
   if (itsz > 0) theItemset = new Array<int>(itsz);
   theTidList = NULL;
   if (tidary->size() > 0) theTidList = new Array<int>(tidary);
   theMemflg = FALSE;   
   theSupport = 0;
   theDiff = 0;
   drop=0;
   theHashval = 0;
}

Itemset::~Itemset(){
   if (theItemset) delete theItemset;
   if (theTidList) delete theTidList;
}

void Itemset::copy(Array<int> *iset, Array<int> *tlist)
{
   if (iset) theItemset->copy(iset);
   if (tlist){
     if (!theTidList) theTidList = new Array<int>(tlist->size());
     theTidList->copy(tlist);
   }
   drop=0;
}


void Itemset::alloc_tmpiset(int sz)
{
   TmpIset = new Array<int>(sz);
}


void Itemset::iset_fill(Itemset *it1, Itemset *it2)
{
  for (int i=0; i < it1->isetsize(); i++)
    theItemset->optadd((*it1)[i]);
  theItemset->optadd((*it2)[it2->isetsize()-1]);
  
   //int i,j;


   //for (i=0, j =0; i < it1->isetsize() && j < it2->isetsize();){
   //  if ((*it1)[i] == (*it2)[j]){
   //    theItemset->optadd((*it1)[i]);
   //    i++;
   //    j++;
   //  }
   //  else if ((*it1)[i] < (*it2)[j]){
   //    theItemset->optadd((*it1)[i]);
   //    i++;
   //  }
   //  else{
   //    theItemset->optadd((*it2)[j]);
   //    j++;
   //  }
   //}
   //while (i < it1->isetsize()){
   //  theItemset->optadd((*it1)[i]);
   //  i++;
   //}
   //while (j < it2->isetsize()){
   //  theItemset->optadd((*it2)[j]);
   //  j++;
   //}                                                 

   //cout << "FILL \t" << *it1 << flush;
   //cout << *it2 << flush;
   //cout << *theItemset << endl;
}

Itemset *Itemset::intersect_kway(Itemset *it1, Array<Itemset *> *it,
                                 int noalloc, int copyit)
{
   int i, k;
   k = it->size();

   NumIntersect++;
   TmpIset->reset();
   //do the k-way intersect
   int *dc = new int[k];
   int *df = new int[k];
   int *pos = new int[k];
   
   for (i=0; i < k; i++){
      dc[i] = (*it)[i]->support()-MINSUPPORT;
      df[i] = 0;
      pos[i] = 0;
   }
   
   int done=0;
   int cnt, lcnt;
   while (!done){
      for (i=0; i < k; i++){
         if (pos[i] >= (*it)[i]->support()) done=1;
         if (!copyit && df[i] > dc[i]) done=1;
      }
      if (done) break;
      cnt=0;
      lcnt=0;
      for (i=1; i < k; i++){
         if ((*(*it)[0]->theTidList)[pos[0]] > (*(*it)[i]->theTidList)[pos[i]]){
            pos[i]++;
            df[i]++;
            lcnt++;
         }
         else if ((*(*it)[0]->theTidList)[pos[0]] ==
                  (*(*it)[i]->theTidList)[pos[i]]){
            cnt++;
         }
      }
      if (lcnt == 0){
         if (cnt == k-1){
            TmpIset->optadd((*(*it)[0]->theTidList)[pos[0]]);
            for (i=0; i < k; i++) pos[i]++;
         }
         else {
            df[0]++;
            pos[0]++;
         }
      }
   }
   delete [] dc;
   delete [] df;
   delete [] pos;
   
   Itemset *ret = NULL;
   if (TmpIset->size() >= MINSUPPORT){
      if (noalloc){
         ret = it1;
         it1->copy(NULL,TmpIset);
         //it1->iset()->optadd((*it2)[it2->isetsize()-1]);
      }
      else{
         ret = new Itemset(it1->isetsize()+1, TmpIset);
         //ret->iset_fill(it1,it2);
      }
   }
   else if (copyit){
      ret = it1;
      it1->copy(NULL,TmpIset);      
   }
   if (ret) ret->setdrop(0);
   return ret;
}

Itemset *Itemset::intersect(Itemset *it1, Itemset *it2, int noalloc, int *subset)
{
   NumIntersect++;
   TmpIset->reset();
   int dc1 = it1->support()-MINSUPPORT;
   int dc2 = it2->support()-MINSUPPORT;
   int tidsum = 0;
   int df1=0;
   int df2=0;
   int i,j;
   for (i=0,j=0; i < it1->support() && j < it2->support();){
      if (df1 > dc1 || df2 > dc2) break;

      if ((*it1->theTidList)[i] > (*it2->theTidList)[j]){
         j++;
         df2++;
      }
      else if ((*it1->theTidList)[i] < (*it2->theTidList)[j]){
         df1++;
         i++;
      }
      else{
         TmpIset->optadd((*it1->theTidList)[i]);
         tidsum += (*it1->theTidList)[i];
         j++;
         i++;
      }
   }

   if (TmpIset->size() >= MINSUPPORT){     
      if( i < it1->support() )  df1++;
      if( j < it2->support() )  df2++;
   }

   Itemset *ret = NULL;
   if (TmpIset->size() >= MINSUPPORT){
      if (noalloc){
         ret = it1;
         it1->copy(NULL,TmpIset);
	 it1->support() = TmpIset->size();
         it1->iset()->optadd((*it2)[it2->isetsize()-1]);
         it1->diff() = MINSUPPORT-it1->support();        
         it1->hashval() = tidsum;
      }
      else{
         ret = new Itemset(it1->isetsize()+1, TmpIset);
         ret->iset_fill(it1,it2);
	 ret->support() = TmpIset->size();
         ret->diff() = MINSUPPORT-ret->support();        
         ret->hashval() = tidsum;
      }
      if (subset != NULL){
         if (df1 == 0 && df2 == 0) *subset = AeqB;
         else if (df1 == 0 && df2 != 0) *subset = AsubB;
         else if (df1 !=0 && df2 == 0) *subset = BsubA;
         else *subset = AneqB;
      }
      ret->setdrop(it1->support()-TmpIset->size());
   }
   return ret;
}

Itemset *Itemset::intersect2it(Itemset *it1, Itemset *it2,int *subset)
{
   NumIntersect++;
   TmpIset->reset();
   int i,j,df1=0,df2=0;
   int tidsum = 0;
   int dc1 = it1->support()-MINSUPPORT;
   int dc2 = it2->support()-MINSUPPORT;

   for (i=0,j=0; i < it1->support() && j < it2->support();){
      if (df1 > dc1 || df2 > dc2) break;
      if ((*it1->theTidList)[i] > (*it2->theTidList)[j]){
         j++;
         df2++;
      }
      else if ((*it1->theTidList)[i] < (*it2->theTidList)[j]){
         i++;
         df1++;
      }
      else{
         TmpIset->optadd((*it1->theTidList)[i]);
         tidsum += (*it1->theTidList)[i];
         j++;
         i++;
      }
   }

   if (TmpIset->size() >= MINSUPPORT){     
      if( i < it1->support() )  df1++;
      if( j < it2->support() )  df2++;
   }   

   Itemset *ret = NULL;
   if (TmpIset->size() >= MINSUPPORT){
      ret = new Itemset(it1->isetsize()+1, TmpIset);
      ret->iset_fill(it1,it2);
      ret->support() = TmpIset->size();
      ret->diff() = MINSUPPORT-ret->support();        
      ret->hashval() = tidsum;

      if (subset != NULL){
         if (df1 == 0 && df2 == 0) *subset = AeqB;
         else if (df1 == 0 && df2 != 0) *subset = AsubB;
         else if (df1 !=0 && df2 == 0) *subset = BsubA;
         else *subset = AneqB;
      }
      ret->setdrop(it1->support()-TmpIset->size());
   }
   return ret;
}

Itemset *Itemset::diff2it(Itemset *it1, Itemset *it2,int *subset)
{
   NumIntersect++;
   TmpIset->reset();
   int i,j,df1=0,df2=0;
   int diffsum=0;
   //int hval = 0;
   int dc1 = it1->support()-MINSUPPORT;
   //int dc2 = it2->support()-MINSUPPORT;

   for (i=0,j=0; i < it1->support() && j < it2->support() 
	  && df1 <= dc1;){
      if (it1->tid(i) > it2->tid(j)){
         j++;
         df2++;
      }
      else if (it1->tid(i) < it2->tid(j)){
	TmpIset->optadd(it1->tid(i));
        diffsum += it1->tid(i);
	i++;
	df1++;
      }
      else{
	//hval += it1->tid(i);
         j++;
         i++;
      }
   }

   for (; i < it1->support() && df1 <= dc1; i++){
     TmpIset->optadd(it1->tid(i));
     diffsum += it1->tid(i);
     df1++;
   }
   if (j < it2->support() && df1 <= dc1) df2++;     

   Itemset *ret = NULL;
   if (df1 <= dc1){
      ret = new Itemset(it1->isetsize()+1, TmpIset);
      ret->iset_fill(it1,it2);
      ret->diff() = df1;
      ret->support() = it1->support() - ret->diff(); 
      ret->hashval() = it1->hashval() - diffsum;
      //ret->hval() = hval;
      if (subset != NULL){
         if (df1 == 0 && df2 == 0) *subset = AeqB;
         else if (df1 == 0 && df2 > 0) *subset = AsubB;
         else if (df1 > 0 && df2 == 0) *subset = BsubA;
         else *subset = AneqB;
      }
      ret->setdrop(TmpIset->size());
   }
   return ret;
}

Itemset *Itemset::diff(Itemset *it1, Itemset *it2, int noalloc, int *subset)
{
   int i,j;

   NumIntersect++;
   TmpIset->reset(); 
   int diff1= 0;
   int diff2 = 0;
   int diffsum = 0;
   //int hval = 0;
   int osup1 = it1->support();
   int dc1 = it1->support()-MINSUPPORT;

   for (i=0,j=0; i < it2->diff() && j < it1->diff() 
	  && diff2 <= dc1;){
      if (it2->tid(i) > it1->tid(j)){
	diff1++;
         j++;
      }
      else if (it2->tid(i) == it1->tid(j)){
         j++;
         i++;
      }
      else{
	//hval += it2->tid(i);
         TmpIset->optadd(it2->tid(i));
         diffsum += it2->tid(i);
         diff2++;
         i++;
      }
   }
   for (; i < it2->diff() && diff2 <= dc1; i++){
     //hval += it2->tid(i);
     TmpIset->optadd(it2->tid(i));
     diffsum += it2->tid(i);
     diff2++;
   }
   if (j < it1->diff() && diff2 <= dc1) diff1++;
   

   //TmpIset->support() = it1->support() - TmpIset->diff();

   Itemset *ret = NULL;
   if (diff2 <= dc1){
      if (noalloc){
         ret = it1;
         it1->copy(NULL,TmpIset);
         it1->iset()->optadd((*it2)[it2->isetsize()-1]);
	 it1->diff() = diff2;
	 it1->support() = osup1 - it1->diff();
	 it1->hashval() = it1->hashval() - diffsum;
      }
      else{
         ret = new Itemset(it1->isetsize()+1, TmpIset);
         ret->iset_fill(it1,it2);
	 ret->diff() = diff2;
	 ret->support() = it1->support() - ret->diff();
	 ret->hashval() = it1->hashval() - diffsum;
      }
      if (subset != NULL){
         if (diff1 == 0 && diff2 == 0) *subset = AeqB;
         else if (diff1 == 0 && diff2 > 0) *subset = BsubA;
         else if (diff1 > 0 && diff2 == 0) *subset = AsubB;
         else *subset = AneqB;
      }
      ret->setdrop(TmpIset->size());
   }
   return ret;
}
 
ostream& operator << (ostream& fout, Itemset& iset){
   int i,j=0;
   
   //if (iset.isetsize() != 2) return fout;
   
   if (iset.iset()){
      int *temp= new int[iset.isetsize()];
      for (i=0; i < iset.isetsize(); i++){
         int k=(*F2Graph)[iset[i]]->item();
#ifndef SORT_ITEM
         fout << k << " ";
      }
#else
         temp[j]=k;
         j++;
      }
      qsort(temp,iset.isetsize(),sizeof(int),cmpint);
      fout <<": ";
      for(i=0;i<  iset.isetsize(); i++)
         fout << temp[i] << " " ;
#endif
      delete temp;
   }
   fout << "- ";
   fout << iset.support();
//fout << " " << iset.hashval();

//   if (iset.tidlist()){
//      fout << " ( ";
//      for (i=0; i < iset.tidlist()->size(); i++)
//         fout << (*iset.tidlist())[i] << " "; 
//   }
//   fout << ")" << endl;
   fout << endl;
   return fout;
}

int Itemset::cmp_iset(const void *a, const void *b)
{
   Itemset *ia = (Itemset *)a;
   Itemset *ib = (Itemset *)b;

   for (int i=0; i < ia->isetsize(); i++){
      if ((*ia)[i] > (*ib)[i]) return 1;
      else if ((*ia)[i] < (*ib)[i]) return -1;
   }
   return 0;
}

int Itemset::cmp_sup(const void *a, const void *b)
{
   Itemset *ia = (Itemset *)a;
   Itemset *ib = (Itemset *)b;

   if (ia->support() > ib->support()) return 1;
   else if (ia->support() < ib->support()) return -1;
   return 0;
}

int Itemset::cmp_drop(const void *a, const void *b)
{
   Itemset *ia = (Itemset *)a;
   Itemset *ib = (Itemset *)b;

   if (ia->idrop() > ib->idrop()) return 1;
   else if (ia->idrop() < ib->idrop()) return -1;
   return 0;
}

void Itemset::insert(int k) {
   theItemset->add((*theItemset)[theItemset->size()-1]);
   (*theItemset)[theItemset->size()-2]=k;
}
   
/*
   Array<int> *tmp = new Array<int>(2);
   for(i=0;i<prefix;i++)
      tmp->add((*theItemset)[i]);
   tmp->add(k);
   for(j=prefix;j<theItemset->size();j++)
      tmp->add((*theItemset)[i]);
   delete theItemset;
   theItemset = tmp;
}
*/
