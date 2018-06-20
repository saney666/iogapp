/*
 * ModbusRTU.cpp
 *
 *  Created on: May 16, 2018
 *      Author: jiasiang
 */

#include "ModbusRTU.h"
#include "../lib/sqlite/SqlTable.h"
#include "../lib/Converter.h"
#include "../other/MemoryArea.h"
#include "../Global.h"
#include <math.h>

void ModbusRTU_Info::valueToChar(string value, char*buf) {
	int val = Converter::stoi(value);
	buf[1] = val & 0xff;
	buf[0] = (val & 0xff00) >> 8;
}

void ModbusRTU_Info::setCommand(DataInfo *info) {
	info->length = 8;
	info->buf = new char[8];

	info->buf[0] = Converter::stoi(unitId);
	info->buf[1] = Converter::stoi(function);
	valueToChar(readAddr, info->buf + 2);
	valueToChar(number, info->buf + 4);
	Global::crcData(info->buf, 6);
}

void ModbusRTU_Info::setReceiveParam(ReceiveParameter* param) {
	param->mode = RECEIVE_MODE::LENGTH_ONLY;
	param->length = dataLength;
}

ModbusRTU::~ModbusRTU() {
}

void ModbusRTU_Info::initialize() {
	uid = Converter::toType<int>(unitId);
	fun = Converter::toType<int>(function);
	raddr = Converter::toType<int>(readAddr);
	num = Converter::toType<int>(number);
	waddr = Converter::toType<int>(writeAddr);
	calculateLength();
}

void ModbusRTU_Info::calculateLength() {
	switch (fun) {
	case 1:
	case 2:
		dataLength = 5 + (num - num % 8) / 8 + (num % 8 > 0 ? 1 : 0);
		break;
	case 3:
	case 4:
		dataLength = num * 2 + 5;
		break;
	default:
		dataLength = 0;
		break;
	}
}

void ModbusRTU_Info::printf() {
	cout << "id : " << unitId << endl;
	cout << "fun : " << function << endl;
	cout << "readAddr : " << readAddr << endl;
	cout << "number : " << number << endl;
	cout << "writeAddr : " << writeAddr << endl;
}

void ModbusRTU::initialize(std::string tid) {
	list.clear();
	index = 0;
	beforeIndex = 0;
	SqlTable table("format_modbus_rtu", "tid=" + tid);
	if (table.rowCount() > 0) {
		for (unsigned int i = 1; i < table.rowCount(); i++) {
			SqlRow row = table.getRow(i);
			ModbusRTU_Info info;
			info.unitId = row.getCell("unitId");
			info.function = row.getCell("function");
			info.readAddr = row.getCell("readAddress");
			info.number = row.getCell("number");
			info.writeAddr = row.getCell("writeAddress");
			info.initialize();
			list.push_back(info);
		}
	}
}

TransactionParameter* ModbusRTU::createCommand() {

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

bool ModbusRTU::processData(TransactionParameter* param) {
	DataInfo *data = &param->data;
	ModbusRTU_Info info = list[beforeIndex];
	if (verifyFormat(data) && verifyData(data)) {
		vector<bool> boolValues;
		vector<short> shortValues;
		switch (info.fun) {
		case 1:
		case 2:
			boolValues = getBool(data->buf + 3, info.num);
			for (unsigned int i = 0; i < info.num; i++) {
				if (info.fun == 1)
					MemoryArea::setBool_RW(info.waddr + i, boolValues[i]);
				else
					MemoryArea::setBool_R(info.waddr + i, boolValues[i]);
			}
			break;
		case 3:
		case 4:
			shortValues = getShort(data->buf + 3, info.num);
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

bool ModbusRTU::verifyFormat(DataInfo*data) {
	if (data->length == (int) list[beforeIndex].dataLength) {
		if (data->buf[2] + 5 == data->length) {
			return true;
		}
	}
	return false;
}

bool ModbusRTU::verifyData(DataInfo * data) {
	ModbusRTU_Info info = list[beforeIndex];
	char crcBuff[2];
	if (data->buf[1] == info.fun && data->buf[0] == info.uid) {
		crcBuff[0] = data->buf[data->length - 2];
		crcBuff[1] = data->buf[data->length - 1];
		Global::crcData(data->buf, data->length - 2);
		if (crcBuff[0] == data->buf[data->length - 2]
				&& crcBuff[1] == data->buf[data->length - 1]) {
			return true;
		}

	}
	return false;
}

vector<bool> ModbusRTU::getBool(const char *buf, int num) {
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

vector<short> ModbusRTU::getShort(const char *buf, int num) {
	vector<short> result;
	for (int i = 0; i < num; i++)
		result.push_back(buf[i * 2] * 256 + buf[i * 2 + 1]);

	return result;
}

TransactionParameter* ModbusRTU::prepareCommand() {
	TransactionParameter *result = new TransactionParameter;
	result->command.verify(300);
	ReceiveParameter *rp = &result->receiveParameter;
	rp->mode = RECEIVE_MODE::LENGTH_ONLY;
	rp->length = 8;
	return result;
}

bool ModbusRTU::processCommand(TransactionParameter * parameter) {
	ModbusRTU_Info info;
	DataInfo *cmd = &parameter->command;
	vector<bool> value_b;
	vector<short> value_s;
	char crcCheck[2];
	int err = 0;
	if (cmd->length == 8) {
		info.uid = cmd->buf[0];
		info.fun = cmd->buf[1];
		info.raddr = (unsigned short) Converter::chartoshort(cmd->buf + 2);
		info.num = (unsigned short) Converter::chartoshort(cmd->buf + 4);
		crcCheck[0] = cmd->buf[6];
		crcCheck[1] = cmd->buf[7];
		Global::crcData(cmd->buf, 6);
		if (crcCheck[0] == cmd->buf[6] && crcCheck[1] == cmd->buf[7]) {

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
				}
			}
			return true;
		}
		return false;
	}
	return false;
}

void ModbusRTU::setBool(vector<bool> values, char *buf) {
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

void ModbusRTU::setShort(vector<short> values, char *buf) {
	unsigned int len = values.size();
	for (unsigned int i = 0; i < len; i++)
		Converter::shorttochar(buf + i * 2, values[i]);
}

void ModbusRTU::setWriteData(DataInfo* data, ModbusRTU_Info info) {
	data->verify(8);
	data->buf[0] = info.uid;
	data->buf[1] = info.fun;
	Global::charcpy(data->buf + 2, info.raddr);
	Global::charcpy(data->buf + 4, info.num);
	Global::crcData(data->buf, 6);
}

void ModbusRTU::setReadData(DataInfo* data, ModbusRTU_Info info) {
	vector<bool> values_b;
	vector<short> values_s;
	info.calculateLength();
	data->verify(info.dataLength);
	data->buf[0] = info.uid;
	data->buf[1] = info.fun;
	data->buf[2] = info.dataLength - 5;
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
		setBool(values_b, data->buf + 3);
		Global::crcData(data->buf, info.dataLength - 2);
		break;
	case 3:
	case 4:
		setShort(values_s, data->buf + 3);
		Global::crcData(data->buf, info.dataLength - 2);
		break;
	}

}

void ModbusRTU::setErrorData(DataInfo*, ModbusRTU_Info modbusRTU_Info,
		int int1) {
}
