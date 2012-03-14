/*
	Redqueen Chess Engine
    Copyright (C) 2008-2012 Ben-Hur Carlos Vieira Langoni Junior

    This file is part of Redqueen Chess Engine.

    Redqueen is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Redqueen is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Redqueen.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * StringUtil.cpp
 *
 *  Created on: Feb 12, 2009
 *      Author: bhlangonijr
 */
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include "stringutil.h"

namespace StringUtil {

// int to str
const std::string toStr(const int value) {
	std::stringstream out;
	out << value;
	return out.str();
};
// str to int
const int toInt(const std::string value) {
	return atoi(value.c_str());
};
// str to long
const long toLong(const std::string value) {
	return atol(value.c_str());
};
// str to int
const int toInt(const char value) {
	return (int)(value-48);
};
// normalize - remove extra space
void normalizeString(std::string &str) {
	std::string tmp="";
	for(unsigned int x=0;x<str.length();x++){
		if (!((str[x]==str[x+1])&&str[x]==' ')) {
			tmp+=str[x];
		}
	}
	str=tmp;
};
// get the middle string
std::string getMiddleString(const std::string source, const std::string before, const std::string after) {
	std::string result="";
	size_t posFirst=source.find(before);
	size_t posLast=source.find(after,posFirst);
	try {
		if (posFirst!=std::string::npos&&posFirst!=std::string::npos){
			result=source.substr(posFirst+before.length(),posLast-(posFirst+before.length()));
		}
	} catch (std::exception &e) {
		std::cout << "Exception(StringUtil.cpp): " << e.what() << std::endl;
	}
	return result;
};
// get the middle string
std::string getMiddleString(const std::string source, const std::string before) {
	std::string result="";
	size_t posFirst=source.find(before);
	try {
		if (posFirst!=std::string::npos){
			result=source.substr(posFirst+before.length(),source.length()-(posFirst+before.length()));
		}
	} catch (std::exception &e) {
		std::cout << "Exception(StringUtil.cpp): " << e.what() << std::endl;
	}
	return result;
};

const bool containsString(const std::string source, const std::string _substring) {
	return source.find(_substring)!=std::string::npos;
}

}
