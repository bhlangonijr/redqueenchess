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

/**
 * main.cpp
 * Redqueen Chess Engine
 *
 * 	Main source
 * 	Author: Ben-Hur Carlos Vieira Langoni Junior
 * 	e-mail: bhlangonijr@gmail.com
 */
#include <iostream>
#include <stdio.h>
#include "uci.h"
#include "constant.h"
#include "board.h"
#include "bitboard.h"
#include "searchagent.h"
#include "mersenne.h"
#include "stringutil.h"
#include "magicmoves.h"

int main(int argc, char* argv[]) {
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	std::cout.rdbuf()->pubsetbuf(NULL, 0);
	std::cin.rdbuf()->pubsetbuf(NULL, 0);
	// initialization methods
	initmagicmoves();
	initMersenne();
	initializeBitboards();
	Board::initialize();
	SearchAgent::getInstance();
	SimplePVSearch::initialize();
	Uci *uci = Uci::getInstance();

	// creating uci options
	std::vector< UciOption *> options;
	options.push_back(new UciOption("Hash",UciOption::SPIN,"128","128",1,16384,""));
	options.push_back(new UciOption("Threads",UciOption::SPIN,"1","1",1,32,""));
	options.push_back(new UciOption("Ponder",UciOption::CHECK,"true","true"));
	options.push_back(new UciOption("Positional_Evaluation_Weight",UciOption::SPIN,"100","100",1,200,""));
	options.push_back(new UciOption("Tactical_Evaluation_Weight",UciOption::SPIN,"100","100",1,200,""));
	options.push_back(new UciOption("Use_Custom_Engine_Settings?",UciOption::CHECK,"true","true"));
	options.push_back(new UciOption("Custom_Settings_File_Path",UciOption::STRING,"parameters.txt","parameters.txt"));
	options.push_back(new UciOption("Clear Hash",UciOption::BUTTON,"",""));
	// set options into uci handler
	uci->setUciOption(options);
	SearchAgent::getInstance()->setThreadNumber(1);
	SearchAgent::getInstance()->initializeThreadPool(SearchAgent::getInstance()->getThreadNumber());
	std::cout << Constant::ENGINE_COPYRIGHT << std::endl;
	if (argc<=1) {
		//uci loop
		Uci::Command command=Uci::NONE;
		while (command != Uci::QUIT) {
			command=uci->getUserInput();
			uci->execute();
		}
	} else if (StringUtil::containsString(std::string(argv[1]),"bench")) {
		uci->setRawInput("test "+std::string(argv[1]));
		uci->executeTest();
	} else {
		std::cout << " Usage: " << std::endl;
		std::cout << ">redqueen<enter> --> Enter in UCI mode " << std::endl;
		std::cout << ">redqueen bench<enter> --> Runs a benchmark" << std::endl;
	}
	uci->clearUciOption();
	return 0;
}
