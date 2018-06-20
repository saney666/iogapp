/*
 * MyTimer.h
 *
 *  Created on: Nov 6, 2017
 *      Author: jiasiang
 */

#ifndef LIB_MYTIMER_H_
#define LIB_MYTIMER_H_

class MyTimer {
private:
	int limit;
	int timeStamp;
	bool isStart;
public:
	MyTimer();
	MyTimer(int);
	void setLimit(int);
	bool check();
	void start();
	void stop();
};

#endif /* LIB_MYTIMER_H_ */
