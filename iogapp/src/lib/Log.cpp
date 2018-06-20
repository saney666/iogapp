/*
 * Log.cpp
 *
 *  Created on: May 2, 2018
 *      Author: jiasiang
 */
#define LOGLIMIT_TRAN 1000
#define LOGLIMIT_SYS 2000

#include <vector>
#include <ctime>
#include <functional>
#include "Log.h"
#include <stdlib.h>
#include <iostream>
#include "Converter.h"
#include "../Global.h"

#include "sqlite/SqlTable.h"

queue<string> Log::commands;
std::mutex Log::lock;

string Log::getTimeNowStr() {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[50];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "%F %T", timeinfo);
	return string(buffer);
}

void Log::addTransaction(string tid, string type, const char *buf, int len,
		bool isWrite) {
	string message = "(" + Converter::itos(len) + ")";
	for (int i = 0; i < len; i++) {
		message += Converter::itos(buf[i]) + " ";
	}
	addTransaction(tid, type, message, isWrite);
}

void Log::addTransaction(string tid, string type, const char *buf, int len) {
	addTransaction(tid, type, buf, len, true);
}

void Log::addTransaction(string tid, string type, string message,
		bool isWrite) {
//	cout << "Transaction (" << tid << ")" << type << ":" << message << endl;

	if (isWrite) {
		string cmd =
				"insert into log_transaction (time, tid, type, message) values ('";
		cmd += getTimeNowStr() + "','";
		cmd += tid + "','";
		cmd += type + "','";
		cmd += message + "')";
		std::lock_guard<std::mutex> mLock(lock);
		commands.push(cmd);
	}
}

void Log::addSystem(TARGET target, string message) {
	cout << "System (" << targetToStr(target) << "):" << message << endl;

	string cmd = "insert into log_system (time, target, message) values ('";
	cmd += getTimeNowStr() + "','";
	cmd += targetToStr(target) + "','";
	cmd += message + "')";
	std::lock_guard<std::mutex> mLock(lock);
	commands.push(cmd);
}

string Log::targetToStr(int target) {
	switch (target) {
	case TARGET::MAIN:
		return "MAIN";
	case TARGET::TRANSACTION:
		return "TRANSACTION";
	case TARGET::TRANSACTION_WORK:
		return "TRANSACTION_WORK";

	}
	return "";
}

void Log::doQueueWork() {

	cout << "start log queue work" << endl;
	while (Global::isDisabledMe() || commands.size() > 0) {
		putCommandInSql(commands.size());
		Global::sleeps(5);
		checkLogSize();
	}
	cout << "stop log queue work" << endl;
}

void Log::putCommandInSql(unsigned int size) {
	if (size == 0)
		return;
	std::lock_guard<std::mutex> mLock(lock);
	for (unsigned int i = 0; i < size; i++) {
		if (commands.empty()) {
			cout << "Command is empty but still have " << size - i << endl;
			break;
		} else {
			SqliteCore::executeLater(commands.front());
			commands.pop();
		}
	}
}

void Log::checkLogSize() {
	checkSystemLog();
	checkTransactionLog();
}

void Log::checkSystemLog() {
	SqlTable t;
	SqlRow row;
	int logSize;
	string command;
	t.directUpdate("select count(*) from log_system");
	if (t.rowCount() > 1) {
		row = t.getRow(1);
		logSize = row.getCellToInt(0);
//		cout << "log system size is " << logSize << endl;
		if (logSize > LOGLIMIT_SYS) {
//			cout << "System Log is too much:" << logSize << endl;
			logSize -= LOGLIMIT_SYS;
			command =
					"delete from log_system where rowid in (select rowid from log_system";
			command += " limit " + Converter::itos(logSize) + ")";
			SqliteCore::executeNow(command);
		}
	}
}

void Log::checkTransactionLog() {
	SqlTable t("tranInfo");
	SqlRow row;
	vector<string> tran;
	int logSize;
	string command;
	if (t.rowCount() > 1) {
		for (unsigned int i = 1; i < t.rowCount(); i++) {
			row = t.getRow(i);
			tran.push_back(row.getCell(0));
		}
//		cout << "Transaction has " << tran.size() << endl;

		for (auto itr = tran.begin(); itr != tran.end(); itr++) {
			t.directUpdate(
					"select count(*) from log_transaction where tid=" + *itr);
			if (t.rowCount() > 1) {
				row = t.getRow(1);
				logSize = row.getCellToInt(0);
//				cout << "log transaction " << *itr << " need delete " << logSize
//											<< endl;
				if (logSize > LOGLIMIT_TRAN) {
//					cout << "Transaction Log is too much:" << *itr << ","
//							<< logSize << endl;
					logSize -= LOGLIMIT_TRAN;
//					cout << "log transaction " << *itr << " need delete " << logSize
//							<< endl;
					command =
							"delete from log_transaction where rowid in (select rowid from log_transaction where tid=";
					command += *itr;
					command += " limit " + Converter::itos(logSize) + ")";

					SqliteCore::executeLater(command);
				}
			}
		}
		SqliteCore::executeNow("");
	}
}

void Log::activeQueue() {
	function<void()> f = Log::doQueueWork;
	Global::addThreadd(f);
}
