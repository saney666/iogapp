/*
 * Client.h
 *
 *  Created on: Nov 24, 2017
 *      Author: jiasiang
 */

#ifndef LIB_SOCKET_CLIENT_H_
#define LIB_SOCKET_CLIENT_H_

#include <functional>
#include <mutex>
#include <string>

namespace Socket {

class Client {
private:
	std::mutex lock;
	bool autoReceiving;
	unsigned int beginIndex, endIndex;
	unsigned char buffer[102400];
	std::function<void(Client*, int)> received;
	std::function<void(Client*)> disconnected;
	bool pingTo(std::string);
	void autoReceiveWork();
	void syncDisconnect();
	void setTimeout();

public:
	int tag;
	int id;
	std::string ip;
	int port;
	Client();
	Client(std::string, int);
	Client(int, std::string, int);
	virtual ~Client();

	bool connectTo(std::string, int);
	bool connectTo(const unsigned char*, int);
	bool connectTo();
	bool isConnected();
	int sendTo(std::string);
	int sendTo(const unsigned char*, int);
	int sendAll(std::string);
	int sendAll(const unsigned char*, int);
	int receivableSize();
	int receiveFrom(unsigned char*, int);
	void autoReceive(std::function<void(Client*, int)>,
			std::function<void(Client*)>);
	void stopAutoRecv();
	unsigned char* getReceivedData(unsigned int);
	unsigned int getReceiveDataSize();
	void disconnect();

	void clear();
};

} /* namespace Socket */

#endif /* LIB_SOCKET_CLIENT_H_ */
