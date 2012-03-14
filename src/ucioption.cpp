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
 * UciOption.cpp
 *
 *  Created on: Feb 9, 2009
 *      Author: bhlangonijr
 */
#include <string>
#include <sstream>
#include "stringutil.h"
#include "ucioption.h"

UciOption::UciOption() {

}

UciOption::UciOption(std::string name, OptionType type, std::string value, std::string defaultValue) {
	this->name = name;
	this->type = type;
	this->value = value;
	this->defaultValue = defaultValue;
	this->minValue = 0;
	this->maxValue = 0;
	this->comboValues = "";
}

UciOption::UciOption(std::string name, OptionType type, std::string value, std::string defaultValue,
		int minValue, int maxValue, std::string comboValues) {
	this->name = name;
	this->type = type;
	this->value = value;
	this->defaultValue = defaultValue;
	this->minValue = minValue;
	this->maxValue = maxValue;
	this->comboValues = comboValues;
}
// getters
std::string UciOption::getName() const {
	return name;
}
UciOption::OptionType UciOption::getType() const {
	return type;
}
std::string UciOption::getValue() const {
	return value;
}
std::string UciOption::getDefaultValue() const {
	return defaultValue;
}
int UciOption::getMinValue() const {
	return minValue;
}
int UciOption::getMaxValue() const {
	return maxValue;
}
std::string UciOption::getComboValues() const {
	return comboValues;
}
// setters
void UciOption::setName( std::string name ){
	this->name = name;
}
void UciOption::setType( OptionType type ){
	this->type = type;
}
void UciOption::setValue( std::string value ){
	this->value = value;
}
void UciOption::setDefaultValue( std::string defaultValue ){
	this->defaultValue = defaultValue;
}
void UciOption::setMinValue( int minValue ){
	this->minValue = minValue;
}
void UciOption::setMaxValue( int maxValue ){
	this->maxValue = maxValue;
}
void UciOption::setComboValues( std::string comboValues ){
	this->comboValues = comboValues;
}

std::string UciOption::toString() const {
	std::string result="option name "+this->name+" type ";
	switch (this->type) {
	case STRING:
		result += "string default " + this->defaultValue;
		break;
	case SPIN:
		result += "spin default " + this->defaultValue+" min " +
		StringUtil::toStr(this->minValue) + " max " +StringUtil::toStr(this->maxValue);
		break;
	case CHECK:
		result += "check default "+this->defaultValue;
		break;
	case COMBO:
		result += "combo default "+this->defaultValue + "  " + this->comboValues ;
		break;
	case BUTTON:
		result += "button ";
		break;
	default:
		result=" ";
		break;
	}
	return result;
}
// destructor
UciOption::~UciOption() {

}
