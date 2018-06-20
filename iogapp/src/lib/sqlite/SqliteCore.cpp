/*
 * SqliteCore.cpp
 *
 *  Created on: Apr 26, 2018
 *      Author: jiasiang
 */

#include <cstdio>
#include <functional>

#if defined(TYPE_DA662)
#define DB_FILE "/home/root/"
#elif defined(TYPE_UC8112)
#define DB_FILE "/home/moxa/"
#elif defined(TYPE_IOPAC)
#define DB_FILE "/home/"
#endif

#include "SqliteCore.h"
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "../../Global.h"

sqlite3 *SqliteCore::db;
vector<string> SqliteCore::executeCommands;
bool SqliteCore::isTran = false;
bool SqliteCore::skipAll = false;
mutex SqliteCore::lock;
mutex SqliteCore::lock_db;
bool SqliteCore::isOpen = false;
bool SqliteCore::isRunProcessTran = false;

string SqliteCore::getColumnText(sqlite3_stmt* stmt, int index) {
	string result = "";
	char *p = (char*) sqlite3_column_text(stmt, index);
	if (p != NULL)
		result = string(p);
	return result;
}

vector<vector<SqlValue>> SqliteCore::select(string command) {

	sqlite3_stmt *stmt = NULL;
	vector<vector<SqlValue>> result;
	if (skipAll)
		return result;
	int ret;
	while ((ret = sqlite3_prepare_v2(db, command.c_str(), command.length() + 1,
			&stmt,
			NULL)) != SQLITE_OK) {
		processError("Select prepare", command, ret);
	}

	int ctotal;
	while ((ctotal = sqlite3_column_count(stmt)) < 0) {
		cout << "select column fail:" << ctotal << "," << command << endl;
		sleep(1);
	}

	vector<SqlValue> columns;
	for (int i = 0; i < ctotal; i++)
		columns.push_back(SqlValue(sqlite3_column_name(stmt, i), ""));
	result.push_back(columns);

	int res = 0;
	while (1) {
		res = sqlite3_step(stmt);
		if (res == SQLITE_ROW) {
			vector<SqlValue> row;
			for (int i = 0; i < ctotal; i++) {
				SqlValue value(sqlite3_column_name(stmt, i),
						string(getColumnText(stmt, i)));
				row.push_back(value);
			}
			result.push_back(row);
		}
		if (res == SQLITE_DONE)
			break;
	}
	sqlite3_finalize(stmt);
	return result;
}

void SqliteCore::executeLater(string command) {
	if (skipAll)
		return;
	lock_guard<mutex> mLock(lock);
	startTran();
	executeCommands.push_back(command);
}

void SqliteCore::executeNow(string command) {
	if (skipAll)
		return;
	endTran();
	run(command);
}
void SqliteCore::close() {
	lock_guard<mutex> mLock(lock_db);
	sqlite3_close(db);
	isOpen = false;
}
void SqliteCore::open() {
	lock_guard<mutex> mLock(lock_db);
	if (isOpen)
		return;
	isOpen = true;
	string filename = DB_FILE;
	string journal = filename + "/db.sqlite3-journal";
	int ret;
	if (access(journal.c_str(), F_OK) != -1) {
		cout << "Has jour file, need delete it" << endl;
		if (remove(journal.c_str()) != 0)
			perror("delete jour file fail:");
		else
			cout << "Delete it successful" << endl;
	}
	filename += "db.sqlite3";
	while ((ret = sqlite3_open(filename.c_str(), &db)) != SQLITE_OK) {
		cout << "Open sql fail:" << ret << endl;
		sleep(1);
	}
	cout << "Sql Opened" << endl;
	isTran = false;
	function<void()> f = bind(&SqliteCore::processTran);
	Global::addThreadd(f);

}

SqliteCore::SqliteCore() {

}

SqliteCore::~SqliteCore() {

}

void SqliteCore::run(string command) {
	if (skipAll)
		return;
	sqlite3_stmt *stmt = NULL;
	int ret;
	if (command.length() == 0)
		return;
	while ((ret = sqlite3_prepare_v2(db, command.c_str(), command.length() + 1,
			&stmt,
			NULL)) != SQLITE_OK) {
		processError("Execute prepare", command, ret);
	}

	while ((ret = sqlite3_step(stmt)) != SQLITE_DONE) {
		if (processError("Execute step", command, ret)) {
			run(command);
			return;
		}
	}

	sqlite3_finalize(stmt);
}

void SqliteCore::startTran() {
	if (isTran)
		return;
	isTran = true;
	//cout << "Start tran" << endl;
	run("begin transaction;");
}

void SqliteCore::endTran() {
	lock_guard<mutex> mLock(lock);
	if (!isTran)
		return;
	unsigned int len = executeCommands.size();
	for (unsigned int i = 0; i < len; i++)
		run(executeCommands[i]);
	executeCommands.clear();

	run("end transaction;");
//	cout << "End taran:" << len << endl;
	isTran = false;
}

void SqliteCore::processTran() {
	if (isRunProcessTran)
		return;
	isRunProcessTran = true;
	while (Global::isDisabledMe()) {
		endTran();
		Global::sleeps(3);
	}
	isRunProcessTran = false;
}

bool SqliteCore::processError(string act, string cmd, int ret) {
	cout << act << " err:" << ret << "," << cmd << endl;
	sleep(1);
	if (ret != SQLITE_BUSY && ret != SQLITE_LOCKED) {
		if (ret == SQLITE_ERROR) {
			cout << "Can't solve problem, exit program." << endl;
			Global::disableMe();
			skipAll = true;
			return true;
		}
		cout << "Has error " << ret << ", need to reopen." << endl;
		close();
		open();
		return true;
	}
	return false;
}
