#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "assoc.h"
#include "partition.h"

int num_partitions = 1;
int *DATAFD, *IDXFD, *IDXFLEN, **ITEMIDX;

void partition_alloc(char *dataf, char *idxf)
{
   DATAFD = new int[num_partitions];
   IDXFD = new int[num_partitions];
   IDXFLEN = new int[num_partitions];
   ITEMIDX = new int*[num_partitions];
   char tmpnam[300];
   for (int i=0; i < num_partitions; i++){
      if (num_partitions > 1) sprintf(tmpnam, "%s.P%d", dataf, i);
      else sprintf(tmpnam, "%s", dataf);
      DATAFD[i] = open(tmpnam, O_RDONLY);
      if (DATAFD[i] < 0){
         perror("can't open data file");
         exit(errno);
      }
      
      if (num_partitions > 1) sprintf(tmpnam, "%s.P%d", idxf, i);
      else sprintf(tmpnam, "%s", idxf);
      IDXFD[i] = open(tmpnam, O_RDONLY);
      if (IDXFD[i] < 0){
         perror("can't open idx file");
         exit(errno);
      }
      IDXFLEN[i] = lseek(IDXFD[i],0,SEEK_END);
      lseek(IDXFD[i],0,SEEK_SET);
#ifdef DEC
      //cout << "IDXLEN " << IDXFLEN[0] << endl;
      //long pgsz = sysconf(_SC_PAGE_SIZE);
      //long fl = ((IDXFLEN[i]+pgsz-1)/pgsz)*pgsz;
      ITEMIDX[i] = (int *) mmap((char *)NULL, IDXFLEN[i], PROT_READ,
                              (MAP_FILE|MAP_VARIABLE|MAP_PRIVATE),
                              IDXFD[i], 0);
#else
      ITEMIDX[i] = (int *) mmap((char *)NULL, IDXFLEN[i], PROT_READ,
                              MAP_PRIVATE,IDXFD[i], 0);   
#endif
      if (ITEMIDX[i] == (int *)-1){
         perror("MMAP ERROR:item_idx");
         exit(errno);
      }
   }
}

void partition_dealloc()
{
   for (int i=0; i < num_partitions; i++){
      close(DATAFD[i]);
      close(IDXFD[i]);
      munmap((caddr_t)ITEMIDX[i], IDXFLEN[i]);
   }
   delete [] DATAFD;
   delete [] IDXFD;
   delete [] IDXFLEN;
   delete [] ITEMIDX;
}

//partition p's size
int partition_get_blk_sz(int p)
{
   return lseek(DATAFD[p],0,SEEK_END);
}

//maximum partition size
int partition_get_max_blksz()
{
   int max = 0;
   int flen;
   for (int i=0; i < num_partitions; i++){
      flen = lseek(DATAFD[i],0,SEEK_END);
      if (max < flen) max = flen;
   }
   return max;
}

//read the whole partition into MAINBUF
void partition_get_blk(int *MAINBUF, int p)
{
   int flen = lseek(DATAFD[p],0,SEEK_END);
   cout << "FILESZ " << flen << endl;
   lseek(DATAFD[p],0,SEEK_SET);
   if (read(DATAFD[p], (char *)MAINBUF, flen) < 0){
      perror("read item1");
      exit(errno);
   }
}

//support over all partitions
int partition_get_idxsup(int it)
{
   int supsz = 0;
   //cout << "IT " << it << " " << ITEMIDX[0][it+1]-ITEMIDX[0][it] << endl;
   for (int i=0; i < num_partitions; i++){
      supsz += partition_get_lidxsup(i, it);
   }
   return supsz;
}

//support only in the local partition
int partition_get_lidxsup(int idx, int it)
{
   int i, sup = 0;
   if (ITEMIDX[idx][it] != -1){
      i = it+1;
      while (ITEMIDX[idx][i] == -1) i++;
      sup = ITEMIDX[idx][i]-ITEMIDX[idx][it];
   }
   
   return sup;   
}

//item's index
int partition_get_idx(int idx, int it)
{
   return ITEMIDX[idx][it];
}

//return index array
int *partition_idx(int idx)
{
   return ITEMIDX[idx];
}

int partition_idxval(int it)
{
   for (int i=0; i < num_partitions; i++){
      if (ITEMIDX[i][it] > -1) return 1;      
   }
   return -1;
}

//read item it from all partitions
void partition_read_item(int *ival, int it)
{
   int ipos=0;
   int supsz;
   for (int i=0; i < num_partitions; i++){
      supsz = partition_get_lidxsup(i, it);
      if (supsz > 0){
         lseek(DATAFD[i], ITEMIDX[i][it]*ITSZ, SEEK_SET);
         if (read(DATAFD[i], (char *)&ival[ipos], supsz*ITSZ) < 0){
            perror("read item1");
            exit(errno);
         }
         ipos+=supsz;
      }
   }
}

//read item it only from local partition
void partition_lclread_item(int *ival, int pnum, int it)
{
   int supsz;
   supsz = partition_get_lidxsup(pnum, it);
   if (supsz > 0){
      lseek(DATAFD[pnum], ITEMIDX[pnum][it]*ITSZ, SEEK_SET);
      if (read(DATAFD[pnum], (char *)ival, supsz*ITSZ) < 0){
         perror("read item1");
         exit(errno);
      }
   }
}

//return the max and min transcation id for partition pnum
void partition_get_minmaxtid(int pnum, int it, int &minv, int &maxv)
{
   int tid, supsz;
   supsz = partition_get_lidxsup(pnum, it);
   if (supsz > 0){
      lseek(DATAFD[pnum], ITEMIDX[pnum][it]*ITSZ, SEEK_SET);
      read(DATAFD[pnum], (char *)&tid, ITSZ);
      if (minv > tid) minv = tid;
      lseek(DATAFD[pnum], (supsz-2)*ITSZ, SEEK_CUR);
      read(DATAFD[pnum], (char *)&tid, ITSZ);
      if (maxv < tid) maxv = tid;
   }
}

