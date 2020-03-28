#ifndef __DATABASE_H
#define __DATABASE_H

#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

#include "Array.h"

extern int use_seq;

#define ITSZ sizeof(int)
#define DCBBUFSZ 16384
//#define TRANSOFF ((use_seq)?3:2)
#define TRANSOFF 3

class Dbase_Ctrl_Blk{
public:
   Dbase_Ctrl_Blk(char *infile, int buf_sz=DCBBUFSZ);
   ~Dbase_Ctrl_Blk();

   void get_next_trans_ext();
   inline void get_first_blk();
   inline void get_next_trans(int *&lbuf, int &numitem, int &tid, int &custid);

   int eof()
   {
      return (readall == 1);
   }

   void delete_tidbuf(){
     if (tidbuf){
       munmap(tidbuf,tidbuflen);      
     }
   }

   int fd;     
   int buf_size;
   int * buf;
   int cur_blk_size; 
   int cur_buf_pos;
   int endpos;
   char readall;

   static int *freqidx; //which items are frequent, what is new order?

   //store the tidlist for frequent items
   static Array<int> **tidlists;
   static int *tidbuf;
   static int tidbuflen;
};

inline void Dbase_Ctrl_Blk::get_first_blk()
{
   readall=0;
   lseek(fd, 0, SEEK_SET);
   cur_blk_size = (read(fd,(void *)buf, (buf_size*ITSZ)))/ITSZ;
   if (cur_blk_size < 0){
      perror("get_first_blk");
      exit(errno);
   }
   cur_buf_pos = 0;
}

inline  void Dbase_Ctrl_Blk::get_next_trans (int *&lbuf,
                                             int &nitems, int &tid, int &cid)
{
   if (cur_buf_pos+TRANSOFF >= cur_blk_size ||
       cur_buf_pos+buf[cur_buf_pos+TRANSOFF-1]+TRANSOFF > cur_blk_size){
      if (lseek(fd, 0, SEEK_CUR) == endpos) readall = 1;
      if (!readall){
         // Need to get more items from file
         get_next_trans_ext();
      }      
   }
   
   if (!readall){
      cid = buf[cur_buf_pos];
      tid = buf[cur_buf_pos+TRANSOFF-2];
      nitems = buf[cur_buf_pos+TRANSOFF-1];
      lbuf = buf + cur_buf_pos + TRANSOFF;
      cur_buf_pos += nitems + TRANSOFF;
   }
}
#endif //__DATABASE_H





