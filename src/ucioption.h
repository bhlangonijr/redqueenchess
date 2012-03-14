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
 * UciOption.h
 *
 *  Created on: Feb 9, 2009
 *      Author: bhlangonijr
 */

#ifndef UCIOPTION_H_
#define UCIOPTION_H_

class UciOption {
public:
	enum OptionType { STRING, SPIN, COMBO, CHECK, BUTTON };
	UciOption();
	UciOption(std::string, OptionType, std::string, std::string);
	UciOption(std::string, OptionType, std::string, std::string, int, int, std::string);
	virtual ~UciOption();
	std::string getName() const;
	OptionType getType() const;
	std::string getValue() const;
	std::string getDefaultValue() const;
	int getMinValue() const;
	int getMaxValue() const;
	std::string getComboValues() const;
	std::string toString() const;
	std::string toStr(int) const;
	void setName( std::string );
	void setType( OptionType );
	void setValue( std::string );
	void setDefaultValue( std::string );
	void setMinValue( int );
	void setMaxValue( int );
	void setComboValues( std::string );
private:
	std::string name;
	OptionType type;
	std::string value;
	std::string defaultValue;
	int minValue;
	int maxValue;
	std::string comboValues;

};

#endif /* UCIOPTION_H_ */
