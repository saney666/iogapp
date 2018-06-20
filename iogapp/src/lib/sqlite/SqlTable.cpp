/*
 * SqlTable.cpp
 *
 *  Created on: Apr 26, 2018
 *      Author: jiasiang
 */

#include "SqlTable.h"
#include <iostream>

SqlTable::SqlTable() {
}

SqlTable::SqlTable(string name) {
	initialTable(name);
}

SqlTable::SqlTable(string name, string where) {
	this->name = name;
	initialTable(name, where);
}

void SqlTable::updateData() {
	data.clear();
	if (isExist()) {
		vector<vector<SqlValue>> values = SqliteCore::select(
				"select rowid, * from " + name);
		for (auto iter = values.begin(); iter != values.end(); ++iter)
			data.push_back(SqlRow((*iter)));
	} else
		cout << name << " table isn't exist." << endl;
}

void SqlTable::updateData(string where) {
	data.clear();
	if (isExist()) {
		vector<vector<SqlValue>> values = SqliteCore::select(
				"select rowid, * from " + name + " where " + where);
		for (auto iter = values.begin(); iter != values.end(); ++iter)
			data.push_back(SqlRow((*iter)));
	} else
		cout << name << " table isn't exist." << endl;
}

bool SqlTable::isExist() {
//	vector<vector<SqlValue>> values;
//	int count = 5;
//	while (count--) {
//		values = sql.select(
//				"select * from sqlite_master where name = '" + name + "'");
//		if (values.size() > 1)
//			return true;
//	}
//	return false;
	return true;
}

unsigned int SqlTable::rowCount() {
	return data.size();
}

void SqlTable::addRow(SqlRow row) {
	string command = "insert into " + name + " (";
	string command2 = " values (";
	for (unsigned int i = 1; i < row.cellCount(); i++) {
		command += row.getTitle(i);
		command2 += "'" + row.getCell(i) + "'";
		if (i + 1 < row.cellCount()) {
			command += ", ";
			command2 += ", ";
		}
	}
	command += ") ";
	command2 += ") ";
	SqliteCore::executeNow(command + command2);
	updateData();
}

SqlRow SqlTable::getRow(unsigned int index) {
	if (index < data.size())
		return data[index];
	return SqlRow();
}

void SqlTable::removeRow(SqlRow row) {
	SqliteCore::executeNow("delete from " + name + " where rowid = " + row.getCell(0));
	updateData();
}

void SqlTable::removeAll() {
	SqliteCore::executeNow("delete from " + name);
	updateData();
}

void SqlTable::setRow(SqlRow row) {
	string command = "update " + name + " set ";
	for (unsigned int i = 1; i < row.cellCount(); i++) {
		command += row.getTitle(i) + " = '" + row.getCell(i) + "'";
		if (i + 1 < row.cellCount())
			command += ", ";
	}
	command += " where rowid = " + row.getCell(0);
	SqliteCore::executeNow(command);
	updateData();
}

void SqlTable::refresh() {
	updateData();
}

SqlRow SqlTable::getNewRow() {
	if (data.size() > 0)
		return data[0];
	else
		return SqlRow();
}

void SqlTable::filterRow(string filter) {
	updateData(filter);
}

void SqlTable::printRow() {
	cout << name << " row size is " << rowCount() << endl;
	for (unsigned int i = 0; i < rowCount(); i++) {
		cout << i + 1 << " : ";
		SqlRow row = getRow(i);
		for (unsigned int j = 0; j < row.cellCount(); j++) {
			cout << row.getTitle(j) << " is " << row.getCell(j) << "  ";
		}
		cout << endl;
	}
}

void SqlTable::initialTable(string name) {
	this->name = name;
	updateData();
}
void SqlTable::initialTable(string name, string where) {
	this->name = name;
	updateData(where);
}

void SqlTable::directUpdate(string text) {
	data.clear();
	if (isExist()) {
		vector<vector<SqlValue>> values = SqliteCore::select(text);
		for (auto iter = values.begin(); iter != values.end(); ++iter)
			data.push_back(SqlRow((*iter)));
	}
}

