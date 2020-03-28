#include "Count.h"

CountAry::CountAry() {
	counter=new Array<int> (1) ;
	counter->add(0);
}

void CountAry::add(int i) {
	int j;

	if(counter->size()<i){
		for(j=0;i!=counter->size();j++) {
			counter->add(0);
		}
	}

	(*counter)[i-1]++;
	
}

void CountAry::kadd(int i,int k) {
	int j;

	if(counter->size()<i){
		for(j=0;i!=counter->size();j++) {
			counter->add(0);
		}
	}
	
	(*counter)[i-1]=k;
	
}



void CountAry::print() {
	int k,c=0;

	for(k=0;k<counter->size();k++) {
		//cout << "Length " << k+1 << " :" << (*counter)[k] << endl;
                cout << (*counter)[k] << " ";
		c+= (*counter)[k];
	}
	cout << c << endl;
}
