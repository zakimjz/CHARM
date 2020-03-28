#ifndef _UTIL_H
#define _UTIL_H

//#ifdef __GNUC__
//#pragma interface
//#endif

#include "assoc.h"

template <class Items>
class Util{
public:
   static boolean Bsearch(int min, int max, Items *itary, Items it,
                          CMPFUNC cfunc, int& ret);
   static int Realloc (int newlen, int elsz, Items*&ary);
};

int Choose(int n, int k);
#endif //_UTIL_H
