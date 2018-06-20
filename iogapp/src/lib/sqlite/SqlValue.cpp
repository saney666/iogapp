/*
 * SqlValue.cpp
 *
 *  Created on: Apr 26, 2018
 *      Author: jiasiang
 */

#include "SqlValue.h"
#include <sstream>

SqlValue::SqlValue() {
	this->column = "";
	this->value = "";
}

SqlValue::SqlValue(string column, string value) {
	this->column = column;
	this->value = value;
}

