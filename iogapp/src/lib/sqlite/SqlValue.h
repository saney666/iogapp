/*
 * SqlValue.h
 *
 *  Created on: Apr 26, 2018
 *      Author: jiasiang
 */

#ifndef LIB_SQLITE_SQLVALUE_H_
#define LIB_SQLITE_SQLVALUE_H_
#include <string>

using namespace std;

class SqlValue {

public:
	string column;
	string value;

	SqlValue();
	SqlValue(string, string);
};

#endif /* LIB_SQLITE_SQLVALUE_H_ */
