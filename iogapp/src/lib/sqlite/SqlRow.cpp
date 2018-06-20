/*
 * SqlRow.cpp
 *
 *  Created on: Apr 27, 2018
 *      Author: jiasiang
 */

#include "SqlRow.h"
#include "../Converter.h"

unsigned int SqlRow::getIndex(string column) {
	unsigned int max = data.size();
	for (unsigned int i = 0; i < max; i++) {
		if (column.compare(data[i].column) == 0)
			return i;
	}
	return 0;
}

SqlRow::SqlRow() {

}

SqlRow::SqlRow(vector<SqlValue> data) {
	this->data = data;
}

unsigned int SqlRow::cellCount() {
	return data.size();
}

string SqlRow::getTitle(unsigned int index) {
	if (index < data.size())
		return data[index].column;

	return "";
}

string SqlRow::getCell(unsigned int index) {
	if (index < data.size())
		return data[index].value;

	return "";
}

string SqlRow::getCell(string column) {
	return data[getIndex(column)].value;
}

int  SqlRow::getCellToInt(string column) {
	return Converter::stoi(data[getIndex(column)].value);
}

void SqlRow::addCell(string title, string value) {
	data.push_back(SqlValue(title, value));
}

void SqlRow::setCell(unsigned int index, string value) {
	if (index < data.size())
		data[index].value = value;
}

int SqlRow::getCellToInt(unsigned int index) {
	return Converter::stoi(data[index].value);
}

void SqlRow::setCell(string column, string value) {
	data[getIndex(column)].value = value;
}

