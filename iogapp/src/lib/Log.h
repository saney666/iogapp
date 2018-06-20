/*
 * Log.h
 *
 *  Created on: May 2, 2018
 *      Author: jiasiang
 */

#ifndef LIB_LOG_H_
#define LIB_LOG_H_
#include <string>
#include <queue>
#include "sqlite/SqliteCore.h"
using namespace std;

class Log {
private:
	static queue<string> commands;
	static std::mutex lock;
	static string getTimeNowStr();
	static string targetToStr(int);
	static void doQueueWork();
	static void checkLogSize();
	static void checkSystemLog();
	static void checkTransactionLog();
	static void putCommandInSql(unsigned int);
public:
	enum TARGET {
		MAIN, TRANSACTION, TRANSACTION_WORK
	};
	static void addTransaction(string, string, const char*, int, bool);
	static void addTransaction(string, string, const char*, int);
	static void addTransaction(string, string, string, bool);
	static void addSystem(TARGET, string);

	static void activeQueue();
};

#endif /* LIB_LOG_H_ */
