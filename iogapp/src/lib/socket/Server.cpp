/*
 * Server.cpp
 *
 *  Created on: Nov 24, 2017
 *      Author: jiasiang
 */

#include "Server.h"
#include <iostream>

#include <unistd.h>
#include <thread>
#include <string.h>
#include <string>
#include "../../Global.h"

namespace Socket {

void Server::processAcceptWork() {
	int newId;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	while (id >= 0) {
		newId = accept(id, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
		if (newId >= 0) {
			Client *c = new Client(newId,
					std::string(inet_ntoa(clnt_addr.sin_addr)),
					ntohs(clnt_addr.sin_port));
			if (accepted)
				accepted(this, c);
			else
				delete c;
		}
	}
	processAcceptLife = false;
}

Server::Server() {
	id = -1;
	port = 0;
	accepted = NULL;
	processAcceptLife = false;
	udpReceived = NULL;
}

Server::~Server() {
	stop();
}

bool Server::isListen() {
	if (id >= 0)
		return true;
	return false;
}

void Server::startListen(int port) {
	if (processAcceptLife)
		return;
	this->port = port;
	id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct timeval timeout;
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;

	setsockopt(id, SOL_SOCKET, SO_RCVTIMEO, (unsigned char *) &timeout,
			sizeof(timeout));
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(id, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		std::cout << port << " bind fail." << std::endl;
		stop();
	} else {
		listen(id, 1);
		processAcceptLife = true;

		std::function<void()> f = std::bind(&Server::processAcceptWork, this);
		Global::addThreadd(f);
	}
}

void Server::stop() {
	if (id == -1)
		return;
	shutdown(id, SHUT_RDWR);
	close(id);
	id = -1;
	while (processAcceptLife)
		sleep(1);
}

void Server::processReceiveWork() {
	int recv_len;
	char buf[1024];
	struct sockaddr_in si_other;
	int slen = sizeof(si_other);
	while (id >= 0) {
		if ((recv_len = recvfrom(id, buf, 1024, 0,
				(struct sockaddr *) &si_other, (socklen_t*) &slen)) <= 0)
			continue;
		if (udpReceived)
			udpReceived(this, buf, recv_len, (struct sockaddr *) &si_other, &slen);
	}
}

void Server::startUdp(int port) {
	this->port = port;
	id = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	struct timeval timeout;
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;

	setsockopt(id, SOL_SOCKET, SO_RCVTIMEO, (unsigned char *) &timeout,
			sizeof(timeout));
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(id, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		std::cout << port << " bind fail." << std::endl;
		stop();
	} else {
		std::function<void()> f = std::bind(&Server::processReceiveWork, this);
		Global::addThreadd(f);
	}
}

int Server::sendUdp(const char*buf, int len, const struct sockaddr*addr, const int *slen) {

	if (isListen()) {
		return sendto(id, buf, len, 0, addr, *slen);
	}
	return 0;
}

} /* namespace socket */
