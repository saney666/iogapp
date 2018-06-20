/*
 * Server.h
 *
 *  Created on: Nov 24, 2017
 *      Author: jiasiang
 */

#ifndef LIB_SOCKET_SERVER_H_
#define LIB_SOCKET_SERVER_H_

#include <functional>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Client.h"

namespace Socket {

class Server {
	bool processAcceptLife;
	void processAcceptWork();
	void processReceiveWork();

public:
	std::function<void(Server*, Client*)> accepted;
	std::function<void(Server*, const char*, int,const struct sockaddr*, int *)> udpReceived;
	int id;
	int port;

	Server();
	virtual ~Server();

	bool isListen();

	void startListen(int);
	void stop();

	void startUdp(int);
	int sendUdp(const char *, int,const struct sockaddr*, const int *);
};

} /* namespace socket */

#endif /* LIB_SOCKET_SERVER_H_ */
