/*
 * Communication.cpp
 *
 *  Created on: Jun 6, 2018
 *      Author: jiasiang
 */

#include <sys/socket.h>
#include <functional>
#include "Communication.h"
#include "../Global.h"
#include <iostream>
#include "../lib/rapidjson/document.h"
#include "../lib/rapidjson/stream.h"
#include "../lib/rapidjson/stringbuffer.h"
#include "../lib/rapidjson/writer.h"

#define PORT 8080

using namespace std;

void Communication::start() {
	server.udpReceived = bind(&Communication::receivedProcess, this,
			placeholders::_1, placeholders::_2, placeholders::_3,
			placeholders::_4, placeholders::_5);

	server.startUdp(PORT);
}

void Communication::receivedProcess(Socket::Server* s, const char* buf, int len,
		const struct sockaddr* addr, const int* slen) {
	char message[1024];
	if (verify(buf, len, message)) {
		//string msg(message);
		//cout << msg << endl;

		if (Global::compareStr(message, "hi", 2)) {
			s->sendUdp("hello", 6, addr, slen);
		} else if (Global::compareStr(message, "version", 7)) {
			s->sendUdp("1.0", 3, addr, slen);
		} else
			s->sendUdp("what", 4, addr, slen);
	}
}

bool Communication::verify(const char*buf, int len, char *msg) {
	if (len > 2) {
		if (buf[0] == 'a' && buf[len - 1] == 'c') {
			Global::charcpy(msg, buf + 1, len - 2);
			msg[len - 2] = 0;
			return true;
		}
	}
	return false;
}

void Communication::stop() {
	server.stop();
}
