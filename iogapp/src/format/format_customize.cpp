/*
 * formatcustomize.cpp
 *
 *  Created on: May 10, 2018
 *      Author: japan
 */

#include "format_customize.h"

#include "../lib/Converter.h"
#include "../lib/sqlite/SqlRow.h"
#include "../lib/sqlite/SqlTable.h"
#include "../other/MemoryArea.h"
#include "../Global.h"

void format_definition::initialize(std::string tid) {
	list.clear();
	SqlTable sqlData;
	sqlData.initialTable("format_custom_client", "tid =" + tid);
	newIndex = 0;
	index = 0;

	for (unsigned int i = 1; i < sqlData.rowCount(); i++) {
		SqlRow rowdata = sqlData.getRow(i);
		FormatCustomDataInfo Data;
		Data.rowid = rowdata.getCell("rowid");
		Data.com = rowdata.getCell("command");
		Data.CommandType = rowdata.getCellToInt("CommandType");
		// 0->string  1->10   2->16
		Data.SplitCode = rowdata.getCell("SplitCode");
		Data.head = rowdata.getCell("head");
		Data.tail = rowdata.getCell("tail");
		Data.length = rowdata.getCell("length");
		list.push_back(Data);
	}

	for (auto itr = list.begin(); itr != list.end(); itr++) {
		sqlData.initialTable("format_custom_client_recv", "ccid=" + itr->rowid);
		for (unsigned int i = 1; i < sqlData.rowCount(); i++) {
			SqlRow rowdata = sqlData.getRow(i);
			FormatCustomRecv recv;
			recv.position = rowdata.getCellToInt("position");
			recv.mode = rowdata.getCellToInt("changeway");
			recv.address = rowdata.getCellToInt("writeaddress");
			recv.param = rowdata.getCellToInt("parameter");
			itr->recv.push_back(recv);
		}
	}
}

unsigned int format_definition::stringToint(string str) {
	unsigned int ss;
	std::stringstream ff;
	ff << std::hex << str;
	ff >> ss;
	return ss;
}

TransactionParameter* format_definition::createCommand() {
	TransactionParameter *result = NULL;
	vector<string> str;
	if (list.size() <= 0)
		return result;

	result = new TransactionParameter();
	FormatCustomDataInfo info = list[newIndex];
	if (newIndex == index)
		newIndex = 0;

	result->data.buf = new char[1024];
	result->data.length = Converter::stoi(info.length);
	ReceiveParameter *rp = &result->receiveParameter;
	if (info.head == "") {
		if (info.tail == "") {
			rp->mode = RECEIVE_MODE::LENGTH_ONLY;
		} else {
			rp->mode = RECEIVE_MODE::TAIL_LENGTH;
		}
	} else {
		if (info.tail == "") {
			rp->mode = RECEIVE_MODE::HEAD_LENGTH;
		} else {
			rp->mode = RECEIVE_MODE::HEAD_TAIL_LENGTH;
		}
	}
	switch (info.CommandType) {
	case 0: //string
		result->command.length = info.com.size();
		result->command.buf = new char[result->command.length];
		strcpy(result->command.buf, info.com.c_str());

		rp->tail.buf = new char[info.tail.size()];
		strcpy(rp->tail.buf, info.tail.c_str());
		rp->tail.length = info.tail.size();
		rp->head.buf = new char[info.head.size()];
		strcpy(rp->head.buf, info.head.c_str());
		rp->head.length = info.head.size();
		rp->length = Converter::stoi(info.length);
		break;
	case 1: //10
		str = Converter::strSplit(info.com, info.SplitCode);
		result->command.verify((int) str.size());
		for (int i = 0; i < result->command.length; i++)
			result->command.buf[i] = Converter::stoi(str[i].c_str());
		str = Converter::strSplit(info.tail, info.SplitCode);
		rp->tail.verify((int) str.size());
		for (int i = 0; i < rp->tail.length; i++)
			rp->tail.buf[i] = Converter::stoi(str[i].c_str());
		str = Converter::strSplit(info.head, info.SplitCode);
		rp->head.verify((int) str.size());
		for (int i = 0; i < rp->head.length; i++)
			rp->head.buf[i] = Converter::stoi(str[i].c_str());
		rp->length = Converter::stoi(info.length);
		break;
	case 2: //16
		str = Converter::strSplit(info.com, info.SplitCode);
		result->command.verify((int) str.size());
		for (int i = 0; i < result->command.length; i++)
			result->command.buf[i] = stringToint(str[i]);

		str = Converter::strSplit(info.tail, info.SplitCode);
		rp->tail.verify((int) str.size());
		for (int i = 0; i < rp->tail.length; i++)
			rp->tail.buf[i] = stringToint(str[i]);

		str = Converter::strSplit(info.head, info.SplitCode);
		rp->head.verify((int) str.size());
		for (int i = 0; i < rp->head.length; i++)
			rp->head.buf[i] = stringToint(str[i]);

		rp->length = Converter::stoi(info.length);
		break;
	}
	index = newIndex;
	newIndex++;
	if (newIndex >= list.size())
		newIndex = 0;
	return result;
}

format_definition::~format_definition() {
}

bool format_definition::processData(TransactionParameter* param) {
	DataInfo *data = &param->data;
	FormatCustomDataInfo info = list[index];
	int Writeindex;
	string str;
	char buff[data->length];
	int position, changeway, writeaddress, parameter;

	for (auto item = info.recv.begin(); item != info.recv.end(); item++) {
		position = item->position;
		changeway = item->mode;
		writeaddress = item->address;
		parameter = item->param;
		switch (changeway) {
		case 0:
			Writeindex = 0;
			for (int i = position; i < position + parameter; i++) {
				if (i >= data->length)
					break;
				MemoryArea::setShort_RW(writeaddress + Writeindex,
						data->buf[i]);
				Writeindex++;
			}
			break;
		case 1:
			if (position >= data->length)
				break;
			MemoryArea::setShort_RW(writeaddress,
					Converter::chartoshort(data->buf + position));
			break;
		case 2:
			Global::charcpy(buff, data->buf + position, parameter);
			buff[parameter] = 0;
			MemoryArea::setShort_RW(writeaddress,Converter::stoi(string(buff)));
					break;
				}
			}
			return true;
		}

		TransactionParameter* format_definition::prepareCommand() {
			return NULL;
		}

		bool format_definition::processCommand(TransactionParameter*) {
			return false;
		}
