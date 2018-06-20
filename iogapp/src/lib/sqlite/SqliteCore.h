/*
 * SqliteCore.h
 *
 *  Created on: Apr 26, 2018
 *      Author: jiasiang
 */

#ifndef LIB_SQLITE_SQLITECORE_H_
#define LIB_SQLITE_SQLITECORE_H_

#include <sqlite3.h>
#include <vector>
#include "SqlValue.h"
#include <mutex>

using namespace std;

class SqliteCore {
private:
	static bool isOpen;
	static bool skipAll;
	static mutex lock, lock_db;
	static bool isTran;
	static bool isRunProcessTran;
	static sqlite3 *db;
	static vector<string> executeCommands;
	static string getColumnText(sqlite3_stmt *, int);
	static void run(string);
	static void startTran();
	static void endTran();
	static void processTran();
	static bool processError(string, string, int);
public:
	SqliteCore();
	~SqliteCore();
	static void open();
	static void close();
	static vector<vector<SqlValue>> select(string);
	static void executeLater(string);
	static void executeNow(string);
};

#endif /* LIB_SQLITE_SQLITECORE_H_ */
