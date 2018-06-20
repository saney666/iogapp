/*
 * Formatedwards.cpp
 *
 *  Created on: May 31, 2018
 *      Author: japan
 */
#include "FormatEdwards.h"

#include "../lib/Converter.h"
#include "../lib/sqlite/SqlRow.h"
#include "../lib/sqlite/SqlTable.h"
#include "../other/MemoryArea.h"
#include "../Global.h"

Format_Edwards::~Format_Edwards() {
}

void Format_Edwards::initialize(std::string tid) {
	SqlTable sqlData;
	sqlData.initialTable("format_edwards", "tid =" + tid);
	index = sqlData.rowCount() - 1;
	newIndex = 0;
	first = true;
	for (unsigned int i = 1; i < sqlData.rowCount(); i++) {
		SqlRow rowdata = sqlData.getRow(i);
		Edwards_DataInfo Data;
		Data.rowid = rowdata.getCell("rowid");
		Data.com = rowdata.getCell("command");
		Data.writeaddress = rowdata.getCell("writeAddress");
		Data.initialize();
//		Data.print();
		//Edwards_DataInfo::print();
		list.push_back(Data);
	}
}

void Edwards_DataInfo::print() {
	cout << "rowid : " << rowid << endl;
	cout << "com : " << com << endl;
	cout << "writeaddress : " << writeaddr << endl;
}

void Edwards_DataInfo::initialize() {
	rowid = Converter::toType<int>(rowid);
	writeaddr = Converter::toType<int>(writeaddress);
}

TransactionParameter* Format_Edwards::createCommand() {
	TransactionParameter *result = new TransactionParameter();
	ReceiveParameter *rp = &result->receiveParameter;
	char CR = 13;
	vector<string> str;
	if (newIndex == index)
		newIndex = 0;
	if (comErrIndex == index)
		comErrIndex = 0;
	rp->mode = RECEIVE_MODE::TAIL_LENGTH;
	result->data.buf = new char[1024];
	result->data.length = 50;
	rp->tail.verify(2);
	rp->tail.buf[0] = 13;
	rp->tail.buf[1] = 10;
	rp->length = 50;
	if (first) {
		string Fomrmat_mode("!F1");
		result->command.verify(4);
		strcpy(result->command.buf, Fomrmat_mode.c_str());
		result->command.buf[3] = CR;
	} else {
		result->command.verify((int) list[newIndex].com.size() + 1);
		strcpy(result->command.buf, list[newIndex].com.c_str());
		result->command.buf[result->command.length - 1] = CR;
		list[newIndex].syntax = result->command.buf[1];
		newIndex++;
	}
	return result;
}

bool Format_Edwards::processData(TransactionParameter* param) {
	DataInfo *data = &param->data;
	string str;
	int index = 0;
	char buff[3];
	if (first) {
		if (data->buf[0] == 49 && data->buf[1] == 13 && data->length == 3
				&& data->buf[2] == 10) {
			first = false;
			return true;
		}
	} else {
		if (verifyFormat(data) && verifyData(data)) {
			switch (list[newIndex - 1].syntax) {
			case 'V':
				valueAnalysis(data, 4);
				switch (MemoryArea::getShort_RW(
						list[newIndex - 1].writeaddr + 1)) {
				case 0:
					break;
				case 1:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 3,
							pow(2, 0));
					break;
				case 9:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 3,
							pow(2, 1));
					break;
				case 10:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 3,
							pow(2, 2));
					break;
				case 11:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 3,
							pow(2, 3));
					break;
				case 12:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 3,
							pow(2, 4));
					break;
				case 13:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 3,
							pow(2, 5));
					break;
				case 14:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 3,
							pow(2, 6));
					break;
				}
				break;

			case 'S':
				for (int i = 0; i < data->length; i += 2) {
					Global::charcpy(buff, data->buf + i, 2);
					buff[2] = 0;
					MemoryArea::setShort_RW(
							list[newIndex - 1].writeaddr + index,
							Converter::chartoshort(buff));
					index++;
				}
				break;
			case 'P':
				valueAnalysis(data, 7);
				switch (MemoryArea::getShort_RW(
						list[newIndex - 1].writeaddr - 1)) {
				case 0:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 6,
							pow(2, 0));
					break;
				case 1:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 6,
							pow(2, 1));
					break;
				case 2:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 6,
							pow(2, 2));
					break;
				case 3:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 6,
							pow(2, 3));
					break;
				case 4:
					MemoryArea::setShort_RW(list[newIndex - 1].writeaddr + 6,
							0);
					break;
				}
				break;
			case 'D':
				MemoryArea::setShort_RW(list[newIndex - 1].writeaddr - 1,
						data->buf[0]);
				break;
			case 'F':
				MemoryArea::setShort_RW(list[newIndex - 1].writeaddr - 1,
						data->buf[0]);
				break;
			}
			return true;
		}
	}
	return false;
}

void Format_Edwards::valueAnalysis(DataInfo* data, int max) {
	vector<string> strlist;
	char buf[data->length - 2];
	for (int x = 0; x < data->length - 2; x++)
		buf[x] = data->buf[x];
	strlist = Converter::strSplit(buf, ",");
	for (int i = 0; i < max; i++) {
		MemoryArea::setShort_RW(list[newIndex - 1].writeaddr - 1 + i,
				Converter::stoi(strlist[i]));
	}
}

bool Format_Edwards::verifyFormat(DataInfo* data) {
	int count = 0;
	for (int i = 0; i < data->length - 3; i++) {
		if (data->buf[i] == 44)
			count++;
	}
	switch (list[newIndex - 1].syntax) {
	case 'V':
		if (count == 3)
			return true;
		break;
	case 'S':
		if (count == 0)
			return true;
		break;
	case 'P':
		if (count == 6)
			return true;
		break;
	case 'D':
		if (count == 0)
			return true;
		break;
	case 'F':
		if (count == 0)
			return true;
		break;
	}
	return false;
}

bool Format_Edwards::verifyData(DataInfo* data) {
	for (int i = 0; i < data->length - 3; i++) {
		if ((data->buf[i] > 57 || data->buf[i] < 48) && data->buf[i] != 44) {
			return false;
		}
	}
	return true;
}

TransactionParameter * Format_Edwards::prepareCommand() {
	return NULL;
}

bool Format_Edwards::processCommand(TransactionParameter*) {
	return false;
}

