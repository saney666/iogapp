/*
 * Global.h
 *
 *  Created on: May 1, 2018
 *      Author: jiasiang
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_
#include <functional>
#include <thread>
#include <string>

enum IOG_TYPE {
	IOG_TYPE_UC8112, IOG_TYPE_DA662, IOG_TYPE_IOPAC, IOG_TYPE_ERROR
};
class Global {
private:
	static bool mainLife;
public:

	static IOG_TYPE getIogType();

	static void disableMe();
	static bool isDisabledMe();

	static void sleeps(unsigned int);
	static void sleepms(unsigned int);

	static void setFileMode(std::string);

	static bool compareStr(const char*, const char*, int);
	static bool compareStr(const char*, const char*);
	static bool compareStr(std::string, std::string);
	static bool compareStr(std::string, std::string, int);

	static void charcpy(char *, const char *, int);
	static void charcpy(char*, short);
	static void charClear(char *, int);
	static void charPrint(const char *, int);

	static void crcData(char *, int len);
	static unsigned short CRCMove(short);


	static std::string executeCMD(std::string);

	//class.member to function
	//bind(&class::member, &class)
	template<typename T>
	static void addThreadd(std::function<T> target) {
		std::thread t(target);
		t.detach();
	}
	template<typename T, typename U>
	static void addThreadd(std::function<T> target, U p1) {
		std::thread t(target, p1);
		t.detach();
	}
	template<typename T, typename U, typename V>
	static void addThreadd(std::function<T> target, U p1, V p2) {
		std::thread t(target, p1, p2);
		t.detach();
	}
	template<typename T, typename U, typename V, typename W>
	static void addThreadd(std::function<T> target, U p1, V p2, W p3) {
		std::thread t(target, p1, p2, p3);
		t.detach();
	}
};

#endif /* GLOBAL_H_ */
