#ifndef _ASSOC_H
#define _ASSOC_H
#include <sys/time.h>
#include <iostream>
#include <fstream>

using namespace std;

#define boolean char
#define FALSE 0
#define TRUE 1
#define DONTCARE 2

//for returning subset info while intersecting
#define AeqB 0
#define AsubB 1
#define BsubA 2
#define AneqB 3

#define MBYTE (1024*1024)
#define ITSZ sizeof(int)
#define getsec(tv) tv.tv_sec+tv.tv_usec/1000000.0
extern struct timeval tp;
#define seconds(tm) gettimeofday(&tp,(struct timezone *)0);\
tm=getsec(tp);

typedef int (*CMPFUNC) (const void * a, const void * b);
typedef void (*CLIQFUNC) (int *cliq, int cliqsz);

extern int DBASE_NUM_TRANS;
extern int DBASE_MAXITEM;
extern float DBASE_AVG_TRANS_SZ;
extern int DBASE_MINTRANS;
extern int DBASE_MAXTRANS;
extern int MINSUPPORT;
extern long AVAILMEM;
extern int num_partitions;
//extern int GBK_threshold;
extern boolean sort_ascend;
extern boolean use_char_extl2;
extern int maxtidlistsz;
extern ofstream FOUT;

extern boolean use_diff;
extern boolean use_diff_f2;
extern boolean diff_input;
extern boolean use_output;
extern boolean use_horizontal;
extern boolean use_hash;
extern boolean use_hash_map;
extern boolean memflg;
extern int process_order;
extern long TOTMEM;

class Graph;
extern Graph *F2Graph;

class Dbase_Ctrl_Blk;
extern Dbase_Ctrl_Blk *DCB;

extern void ECLAT_Find_Freq();
extern int cmpint (const void *, const void *);
#endif //_ASSOC_H



