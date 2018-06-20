/*
 * SqlRow.h
 *
 *  Created on: Apr 27, 2018
 *      Author: jiasiang
 */

#ifndef LIB_SQLITE_SQLROW_H_
#define LIB_SQLITE_SQLROW_H_

#include <vector>
#include "SqlValue.h"


class SqlRow {
private:
	vector<SqlValue> data;
	unsigned int getIndex(string);
public:
	SqlRow();
	SqlRow(vector<SqlValue>);
	unsigned int cellCount();
	string getTitle(unsigned int);
	string getCell(unsigned int);
	string getCell(string);
	int getCellToInt(string);
	int getCellToInt(unsigned int);
	void addCell(string, string);
	void setCell(unsigned int, string);
	void setCell(string, string);
};

#endif /* LIB_SQLITE_SQLROW_H_ */
