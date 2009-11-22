/*
	Redqueen Chess Engine
    Copyright (C) 2008-2009 Ben-Hur Carlos Vieira Langoni Junior

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
/**
 * Uci.h
 * 	Singleton to Handle Universal Chess Interface
 *
 *  Created on: Feb 7, 2009
 *      Author: bhlangonijr
 */

#ifndef UCI_H_
#define UCI_H_

#include <vector>
#include "UciOption.h"

// Singleton to Handle Universal Chess Interface
class Uci {

public:

	// UCI commands enumeration
	enum Command {
		QUIT,
		UCI,
		UCINEWGAME,
		ISREADY,
		POSITION,
		SETOPTION,
		GO,
		STOP,
		TEST,
		UNKNOW
	};
	static Uci* getInstance();
	Command getUserInput();
	bool execute();

	void setCommand(Command cmd);
	Command getCommand() const;

	void setRawInput(const std::string input);
	std::string getRawInput() const;

	const UciOption getUciOption(std::string name);
	void setUciOption(std::vector< UciOption *>);
	size_t indexOfUciOption(std::string optionName);
	void clearUciOption();

	void info(const std::string text) const;
	void text(const std::string text) const;

protected:
	Uci();
	Uci(const Uci&);
	Uci& operator= (const Uci&);

private:
	static Uci* uci;
	Command command;
	std::string rawInput;
	std::vector< UciOption *> uciOption;

	void executeSetOption();
	void executePosition();
	void executeGo();
	void executeUciNewGame();
	void executeStop();
	void executeTest();

};

#endif /* UCI_H_ */

