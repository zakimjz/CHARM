#ifdef __GNUC__
#pragma implementation "Array.h"
#pragma implementation "Util.h"
#pragma implementation "List.h"
#endif

#include <iostream>
#include <fstream>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>  //file io

#include "assoc.h"
#include "eclat.h"
#include "extl2.h"
#include "partition.h"
#include "Graph.h"
#include "Itemset.h"
#include "timetrack.h"
#include "Count.h"
#include "calcdb.h"

//--------------------------------------
#include "stats.h" //MJZ
double Stats::tottime = 0;
int Stats::totcand = 0;
int Stats::totlarge = 0;
Stats stats;
ofstream summary("summary.out", ios::app);
//--------------------------------------


unsigned long int DB_size=0;
unsigned long int total_scan=0;
unsigned long int sumtidlist=0;

struct timeval tp;
char hdataf[300];
char dataf[300];
char idxf[300];
char conf[300];
char it2f[300];
char tempf[300];

boolean ext_l2_pass = FALSE;
boolean use_char_extl2 = FALSE;
boolean use_maximal = FALSE;
boolean use_lb = FALSE;
boolean use_diff = FALSE;
boolean diff_input = FALSE;
boolean use_output = FALSE;
boolean use_diff_f2 = FALSE;
boolean use_horizontal=FALSE;
boolean use_hash=FALSE;
boolean use_hash_map=FALSE;
boolean memflg = FALSE; //compute tot mem usage
int process_order = IN_SUP;

int maxtidlistsz;
ofstream FOUT;
extern CountAry FreCount,CloCount;
Dbase_Ctrl_Blk *DCB=NULL; 

//which algorithm to run and parameters
char *alg_name[] = {
#define BFS 0
   "bfs",
#define EQC 1
   "eqc",
#define SEARCH 2
   "search",
   NULL
};

boolean sort_ascend = TRUE;

int DBASE_NUM_TRANS;
int DBASE_MAXITEM;
float DBASE_AVG_TRANS_SZ;
int DBASE_MINTRANS; //works only with 1 partition
int DBASE_MAXTRANS;
int MINSUPPORT=-1;
double MINSUP_PER;
long AVAILMEM = 64*MBYTE;
long TOTMEM=0;
Graph *F2Graph;

void parse_args(int argc, char **argv)
{
   extern char * optarg;
   char *options, *value;
   extern int optind;
   int c;

   sprintf(tempf,"/tmp/tmp");
   if (argc < 2)
      cout << "usage: getsup -i<infile>\n";
   else{
      while ((c=getopt(argc,argv,"a:dDce:hH:i:lm:MoS:s:zZ:x:"))!=-1){
         switch(c){
         case 'a': //which algorithm to run
            options = optarg;
            //cout << ((ECLAT_eqc)?"eqc=1":"eqc=0") << " " <<
            //   ((ECLAT_bfs)?"bfs=1":"bfs=0") << " "<< 
            //   ((ECLAT_search==BOTUP)?"bot":"hyb") << endl;
            break;
         case 'c':
            use_char_extl2 = TRUE;
            break;
	 case 'd':
	   use_diff = TRUE;
	   break;
         case 'D':
            diff_input = TRUE;
            use_diff = TRUE;            
            break;            
         case 'e': //calculate L2 from inverted dbase
            num_partitions = atoi(optarg);
            ext_l2_pass = TRUE;
            break;
         case 'h':
            use_horizontal = TRUE;
            break;
         case 'H':
            if (atoi(optarg) == 1) use_hash_map = TRUE;
            else use_hash = TRUE;
            break;
         case 'i': //input files
            sprintf(dataf,"%s.tpose", optarg);
            sprintf(idxf,"%s.idx", optarg);
            sprintf(conf,"%s.conf", optarg);
            sprintf(it2f,"%s.2it", optarg);
            sprintf(hdataf,"%s.data", optarg);
            break;
         case 'l':
            use_diff_f2 = TRUE;
            use_diff = TRUE;
            break;                 
         case 'm': //amount of mem available
            AVAILMEM = (long) atof(optarg)*MBYTE;
            break;
	 case 'M':
	   memflg = TRUE;
	   break;
 	 case 'o':
            use_output = TRUE;
            break;
         case 's': //min support
            MINSUP_PER = atof(optarg);
            break;
         case 'S': //min support
            MINSUPPORT = atoi(optarg);
            break;
         case 'z': //sort items in descending order
            sort_ascend = FALSE;
            break;
         case 'Z': //processing order (lex=0, incr=1 (default) ,decr=2)
            process_order = atoi(optarg);
            break;
         case 'x':
            sprintf(tempf,"%s",optarg);
            break;
         }
      }
   }
   if (diff_input) use_diff_f2 = 0;
   
   if (process_order == IN_SUP) sort_ascend = TRUE;
   else if (process_order == DE_SUP) sort_ascend = FALSE;
   
   ifstream cfd(conf,ios::in);
   if (!cfd){
      perror("ERROR: invalid conf file\n");
      exit(errno);
   }
   cfd.read((char *)&DBASE_NUM_TRANS,ITSZ);
   if (MINSUPPORT == -1)
      MINSUPPORT = (int) (MINSUP_PER*DBASE_NUM_TRANS+0.5);
   //ensure that support is at least 2
   if (MINSUPPORT < 1) MINSUPPORT = 1;
   //cout << "MINSUPPORT " << MINSUPPORT << " " << " " <<
      //MINSUP_PER << " " << DBASE_NUM_TRANS << endl;
   cfd.read((char *)&DBASE_MAXITEM,ITSZ);
   cfd.read((char *)&DBASE_AVG_TRANS_SZ,sizeof(float));
   cfd.read((char *)&DBASE_MINTRANS,ITSZ);
   cfd.read((char *)&DBASE_MAXTRANS,ITSZ);
   //cout << "CONF " << DBASE_NUM_TRANS << " " << DBASE_MAXITEM << " "
        //<< DBASE_AVG_TRANS_SZ << endl;
   cfd.close();


   if (use_horizontal){
      diff_input = FALSE; //no diff input
      ext_l2_pass = FALSE; //no vertical ext pass
   }
   
   if ((use_hash || use_hash_map) && diff_input){
      cout << "HASH AND DIFFIN DO NOT WORK!\n";
      exit(0);
   }
   
}


int main(int argc, char **argv)
{
   TimeTracker tt,tt1;
   tt.Start();

   int i;
   double te,ts;
   
   seconds(ts);

   parse_args(argc, argv);

   if (use_horizontal) DCB =  new Dbase_Ctrl_Blk(hdataf);
   else partition_alloc(dataf, idxf);

   maxtidlistsz = make_l1_pass(*DCB);
   seconds(te);
   
   iterstat *is = new iterstat(DBASE_MAXITEM, 0, te-ts);
   stats.add(is);
   
   Itemset::alloc_tmpiset(maxtidlistsz);
   
   if (use_horizontal) Eqclass::alloc_tmpiset(0);
   else Eqclass::alloc_tmpiset(maxtidlistsz);

   tt1.Start();
   make_l2_pass(ext_l2_pass, it2f, *DCB);
   double l2time=tt1.Stop();

   is = new iterstat(DBASE_MAXITEM*(DBASE_MAXITEM-1)/2,0,l2time);
   stats.add(is);      
   
   ECLAT_Find_Freq();
   
   seconds(te);
   cout << "TOT TIME " << te-ts << endl;
   
   //cout << "LARGE PATTERNS ";
   //for (i=0; i < NumLargeItemset.size(); i++)
  //    cout << " " << NumLargeItemset[i];
  // cout << endl;
   // cout << "Total elapsed time " << te-ts << endl;

   /*   fout << "ASSOC ";
   if (ECLAT_search == HYBRID) fout << "MAX ";
   if (ECLAT_eqc == 1) fout << "ECLAT ";
   else fout << "CLIQUE ";
   if (ECLAT_bfs == 0) fout << "DFS ";
   fout << dataf << " " << MINSUP_PER << " ";
   for (i=0; i < NumLargeItemset.size(); i++)
      fout << " " << NumLargeItemset[i];
   fout << " " << te-ts << endl;
*/
 
   double cputime1 = tt.Stop();
   //cout << "NumIntersect=" << Itemset::NumIntersect 
   //   << " DB size:" << DB_size << " total scan:" << total_scan
   //   << " scan#:" << (float)total_scan/DB_size << endl; 
   //cout << "cputime=" << cputime1 << "  L2Time=" << l2time 
   //   << "  CT-L2T=" << cputime1-l2time << endl;  
   

   cout << "CHARM ";
   if (diff_input) cout << "DIFFIN ";
   else if (use_diff) cout << "DIFF ";
   if (process_order == IN_SUP) cout << "IN_SUP ";
   else if (process_order == DE_SUP) cout << "DE_SUP ";
   else cout << "LEX ";
   cout << dataf << " " << MINSUP_PER << " " << DBASE_NUM_TRANS
           << " " << MINSUPPORT << endl;

   summary << "CHARM ";
   if (diff_input) summary << "DIFFIN ";
   else if (use_diff_f2) summary << "DIFF2 ";   
   else if (use_diff) summary << "DIFF ";
   if (process_order == IN_SUP) summary << "IN_SUP ";
   else if (process_order == DE_SUP) summary << "DE_SUP ";
   else summary << "LEX ";
   if (use_hash) summary << "USEHASH ";
   if (use_hash_map) summary << "USEHASHMAP ";
   if (use_horizontal) summary << "HORIZONTAL ";
   
   summary << dataf << " " << MINSUP_PER << " " << DBASE_NUM_TRANS
           << " " << MINSUPPORT << " ";
   
   for (i=0; i < CloCount.ctr()->size(); i++){
      stats.setlarge(i,(*CloCount.ctr())[i]);   
      stats[i]->avgtid /= (*FreCount.ctr())[i];
      cout << i+1 << " " << (*CloCount.ctr())[i] << " " 
           << (*FreCount.ctr())[i] << flush << endl;
   }
   
   stats.tottime = cputime1;
   
   summary << stats << " " << Itemset::NumIntersect << " " 
           << DB_size << " " << total_scan << " " << sumtidlist << " " 
           << (float)total_scan/DB_size << " " << te-ts << " "<< maxiter;   

   struct rusage ruse;  
   getrusage(RUSAGE_SELF,&ruse);   
   summary << " " << getsec(ruse.ru_utime) << " " 
           << getsec(ruse.ru_stime) << endl;   
   summary.close();
   
   if (use_horizontal) DCB->delete_tidbuf();
   else partition_dealloc();

   cout << "TOTCLOSED " << stats.totlarge << endl;
   exit(0);
}
