/*
 * Converter.cpp
 *
 *  Created on: Apr 30, 2018
 *      Author: jiasiang
 */

#include "Converter.h"
#include <algorithm>

short Converter::chartoshort(const char *buf) {
	return buf[0] * 256 + buf[1];
}

void Converter::shorttochar(char *buf, short value) {
	buf[0] = (value & 0xff00) >> 8;
	buf[1] = value & 0xff;
}

string Converter::itos(int value) {
	string result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return "";
}

string Converter::uitos(unsigned int value) {
	string result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return "";
}

int Converter::stoi(string value) {
	int result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return 0;
}

string Converter::ftos(float value) {
	string result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return "";
}

string Converter::dtos(double value) {
	string result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return "";
}

unsigned int Converter::stoui(string value) {
	unsigned int result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return 0;
}

float Converter::stof(string value) {
	float result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return 0;
}

string Converter::ltos(long value) {
	string result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return "";
}

double Converter::stod(string value) {
	double result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return 0;
}

long Converter::stol(string value) {
	long result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return 0;
}

string Converter::stoupper(string value) {
	string result = value;
	transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}

string Converter::stolower(string value) {
	string result = value;
	transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

vector<string> Converter::strSplit(string s, string seperator) {
	vector<string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	while (i != s.size()) {
		int flag = 0;
		while (i != s.size() && flag == 0) {
			flag = 1;
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[i] == seperator[x]) {
					++i;
					flag = 0;
					break;
				}
		}

		flag = 0;
		string_size j = i;
		while (j != s.size() && flag == 0) {
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[j] == seperator[x]) {
					flag = 1;
					break;
				}
			if (flag == 0)
				++j;
		}
		if (i != j) {
			result.push_back(s.substr(i, j - i));
			i = j;
		}
	}
	return result;
}

