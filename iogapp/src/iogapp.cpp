//============================================================================
// Name        : iogapp.cpp
// Author      : jiasiang
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#include <iostream>
#include "lib/sqlite/SqliteCore.h"
#include "transaction/TransactionWork.h"
#include "Global.h"
#include "lib/Log.h"
#include "lib/sqlite/SqlTable.h"
#include "lib/Converter.h"
#include "other/NetInterface.h"
#include "other/Communication.h"

using namespace std;

TransactionWork twork;
Communication communication;
SqlTable table;

bool initialAll();
void showRunModel();
void initializeOpt();
//----------------
void startWork();
//-------
void waitToStop();
void checkOpt();
//-------------------
void stopWork();
void clearAll();

int main(int len, char** param) {
	SqliteCore::open();
	Log::activeQueue();
	Log::addSystem(Log::MAIN, "Start");

	if (initialAll()) {
		startWork();
		waitToStop();
		stopWork();
		clearAll();
	}
	Log::addSystem(Log::MAIN, "End");
	return 0;
}

inline bool initialAll() {
	showRunModel();
	initializeOpt();
	NetInterface::initialize();
	if (twork.initialize())
		return true;
	return false;
}
inline void showRunModel() {
	switch (Global::getIogType()) {
	case IOG_TYPE_DA662:
		cout << "You are DA662" << endl;
		break;
	case IOG_TYPE_UC8112:
		cout << "You are UC8112" << endl;
		break;
	case IOG_TYPE_IOPAC:
		cout << "You are IOPAC" << endl;
		break;
	case IOG_TYPE_ERROR:
		cout << "You are Unknown" << endl;
		break;
	}
}

inline void startWork() {
	twork.start();
	communication.start();
}

inline void waitToStop() {
	while (Global::isDisabledMe()) {
		checkOpt();
		Global::sleeps(1);
	}
}

inline void stopWork() {
	twork.stop();
	communication.stop();
}

inline void initializeOpt() {
	table.initialTable("operation");
	SqlRow row;

	if (table.rowCount() <= 1) {
		row.addCell("reboot", "0");
		row.addCell("resetLan", "0");
		row.addCell("updateTran", "0");
		row.addCell("endProgram", "0");

		table.addRow(row);
	} else {
		row = table.getRow(1);
		for (unsigned int i = 1; i <= row.cellCount() - 1; i++)
			row.setCell(i, "0");
		table.setRow(row);
	}
}

inline void checkOpt() {
	table.initialTable("operation");
	SqlRow row;
//	t.printRow();
	bool hasEvent = false;
	if (table.rowCount() > 1) {
		row = table.getRow(1);
		hasEvent = true;
		if (row.getCellToInt("reboot") == 1) {
			Log::addSystem(Log::MAIN, "Operation: reboot");
			Global::disableMe();
			Global::executeCMD("reboot");
		} else if (row.getCellToInt("resetLan") == 1) {
			Log::addSystem(Log::MAIN, "Operation: resetLan");
			NetInterface::reconfigure();
			NetInterface::initialize();
		} else if (row.getCellToInt("updateTran") == 1) {
			Log::addSystem(Log::MAIN, "Operation: updateTran");
			twork.update();
		} else if (row.getCellToInt("endProgram") == 1) {
			Log::addSystem(Log::MAIN, "Operation: endProgram");
			Global::disableMe();
		} else
			hasEvent = false;

		if (hasEvent) {
			hasEvent = false;
			for (unsigned int i = 1; i <= row.cellCount() - 1; i++)
				row.setCell(i, "0");
			table.setRow(row);
		}
	}
}

inline void clearAll() {
	SqliteCore::close();
}
