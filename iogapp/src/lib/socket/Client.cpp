/*
 * Client.cpp
 *
 *  Created on: Nov 24, 2017
 *      Author: jiasiang
 */

#include "Client.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "../../Global.h"

using namespace std;
namespace Socket {

bool Client::pingTo(std::string ip) {
//	std::string command = "ping -i 0.1 -c 2 ";
//	command.append(ip);
//	if (system(command.c_str()) != 0)
//		return false;
	return true;
}

void Client::autoReceiveWork() {
	unsigned char buffer[10240];
	int size = 0;
	//cout << "autoReceiveWork:" << ip << ":" << port << endl;
	while (id >= 0) {
		size = receiveFrom(buffer, sizeof(buffer));
		if (size > 0) {
			std::lock_guard<std::mutex> mLock(lock);
			if (getReceiveDataSize() + size < sizeof(this->buffer)) {
				for (int i = 0; i < size; i++) {
					this->buffer[endIndex++] = buffer[i];
					if (endIndex >= sizeof(this->buffer))
						endIndex = 0;
				}
				if (received) {
					Global::addThreadd<void(Client*, int), Client*, unsigned int>(
							received, this, getReceiveDataSize());

				}
			}
		} else
			break;
	}
	//cout << "End autoReceiveWork:" << ip << ":" << port << endl;
	autoReceiving = false;
	if (size <= 0)
		syncDisconnect();
}
void Client::syncDisconnect() {
	Global::addThreadd<void(Client*)>(disconnected, this);
}

void Client::setTimeout() {
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	setsockopt(id, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	int mode = 1;
	ioctl(id, FIONBIO, &mode);
}

Client::Client() {
	tag = -1;
	id = -1;
	ip = "";
	port = 0;
	autoReceiving = false;
	received = NULL;
	beginIndex = 0;
	endIndex = 0;
	disconnected = NULL;
}

Client::Client(std::string ip, int port) {
	tag = -1;
	autoReceiving = false;
	received = NULL;
	beginIndex = 0;
	endIndex = 0;
	disconnected = NULL;
	this->id = -1;
	this->ip = ip;
	this->port = port;
}

Client::Client(int id, std::string ip, int port) {
	tag = -1;
	autoReceiving = false;
	received = NULL;
	beginIndex = 0;
	endIndex = 0;
	disconnected = NULL;
	this->id = id;
	this->ip = ip;
	this->port = port;
	setTimeout();
}

Client::~Client() {
	disconnect();
}

bool Client::connectTo(std::string ip, int port) {
	this->ip = ip;
	this->port = port;
	if (!pingTo(ip))
		return false;
	if(id >= 0)
		disconnect();
	id = socket(AF_INET, SOCK_STREAM, 0);
	setTimeout();
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	serv_addr.sin_port = htons(port);
	if (connect(id, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		return false;
	return true;
}

bool Client::connectTo(const unsigned char* ip, int port) {
	return connectTo(std::string((char*) ip), port);
}

bool Client::connectTo() {
	return connectTo(ip, port);
}

bool Client::isConnected() {
	if (id < 0)
		return false;
	return true;
}

int Client::sendTo(std::string value) {
	return sendTo((unsigned char*) value.c_str(), value.size());
}

int Client::sendTo(const unsigned char* buffer, int length) {
	if (id >= 0)
		return send(id, buffer, length, MSG_NOSIGNAL);
	return -1;
}

int Client::sendAll(std::string value) {
	return sendAll((unsigned char*) value.c_str(), value.length());
}
int Client::sendAll(const unsigned char* buffer, int length) {
	int size;
	int total = 0;
	do {
		size = sendTo(&buffer[total], length - total);
		if (size > 0)
			total += size;
		else
			break;
	} while (total < length);
	if (size <= 0)
		disconnect();
	return 0;
}

int Client::receivableSize() {
	if (id >= 0) {
		int result = 0;
		ioctl(id, FIONREAD, &result);
		return result;
	}
	return 0;
}

int Client::receiveFrom(unsigned char* buffer, int length) {
	if (id >= 0)
		return recv(id, buffer, length, MSG_NOSIGNAL);
	return 0;
}

void Client::autoReceive(std::function<void(Client*, int)> receivedHandler,
		std::function<void(Client*)> disconnectHandler) {
	received = receivedHandler;
	disconnected = disconnectHandler;
	autoReceiving = true;
	function<void()> f = bind(&Client::autoReceiving, this);
	Global::addThreadd(f);
}

void Client::stopAutoRecv() {
	if (!autoReceiving)
		return;
	int iid = id;
	id = -1;
	while (autoReceiving)
		sleep(1);
	id = iid;
}

unsigned char* Client::getReceivedData(unsigned int size) {
	std::lock_guard<std::mutex> mLock(lock);
	if (size <= getReceiveDataSize()) {
		unsigned char *result = new unsigned char[size];
		for (unsigned int i = 0; i < size; i++) {
			result[i] = buffer[beginIndex++];
			if (beginIndex >= sizeof(buffer))
				beginIndex = 0;
		}
		return result;
	}
	return NULL;
}

unsigned int Client::getReceiveDataSize() {
	if (beginIndex <= endIndex)
		return endIndex - beginIndex;
	else
		return sizeof(buffer) + endIndex - beginIndex;
}

void Client::disconnect() {
	if (id < 0)
		return;
	//cout << "disconnect:" << ip << ":" << port << endl;
	int iid = id;
	id = -1;
	shutdown(iid, SHUT_RDWR);
	close(iid);
	if (autoReceiving)
		stopAutoRecv();
	if (disconnected)
		disconnected(this);
	//cout << "end disconnect" << endl;
}

void Client::clear() {
	if (id < 0)
		return;
	int size = receivableSize();
	if (size > 0) {
		char *buf = new char[size];
		receiveFrom((unsigned char*) buf, size);
		delete[] buf;
	}
}

} /* namespace Socket */
