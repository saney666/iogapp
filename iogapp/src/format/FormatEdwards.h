/*
 * Formatedwards.h
 *
 *  Created on: May 31, 2018
 *      Author: japan
 */

#ifndef FORMAT_FORMATEDWARDS_H_
#define FORMAT_FORMATEDWARDS_H_

#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "../class/TransactionParameter.h"
#include "FormatBase.h"

using namespace std;
class Edwards_DataInfo {
public:
	string rowid;
	string com;
	string writeaddress;
	char syntax;
	int ID;
	int writeaddr;


	void initialize();
	void print();
};

class Format_Edwards: public FormatBase {
private:
	int index;
	int newIndex;
	int comErrIndex;
	bool first;
	int id;
	string command;
	int writeaddress;

	bool verifyFormat(DataInfo*);
	bool verifyData(DataInfo*);
	void valueAnalysis(DataInfo*,int);
	vector<Edwards_DataInfo> list;

public:

	virtual ~Format_Edwards();
	virtual void initialize(string);
	virtual TransactionParameter* createCommand();
	virtual bool processData(TransactionParameter*);

	virtual TransactionParameter* prepareCommand();
	virtual bool processCommand(TransactionParameter*);
};

#endif /* FORMAT_FORMATEDWARDS_H_ */
