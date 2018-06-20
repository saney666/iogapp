/*
 * Transaction.cpp
 *
 *  Created on: May 3, 2018
 *      Author: jiasiang
 */

#define RECEIVE_DELAY_MS 50

#include "Transaction.h"

#include <string.h>
#include <functional>
#include <iostream>
#include <string>
#include <math.h>

#include "../Global.h"
#include "../lib/Converter.h"
#include "../lib/MyTimer.h"
#include "../lib/sqlite/SqlRow.h"
#include "../lib/sqlite/SqlTable.h"
#include "../lib/Log.h"
#include "../other/MemoryArea.h"

void Transaction::clientMode() {
	addLog("start client");
	life = true;
	int tryCount;
	bool isAbnormal;
	while (sw) {
		TransactionParameter *parameter = createCommand();
		if (parameter == NULL) {
			MemoryArea::setBool_R(abnormalPoint, true);
			Global::sleeps(1);
			continue;
		}
		tryCount = retry;
		isAbnormal = true;
		while (tryCount--) {
			if (interface == LAN) {
				if (!clientAvailable()) {
					if (!client->connectTo()) {
						Log::addSystem(Log::TRANSACTION,
								rowid + ", Connection fail.");
						Global::sleeps(1);
					}
				}
			}
			if (sendCommand(parameter)) {
				if (receiveData(parameter)) {
					Log::addTransaction(rowid, "Receive", parameter->data.buf,
							parameter->data.length, activeLog);
					if (processData(parameter)) {
						Global::sleepms(delay);
						tryCount = 0;
						isAbnormal = false;
						break;
					}
				}
			}
		}
		MemoryArea::setBool_R(abnormalPoint, isAbnormal);
		delete parameter;
	}
	life = false;
	addLog("stop client");
}

TransactionParameter *Transaction::createCommand() {
	TransactionParameter *result = NULL;
	if (formatBase)
		result = formatBase->createCommand();
	return result;
}

bool Transaction::sendCommand(TransactionParameter *parameter) {
	return sendData(&parameter->command);
}

bool Transaction::receiveData(TransactionParameter *parameter) {
	ReceiveParameter *rp = &parameter->receiveParameter;
	if (rp->length <= 0)
		return true;

	parameter->data.verify(rp->length);

	switch (rp->mode) {
	case RECEIVE_MODE::LENGTH_ONLY:
		return receiveMode_Length(rp, &parameter->data);
	case RECEIVE_MODE::HEAD_LENGTH:
		return receiveMode_HeadLength(rp, &parameter->data);
	case RECEIVE_MODE::TAIL_LENGTH:
		return receiveMode_TailLength(rp, &parameter->data);
	case RECEIVE_MODE::HEAD_TAIL_LENGTH:
		return receiveMode_HeadTailLength(rp, &parameter->data);
	case RECEIVE_MODE::BYTE_LENGTH:
		return receiveMode_BytelLength(rp, &parameter->data);
	}

	return false;
}

bool Transaction::receiveMode_Length(ReceiveParameter *rp, DataInfo *info) {
	MyTimer timer(timeout);
	DataInfo data(rp->length);

	timer.start();
	while (!timer.check()) {
		if (receiveByLength(&data)) {
			Global::charcpy(info->buf, data.buf, data.length);
			info->length = data.length;
			return true;
		}
		Global::sleepms(RECEIVE_DELAY_MS);
	}
	return false;
}

bool Transaction::receiveMode_HeadLength(ReceiveParameter *rp, DataInfo *info) {
	MyTimer timer(timeout);
	int index = 0;
	DataInfo data(rp->length - rp->head.length);
	timer.start();
	while (!timer.check()) {
		switch (index) {
		case 0:
			if (receiveByData(&rp->head))
				index = 1;
			break;
		case 1:
			if (receiveByLength(&data)) {
				Global::charcpy(info->buf, rp->head.buf, rp->head.length);
				Global::charcpy(info->buf + rp->head.length, data.buf,
						data.length);
				info->length = rp->length;
				return true;
			}
			break;
		}
		Global::sleepms(RECEIVE_DELAY_MS);
	}
	return false;
}

bool Transaction::receiveMode_TailLength(ReceiveParameter *rp, DataInfo *info) {
	MyTimer timer(timeout);
	DataInfo *tail = &rp->tail;

	int index = 0;
	int len = rp->length;
	int size = 0;
	DataInfo data(len);
	timer.start();
	while (!timer.check()) {
		if (receivableData() > 0) {
			size = receiveData(&data.buf[index], len);
			if (size + index >= tail->length) {
				for (int i = 0; i < size; i++) {
					if (tail->buf[0] != data.buf[i])
						continue;
					if (Global::compareStr(tail->buf, &data.buf[i],
							tail->length)) {
						Global::charcpy(info->buf, data.buf, i + tail->length);
						info->length = i + tail->length;
						return true;
					}
				}
			}
			index += size;
			len -= size;
			if (len <= 0)
				break;
		}
		Global::sleepms(RECEIVE_DELAY_MS);
	}
	return false;
}

bool Transaction::receiveMode_HeadTailLength(ReceiveParameter *rp,
		DataInfo *info) {
	MyTimer timer(timeout);
	DataInfo *tail = &rp->tail;

	int index = 0;
	int funIndex = 0;
	int len = rp->length - rp->head.length;
	int size = 0;
	DataInfo data(len);
	timer.start();
	while (!timer.check()) {
		switch (funIndex) {
		case 0:
			if (receiveByData(&rp->head))
				funIndex = 1;
			break;
		case 1:
			if (receivableData() > 0) {
				size = receiveData(&data.buf[index], len);
				if (size + index >= tail->length) {
					for (int i = 0; i < size; i++) {
						if (tail->buf[0] != data.buf[i])
							continue;
						if (Global::compareStr(tail->buf, &data.buf[i],
								tail->length)) {
							Global::charcpy(info->buf, rp->head.buf,
									rp->head.length);
							Global::charcpy(info->buf + rp->head.length,
									data.buf, i + tail->length);
							info->length = rp->head.length + i + tail->length;
							return true;
						}
					}
				}
				index += size;
				len -= size;
				if (len <= 0)
					return false;
			}
			break;
		}
		Global::sleepms(RECEIVE_DELAY_MS);
	}
	return false;
}

bool Transaction::receiveMode_BytelLength(ReceiveParameter *rp,
		DataInfo *info) {
	MyTimer timer(timeout);
	int funIndex = 0;
	int value = 0;
	DataInfo head(rp->length);
	timer.start();
	while (!timer.check()) {
		switch (funIndex) {
		case 0:
			if (receiveByLength(&head)) {
				for (int i = 0; i < rp->byteLen; i++)
					value += head.buf[rp->bytePos + i]
							* pow(256, rp->byteLen - i - 1);
				if (value <= rp->byteValueMax)
					funIndex = 1;
				else
					return false;
			}
			break;
		case 1:
			if (receivableData() >= value) {
				receiveData(info->buf + rp->length, value);
				Global::charcpy(info->buf, head.buf, rp->length);
				info->length = value + rp->length;
				return true;
			}
			break;
		}
		Global::sleepms(RECEIVE_DELAY_MS);
	}
	return false;
}

int Transaction::receivableData() {
	int result = 0;
	switch (interface) {
	case INTERFACE::LAN:
		if (clientAvailable()) {
			result = client->receivableSize();
		}
		break;
	case INTERFACE::SERIAL:
		result = com.receivableSize();
		break;
	}
	return result;
}

int Transaction::receiveData(char *buf, int len) {
	int receiveSize = 0;
	switch (interface) {
	case INTERFACE::LAN:
		if (clientAvailable()) {
			receiveSize = client->receiveFrom((unsigned char*) buf, len);
			if (receiveSize > 0)
				return receiveSize;
			else if (receiveSize == 0)
				client->disconnect();
		}
		break;
	case INTERFACE::SERIAL:
		receiveSize = com.receiveData(buf, len);
		break;
	}
	return receiveSize;
}

bool Transaction::receiveByData(const DataInfo *info) {
	if (info->length > 0 && receivableData() >= info->length) {
		DataInfo data(info->length);
		receiveData(data.buf, 1);
		if (data.buf[0] == info->buf[0]) {
			if (info->length - 1 > 0) {
				receiveData(data.buf + 1, info->length - 1);
				if (memcmp(data.buf, info->buf, info->length) == 0)
					return true;
			}
			return true;
		}
	}
	return false;
}

bool Transaction::receiveByLength(DataInfo *info) {
	if (receivableData() >= info->length) {
		receiveData(info->buf, info->length);
		return true;
	}
	return false;
}

bool Transaction::processData(TransactionParameter *parameter) {
	if (formatBase)
		return formatBase->processData(parameter);
	return false;
}

void Transaction::serverMode() {
	addLog("start server");
	life = true;
	bool isAbnormal;
	while (sw) {
		if (interface == 0) {
			if (!server.isListen())
				server.startListen(server.port);
			if (!client) {
				Global::sleepms(100);
				MemoryArea::setBool_R(abnormalPoint, true);
				continue;
			}
		}

		TransactionParameter *parameter = prepareCommand();
		if (parameter == NULL) {
			MemoryArea::setBool_R(abnormalPoint, true);
			Global::sleeps(1);
			continue;
		}
		isAbnormal = true;
		if (receiveCommand(parameter)) {
			Log::addTransaction(rowid, "Receive", parameter->command.buf,
					parameter->command.length, activeLog);
			if (processCommand(parameter)) {
				sendResponse(parameter);
				isAbnormal = false;
				Global::sleepms(delay);
			}
		} else {
			if (interface == INTERFACE::LAN) {
				addLog("timeout to kill client");
				delete client;
				client = NULL;
			}
		}
		MemoryArea::setBool_R(abnormalPoint, isAbnormal);
		delete parameter;
	}
	life = false;
	addLog("stop server");
}

TransactionParameter * Transaction::prepareCommand() {
	TransactionParameter *result = NULL;
	if (formatBase)
		result = formatBase->prepareCommand();
	return result;
}

bool Transaction::receiveCommand(TransactionParameter *parameter) {
	ReceiveParameter *rp = &parameter->receiveParameter;
	parameter->command.verify(rp->length);

	switch (rp->mode) {
	case RECEIVE_MODE::LENGTH_ONLY:
		return receiveMode_Length(rp, &parameter->command);
	case RECEIVE_MODE::HEAD_LENGTH:
		return receiveMode_HeadLength(rp, &parameter->command);
	case RECEIVE_MODE::TAIL_LENGTH:
		return receiveMode_TailLength(rp, &parameter->command);
	case RECEIVE_MODE::HEAD_TAIL_LENGTH:
		return receiveMode_HeadTailLength(rp, &parameter->command);
	case RECEIVE_MODE::BYTE_LENGTH:
		return receiveMode_BytelLength(rp, &parameter->command);
	}

	return false;
}

bool Transaction::processCommand(TransactionParameter *parameter) {
	if (formatBase)
		return formatBase->processCommand(parameter);
	return false;
}

void Transaction::sendResponse(TransactionParameter *parameter) {
	sendData(&parameter->data);
}

bool Transaction::sendData(DataInfo *info) {
	int index, length, sendSize;
	sendSize = 0;
	index = 0;
	length = info->length;
	Log::addTransaction(rowid, "Send", info->buf, info->length, activeLog);
	while (sendSize < length) {
		switch (interface) {
		case INTERFACE::LAN:
			if (clientAvailable()) {
				client->clear();
				sendSize = client->sendTo((unsigned char*) &info->buf[index],
						length);
				if (sendSize <= 0) {
					client->disconnect();
					return false;
				}
			} else
				return false;
			break;
		case INTERFACE::SERIAL:
			com.cleanPort();
			sendSize = com.sendCommand(&info->buf[index], length);
			break;
		}

		if (sendSize < length) {
			length -= sendSize;
			index = sendSize;
			sendSize = 0;
		}
	}
	return true;
}

void Transaction::defaultSetting() {
	rowid = "";
	tid = 0;
	interface = 0;
	iid = 0;
	timeout = 0;
	retry = 0;
	delay = 0;
	direction = 0;
	format = 0;
	sw = false;
	life = false;
	client = NULL;
	server.accepted = bind(&Transaction::socketAccepted, this, placeholders::_1,
			placeholders::_2);
	formatBase = NULL;
	activeLog = false;
	abnormalPoint = 0;
}

void Transaction::initialInfo() {
	SqlTable table;
	table.initialTable("tranInfo", "rowid=" + rowid);
	if (table.rowCount() > 1) {
		SqlRow row = table.getRow(1);
		interface = row.getCellToInt("interface");
		iid = row.getCellToInt("iid");
		timeout = row.getCellToInt("timeout");
		retry = row.getCellToInt("retry");
		delay = row.getCellToInt("delay");
		direction = row.getCellToInt("direction");
		format = row.getCellToInt("format");
		activeLog = row.getCellToInt("activeLog");
		abnormalPoint = row.getCellToInt("abnormalPoint");
		sw = false;
		life = false;
	}
	initialFormat();
}

void Transaction::initialFormat() {
	//TODO when you add format, add here
	switch (format) {
	case FORMAT::CUSTOM:
		formatBase = new format_definition();
		break;
	case FORMAT::MODBUS_RTU:
		formatBase = new ModbusRTU();
		break;
	case FORMAT::MODBUS_TCP:
		formatBase = new ModbusTCP();
		break;
	case FORMAT::EDWARDS:
		formatBase = new Format_Edwards();
		break;
	}
	if (formatBase)
		formatBase->initialize(rowid);
}

void Transaction::initialInterface() {
	SqlTable t;
	SqlRow row;
	switch (interface) {
	case INTERFACE::LAN:
		t.initialTable("connection", "rowid=" + Converter::itos(iid));
		if (t.rowCount() > 1) {
			row = t.getRow(1);
			if (direction == DIRECTION::SERVER) {
				server.startListen(Converter::stoi(row.getCell("port")));
			} else {
				client = new Socket::Client(row.getCell("ip"),
						row.getCellToInt("port"));
				//client->connectTo(row.getCell("ip"), row.getCellToInt("port"));
			}
		}
		break;
	case INTERFACE::SERIAL:
		t.initialTable("coms", "rowid=" + Converter::itos(iid));
		if (t.rowCount() > 1) {
			row = t.getRow(1);
			com.openPort(Converter::stoi(row.getCell("com")));
			com.setMode(Converter::stoi(row.getCell("mode")));
			com.setBaud(Converter::stoi(row.getCell("baud")));
			com.setParameter(Converter::stoi(row.getCell("parity")),
					Converter::stoi(row.getCell("databits")),
					Converter::stoi(row.getCell("stopbits")));
			com.setFlowControl(Converter::stoi(row.getCell("flow")));
			com.print();
		}
		break;
	}
}

void Transaction::shutdownInterface() {
	switch (interface) {
	case INTERFACE::LAN:
		if (client) {
			if (client->isConnected())
				client->disconnect();
			delete client;
			client = NULL;
		}
		if (direction == DIRECTION::SERVER)
			server.stop();
		break;
	case INTERFACE::SERIAL:
		com.closePort();
		break;
	}
}

void Transaction::syncRestart() {
	function<void()> f = bind(&Transaction::restart, this);
	Global::addThreadd(f);
}

void Transaction::syncStop() {
	function<void()> f = bind(&Transaction::stop, this);
	Global::addThreadd(f);
}

bool Transaction::clientAvailable() {
	if (client) {
		return client->isConnected();
	}
	return false;
}

void Transaction::socketAccepted(Socket::Server *server,
		Socket::Client *client) {

	if (this->client) {
		addLog("Get client to kill");
		client->disconnect();
		delete client;
	} else {
		addLog("Get client to bind");
		this->client = client;
	}
}

void Transaction::addLog(string message) {
	Log::addSystem(Log::TRANSACTION, rowid + "," + message);
}

Transaction::Transaction() {
	defaultSetting();
}

Transaction::Transaction(string rowid) {
	defaultSetting();
	this->rowid = rowid;
	tid = Converter::stoi(rowid);
}

void Transaction::start() {
	addLog("Start ");
	initialInfo();
	initialInterface();
	function<void()> f;
	if (direction == DIRECTION::SERVER)
		f = bind(&Transaction::serverMode, this);
	else
		f = bind(&Transaction::clientMode, this);
	sw = true;
	Global::addThreadd(f);
}

void Transaction::stop() {
	addLog("Stopping ");
	sw = false;
	while (life)
		Global::sleeps(1);
	shutdownInterface();
	if (formatBase) {
		delete formatBase;
		formatBase = NULL;
	}
	addLog(" is stopped");
}

void Transaction::restart() {
	stop();
	start();
}

