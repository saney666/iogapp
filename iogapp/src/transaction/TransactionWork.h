/*
 * TransactionWork.h
 *
 *  Created on: May 3, 2018
 *      Author: jiasiang
 */

#ifndef TRANSACTIONWORK_H_
#define TRANSACTIONWORK_H_

#include <string>

#if defined(TYPE_DA662)
#define COMSIZE 16
#else
#define COMSIZE 2
#endif

#include "Transaction.h"
#include <map>
#include <vector>
using namespace std;

class TransactionWork {
private:
	map<string, Transaction*> trans;
	void initialComs();
public:
	bool initialize();
	void start();
	void stop();
	void clear();
	void update();
};

#endif /* TRANSACTIONWORK_H_ */
