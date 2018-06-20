/*
 * Transaction.h
 *
 *  Created on: May 3, 2018
 *      Author: jiasiang
 */

#ifndef TRANSACTION_H_
#define TRANSACTION_H_

#include "../lib/socket/Client.h"
#include "../lib/socket/Server.h"
#include "../lib/serial/Com.h"
#include "../class/TransactionParameter.h"
#include "../format/IncluesFile.h"

class Transaction {
private:
	enum INTERFACE {
		LAN, SERIAL
	};
	enum DIRECTION {
		SERVER, CLIENT
	};
	enum FORMAT {
		NONE, CUSTOM, MODBUS_RTU, MODBUS_TCP, EDWARDS
	};
	enum SWITCHKEY {
		NOKEY, RESTART, STOP
	};
	bool sw;
	bool life;
	string rowid;
	unsigned long tid;
	int interface;
	int iid;
	int timeout; //s
	int retry;
	int delay; //ms
	int direction; //0:server 1:client
	int format;
	int abnormalPoint;


	Com com;
	Socket::Server server;
	Socket::Client *client;

	FormatBase *formatBase;

	//..................................
	void clientMode();
	TransactionParameter *createCommand();
	bool sendCommand(TransactionParameter*);
	bool receiveData(TransactionParameter*);
	bool processData(TransactionParameter*);
	//..................................
	//..................................
	void serverMode();
	TransactionParameter *prepareCommand();
	bool receiveCommand(TransactionParameter*);
	bool processCommand(TransactionParameter*);
	void sendResponse(TransactionParameter*);
	//...................................
	void defaultSetting();
	void initialInfo();
	void initialFormat();
	void initialInterface();
	void shutdownInterface();


	void syncRestart();
	void syncStop();

	bool clientAvailable();
	void socketAccepted(Socket::Server*, Socket::Client*);

	bool sendData(DataInfo *);

	bool receiveMode_Length(ReceiveParameter *, DataInfo *);
	bool receiveMode_HeadLength(ReceiveParameter *, DataInfo *);
	bool receiveMode_TailLength(ReceiveParameter *, DataInfo *);
	bool receiveMode_HeadTailLength(ReceiveParameter *, DataInfo *);
	bool receiveMode_BytelLength(ReceiveParameter *, DataInfo *);
	bool receiveByData(const DataInfo *);
	bool receiveByLength(DataInfo *);
	int receivableData();
	int receiveData(char *, int);

	void addLog(string);
public:
	bool activeLog;

	Transaction();
	Transaction(string);

	void start();
	void stop();
	void restart();
};

#endif /* TRANSACTION_H_ */
