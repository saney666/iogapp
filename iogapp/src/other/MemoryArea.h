/*
 * MemoryArea.h
 *
 *  Created on: May 2, 2018
 *      Author: jiasiang
 */

#ifndef OTHER_MEMORYAREA_H_
#define OTHER_MEMORYAREA_H_
#define MEMORYSIZE 65536

class MemoryArea {
private:
	static bool bool_r[MEMORYSIZE];
	static bool bool_rw[MEMORYSIZE];
	static short short_r[MEMORYSIZE];
	static short short_rw[MEMORYSIZE];

public:
	static bool getBool_R(unsigned short);
	static bool getBool_RW(unsigned short);
	static short getShort_R(unsigned short);
	static short getShort_RW(unsigned short);

	static void setBool_R(unsigned short, bool);
	static void setBool_RW(unsigned short, bool);
	static void setShort_R(unsigned short, short);
	static void setShort_RW(unsigned short, short);

};

#endif /* OTHER_MEMORYAREA_H_ */
