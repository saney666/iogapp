/*
 * Converter.h
 *
 *  Created on: Apr 30, 2018
 *      Author: jiasiang
 */

#ifndef LIB_CONVERTER_H_
#define LIB_CONVERTER_H_
#include <string>
#include <sstream>
#include <vector>

using namespace std;

class Converter {
public:
	static vector<string> strSplit(string, string);

	static string stoupper(string);
	static string stolower(string);

	static short chartoshort(const char*);
	static void shorttochar(char*, short);

	static string itos(int);
	static string uitos(unsigned int);
	static string ftos(float);
	static string dtos(double);
	static string ltos(long);

	template<typename T>
	static string toString(T);

	static int stoi(string);
	static unsigned int stoui(string);
	static float stof(string);
	static double stod(string);
	static long stol(string);

	template<typename T>
	static T toType(string);
};

template<typename T>
inline string Converter::toString(T value) {
	string result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return "";
}

template<typename T>
inline T Converter::toType(string value) {
	T result;
	stringstream ss;
	ss << value;
	ss >> result;
	if (ss)
		return result;
	return 0;
}

#endif /* LIB_CONVERTER_H_ */
