/*
 * SqlTable.h
 *
 *  Created on: Apr 26, 2018
 *      Author: jiasiang
 */

#ifndef LIB_SQLITE_SQLTABLE_H_
#define LIB_SQLITE_SQLTABLE_H_

#include <string>
#include <vector>

#include "SqliteCore.h"
#include "SqlRow.h"

using namespace std;

class SqlTable {
private:
	vector<SqlRow> data;
	void updateData();
	void updateData(string);
	bool isExist();
public:
	string name;
	SqlTable();
	SqlTable(string);
	SqlTable(string, string);
	unsigned int rowCount();
	void addRow(SqlRow);
	SqlRow getRow(unsigned int);
	void removeRow(SqlRow);
	void removeAll();
	void setRow(SqlRow);

	void filterRow(string);

	SqlRow getNewRow();
	void printRow();

	void refresh();

	void initialTable(string);
	void initialTable(string, string);
	void directUpdate(string);
};

#endif /* LIB_SQLITE_SQLTABLE_H_ */
