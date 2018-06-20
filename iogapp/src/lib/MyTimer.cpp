/*
 * MyTimer.cpp
 *
 *  Created on: Nov 6, 2017
 *      Author: jiasiang
 */

#include "MyTimer.h"
#include <time.h>
#include <stdlib.h>

MyTimer::MyTimer() {
	limit = 0;
	timeStamp = 0;
	isStart = false;
}

MyTimer::MyTimer(int interval) {
	timeStamp = 0;
	isStart = false;
	this->limit = interval;
}

void MyTimer::setLimit(int interval) {
	this->limit = interval;
}

bool MyTimer::check() {
	if (!isStart)
		return false;
	int tmp = time((time_t*) NULL);
	int r = tmp - timeStamp;
	if (abs(r) > limit)
		return true;
	return false;
}

void MyTimer::start() {
	timeStamp = time((time_t*) NULL);
	isStart = true;
}

void MyTimer::stop() {
	isStart = false;
}
