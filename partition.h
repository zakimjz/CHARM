#ifndef __PARTITION_H_
#define __PARTITION_H_

extern void partition_alloc(char *dataf, char *idxf);
extern void partition_dealloc();
extern void partition_get_blk(int *MAINBUF, int p);
extern int partition_get_blk_sz(int p);
extern int partition_get_max_blksz();
extern int partition_get_idxsup(int it);
extern int partition_get_lidxsup(int idx, int it);
extern int partition_get_idx(int idx, int it);
extern int partition_idxval(int it);
extern int *partition_idx(int idx);
extern void partition_read_item(int *ival, int it);
extern void partition_lclread_item(int *ival, int pnum, int it);
extern void partition_get_minmaxtid(int pnum, int it,
                                       int &minv, int &maxv);

#endif// __PARTITION_H_
