/*
 * NetInterface.cpp
 *
 *  Created on: May 22, 2018
 *      Author: jiasiang
 */

#include "NetInterface.h"

#include <cstdio>

#include "../Global.h"
#include "../lib/Converter.h"
#include "../lib/sqlite/SqlRow.h"
#include "../lib/sqlite/SqlTable.h"

vector<Interface> NetInterface::network;

void NetInterface::initialize() {
	getConfig();
	initialDatabase();
}

void NetInterface::getConfig() {
	network.clear();
	string result = Global::executeCMD("cat /etc/network/interfaces");
	vector<string> line = Converter::strSplit(result, "\n");
	for (unsigned int i = 0; i < line.size(); i++) {
		if (Global::compareStr(line[i], "iface ", 5)) {
			vector<string> parts = Converter::strSplit(line[i], " ");
			Interface iface;
			iface.name = parts[1];

			if (Global::compareStr(iface.name, "lo"))
				continue;

			iface.type = parts[3];
			for (unsigned int j = 1; j <= 5; j++) {
				if (i + j >= line.size())
					continue;
				parts = Converter::strSplit(line[i + j], " ");
				if (Global::compareStr(parts[0], "address"))
					iface.address = parts[1];
				else if (Global::compareStr(parts[0], "netmask"))
					iface.netmask = parts[1];
				else if (Global::compareStr(parts[0], "gateway"))
					iface.gateway = parts[1];
				else if (Global::compareStr(parts[0], "broadcast"))
					iface.broadcast = parts[1];
				else if (Global::compareStr(parts[0], "network"))
					iface.network = parts[1];
			}
			iface.getMAC();
			network.push_back(iface);
		}
	}
}

void NetInterface::initialDatabase() {
	SqlTable table("lan");
	SqlRow row;
	if (table.rowCount() - 1 == network.size()) {
		for (unsigned int i = 0; i < network.size(); i++) {
			Interface iface = network[i];
			row = table.getRow(i + 1);
			row.setCell("name", iface.name);
			row.setCell("type", iface.type);
			row.setCell("address", iface.address);
			row.setCell("netmask", iface.netmask);
			row.setCell("gateway", iface.gateway);
			row.setCell("broadcast", iface.broadcast);
			row.setCell("network", iface.network);
			row.setCell("mac", iface.mac);
			table.setRow(row);
		}
	} else {
		table.removeAll();
		for (unsigned int i = 0; i < network.size(); i++) {
			Interface iface = network[i];
			row = table.getNewRow();
			row.setCell("name", iface.name);
			row.setCell("type", iface.type);
			row.setCell("address", iface.address);
			row.setCell("netmask", iface.netmask);
			row.setCell("gateway", iface.gateway);
			row.setCell("broadcast", iface.broadcast);
			row.setCell("network", iface.network);
			row.setCell("mac", iface.mac);
			table.addRow(row);
		}
	}
}

void NetInterface::reconfigure() {
	updateConfig();
	setConfig();
	reboot();
}

void NetInterface::updateConfig() {
	SqlTable table("lan");
	SqlRow row;
	Interface iface;
	if (table.rowCount() - 1 == network.size()) {
		for (unsigned int i = 1; i < table.rowCount(); i++) {
			row = table.getRow(i);
			iface.address = row.getCell("address");
			iface.netmask = row.getCell("netmask");
			iface.gateway = row.getCell("gateway");
			iface.broadcast = row.getCell("broadcast");
			iface.network = row.getCell("network");
			iface.type = row.getCell("type");
			iface.name = row.getCell("name");
			iface.flag = row.getCellToInt("setflag");
			row.setCell("setflag", "0");
			table.setRow(row);
			for (unsigned int j = 0; j < network.size(); j++) {
				if (Global::compareStr(iface.name, network[j].name)) {
					network[j].replace(iface);
					break;
				}
			}
		}
	}
}

void NetInterface::setConfig() {
	FILE *fp = fopen("/etc/network/interfaces", "w");
	string head = "auto ";
	string body = "iface lo inet loopback\n";
	for (unsigned int i = 0; i < network.size(); i++) {
		head += network[i].name + " ";
		body += network[i].getInfo();
	}
	head += "lo\n";
	fwrite(head.c_str(), 1, head.length(), fp);
	fwrite(body.c_str(), 1, body.length(), fp);
	fclose(fp);
}

void NetInterface::reboot() {
	for (unsigned int i = 0; i < network.size(); i++) {
		if (network[i].flag == 1) {
			Global::executeCMD("ifdown " + network[i].name);
			Global::executeCMD("ifup " + network[i].name);
		}
	}
}

void Interface::print() {
	cout << name << "  " << type << endl;
	cout << "address " << address << endl;
	cout << "netmask " << netmask << endl;
	cout << "gateway " << gateway << endl;
	cout << "broadcast " << broadcast << endl;
	cout << "network " << network << endl;
	cout << "mac " << mac << endl;
	cout << "flag " << flag << endl;
}

void Interface::getMAC() {
	string result = Global::executeCMD("ifconfig " + name + " | grep HWaddr");
	vector<string> parts = Converter::strSplit(result, " ");
	mac = Converter::stoupper(parts[4]);
}

string Interface::getInfo() {
	string result = "iface " + name + " inet " + type + "\n";
	result += "address " + address + "\n";
	result += "netmask " + netmask + "\n";
	//result += "gateway " + gateway + "\n";
	result += "broadcast " + broadcast + "\n";
	result += "network " + network + "\n";
	return result;
}

void Interface::replace(Interface interface) {
	address = interface.address;
	netmask = interface.netmask;
	gateway = interface.gateway;
	broadcast = interface.broadcast;
	network = interface.network;
	flag = interface.flag;
}
