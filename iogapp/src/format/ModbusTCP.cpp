/*
 * ModbusTCP.cpp
 *
 *  Created on: May 16, 2018
 *      Author: jiasiang
 */

#include "ModbusTCP.h"
#include "../lib/sqlite/SqlTable.h"
#include "../lib/Converter.h"
#include "../other/MemoryArea.h"
#include "../Global.h"
#include <math.h>

void ModbusTCP::initialize(std::string tid) {
	list.clear();
	index = 0;
	beforeIndex = 0;
	SqlTable table("format_modbus_tcp", "tid=" + tid);

	if (table.rowCount() > 0) {
		for (unsigned int i = 1; i < table.rowCount(); i++) {
			SqlRow row = table.getRow(i);
			ModbusTCP_Info info;
			info.transactionId = row.getCell("transactionId");
			info.protocolId = row.getCell("protocolId");
			info.unitId = row.getCell("unitId");
			info.function = row.getCell("function");
			info.readAddr = row.getCell("readAddr");
			info.number = row.getCell("number");
			info.writeAddr = row.getCell("writeAddr");
			info.initialize();
			list.push_back(info);
		}
	}
}

TransactionParameter* ModbusTCP::createCommand() {
	TransactionParameter *result = NULL;
	if (list.size() == 0)
		return result;

	if (index >= list.size())
		index = 0;
	result = new TransactionParameter();
	list[index].setReceiveParam(&result->receiveParameter);
	list[index].setCommand(&result->command);
	result->data.verify(result->receiveParameter.length + 10);

	beforeIndex = index;
	++index;
	return result;
}

bool ModbusTCP::processData(TransactionParameter* param) {
	DataInfo *data = &param->data;
	ModbusTCP_Info info = list[beforeIndex];
	if (verifyFormat(data) && verifyData(data)) {
		vector<bool> boolValues;
		vector<short> shortValues;
		switch (info.fun) {
		case 1:
		case 2:
			boolValues = getBool(data->buf + 9, info.num);
			for (unsigned int i = 0; i < info.num; i++) {
				if (info.fun == 1)
					MemoryArea::setBool_RW(info.waddr + i, boolValues[i]);
				else
					MemoryArea::setBool_R(info.waddr + i, boolValues[i]);
			}
			break;
		case 3:
		case 4:
			shortValues = getShort(data->buf + 9, info.num);
			for (unsigned int i = 0; i < info.num; i++) {
				if (info.fun == 3)
					MemoryArea::setShort_RW(info.waddr + i, shortValues[i]);
				else
					MemoryArea::setShort_R(info.waddr + i, shortValues[i]);
			}
			break;
		}
		return true;
	}

	return false;
}

bool ModbusTCP::verifyFormat(DataInfo*data) {

	if (data->length == (int) list[beforeIndex].dataLength) {
		if (data->buf[4] * 256 + data->buf[5] + 6 == data->length) {
			if (data->buf[8] + 9 == data->length) {
				return true;
			}
		}
	}
	return false;
}

bool ModbusTCP::verifyData(DataInfo*data) {
	ModbusTCP_Info info = list[beforeIndex];
	if (data->buf[7] == info.fun && data->buf[6] == info.uid) {
		if (info.tid == (unsigned short) Converter::chartoshort(data->buf)) {
			if (info.pid
					== (unsigned short) Converter::chartoshort(data->buf + 2)) {
				return true;
			}
		}
	}
	return false;
}

TransactionParameter* ModbusTCP::prepareCommand() {
	TransactionParameter *result = new TransactionParameter;
	result->command.verify(300);
	ReceiveParameter *rp = &result->receiveParameter;
	rp->mode = RECEIVE_MODE::BYTE_LENGTH;
	rp->length = 6;
	rp->bytePos = 4;
	rp->byteLen = 2;
	rp->byteValueMax = 280;
	return result;
}

void ModbusTCP::setErrorData(DataInfo* data, ModbusTCP_Info info, int err) {
	data->verify(9);
	Global::charcpy(data->buf, info.tid);
	Global::charcpy(data->buf + 2, info.pid);
	Global::charcpy(data->buf + 4, 3);
	data->buf[6] = info.uid;
	data->buf[7] = info.fun + 0x80;
	data->buf[8] = err;
}

void ModbusTCP::setWriteData(DataInfo* data, ModbusTCP_Info info) {
	data->verify(12);
	Global::charcpy(data->buf, info.tid);
	Global::charcpy(data->buf + 2, info.pid);
	Global::charcpy(data->buf + 4, 6);
	data->buf[6] = info.uid;
	data->buf[7] = info.fun;
	Global::charcpy(data->buf + 8, info.raddr);
	Global::charcpy(data->buf + 10, info.num);
}

void ModbusTCP::setReadData(DataInfo* data, ModbusTCP_Info info) {
	vector<bool> values_b;
	vector<short> values_s;
	info.calculateLength();
	data->verify(info.dataLength);
	Global::charcpy(data->buf, info.tid);
	Global::charcpy(data->buf + 2, info.pid);
	Global::charcpy(data->buf + 4, info.dataLength - 6);
	data->buf[6] = info.uid;
	data->buf[7] = info.fun;
	data->buf[8] = info.dataLength - 9;
	for (unsigned int i = 0; i < info.num; i++) {
		switch (info.fun) {
		case 1:
			values_b.push_back(MemoryArea::getBool_RW(info.raddr + i));
			break;
		case 2:
			values_b.push_back(MemoryArea::getBool_R(info.raddr + i));
			break;
		case 3:
			values_s.push_back(MemoryArea::getShort_RW(info.raddr + i));
			break;
		case 4:
			values_s.push_back(MemoryArea::getShort_R(info.raddr + i));
			break;
		}
	}
	switch (info.fun) {
	case 1:
	case 2:
		setBool(values_b, data->buf + 9);
		break;
	case 3:
	case 4:
		setShort(values_s, data->buf + 9);
		break;
	}
}

ModbusTCP::~ModbusTCP() {

}

bool ModbusTCP::processCommand(TransactionParameter*parameter) {
	ModbusTCP_Info info;
	DataInfo *cmd = &parameter->command;
	vector<bool> value_b;
	vector<short> value_s;
	int err = 0;
	if (cmd->length >= 12) {
		info.tid = (unsigned short) Converter::chartoshort(cmd->buf);
		info.pid = (unsigned short) Converter::chartoshort(cmd->buf + 2);
		info.uid = cmd->buf[6];
		info.fun = cmd->buf[7];
		info.raddr = (unsigned short) Converter::chartoshort(cmd->buf + 8);
		info.num = (unsigned short) Converter::chartoshort(cmd->buf + 10);

		switch (info.fun) {
		case 1:
		case 2:
		case 3:
		case 4:
			if (info.num > 127)
				err = 3;
			else if ((info.num + info.raddr) > 65536)
				err = 2;

			break;
		}

		if (err == 0) {
			switch (info.fun) {
			case 1:
			case 2:
			case 3:
			case 4:
				setReadData(&parameter->data, info);
				break;
			case 5:
			case 6:
			case 15:
			case 16:
				setWriteData(&parameter->data, info);
				break;
			default:
				err = 1;
				break;
			}
		}

		if (err > 0)
			setErrorData(&parameter->data, info, err);
		else {
			switch (info.fun) {
			case 5:
				if (info.num)
					MemoryArea::setBool_RW(info.raddr, true);
				else
					MemoryArea::setBool_RW(info.raddr, false);
				break;
			case 6:
				MemoryArea::setShort_RW(info.raddr, info.num);
				break;
			case 15:
				value_b = getBool(cmd->buf + 13, info.num);
				for (unsigned int i = 0; i < info.num; i++)
					MemoryArea::setBool_RW(info.raddr + i, value_b[i]);
				break;
			case 16:
				value_s = getShort(cmd->buf + 13, info.num);
				for (unsigned int i = 0; i < info.num; i++)
					MemoryArea::setShort_RW(info.raddr + i, value_s[i]);
				break;
			}
		}
		return true;
	}
	return false;
}

void ModbusTCP_Info::valueToChar(string value, char *buf) {
	int val = Converter::stoi(value);
	buf[1] = val & 0xff;
	buf[0] = (val & 0xff00) >> 8;
}

void ModbusTCP_Info::initialize() {
	tid = Converter::toType<int>(transactionId);
	pid = Converter::toType<int>(protocolId);
	uid = Converter::toType<int>(unitId);
	fun = Converter::toType<int>(function);
	raddr = Converter::toType<int>(readAddr);
	num = Converter::toType<int>(number);
	waddr = Converter::toType<int>(writeAddr);

	calculateLength();

}

void ModbusTCP_Info::calculateLength() {
	switch (fun) {
	case 1:
	case 2:
		dataLength = 9 + (num - num % 8) / 8 + (num % 8 > 0 ? 1 : 0);
		break;
	case 3:
	case 4:
		dataLength = num * 2 + 9;
		break;
	default:
		dataLength = 0;
		break;
	}
}

void ModbusTCP_Info::setCommand(DataInfo *info) {
	info->length = 12;
	info->buf = new char[12];

	valueToChar(transactionId, info->buf);
	valueToChar(protocolId, info->buf + 2);
	valueToChar("6", info->buf + 4);
	info->buf[6] = Converter::stoi(unitId);
	info->buf[7] = Converter::stoi(function);
	valueToChar(readAddr, info->buf + 8);
	valueToChar(number, info->buf + 10);
}

void ModbusTCP_Info::setReceiveParam(ReceiveParameter *param) {
	param->mode = RECEIVE_MODE::LENGTH_ONLY;
	param->length = dataLength;
}

vector<bool> ModbusTCP::getBool(const char *buf, int num) {
	vector<bool> result;
	int index = 0;
	int count = 0;
	for (int i = 0; i < num; i++) {
		if (buf[index] & (int) pow(2, count++))
			result.push_back(true);
		else
			result.push_back(false);
		if (count >= 8) {
			++index;
			count = 0;
		}
	}

	return result;
}

vector<short> ModbusTCP::getShort(const char * buf, int num) {
	vector<short> result;
	for (int i = 0; i < num; i++)
		result.push_back(buf[i * 2] * 256 + buf[i * 2 + 1]);

	return result;
}

void ModbusTCP::setBool(vector<bool> values, char *buf) {
	unsigned int len = values.size();
	int index = 0;
	int count = 0;
	for (unsigned int i = 0; i < len; i++) {

		if (values[i])
			buf[index] += 1 << count;

		if (++count >= 8) {
			count = 0;
			++index;
		}
	}
}

void ModbusTCP::setShort(vector<short> values, char *buf) {
	unsigned int len = values.size();
	for (unsigned int i = 0; i < len; i++)
		Converter::shorttochar(buf + i * 2, values[i]);
}

