/*
 * Com.cpp
 *
 *  Created on: Apr 30, 2018
 *      Author: jiasiang
 */

#include "Com.h"
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../Converter.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#if defined(TYPE_DA662)
#include "moxadevice.h"
#endif

Com::Com() {
	initialize();
}

Com::Com(int port) {
	initialize();
	this->port = port;
}

bool Com::openPort() {
	struct termios tio;
	device = "/dev/ttyM" + Converter::itos(port - 1);
	fd = open(device.c_str(), O_RDWR | O_NOCTTY);
	if (fd < 0)
		return false;

	bzero(&tio, sizeof(tio)); ///< clear struct for new port settings

	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
	tio.c_lflag = 0;

	tio.c_cc[VTIME] = 0; ///< inter-unsigned character timer unused
	tio.c_cc[VMIN] = 1;  ///< blocking read until 1 unsigned character arrives

	tcgetattr(fd, &oldtio); ///< save current serial port settings
	newtio = tio;

	tcsetattr(fd, TCSANOW, &newtio);
	return true;
}
bool Com::openPort(int port) {
	this->port = port;
	return openPort();
}

void Com::closePort() {
	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);
}

void Com::setBaud(int value) {
	int baudrate = baudrateList[value];
	cfsetispeed(&newtio, baudrate);
	cfsetospeed(&newtio, baudrate);
	tcsetattr(fd, TCSANOW, &newtio);
}

void Com::setParameter(int parity, int databits, int stopbits) {
	setParity(parity);
	setDatabits(databits);
	setStopbits(stopbits);
	tcsetattr(fd, TCSANOW, &newtio);
}

void Com::setMode(int mode) {
#if defined(TYPE_DA662)
	ioctl(fd, MOXA_SET_OP_MODE, &mode);
#elif defined(TYPE_UC8112)
	char command[50];
	sprintf(command, "setinterface %s %d", device.c_str(), mode);
	system(command);
#elif defined(TYPE_IOPAC)
	cout << "IOPAC" << endl;
#else
	cout << "None" << endl;
#endif
this->mode = mode;
}
void Com::setFlowControl(int flow) {
	switch (flow) {
	case 0: //None
		newtio.c_cflag &= ~CRTSCTS;
		newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
		break;
	case 1: //FlowControl::HW:
		newtio.c_cflag |= CRTSCTS;
		break;
	case 2: //FlowControl::SW:
		newtio.c_iflag |= (IXON | IXOFF | IXANY);
		break;
	}
	tcsetattr(fd, TCSANOW, &newtio);
	this->flow = flow;

}

void Com::cleanPort() {
	tcflush(fd, TCIFLUSH);
}

int Com::sendCommand(const char*buf, int len) {
	return write(fd, buf, len);
}

int Com::receiveData(char*buf, int len) {
	fcntl(fd, F_SETFL, FNDELAY);
	return read(fd, buf, len);
}

int Com::receivableSize() {
	int result = 0;
	ioctl(fd, FIONREAD, &result);
	return result;
}

void Com::initialize() {
	device = "";
	fd = -1;
	port = 0;
	mode = 0;
	baud = 9600;
	databits = 8;
	parity = 0;
	stopbits = 1;
	flow = 0;

	baudrateList[0] = B0;
	baudrateList[50] = B50;
	baudrateList[75] = B75;
	baudrateList[110] = B110;
	baudrateList[134] = B134;
	baudrateList[150] = B150;
	baudrateList[200] = B200;
	baudrateList[300] = B300;
	baudrateList[600] = B600;
	baudrateList[1200] = B1200;
	baudrateList[1800] = B1800;
	baudrateList[2400] = B2400;
	baudrateList[4800] = B4800;
	baudrateList[9600] = B9600;
	baudrateList[19200] = B19200;
	baudrateList[38400] = B38400;
	baudrateList[57600] = B57600;
	baudrateList[115200] = B115200;
	baudrateList[230400] = B230400;
	baudrateList[460800] = B460800;
	baudrateList[500000] = B500000;
	baudrateList[576000] = B576000;
	baudrateList[921600] = B921600;
}

void Com::setParity(int parity) {
	if (parity == 0) {
		newtio.c_cflag &= ~PARENB;
		newtio.c_iflag &= ~INPCK;
	} else if (parity == 1) {
		newtio.c_cflag |= PARODD;
		newtio.c_cflag |= PARENB;
	} else if (parity == 2) {
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
	} else if (parity == 3) {
		newtio.c_cflag |= PARENB | CMSPAR;
		newtio.c_cflag &= ~PARODD;
	} else if (parity == 4) {
		newtio.c_cflag |= PARENB | CMSPAR | PARODD;
	}
	this->parity = parity;
}

void Com::setDatabits(int databits) {
	newtio.c_cflag &= ~CSIZE;
	if (databits == 5)
		newtio.c_cflag |= CS5;
	else if (databits == 6)
		newtio.c_cflag |= CS6;
	else if (databits == 7)
		newtio.c_cflag |= CS7;
	else if (databits == 8)
		newtio.c_cflag |= CS8;
	this->databits = databits;
}

void Com::setStopbits(int stopbits) {
	if (stopbits == 1)
		newtio.c_cflag &= ~CSTOPB;
	else if (stopbits == 2)
		newtio.c_cflag |= CSTOPB;
	this->stopbits = stopbits;
}

void Com::print() {
	cout << "com:" << port;
	cout << " mode:" << mode;
	cout << " baud:" << baud;
	cout << " databits:" << databits;
	cout << " parity:" << parity;
	cout << " stopbits:" << stopbits;
	cout << " flow:" << flow << endl;
}
