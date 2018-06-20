/*
 * Global.cpp
 *
 *  Created on: May 1, 2018
 *      Author: jiasiang
 */

#include "Global.h"

#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>

bool Global::mainLife = true;

void Global::disableMe() {
	mainLife = false;
}

bool Global::isDisabledMe() {
	return mainLife;
}

void Global::sleeps(unsigned int value) {
	sleep(value);
}

void Global::sleepms(unsigned int value) {
	usleep(value * 1000);
}

void Global::setFileMode(std::string path) {
	chmod(path.c_str(), 0777);
}

bool Global::compareStr(const char *str1, const char *str2, int len) {
	if (memcmp(str1, str2, len) == 0)
		return true;
	return false;
}

bool Global::compareStr(const char *str1, const char *str2) {
	if (strcmp(str1, str2) == 0)
		return true;
	return false;
}

bool Global::compareStr(std::string str1, std::string str2) {
	return compareStr(str1.c_str(), str2.c_str());
}

bool Global::compareStr(std::string str1, std::string str2, int len) {
	return compareStr(str1.c_str(), str2.c_str(), len);
}

void Global::charcpy(char *buf, const char *target, int len) {
	memcpy(buf, target, len);
}

void Global::charcpy(char *buf, short value) {
	buf[0] = (value & 0xff00) >> 8;
	buf[1] = value & 0xff;
}

void Global::charClear(char *buf, int len) {
	memset(buf, 0, len);
}

void Global::charPrint(const char *target, int len) {
	static int index = 0;
	printf("print--------%d\n", index++);
	printf("byte:\n");
	for (int i = 0; i < len; i++) {
		printf("%03d ", target[i]);
		if ((i + 1) % 5 == 0)
			printf("\n");
	}
	printf("\n");
	printf("hex:\n");
	for (int i = 0; i < len; i++) {
		printf("%0X ", target[i]);
		if ((i + 1) % 5 == 0)
			printf("\n");
	}
	printf("\n");
	printf("ascii:\n");
	for (int i = 0; i < len; i++) {
		if (target[i] > 32 && target[i] < 127)
			printf("%c", target[i]);
		else
			printf("(.)");
		if ((i + 1) % 5 == 0)
			printf("\n");
	}
	printf("\n");
}

void Global::crcData(char *res, int len) {
	short x;
	unsigned short crc = 0xffff;
	for (x = 0; x < len; x++) {
		crc ^= res[x];
		crc = CRCMove(crc);
	}
	res[len + 1] = (crc & 0XFF00) / 0x100;
	res[len] = crc & 0xff;
}

unsigned short Global::CRCMove(short crc) {
	unsigned short x = crc;
	short y;
	for (y = 1 ; y <=8; y++)
	{
		if ((x & 0x1) == 1)
		{
			x >>=1;
			x^= 0xA001;
		}
		else
			x >>= 1;
	}
	return x;
}

std::string Global::executeCMD(std::string cmd) {
	std::cout << "execute command:" << cmd << std::endl;
	FILE *fp = NULL;
	char result[2048];
	int index;
	fp = popen(cmd.c_str(), "r");

	if (fp) {
		fseek(fp, SEEK_SET, 0);
		index = fread(result, 1, 2048, fp);
		if (index > 0 && index < 2048) {
			result[index] = 0;
		}

		fclose(fp);
		return std::string(result);
	}
	return "";
}



IOG_TYPE Global::getIogType() {
#if defined(TYPE_DA662)
	return IOG_TYPE_DA662;
#elif defined(TYPE_UC8112)
	return IOG_TYPE_UC8112;
#elif defined(TYPE_IOPAC)
	return IOG_TYPE_IOPAC;
#else
	return IOG_TYPE_ERROR;
#endif
}
