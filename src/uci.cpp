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
 * Uci.cpp
 *
 *  Created on: Feb 7, 2009
 *      Author: bhlangonijr
 */
#include "uci.h"

using namespace StringUtil;

Uci* Uci::uci = 0;

Uci* Uci::getInstance () {
	if (uci == 0) {
		uci = new Uci();
	}
	return uci;
}

Uci::Uci() : command(UNKNOW) {

}
// wait to get user input
Uci::Command Uci::getUserInput() {
	command=UNKNOW;
	std::string input="";
	if (!std::getline(std::cin, input)) {
		command=QUIT;
	}
	normalizeString(input);
	this->setRawInput(input);
	input = input.substr(0,input.find(" "));
	for(int x=0;x<NU_COMMANDS;x++) {
		if (input==strCommand[x]) {
			command=Command(x);
		}
	}
	return command;
}

// execute a command
bool Uci::execute() {
	switch (this->getCommand()) {
	case UCI :
		std::cout << "id name " << Constant::ENGINE_NAME << " " << Constant::ENGINE_VERSION << std::endl;
		std::cout << "id author " << Constant::ENGINE_AUTHOR << std::endl;
		//print options
		for(size_t i=0;i<this->uciOption.size();i++){
			std::cout << uciOption[i]->toString() << std::endl;
		}
		std::cout << "uciok" << std::endl;
		break;
	case UCINEWGAME :
		executeUciNewGame();
		break;
	case ISREADY : {
		std::cout << "readyok" << std::endl;
		break;
	}
	case POSITION :
		executePosition();
		break;
	case SETOPTION :
		executeSetOption();
		break;
	case GO :
		executeGo();
		break;
	case QUIT:
		executeStop();
		break;
	case STOP:
		executeStop();
		break;
	case PERFT:
		executePerft();
		break;
	case PONDERHIT:
		executePonderHit();
		break;
	case TEST :
		executeTest();
		break;
	case NONE:
		//do nothing
		break;
	default:
		std::cout << "Unknown command(" << this->rawInput << "). " << std::endl;
		break;
	}
	return true;
}

// sets command
void Uci::setCommand(Command cmd) {
	Uci::command = cmd;
}
// gets command
Uci::Command Uci::getCommand() const {
	return Uci::command;
}
// sets rawinput
void Uci::setRawInput(const std::string input) {
	Uci::rawInput = input;
}
// gets command
std::string Uci::getRawInput() const {
	return Uci::rawInput;
}
// sets uci options
void Uci::setUciOption(std::vector< UciOption *> options) {
	Uci::uciOption=options;
}
// gets uci option
const UciOption Uci::getUciOption(std::string name) {
	return *uciOption[this->indexOfUciOption(name)];
}
// returns the index number of a given uci option by its name - if not found returns -1
size_t Uci::indexOfUciOption(std::string optionName) {
	for(size_t i=0;i<this->uciOption.size();i++){
		if (uciOption[i]->getName()==optionName) {
			return i;
		}
	}
	return -1;
}
// clear uci option set
void Uci::clearUciOption() {
	for(size_t i=0;i<this->uciOption.size();i++){
		delete uciOption[i];
	}
	uciOption.clear();
}
// execute setoption uci command
void Uci::executeSetOption() {
	std::string optionName;
	std::string optionValue;
	try {
		optionName=getMiddleString(this->rawInput,"setoption name "," value");
		optionValue=getMiddleString(this->rawInput,"value ");
		for(size_t i=0;i<this->uciOption.size();i++){
			if (uciOption[i]->getName()==optionName) {
				uciOption[i]->setValue(optionValue);
			}
		}
	} catch (std::exception &e) {
		std::cout << "Exception(Uci.cpp): " << e.what() << std::endl;
	}

	if (optionName=="Clear Hash") {
		SearchAgent::getInstance()->clearHash();
	} else if (optionName=="Threads") {
		SearchAgent::getInstance()->setThreadNumber(toInt(this->getUciOption("Threads").getValue()));
		SearchAgent::getInstance()->initializeThreadPool(SearchAgent::getInstance()->getThreadNumber());
	} else if (optionName=="Hash") {
		SearchAgent::getInstance()->setHashSize(toInt(this->getUciOption("Hash").getValue()));
		SearchAgent::getInstance()->destroyHash();
		SearchAgent::getInstance()->createHash();
	} else if (optionName=="Positional_Evaluation_Weight") {
		for(int i=0;i<SearchAgent::getInstance()->getThreadPoolSize();i++) {
			SearchAgent::getInstance()->getSearcher(i)->getEvaluator().
					setPositionalWeight(toInt(this->getUciOption("Positional_Evaluation_Weight").getValue()));
		}
	} else if (optionName=="Tactical_Evaluation_Weight") {
		for(int i=0;i<SearchAgent::getInstance()->getThreadPoolSize();i++) {
			SearchAgent::getInstance()->getSearcher(i)->getEvaluator().
					setTacticalWeight(toInt(this->getUciOption("Tactical_Evaluation_Weight").getValue()));
		}
	}  else if (optionName=="Use_Custom_Engine_Settings?") {
		bool useSettings = this->getUciOption("Use_Custom_Engine_Settings?").getValue()=="true"?true:false;
		setUseSettings(useSettings);
	} else if (optionName=="Custom_Settings_File_Path") {
		std::string settings = this->getUciOption("Custom_Settings_File_Path").getValue();
		setSettingsFile(settings);
		readParameters(settings);
	}
}
// execute go uci command
void Uci::executeGo() {
	SearchAgent *searchAgent = SearchAgent::getInstance();
	searchAgent->clearParameters();

	searchAgent->setSearchMode(SearchAgent::SEARCH_NONE);
	if (containsString(this->rawInput, "wtime")) {
		searchAgent->setSearchMode(SearchAgent::SEARCH_TIME);
		searchAgent->setWhiteTime(toLong(getMiddleString(this->rawInput,"wtime "," ")));
		searchAgent->setWhiteIncrement(toLong(getMiddleString(this->rawInput,"winc "," ")));
		searchAgent->setBlackTime(toLong(getMiddleString(this->rawInput,"btime "," ")));
		searchAgent->setBlackIncrement(toLong(getMiddleString(this->rawInput,"binc ")));
	} else if (containsString(this->rawInput, "go depth")) {
		searchAgent->setSearchMode(SearchAgent::SEARCH_DEPTH);
		searchAgent->setDepth(toInt(getMiddleString(this->rawInput,"go depth ")));
	} else if (containsString(this->rawInput, "go movetime")) {
		searchAgent->setSearchMode(SearchAgent::SEARCH_MOVETIME);
		searchAgent->setMoveTime(toLong(getMiddleString(this->rawInput,"go movetime ")));
	} else if (containsString(this->rawInput, "go infinite")) {
		searchAgent->setSearchMode(SearchAgent::SEARCH_INFINITE);
	} else if (containsString(this->rawInput, "go ponder")) {
		searchAgent->setSearchMode(SearchAgent::SEARCH_INFINITE);
	}
	if (containsString(this->rawInput, "movestogo ")) {
		searchAgent->setMovesToGo(toInt(getMiddleString(this->rawInput,"movestogo ")));
	}
	if (containsString(this->rawInput, "searchmoves ")) {
		searchAgent->setSearchMoves(getMiddleString(this->rawInput,"searchmoves "));
	}
	if (containsString(this->rawInput, "nodes ")) {
		if (searchAgent->getSearchMode()==SearchAgent::SEARCH_NONE) {
			searchAgent->setSearchMode(SearchAgent::SEARCH_INFINITE);
		}
		searchAgent->setSearchNodes(toLong(getMiddleString(this->rawInput,"nodes ")));
	}
	searchAgent->setPonder(containsString(this->rawInput, " ponder"));
	searchAgent->startSearch();
}
// execute position uci command
void Uci::executePosition() {
	SearchAgent *searchAgent = SearchAgent::getInstance();
	if (containsString(this->rawInput,"position startpos moves ")) {
		std::string startPosMoves = getMiddleString(this->rawInput,"position startpos moves ");
		searchAgent->setPositionFromSAN(startPosMoves,true);
	} else if (containsString(this->rawInput,"position fen ")) {
		std::string startPosMoves = getMiddleString(this->rawInput,"position fen ");
		searchAgent->setPositionFromFEN(startPosMoves);
		if (containsString(this->rawInput," moves ")) {
			startPosMoves = getMiddleString(this->rawInput," moves ");
			searchAgent->setPositionFromSAN(startPosMoves,false);
		}
	}
}
// execute position uci command
void Uci::executeUciNewGame() {
	SearchAgent *searchAgent = SearchAgent::getInstance();
	searchAgent->newGame();
}
// execute Stop
void Uci::executeQuit() {
	SearchAgent *searchAgent = SearchAgent::getInstance();
	searchAgent->stopSearch();
	searchAgent->shutdown();
}
// execute Stop
void Uci::executeStop() {
	SearchAgent *searchAgent = SearchAgent::getInstance();
	searchAgent->stopSearch();
}
// execute ponderhit
void Uci::executePonderHit() {
	SearchAgent *searchAgent = SearchAgent::getInstance();
	searchAgent->ponderHit();
}
// execute perft
void Uci::executePerft() {
	SearchAgent *searchAgent = SearchAgent::getInstance();
	if (containsString(this->rawInput, "perft")) {
		searchAgent->setSearchMode(SearchAgent::SEARCH_DEPTH);
		searchAgent->setDepth(toInt(getMiddleString(this->rawInput,"perft ")));
	}
	searchAgent->doPerft();
}
// execute Test
void Uci::executeTest() {
	SearchAgent *searchAgent = SearchAgent::getInstance();

	if (containsString(this->rawInput,"test bench")) {
		searchAgent->doBench();
	} else if (containsString(this->rawInput,"test eval")) {
		searchAgent->doEval();
	}
}
// uci info
void Uci::info(const std::string text) const {
	std::cout << "info " << text << std::endl;
}
// output text
void Uci::text(const std::string text) const {
	std::cout << text << std::endl;
}
