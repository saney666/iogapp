/*
 * NetInterface.h
 *
 *  Created on: May 22, 2018
 *      Author: jiasiang
 */

#ifndef OTHER_NETINTERFACE_H_
#define OTHER_NETINTERFACE_H_
#include <string>
#include <vector>
#include <iostream>
using namespace std;

class Interface {
public:
	string name;
	string type;
	string address;
	string netmask;
	string gateway;
	string broadcast;
	string network;
	string mac;
	int flag;

	void print();
	void getMAC();
	string getInfo();
	void replace(Interface);
};

class NetInterface {
private:
	static vector<Interface> network;
	static void getConfig();
	static void initialDatabase();

	static void updateConfig();
	static void setConfig();
	static void reboot();
public:
	static void initialize();
	static void reconfigure();
};

#endif /* OTHER_NETINTERFACE_H_ */
