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
 * StringUtil.h
 *
 *  Created on: Feb 12, 2009
 *      Author: bhlangonijr
 */

#ifndef STRINGUTIL_H_
#define STRINGUTIL_H_

namespace StringUtil {

const std::string toStr(int value);
const int toInt(const std::string value);
const long toLong(const std::string value);
const int toInt(const char value);
void normalizeString(std::string &str);
std::string getMiddleString(const std::string, const std::string, const std::string);
std::string getMiddleString(const std::string, const std::string);
const bool containsString(const std::string source, const std::string _substring);

}

#endif /* STRINGUTIL_H_ */
