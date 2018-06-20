/*
 * MemoryArea.cpp
 *
 *  Created on: May 2, 2018
 *      Author: jiasiang
 */

#include "MemoryArea.h"
#include <iostream>

using namespace std;

bool MemoryArea::bool_r[MEMORYSIZE];
bool MemoryArea::bool_rw[MEMORYSIZE];
short MemoryArea::short_r[MEMORYSIZE];
short MemoryArea::short_rw[MEMORYSIZE];

bool MemoryArea::getBool_R(unsigned short address) {
	return bool_r[address];
}

bool MemoryArea::getBool_RW(unsigned short address) {
	return bool_rw[address];
}

short MemoryArea::getShort_R(unsigned short address) {
	return short_r[address];
}

short MemoryArea::getShort_RW(unsigned short address) {
	return short_rw[address];
}

void MemoryArea::setBool_R(unsigned short address, bool value) {
	bool_r[address] = value;
}

void MemoryArea::setBool_RW(unsigned short address, bool value) {
	bool_rw[address] = value;
}

void MemoryArea::setShort_R(unsigned short address, short value) {
	short_r[address] = value;
}

void MemoryArea::setShort_RW(unsigned short address, short value) {
	short_rw[address] = value;
}
