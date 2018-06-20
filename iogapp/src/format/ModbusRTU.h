/*
 * ModbusRTU.h
 *
 *  Created on: May 16, 2018
 *      Author: jiasiang
 */

#ifndef FORMAT_MODBUSRTU_H_
#define FORMAT_MODBUSRTU_H_

#include <string>
#include <vector>
#include <iostream>
#include "../class/TransactionParameter.h"
#include "FormatBase.h"

using namespace std;

class ModbusRTU_Info {
private:
	void valueToChar(string, char *);
public:

	string unitId;
	string function;
	string readAddr;
	string number;
	string writeAddr;


	unsigned int uid;
	unsigned int fun;
	unsigned int raddr;
	unsigned int num;
	unsigned int waddr;
	unsigned int dataLength;


	void initialize();
	void calculateLength();
	void setCommand(DataInfo *);
	void setReceiveParam(ReceiveParameter *);
	void printf();
};



class ModbusRTU: public FormatBase {
private:
	vector<ModbusRTU_Info> list;
	unsigned int index, beforeIndex;
	bool verifyFormat(DataInfo*);
	bool verifyData(DataInfo*);

	vector<bool> getBool(const char *, int);
	vector<short> getShort(const char *, int);
	void setBool(vector<bool>, char *);
	void setShort(vector<short>, char *);

	void setErrorData(DataInfo *, ModbusRTU_Info, int);
	void setWriteData(DataInfo *, ModbusRTU_Info);
	void setReadData(DataInfo *, ModbusRTU_Info);


public:
	virtual ~ModbusRTU();
	virtual void initialize(string);
	virtual TransactionParameter* createCommand();
	virtual bool processData(TransactionParameter*);

	virtual TransactionParameter* prepareCommand();
	virtual bool processCommand(TransactionParameter*);
};

#endif /* FORMAT_MODBUSRTU_H_ */
