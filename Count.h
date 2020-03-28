#include "Array.h"


class CountAry 
{
	Array<int> *counter;
	
public:

	CountAry(); 
	~CountAry() { delete counter; }

	void add(int i);
	void kadd(int index,int n);
   Array<int> *ctr(){ return counter; }
   
	void print();
};
