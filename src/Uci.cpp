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
 * Uci.cpp
 *
 *  Created on: Feb 7, 2009
 *      Author: bhlangonijr
 */
#include <iostream>
#include <string>
#include <vector>
//#include <boost/regex.hpp>
#include <exception>

#include "Uci.h"
#include "StringUtil.h"
#include "Constant.h"
#include "Board.h"

Uci* Uci::uci = 0;

Uci* Uci::getInstance ()
{
	if (uci == 0)
	{
		uci = new Uci();
	}
	return uci;
}

Uci::Uci()
{

}

// wait to get user input
Command Uci::getUserInput()
{
	Command result=UNKNOW;
	std::string input;
	std::string first;

	if (!std::getline(std::cin, input)) {
		result=QUIT;
	}

	StringUtil::normalizeString(input);

	this->setRawInput(input);
	first = input.substr(0,input.find(" "));

	if (first=="uci") {
		result=UCI;
	} else if (first=="ucinewgame"){
		result=UCINEWGAME;
	}else if (first=="isready"){
		result=ISREADY;
	}else if (first=="position"){
		result=POSITION;
	}else if (first=="setoption"){
		result=SETOPTION;
	}else if (first=="go"){
		result=GO;
	}else if (first=="test"){
			result=TEST;
		}
	else if (first=="quit"){
		result=QUIT;
	}

	command = result;
	return result;
}

// execute a command
bool Uci::execute()
{
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
	case ISREADY :
		std::cout << "readyok" << std::endl;
		break;
	case POSITION :
		executePosition();
		break;
	case SETOPTION :
		executeSetOption();
		break;
	case GO :
		executeGo();
		break;
	case STOP :
		executeStop();
		break;
	case TEST :
		executeTest();
		break;
	default:
		std::cout << "Unknown command(" << this->rawInput << "). " << std::endl;
		break;

	}

	return true;
}

// sets command
void Uci::setCommand(Command cmd)
{
	Uci::command = cmd;
}

// gets command
Command Uci::getCommand() const
{
	return Uci::command;
}

// sets rawinput
void Uci::setRawInput(const std::string input)
{
	Uci::rawInput = input;
}

// gets command
std::string Uci::getRawInput() const
{
	return Uci::rawInput;
}

// sets uci options
void Uci::setUciOption(std::vector< UciOption *> options) {
	Uci::uciOption=options;
}

// gets uci options
std::vector< UciOption *> Uci::getUciOption() const {
	return uciOption;
}

// returns the index number of a given uci option by its name - if not found returns -1
size_t Uci::indexOfUciOption(std::string optionName){
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
		optionName=StringUtil::getMiddleString(this->rawInput,"setoption name "," value");
		optionValue=StringUtil::getMiddleString(this->rawInput,"value ");

		for(size_t i=0;i<this->uciOption.size();i++){
			if (uciOption[i]->getName()==optionName) {
				uciOption[i]->setValue(optionValue);
				//std::cout <<"New Value to Option " << uciOption[i]->getName() << " value " << uciOption[i]->getValue() << std::endl;
			}
		}
	} catch (std::exception &e) {
		std::cout << "Exception(Uci.cpp): " << e.what() << std::endl;
	}

}

// execute go uci command
void Uci::executeGo() {

}

// execute position uci command
void Uci::executePosition() {

}

// execute position uci command
void Uci::executeUciNewGame() {

}

// execute Stop
void Uci::executeStop() {

}

// execute Test
void Uci::executeTest() {
	//test
	Board* board = new Board();
	MoveBackup backup;
	//board2->printBoard();

	std::cout << "teste -----------------------------" << std::endl;

	//board->setInitialPosition();

	board->doMove(Move(D2,D4,EMPTY),backup);

	Board* board2 = new Board(*board);

	//"d2d4 g8f6 c2c4 e7e6 g1f3 b7b6 b1c3 c8b7 a2a3 d7d5 c4d5 f6d5 e2e3 f8e7 f1b5 c7c6 b5d3 d5c3 b2c3 e8g8"
	//"e2e4 c7c5 g1f3 a7a6 c2c3 d7d5 e4d5 d8d5 d2d4 g8f6 f1e2 g7g6 e1g1 f8g7 c3c4 d5d6 d4c5 d6c5 b1c3 e8g8 c1e3 c5a5 h2h3 b8c6"
	board->loadFromString("g1f3 d7d5 d2d4 g8f6 e2e3 c8g4 c2c4 d5c4 f1c4 e7e6 b1c3 a7a6 e1g1 c7c5 d4c5 d8d1 f1d1 g4f3 g2f3 f8c5 f3f4 c5b4 a2a3 b4a5 b2b4 a5b6 d1d6 b6a7 a3a4 e8e7 d6d3 f6h5 c1a3 h8c8 b4b5 a7c5 a3c5 c8c5 d3d4 a6b5 c3b5 b8c6 d4e4 h5f6 b5c7 f6e4 c7a8 c5c4 a4a5 c4c2 a5a6 c2f2 a6b7 f2b2 a1c1 c6a5 c1c7 e7f6 b7b8q b2b1 b8b1");

	board->printBoard();

	board2->printBoard();

	Board* board3 = new Board();

	board3->loadFromString("g1f3 d7d5 d2d4 g8f6 e2e3 c8g4 c2c4 d5c4 f1c4 e7e6 b1c3 a7a6 e1g1 c7c5");
	board3->printBoard();

	delete board;
	delete board2;
	delete board3;

}

void Uci::info(const std::string text) const {
	std::cout << "info " << text << std::endl;
}
