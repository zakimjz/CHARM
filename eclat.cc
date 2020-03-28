#include <iostream>
#include <stdlib.h>

using namespace std;

#include "eclat.h"
#include "assoc.h"
#include "Graph.h"
#include "Itemset.h"
#include "memman.h"
#include "Array.h"
//#include "gbk.h"
#include "Util.h"
//#include "cliques.h"
#include "maxset.h"
#include "defset.h"
#include "Count.h"
#include "hashtable.h"
#include "stats.h"

extern unsigned long sumtidlist;

extern int DBASE_MAXITEM;
boolean ECLAT_bfs=TRUE;
boolean ECLAT_eqc=TRUE;
int ECLAT_search=BOTUP;
#define ECLAT_maxisetsz 100

CountAry FreCount, CloCount;
Itemset *Eqclass::iset = NULL;

Array<int> NumLargeItemset;
int maxiter = 2;
maxset *MaxSet;
HashTable *HT;
cHashTable *cHT;

void Eqclass::alloc_tmpiset(int sz)
{
   iset = new Itemset(ECLAT_maxisetsz,sz);
}

#ifdef PRUNE
boolean prune_test(Eqclass *EQ) {
   ListNodes<Itemset *> *tempnode=EQ->head();
   Array<int> *tempAry=new Array<int>(tempnode->item()->iset());
   
   for(tempnode=tempnode->next();tempnode;tempnode=tempnode->next()) {
      tempAry->add(tempnode->item()->litem());
   }
   boolean TF=MaxSet->subset(tempAry);
   delete tempAry;
   return TF;
}
#endif

//calculate totmem for elements of an eqclass,
//if add is true we add to TOTMEM, else we substract
void getmem(Eqclass *cluster, boolean add){
  long sum=0;
  ListNodes<Itemset *> *hdr;

  for (hdr = cluster->head(); hdr; hdr=hdr->next()){
    sum += hdr->item()->tidsize();
  }

  if (add) TOTMEM += sum;
  else TOTMEM -= sum;
  cout << TOTMEM << endl; //print out the tot mem
}

//here
void process_cluster(Eqclass *cluster, int iter)
{
  if (memflg) getmem(cluster, TRUE);

   if (maxiter < iter) maxiter = iter;
  
   //cout << "cluster:" << *cluster ;
   ListNodes<Itemset *> *hdr, *hdr2,*phdr2,*tempnode;
   int subset;
   tempnode=cluster->head();

   for (hdr = cluster->head(); hdr; hdr=hdr->next()){
      Eqclass *EQ = new Eqclass;
      phdr2 =hdr; 

      sumtidlist += hdr->item()->tidsize();
      for (hdr2=hdr->next(); hdr2 ; ){
         sumtidlist += hdr2->item()->tidsize();
	Itemset *temp;
         //cout << "hdr->item" << *hdr->item() ;
         //cout << "hdr->item()->tid():"  << *hdr->item()->tidlist() << endl;
         //cout << "hdr2->item()" << *hdr2->item() ;
         //cout << "hdr2->item()->tid():"  << *hdr2->item()->tidlist() << endl;
        if (use_diff){
           if (!diff_input && !use_diff_f2 && iter == 2)
              temp=Itemset::diff2it(hdr->item(), hdr2->item(),&subset);        
           else
              temp=Itemset::diff(hdr->item(), hdr2->item(),0,&subset);
        }       
	else temp=Itemset::intersect(hdr->item(), hdr2->item(),0,&subset);

        if (hdr->item()->isetsize() > 1)
           stats.incrcand(hdr->item()->isetsize());
        
         if (temp){
          FreCount.add(temp->iset()->size());
          stats.incravgtid(temp->iset()->size()-1,temp->tidsize()); 

          switch(subset){
          case AeqB: //A=B, A=AB, & delete B
             //cout << "subset: " << subset << " A=B" << endl;
             //cout << "hdr->item" << *hdr->item() ;
             //cout << "hdr->item()->tid():"  << *hdr->item()->tidlist() << endl;
             //cout << "hdr2->item()" << *hdr2->item() ;
             //cout << "hdr2->item()->tid():"  << *hdr2->item()->tidlist() << endl;
	    if (use_diff){
	      //hdr->item()->insert(hdr2->item()->litem());
	      hdr->item()->copy(temp->iset(), NULL);
	      //hdr->item()->hval() = temp->hval();
	      //temp->copy(NULL, hdr->item()->tidlist());
	      //temp->support() = hdr->item()->support();
	      //temp->diff() = hdr->item()->diff();
	      //temp->idrop() = hdr->item()->idrop();
	    }
	    else{
               //delete hdr->item();
               //hdr->item()=new Itemset(*temp);
               //hdr->item()->support() = temp->support();
               //hdr->item()->diff() = temp->diff();
               hdr->item()->insert(hdr2->item()->litem());
               delete temp;
               
	    }
	     for(tempnode= EQ->head(); tempnode; tempnode=tempnode->next()){
             //replace A with AB
                tempnode->item()->insert(hdr2->item()->litem());
                FreCount.add(tempnode->item()->iset()->size());
                stats.incravgtid(tempnode->item()->iset()->size()-1,
                                 tempnode->item()->tidsize()); 

             }
             if(phdr2){
                phdr2->lnext()=hdr2->next();
                delete hdr2;
                hdr2=phdr2;
             }
             else {
                cluster->set_head(hdr2->next());
                delete hdr2;
                hdr2=NULL;
             }
             //cout << "after A=B" << *hdr->item() << endl;
             break;
          case AsubB: // A<B,replace A with AB
             //cout << "subset: " << subset << " A<B" << endl;
             //cout << "temp" << *temp ;
             //hdr->set_bit();
             if (use_diff){
	       //hdr->item()->insert(hdr2->item()->litem());
	       hdr->item()->copy(temp->iset(), NULL);
	       //hdr->item()->hval() = temp->hval();
	       //temp->copy(NULL, hdr->item()->tidlist());
	       //temp->support() = hdr->item()->support();
	       //temp->diff() = hdr->item()->diff();
	       //temp->idrop() = hdr->item()->idrop();
	     }
	     else {
                //delete hdr->item();
                //hdr->item()=new Itemset(*temp);
                //hdr->item()->support() = temp->support();
                //hdr->item()->diff() = temp->diff();
               hdr->item()->insert(hdr2->item()->litem());
               delete temp;
	     }
             tempnode=EQ->head();
             for(; tempnode; tempnode=tempnode->next()){ //replace A with AB
                tempnode->item()->insert(hdr2->item()->litem());
                FreCount.add(tempnode->item()->iset()->size());
                stats.incravgtid(tempnode->item()->iset()->size()-1,
                                 tempnode->item()->tidsize()); 

             }
             //cout << "hdr: " << *hdr->item() << endl;
             break;
          case BsubA: // A>B, delete B & save AB
             //cout << "subset: " << subset << " A>B" << endl;
             //hdr->set_bit();
             if(phdr2) {
                phdr2->lnext()=hdr2->next();
                delete hdr2;
                hdr2=phdr2;
             }
             else {
                cluster->set_head(hdr2->next());
                delete hdr2;
                hdr2=NULL;
             }
             break;
          case AneqB: //A<>B,nothing
             //cout << "subset: " << subset << " A<>B" << endl;
             //hdr->set_bit();
             //hdr2->set_bit();
             break;
          }            // edn of switch
          //cout << "join: " << *temp ;
          if(subset == BsubA || subset == AneqB ){            
             if (process_order == DE_SUP)
                EQ->addsort(temp,Itemset::cmp_sup,0);
             else if (process_order == IN_SUP)
                EQ->addsort(temp,Itemset::cmp_sup,1);
             else
                EQ->append(temp);
          }  //end of if (subset>=2)
         } //end if temp
         if(hdr2) {
            phdr2=hdr2;
            hdr2=hdr2->next();
         }
         else {
           cout << "ERROR"  << endl;
           //hdr2=cluster->head();
         }
      }// end of hdr2

      bool flg = true;
      
      if(use_hash){
         if(!HT->add(hdr->item())) flg = false;
      }
      else if (use_hash_map){
         if (!cHT->add(hdr->item())) flg = false;
      }
      if (flg){
         CloCount.add(hdr->item()->iset()->size());
         if (use_output) cout  << *hdr->item();
      }
      
      if (EQ->size() > 1){
         process_cluster(EQ, iter+1);
         delete EQ;
      }
      else   {
        if(EQ->size()==1) {
           flg = true;
           
           if(use_hash){
              if (!HT->add(EQ->head()->item())) flg = false;
           }
           else if (use_hash_map){
              if (!cHT->add(EQ->head()->item())) flg = false;
           }
           if (flg){
              CloCount.add(EQ->head()->item()->iset()->size());
              if (use_output) cout  << *EQ->head()->item();
           }
        }
        if(EQ) delete EQ;
     }  //end of EQ->size()
   }  // end of hdr
/*
   for(tempnode=cluster->head();tempnode ;tempnode=tempnode->next()) {
      if(!tempnode->chbit()){
        CloCount.add(tempnode->item()->iset()->size());
        cout << *tempnode->item() ;
      }
   }
*/
  if (memflg) getmem(cluster, FALSE);
}

void process_F1(Eqclass *F1EQ) 
{
   int iter =1;   
   ListNodes<Itemset *> *hdr, *hdr2,*phdr2,*tempnode;
   int subset;
   Array <int> *cls=new Array<int>;
   Itemset *it1;
   
   hdr = F1EQ->head();
   for (; hdr; hdr=hdr->next()){
      it1 = Memman::read_from_disk(hdr->item()->litem(),DCB);     
      stats.incravgtid(0,it1->tidsize()); 
      delete hdr->item(); //new
      hdr->item()=it1; //new
      Eqclass *EQ = new Eqclass;
      phdr2=hdr;      
      GrNode *grn = (*F2Graph)[hdr->item()->litem()];
      cls->reset();
      for (int j=0; j < grn->size(); j++){
         cls->add((*grn)[j]->adj());
      }      
      for (hdr2 = hdr->next(); hdr2 ; hdr2=hdr2->next()){
         int i2=hdr2->item()->litem();
         //cout << "hdr2:" << *hdr2->item() ;
         if(cls->search(i2)==-1) {
           phdr2=hdr2;
           continue;
         }
         //else cout << "found" << endl;

         if (use_horizontal){
            Itemset *it2 = Memman::read_from_disk(hdr2->item()->litem(),DCB);
            //delete Eqclass::iset;
            Eqclass::iset = it2;            
         }
         else{
            (*Eqclass::iset->iset())[0] = hdr2->item()->litem();
            Eqclass::iset->iset()->size()=1;
            Eqclass::iset->memflg() = FALSE;
            Memman::read_from_disk(Eqclass::iset,0);
         }
         
	 Itemset * temp;
         if (diff_input)
            temp = Itemset::diff(hdr->item(), Eqclass::iset, 0, &subset);
         else if (use_diff_f2) 
            temp = Itemset::diff2it(hdr->item(), Eqclass::iset, 
                                    &subset);
	 else
            temp = Itemset::intersect2it(hdr->item(), Eqclass::iset, 
                                         &subset);
         


	 // cout << "hdr->item" << *hdr->item() ;
         //cout << "hdr->item()->tid():"  << *it1->tidlist() << endl;
         //cout << "hdr2->item()" << *hdr2->item() ;
         //cout << "hdr2->item()->tid():"  << *Eqclass::iset->tidlist() << endl;

         if (temp){
            //if ((*F2Graph)[(*temp)[0]]->item() == 46 && 
            //    (*F2Graph)[(*temp)[1]]->item() == 17)
            //   cout << "XXX " << *temp;
            
            FreCount.add(temp->iset()->size());
            stats.incravgtid(temp->iset()->size()-1,temp->tidsize()); 

            switch(subset){
            case AeqB: //A=B, A=AB, & delete B
	    //cout << "subset: " << subset << " A=B" << endl;
	    if (use_diff_f2 || diff_input){
	      //hdr->item()->insert(hdr2->item()->litem());
	      hdr->item()->copy(temp->iset(), NULL);
	      //hdr->item()->hval() = temp->hval();
	      //temp->copy(NULL, hdr->item()->tidlist());
	      //temp->support() = hdr->item()->support();
	      //temp->diff() = hdr->item()->diff();
	      //temp->idrop() = hdr->item()->idrop();
              //hdr->item()->support() = temp->support();
              //hdr->item()->diff() = temp->diff();
	    }
	    else{
               //delete hdr->item();
               //hdr->item()=new Itemset(*temp);
               //hdr->item()->support() = temp->support();
               //hdr->item()->diff() = temp->diff();
               hdr->item()->insert(hdr2->item()->litem());
               delete temp;               
	    }
	     for(tempnode= EQ->head();tempnode; tempnode=tempnode->next()){ 
             //replace A with AB
                tempnode->item()->insert(hdr2->item()->litem());
                FreCount.add(tempnode->item()->iset()->size()); 
                stats.incravgtid(tempnode->item()->iset()->size()-1,
                                 tempnode->item()->tidsize()); 

             }
             phdr2->lnext()=hdr2->next();
             delete hdr2;
             hdr2=phdr2;             
             break;
          case AsubB: // A<B,replace A with AB
	    //cout << "subset: " << subset << " A<B" << endl;
             //hdr->set_bit();
	    if (use_diff_f2 || diff_input){
	      //hdr->item()->insert(hdr2->item()->litem());
	      hdr->item()->copy(temp->iset(), NULL);
	      //hdr->item()->hval() = temp->hval();
	      //temp->copy(NULL, hdr->item()->tidlist());
	      //temp->support() = hdr->item()->support();
	      //temp->diff() = hdr->item()->diff();
	      //temp->idrop() = hdr->item()->idrop();
              //hdr->item()->support() = temp->support();
              //hdr->item()->diff() = temp->diff();
	    }
	    else{
	       //delete hdr->item();
	       //hdr->item()=new Itemset(*temp);
	       //hdr->item()->support() = temp->support();
	       //hdr->item()->diff() = temp->diff();
               hdr->item()->insert(hdr2->item()->litem());
               delete temp;              
	    }
             tempnode=EQ->head();
             for(; tempnode; tempnode=tempnode->next()){ //replace A with AB
                tempnode->item()->insert(hdr2->item()->litem());
                FreCount.add(tempnode->item()->iset()->size()); 
                stats.incravgtid(tempnode->item()->iset()->size()-1,
                                 tempnode->item()->tidsize());
             }
             break;
          case BsubA: // A>B, delete B & save AB
	    //cout << "subset: " << subset << " A>B" << endl;
             //hdr->set_bit();
             phdr2->lnext()=hdr2->next();
             delete hdr2;
             hdr2=phdr2;
             break;
          case AneqB: //A<>B,nothing
	    //cout << "subset: " << subset << " A<>B" << endl;
             //hdr->set_bit();
             //hdr2->set_bit();
             break;
          }            // edn of switch

          //cout << "  d:" << temp->idrop() << "join: " << *temp << endl;
          if(subset == BsubA || subset == AneqB ){
             if (process_order == DE_SUP)
                EQ->addsort(temp,Itemset::cmp_sup,0);
             else if (process_order == IN_SUP)
                EQ->addsort(temp,Itemset::cmp_sup,1);
             else
                EQ->append(temp);
                  //cout << "EQ:" << *EQ << endl;
          }  //end of if (subset>=2)
         } //end if temp
       phdr2=hdr2;
       //cout<< "F1EQ" << *F1EQ << endl;
      }// end of hdr2     
      
      //if (EQ) cout << "EQ:" << *EQ << endl;

      bool flg = true;
      
      if(use_hash){
         if (!HT->add(hdr->item())) flg = false;
      }
      else if (use_hash_map){
         if (!cHT->add(hdr->item())) flg = false;
      }
      if (flg){
         CloCount.add(hdr->item()->iset()->size());
         if (use_output) cout  << *hdr->item();
      }

      if (EQ->size() > 1){       
         process_cluster(EQ, iter+1);
         delete EQ;        
      }
      else      {
        if(EQ->size()==1) {
           flg = true;
           if(use_hash){
              if (!HT->add(EQ->head()->item())) flg = false;
           }
           else if (use_hash_map){
              if (!cHT->add(EQ->head()->item())) flg = false;
           }
           if (flg){
              CloCount.add(EQ->head()->item()->iset()->size());
              if (use_output) cout  << *EQ->head()->item();
           }
        }
        else if(!hdr->chbit()){
           //cout << *hdr->item();
#ifdef PRUNE
           if(hdr->item()->isetsize()>3)
			   MaxSet->add(hdr->item());
           //cout << "MaxSet" << *MaxSet << endl;
#endif
        }
        if(EQ) delete EQ;
      } // End of EQ->size
   }  // end of hdr
/*
   for(tempnode=F1EQ->head();tempnode ;tempnode=tempnode->next()) {
     CloCount.add(tempnode->item()->iset()->size());
     cout << *tempnode->item() ;
   }
*/
}

void process_eqc()
{
 // creat F1EQClass
   //cout << Graph::numF1 << endl;
   Eqclass *F1EQ= new Eqclass;

   if (use_hash) HT= new HashTable; 
   else if (use_hash_map) cHT = new cHashTable;
   
   FreCount.kadd(1,F2Graph->size());
   for(int i=0; i < F2Graph->size(); i++){
      Itemset *F1= new Itemset(1,0);
      F1->iset()->add(i);    
      F1EQ->append(F1);
   }
   
   process_F1(F1EQ);
   if (use_hash_map) cHT->print_hashstats();

}

void ECLAT_Find_Freq()
{
   NumLargeItemset.add(Graph::numF1);
   NumLargeItemset.add(0);   
   process_eqc();   
   //FreCount.print();
   //if (use_output) CloCount.print();
   //cout << *HT << endl;
   //HT->print();
//   RCloCount.print();
}




