#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <strings.h>
#include <limits.h>

#include "extl2.h"
#include "assoc.h"
#include "partition.h"
#include "Graph.h"
#include "Util.h"

#define isetbufsz 1024

extern unsigned long int DB_size;
int isetbuf[isetbufsz];
int isetpos=0;
int isetfd;

unsigned char **cset_sup;
unsigned int **set_sup;

invdb *invDB;
int EXTBLKSZ;

int Graph::numF1=0;
extern char tempf[];

invdb::invdb(int sz): Array<Array<int> *>(sz)
{
   for (int i=0; i < totsize(); i++){
      (*this)[i] = NULL;
   }   
}

invdb::~invdb()
{
   for (int i=0; i < totsize(); i++){
      if ((*this)[i]) delete (*this)[i];
   }
}

void invdb::add_db(int tid, int it)
{
   if ((*this)[tid] == NULL){
      (*this)[tid] = new Array<int>((int)DBASE_AVG_TRANS_SZ);
   }
   (*this)[tid]->add(it);
}

int cmp2it(const void *a, const void *b)
{
   int *ary = (int *)a;
   int *bry = (int *)b;
   if (ary[0] < bry[0]) return -1;
   else if (ary[0] > bry[0]) return 1;
   else{
      if (ary[1] < bry[1]) return -1;
      else if (ary[1] > bry[1]) return 1;
      else return 0;
   }
}

int make_horizontal_l1_pass(Dbase_Ctrl_Blk &DCB)
{
   int i,j;
   int supsz;
   int maxsup = 0;
   
   int *itcnt = new int [DBASE_MAXITEM];
   bzero((char *)itcnt, DBASE_MAXITEM*sizeof(int));

   F2Graph = new Graph(100);
   Graph::numF1 = 0;

   int *buf, custid, tid, numitem;
   DCB.get_first_blk();
   DCB.get_next_trans(buf, numitem, tid, custid);
   while (!DCB.eof()){
      for (j=0; j < numitem; j++){
         itcnt[buf[j]]++;
      }
      DCB.get_next_trans(buf, numitem, tid, custid);
      //cout << "TRANS " << tid << " " <<numitem << endl;
   }
 
   DCB.freqidx = new int[DBASE_MAXITEM];


   DCB.tidbuflen=0;
   for (i=0; i < DBASE_MAXITEM; i++){
      supsz = itcnt[i];
      DB_size+=supsz;
      if (diff_input) supsz = DBASE_NUM_TRANS - supsz;      
      if (supsz >= MINSUPPORT){
         //if (diff_input) stats[0]->avgtid += DBASE_NUM_TRANS - supsz;
         //else stats[0]->avgtid += supsz;
         
         F2Graph->add_node(i,supsz);
         DCB.freqidx[i] = Graph::numF1;
	 //cout << "LARGE " << i << " " << Graph::numF1 << " " << supsz << endl;
         Graph::numF1++;
         DCB.tidbuflen += supsz;
      }
      else DCB.freqidx[i] = -1;
      
      if (supsz > maxsup) maxsup = supsz;
   }
   cout << "xx  Graph::numF1 " << Graph::numF1 << endl;
   
   F2Graph->compact();
   delete [] itcnt;

   int fd=-1;
   //if ((fd = open ("/tmp/tidbuf", (O_WRONLY|O_CREAT|O_TRUNC), 0666)) < 0){
   //  perror("Can't open tidbuf file");
   //  exit (errno);
   //}                                                               
   DCB.tidbuf = (int *) mmap ((char *)NULL, DCB.tidbuflen*ITSZ, 
			      (PROT_READ|PROT_WRITE), 
			      MAP_ANON|MAP_PRIVATE, fd, 0);
   if (DCB.tidbuf == (int *)-1){
     perror("tidbuf MMAP ERROR");
     exit(errno);
   }
   DCB.tidlists = new Array<int> *[Graph::numF1];
   
   int offt = 0;
   for (i=0; i < Graph::numF1; i++){
     DCB.tidlists[i] = new Array<int> ((*F2Graph)[i]->sup(), &DCB.tidbuf[offt]);
     offt += (*F2Graph)[i]->sup();
   }

   return maxsup;
}


int make_vertical_l1_pass()
{
   int i;
   int supsz;
   int idxval = -1;   
   int maxsup = 0;
   
   F2Graph = new Graph(100);
   Graph::numF1 = 0;
   for (i=0; i < DBASE_MAXITEM; i++){
      idxval = partition_idxval(i);
      if (idxval == -1) continue; //item has no tidlist      
      supsz = partition_get_idxsup(i);
      DB_size+=supsz;
      if (diff_input) supsz = DBASE_NUM_TRANS - supsz;      
      if (supsz >= MINSUPPORT){
         F2Graph->add_node(i,supsz);
         Graph::numF1++;
      }
      if (supsz > maxsup) maxsup = supsz;
   }
   cout << "Graph::numF1 " << Graph::numF1 << endl;
   
   F2Graph->compact();
   return maxsup;
}

int make_l1_pass(Dbase_Ctrl_Blk &DCB)
{
   if (use_horizontal) return make_horizontal_l1_pass(DCB);
   else return make_vertical_l1_pass();

}


void process_cust_invert(int curcnt, int *curit){
   int i,j;
   int it1, it2;

   //for (i=0; i < curcnt; i++)
   //   cout << " " << (*F2Graph)[curit[i]]->item();
   //cout << endl;

   if (use_char_extl2){
      for (i=0; i < curcnt; i++){
         it1 = curit[i];
         if (cset_sup[it1]){
            for (j=i+1; j < curcnt; j++){
               it2 = curit[j];
               if ((++cset_sup[it1][it2-it1-1]) == 0){
                  if (isetpos+2 > isetbufsz){
                     write(isetfd, (char *)isetbuf, isetpos*ITSZ);
                     isetpos = 0;
                  }
                  isetbuf[isetpos++] = it1;
                  isetbuf[isetpos++] = it2;
               }
            }
         }
      }
   }
   else{
      for (i=0; i < curcnt; i++){
         it1 = curit[i];
         if (set_sup[it1]){
            for (j=i+1; j < curcnt; j++){
               it2 = curit[j];
               set_sup[it1][it2-it1-1]++;
            }
         }
      }      
   }
}

void process_diff_cust(int curcnt, int *curit){
   int i,j;
   static Array<int> trans(100);
   trans.reset();
   
   //for (i=0; i < curcnt; i++)
   //   cout << " " << curit[i];
   //cout << endl;

   i = 0; //iterate over numitems
   j = 0; //idx into curit
   
   for (j=0; j < curcnt; j++){
      while (i < curit[j]){
         if (partition_idxval((*F2Graph)[i]->item()) != -1) trans.add(i);
         i++;
      }
      i++; //skip over curit[j]
   }
   for (; i < Graph::numF1; i++){
      if (partition_idxval((*F2Graph)[i]->item()) != -1) trans.add(i);
   }
   process_cust_invert(trans.size(), trans.array());
   
}

void process_invert(int pnum)
{
   int i,k;
   int minv, maxv;
   if (diff_input){
      minv = DBASE_MINTRANS;
      maxv = DBASE_MAXTRANS;      
   }
   else{      
      minv = INT_MAX;
      maxv = 0;
      for (i=0; i < Graph::numF1; i++){
         partition_get_minmaxtid(pnum, (*F2Graph)[i]->item(), minv, maxv);
      }
   }

   //cout << "MINMAX " << minv << " " << maxv << endl;
   if (invDB->totsize() < maxv-minv+1)
      invDB->Realloc(maxv-minv+1);
   
   int supsz;
   int ivalsz=0;
   int *ival = NULL;
   for (i=0; i < Graph::numF1; i++){
      supsz = partition_get_lidxsup(pnum, (*F2Graph)[i]->item());
      if (ivalsz < supsz){
         ivalsz = Util<int>::Realloc(supsz, ITSZ, ival);
      }
      partition_lclread_item(ival, pnum, (*F2Graph)[i]->item());
      int cid;
      int midx;
      for (int pos=0; pos < supsz; pos++)
      {
         cid = ival[pos];
         midx = cid - minv;
         invDB->add_db(midx,i);
      }
   }
   for (k=0; k < maxv-minv+1; k++){
      if (diff_input){
         if ((*invDB)[k]){
            if (Graph::numF1-(*invDB)[k]->size() > 0)
               process_diff_cust((*invDB)[k]->size(), (*invDB)[k]->array());
            (*invDB)[k]->reset();
         }
         else process_diff_cust(0, NULL);
      }
      else{
         if ((*invDB)[k] && (*invDB)[k]->size() > 0){
            //cout << "MVAL " << minv+k << " " << (*invDB)[k]->size() << endl;
            //for (int x = 0; x < (*invDB)[k]->size(); x++){
            //   cout << " " << (*F2Graph)[(*(*invDB)[k])[x]]->item();
            //}
            //cout << endl;
            
            process_cust_invert((*invDB)[k]->size(), (*invDB)[k]->array());
            (*invDB)[k]->reset();
         }
      }
   }
}

 
void sort_get_l2(int &l2cnt, int fd)
{
   //write 2-itemsets counts to file

   int i, j, k, fcnt;
   long sortflen;
   int *sortary;

   sortflen = lseek(fd, 0, SEEK_CUR);
   if (sortflen < 0){
      perror("SEEK SEQ");
      exit(errno);
   }
   //cout << "SORT " << sortflen << endl;
   if (sortflen > 0){
#ifdef DEC
      sortary = (int *) mmap((char *)NULL, sortflen,
                             (PROT_READ|PROT_WRITE),
                             (MAP_FILE|MAP_VARIABLE|MAP_PRIVATE), fd, 0);
#else
      sortary = (int *) mmap((char *)NULL, sortflen,
                             (PROT_READ|PROT_WRITE),
                             MAP_PRIVATE, fd, 0);
#endif
      if (sortary == (int *)-1){
         perror("SEQFd MMAP ERROR");
         exit(errno);
      }
      
      qsort(sortary, (sortflen/ITSZ)/2, 2*ITSZ, cmp2it);
      
   }
   
   int numel = sortflen/ITSZ;
   i = 0;
   fcnt = 0;
   for (j=0; j < Graph::numF1;j++){
      for (k=j+1; k < Graph::numF1;k++){
         fcnt = 0;
         if (sortflen > 0 && i < numel){
            while (i < numel && j == sortary[i] && k == sortary[i+1]){
               fcnt += 256;
               i += 2;
            }
         }
         if (cset_sup[j]) fcnt += (int) cset_sup[j][k-j-1];
         
         if (fcnt >= MINSUPPORT){
            //F2Graph->add_adj(j, (*F2Graph)[k]->item(), fcnt);
            F2Graph->add_adj(j, k, fcnt);
            (*F2Graph)[j]->supsum() += fcnt;
            (*F2Graph)[k]->supsum() += fcnt;
            //cout << "LARGE " << (*F2Graph)[j]->item() << " " <<
            //   (*F2Graph)[k]->item() << " SUPP " << fcnt << endl;
            l2cnt++;
         }
      }
   }
   if (sortflen > 0) munmap((caddr_t)sortary, sortflen);
}

void get_l2(int &l2cnt)
{
   int j,k;
   int fcnt;
   
   for (j=0; j < Graph::numF1;j++){
      if (set_sup[j]){
         for (k=j+1; k < Graph::numF1;k++){
            fcnt = set_sup[j][k-j-1];
            if (fcnt >= MINSUPPORT){
               F2Graph->add_adj(j, k, fcnt);
               (*F2Graph)[j]->supsum() += fcnt;
               (*F2Graph)[k]->supsum() += fcnt;
               l2cnt++;
            }
         }
      }
   }
}

void get_ext_l2(int &l2cnt)
{
   int i;

   int mem_used=0;

   EXTBLKSZ = num_partitions+(DBASE_NUM_TRANS+num_partitions-1)/num_partitions;
   int tsz = (int) (DBASE_AVG_TRANS_SZ);
   invDB = new invdb(EXTBLKSZ);
   mem_used += EXTBLKSZ*ITSZ;
   mem_used += (int) (EXTBLKSZ*tsz*ITSZ);
   //cout << "CURITSZ " << tsz << " " << EXTBLKSZ << " " << mem_used << endl;
   
   char TEMPF[300];
   sprintf(TEMPF,"%siset",tempf);
   if (use_char_extl2){
      if ((isetfd = open(TEMPF, (O_RDWR|O_CREAT|O_TRUNC), 0666)) < 0){
         perror("Can't open out file");
         exit (errno);      
      }
      cset_sup = new unsigned char *[Graph::numF1];        // support for 2-itemsets
      bzero((void *)cset_sup, Graph::numF1*sizeof(unsigned char *));   
      mem_used += Graph::numF1*sizeof(unsigned char *);
   }
   else{
      set_sup = new unsigned int *[Graph::numF1];        // support for 2-itemsets
      bzero((void *)set_sup, Graph::numF1*sizeof(unsigned int *));   
      mem_used += Graph::numF1*sizeof(unsigned int *);      
   }
   int low, high;
      
   int itsz;
   if (use_char_extl2) itsz = sizeof(unsigned char);
   else itsz = sizeof(unsigned int);
   
   for (low = 0; low < Graph::numF1; low = high){
      if (use_char_extl2){
         isetpos = 0;
         lseek(isetfd, 0, SEEK_SET);
      }
      
      for (high = low; high < Graph::numF1 &&
              (mem_used+(Graph::numF1-high-1)*itsz) < AVAILMEM; high++){
         if (Graph::numF1-high-1 > 0){
            if (use_char_extl2){
               cset_sup[high] = new unsigned char [Graph::numF1-high-1];
               bzero((void *)cset_sup[high], (Graph::numF1-high-1)*itsz);
            }
            else{
               set_sup[high] = new unsigned int [Graph::numF1-high-1];
               //cout << "ALLOC " << high << " " << set_sup[high] << " "
               //     << Graph::numF1-high-1 << endl;
               bzero((void *)set_sup[high], (Graph::numF1-high-1)*itsz);
            }
         }
         mem_used += (Graph::numF1-high-1) * itsz;
         //cout << "MEMUSEDLOOP " << mem_used << " " << endl;
      }
      //cout << "MEMUSEDLOOP " << mem_used << endl;  
      //cout << "LOWHIGH " << high << " " << low << endl;

      for (int p=0; p < num_partitions; p++){
         process_invert(p);
      }
      
      if (use_char_extl2){
         if (isetpos > 0){
            write(isetfd, (char *)isetbuf, isetpos*ITSZ);
            isetpos = 0;
         }
         sort_get_l2(l2cnt, isetfd);
      }
      else get_l2(l2cnt);
      
      // reclaim memory
      for (i = low; i < high; i++)
      {
         if (use_char_extl2){
            //cout << "i " << i << " " << cset_sup[i] << endl << flush;
            if (cset_sup[i]) delete [] cset_sup[i];
            cset_sup[i] = NULL;
         }
         else{
            //cout << "DEL " << i << " " << set_sup[i] << endl << flush;
            if (set_sup[i]) delete [] set_sup[i];
            set_sup[i] = NULL;
         }
         mem_used -= (Graph::numF1-i-1) * itsz;
      }
   }

   
   if (use_char_extl2){
      close(isetfd);
      unlink(TEMPF);
      delete [] cset_sup;
   }
   else delete [] set_sup;
   delete invDB;
}


void get_file_l2(char *fname, int &l2cnt)
{
   int *cntary;
   int fd = open(fname, O_RDONLY);
   if (fd < 1){
      perror("can't open l2 file");
      exit(errno);
   }   
   int flen = lseek(fd,0,SEEK_END);
   if (flen > 0){
#ifdef DEC
      cntary = (int *) mmap((char *)NULL, flen, PROT_READ,
                             (MAP_FILE|MAP_VARIABLE|MAP_PRIVATE), fd, 0);
#else
      cntary = (int *) mmap((char *)NULL, flen, PROT_READ,
                             MAP_PRIVATE, fd, 0);
#endif
      if (cntary == (int *)-1){
         perror("MMAP ERROR:cntary");  
         exit(errno);
      }
      
      // build F2graph -- large 2-itemset relations
      int lim = flen/ITSZ;
      //for (int i=0; i < lim; i += 3){
      int i,j,k,res;
      int git[2];
      for (j=0, i=0; j < Graph::numF1 && i < lim;j++){
         for (k=j+1; k < Graph::numF1 && i < lim; k++){
            git[0] = (*F2Graph)[j]->item();
            git[1] = (*F2Graph)[k]->item();

            //if (git[0] < cntary[i]) res = -1;
            //else if (git[0] > cntary[i]) res = 1;
            //else{
            //   if (git[0] < cntary[i+1]) res = -1;
            //   else if (git[0] > cntary[i+1]) res = 1;
            //   else res = 0;
            //}
            
            res = cmp2it(&git[0], &cntary[i]);
            if (res == 0){
               if (cntary[i+2] >= MINSUPPORT){
                  //F2Graph->add_adj(j, cntary[i+1], cntary[i+2]);
                  F2Graph->add_adj(j, k, cntary[i+2]);
                  (*F2Graph)[j]->supsum() += cntary[i+2];
                  (*F2Graph)[k]->supsum() += cntary[i+2];
                  //cout << "LARGE " << cntary[i] << " " << cntary[i+1]
                  //     << " " << cntary[i+2] << endl;
                  l2cnt++;
               }
               i += 3;
            }
            else if (res > 0){
               i += 3;
               k--;
            }
         }
      }
      
      munmap((caddr_t)cntary, flen);
   }
   close(fd);
}


void get_horizontal_ext_l2(int &l2cnt, Dbase_Ctrl_Blk &DCB)
{
   int i, j, k, idx;
   int it1, it2;
   
   int *itcnt2 = new int [(Graph::numF1*(Graph::numF1-1))/2];
   bzero((char *)itcnt2, ((Graph::numF1*(Graph::numF1-1))/2)*sizeof(int));
   int *offsets = new int [Graph::numF1];
   int offt = 0;
   for (i=Graph::numF1-1; i >= 0; i--){
      offsets[Graph::numF1-i-1] = offt;
      offt += i;
   }
    
   int *buf, custid, tid, numitem;
   DCB.get_first_blk();
   DCB.get_next_trans(buf, numitem, tid, custid);
   while (!DCB.eof()){
      for (i=0; i < numitem; i++){
         it1 = DCB.freqidx[buf[i]];
         if (it1 == -1) continue;
	 DCB.tidlists[it1]->add(tid);
         idx = offsets[it1]-it1-1;
         for (j=i+1; j < numitem; j++){
            it2 = DCB.freqidx[buf[j]];
            if (it2 == -1) continue;
            itcnt2[idx+it2]++;
         }
      }
      DCB.get_next_trans(buf, numitem, tid, custid);
   }

   for (i=0; i < Graph::numF1-1; i++){
      idx = offsets[i]-i-1;
      for (j=i+1; j < Graph::numF1; j++){
         if (itcnt2[idx+j] >= MINSUPPORT){
            it1 = (*F2Graph)[i]->item();
            it2 = (*F2Graph)[j]->item();
            F2Graph->add_adj(i, j, itcnt2[idx+j]);
            (*F2Graph)[i]->supsum() += itcnt2[idx+j];
            (*F2Graph)[j]->supsum() += itcnt2[idx+j];
            //cout << "LARGE " << it1 << " " << it2
            //     << " " << itcnt2[idx+j] << endl;
            l2cnt++;
         }  
      }
   }
   
   delete [] itcnt2;  

   for (i=0; i < Graph::numF1; i++){
     if ((*F2Graph)[i]->sup() != DCB.tidlists[i]->size())
       cout << "error in size " << i << endl;
   }
}


void sort_freqidx(Dbase_Ctrl_Blk &DCB){
  int i,j;

  Array<int> **oldadd = new Array<int> *[Graph::numF1];
  for (i=0; i < Graph::numF1; i++)
    oldadd[i] = DCB.tidlists[i];

  for (i=0; i < DBASE_MAXITEM; i++){
    DCB.freqidx[i] = -1;
  }

  for (i=0; i < Graph::numF1; i++){
    DCB.freqidx[(*F2Graph)[i]->item()] = i;
  }

  for (i=0, j=0; i < DBASE_MAXITEM; i++){
    if (DCB.freqidx[i] == -1) continue;
    DCB.tidlists[DCB.freqidx[i]] = oldadd[j];
    j++;
  }

  //for (i=0; i < Graph::numF1; i++){
  //   cout << "F2N " << i << " " << (*F2Graph)[i]->item() << " " 
  //	  << (*F2Graph)[i]->sup() << " ";
     
  //     if (use_horizontal){
  //   cout << " -- " << DCB.tidlists[i]->size();
  // }
  // cout << endl;
  // }

  delete [] oldadd;
}



int make_l2_pass(boolean ext_l2_pass, char *it2f, Dbase_Ctrl_Blk &DCB)
{
   int i;
   int l2cnt=0;
   if (use_horizontal) get_horizontal_ext_l2(l2cnt,DCB);
   else if (ext_l2_pass) get_ext_l2(l2cnt);
   else get_file_l2(it2f, l2cnt);

   //cout << "L2 : " << l2cnt << endl;
   
   for (i=0; i < F2Graph->size(); i++){
      if ((*F2Graph)[i]->array() != NULL) (*F2Graph)[i]->compact();
      //cout << "NODE " << i << " " << (*F2Graph)[i]->supsum() << " : ";
      //if ((*F2Graph)[i]->array() != NULL) cout << " " << *(*F2Graph)[i];
      //cout << endl;
   }

   if (process_order == IN_SUP || process_order == DE_SUP){     
      F2Graph->sort();
      //cout << "ORDER :";
      for (i=0; i < F2Graph->size(); i++){
         if ((*F2Graph)[i]->array() != NULL) (*F2Graph)[i]->compact();
         //cout << " " << (*F2Graph)[i]->item();
         //cout << "NODE " << i << " : ";
         //if ((*F2Graph)[i]->array() != NULL) cout << " " << *(*F2Graph)[i];
         //cout << endl;
      }
      //cout << endl;
      if (use_horizontal) sort_freqidx(DCB);
      
      //cout << "GRAPH DENSITY " <<
      //   (2.0 *l2cnt)/(F2Graph->size()*(F2Graph->size()-1)) << endl;
   }
   
   return l2cnt;
}


