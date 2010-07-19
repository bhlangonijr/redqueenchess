/*
	Redqueen Chess Engine
    Copyright (C) 2008-2010 Ben-Hur Carlos Vieira Langoni Junior

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
 * 	e-mail: bhlangonijr@gmail.com.br
 */
#include <iostream>
#include <stdio.h>

#include "uci.h"
#include "constant.h"
#include "board.h"
#include "bitboard.h"
#include "searchagent.h"
#include "mersenne.h"
#include "inline.h"
#include "stringutil.h"
#include "magicmoves.h"

int main() {

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	std::cout.rdbuf()->pubsetbuf(NULL, 0);
	std::cin.rdbuf()->pubsetbuf(NULL, 0);

	// initialization methods
	initmagicmoves();
	init_mersenne();
	initializeBitboards();
	Board::initializeZobrist();
	SearchAgent::getInstance();
	Uci *uci = Uci::getInstance();

	std::string paramNumProcs = StringUtil::toStr(getNumProcs());

	// creating uci options
	std::vector< UciOption *> options;
	options.push_back(new UciOption("Hash",UciOption::SPIN,"128","128",1,4096,""));
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
	SearchAgent::getInstance()->shutdown();

	return 0;
}
