/*
 * TransactionWork.cpp
 *
 *  Created on: May 3, 2018
 *      Author: jiasiang
 */

#include "TransactionWork.h"
#include "../lib/sqlite/SqlTable.h"
#include "../lib/Converter.h"
#include "../lib/Log.h"

bool TransactionWork::initialize() {
	initialComs();
	SqlTable table("tranInfo");
	if (table.rowCount() > 1) {
		for (unsigned int i = 1; i < table.rowCount(); i++) {
			SqlRow row = table.getRow(i);
			row.setCell("sw", "0");
			table.setRow(row);
			trans[row.getCell(0)] = new Transaction(row.getCell("rowid"));
		}
	}

	return true;
}

void TransactionWork::start() {
	for (auto iter = trans.begin(); iter != trans.end(); iter++) {
		iter->second->start();
	}
}

void TransactionWork::initialComs() {
	SqlTable coms("coms");
	if (coms.rowCount() - 1 != COMSIZE) {
		coms.removeAll();
		for (int i = 0; i < COMSIZE; i++) {
			SqlRow row = coms.getNewRow();
			row.setCell(1, Converter::itos(i + 1));
			row.setCell(2, "0");
			row.setCell(3, "9600");
			row.setCell(4, "8");
			row.setCell(5, "0");
			row.setCell(6, "1");
			row.setCell(7, "0");
			row.setCell(8, "0");
			coms.addRow(row);
		}
	}
}

void TransactionWork::stop() {
	for (auto iter = trans.begin(); iter != trans.end(); iter++) {
		iter->second->stop();
	}
}

void TransactionWork::clear() {
	for (auto iter = trans.begin(); iter != trans.end(); iter++) {
		delete iter->second;
	}
	trans.clear();
}

void TransactionWork::update() {
	SqlTable table("tranInfo");
	vector<string> del;
	SqlRow row;
	string rowid;
	if (table.rowCount() > 1) {
		for (unsigned int i = 1; i < table.rowCount(); i++) {
			row = table.getRow(i);
			rowid = row.getCell("rowid");
			if (trans.find(rowid) == trans.end()) {
				//not find rowid, add new tran
				trans[rowid] = new Transaction(rowid);
				trans[rowid]->start();
			} else {
				//find rowid, do something
				switch (row.getCellToInt("sw")) {
				case 1:
					Log::addSystem(Log::TRANSACTION_WORK, "Restart " + rowid);
					trans[rowid]->restart();
					break;
				case 2:
					del.push_back(rowid);
					continue;
				case 3:
					trans[rowid]->activeLog = row.getCellToInt("activeLog");
					break;
				}
			}
			row.setCell("sw", "0");
			table.setRow(row);
		}

		for (unsigned int i = 0; i < del.size(); i++) {
			rowid = del[i];
			row = table.getNewRow();
			row.setCell("rowid", rowid);
			Log::addSystem(Log::TRANSACTION_WORK, "Delete " + rowid);
			trans[rowid]->stop();
			delete trans[rowid];
			trans.erase(rowid);
			table.removeRow(row);

			SqliteCore::executeLater("delete from log_transaction where tid=" + rowid);
		}
		SqliteCore::executeNow("");
	}
}
