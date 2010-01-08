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
 * main.cpp
 * Redqueen Chess Engine
 *
 * 	Main source
 * 	Author: Ben-Hur Carlos Vieira Langoni Junior
 * 	e-mail: bhlangonijr@yahoo.com.br
 */
#include <iostream>
#include <stdio.h>

#include "Uci.h"
#include "Constant.h"
#include "Board.h"
#include "Bitboard.h"
#include "SearchAgent.h"
#include "mersenne.h"
#include "Inline.h"
#include "StringUtil.h"

int main() {

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	std::cout.rdbuf()->pubsetbuf(NULL, 0);
	std::cin.rdbuf()->pubsetbuf(NULL, 0);

	// initialization methods
	init_mersenne();
	Board::initializeZobrist();
	SearchAgent::getInstance();
	initializeBitboards();

	Uci *uci = Uci::getInstance();

	std::string paramNumProcs = StringUtil::toStr(getNumProcs());

	// creating uci options
	std::vector< UciOption *> options;
	options.push_back(new UciOption("Hash",UciOption::SPIN,"64","64",1,4096,""));
	options.push_back(new UciOption("Threads",UciOption::SPIN,paramNumProcs,paramNumProcs,1,8,""));
	//options.push_back(new UciOption("Ponder",UciOption::CHECK,"false","false"));
	options.push_back(new UciOption("Clear Hash",UciOption::BUTTON,"",""));
	// set options into uci handler
	uci->setUciOption(options);
	std::cout << Constant::ENGINE_COPYRIGHT << std::endl;

	//uci loop
	Uci::Command command=Uci::NONE;
	while (command != Uci::QUIT) {
		command=uci->getUserInput();
		uci->execute();
	}

	uci->clearUciOption();
	SearchAgent::getInstance()->destroyHash();

	return 0;
}
