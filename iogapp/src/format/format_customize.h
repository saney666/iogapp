/*
 * formatcustomize.h
 *
 *  Created on: May 10, 2018
 *      Author: japan
 */

#ifndef FORMAT_FORMAT_CUSTOMIZE_H_
#define FORMAT_FORMAT_CUSTOMIZE_H_

#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "../class/TransactionParameter.h"
#include "FormatBase.h"

using namespace std;

enum CustomRecvMode {
	CUSTOM_RECV_MODE_BYTE, CUSTOM_RECV_MODE_WORD, CUSTOM_RECV_MODE_ASCII
};

class FormatCustomRecv {
public:
	int position;
	int mode;
	int address;
	int param;
};

class FormatCustomDataInfo {
public:
	string rowid;
	string com;
	int CommandType;
	string SplitCode;
	string head;
	string tail;
	string length;
	vector<FormatCustomRecv> recv;
};

class format_definition: public FormatBase {
private:
	unsigned int index;
	unsigned int newIndex;
	vector<FormatCustomDataInfo> list;
	unsigned int stringToint(string);
public:
	virtual ~format_definition();
	virtual void initialize(string);
	virtual TransactionParameter* createCommand();
	virtual bool processData(TransactionParameter*);

	virtual TransactionParameter* prepareCommand();
	virtual bool processCommand(TransactionParameter*);
};

#endif /* FORMAT_FORMAT_CUSTOMIZE_H_ */
