/*
 * Communication.h
 *
 *  Created on: Jun 6, 2018
 *      Author: jiasiang
 */

#ifndef OTHER_COMMUNICATION_H_
#define OTHER_COMMUNICATION_H_
#include "../lib/socket/Server.h"

class Communication {
private:
	Socket::Server server;
	void receivedProcess(Socket::Server *s, const char *buf, int len,
			const struct sockaddr*addr, const int *slen);
	bool verify(const char *, int len, char *);
public:
	void start();
	void stop();
};

#endif /* OTHER_COMMUNICATION_H_ */
