/*
 * TransactionParameter.h
 *
 *  Created on: May 4, 2018
 *      Author: jiasiang
 */

#ifndef CLASS_TRANSACTIONPARAMETER_H_
#define CLASS_TRANSACTIONPARAMETER_H_
#include <iostream>

class DataInfo {
public:
	char *buf;
	int length;
	DataInfo() {
		buf = NULL;
		length = 0;
	}
	DataInfo(int len) {
		buf = new char[len];
		length = len;
		for (int i = 0; i < length; i++)
			buf[i] = 0;
	}

	virtual ~DataInfo() {
		delete[] buf;
	}

	void verify(int len) {
		if (!buf) {
			length = len;
			buf = new char[len];
			for (int i = 0; i < length; i++)
				buf[i] = 0;
		}
	}

	void copy(DataInfo *info) {
		length = info->length;
		verify(length);
		for (int i = 0; i < length; i++)
			buf[i] = info->buf[i];
	}
};
enum RECEIVE_MODE {
	LENGTH_ONLY, HEAD_LENGTH, TAIL_LENGTH, HEAD_TAIL_LENGTH,
	BYTE_LENGTH
};
class ReceiveParameter {
public:

	DataInfo head;
	DataInfo tail;
	int length;

	int bytePos;
	int byteLen;
	int byteValueMax;
	RECEIVE_MODE mode;

	ReceiveParameter() {
		length = 0;
		mode = LENGTH_ONLY;
		bytePos = 0;
		byteLen = 0;
		byteValueMax = 0;
	}
};

class TransactionParameter {
public:

	DataInfo command;
	DataInfo data;

	ReceiveParameter receiveParameter;
};

#endif /* CLASS_TRANSACTIONPARAMETER_H_ */
