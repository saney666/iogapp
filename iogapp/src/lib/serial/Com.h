/*
 * Com.h
 *
 *  Created on: Apr 30, 2018
 *      Author: jiasiang
 */

#ifndef LIB_SERIAL_COM_H_
#define LIB_SERIAL_COM_H_

#include <string>
#include <map>
#include <termios.h>

using namespace std;

class Com {
private:
	int fd;
	string device;
	struct termios oldtio, newtio;
	map<int, int> baudrateList;

	void initialize();

	void setParity(int);
	void setDatabits(int);
	void setStopbits(int);
public:
	int port;
	int mode;
	int baud;
	int databits;
	int parity;
	int stopbits;
	int flow;
	Com();
	Com(int);
	bool openPort();
	bool openPort(int);
	void closePort();

	void setBaud(int);
	void setParameter(int, int, int);
	void setMode(int);
	void setFlowControl(int);

	void cleanPort();
	int sendCommand(const char *, int);
	int receiveData(char *, int);
	int receivableSize();

	void print();
};

#endif /* LIB_SERIAL_COM_H_ */
