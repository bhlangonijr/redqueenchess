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
/*
 * SearchAgent.cpp
 *
 *  Created on: 01/05/2009
 *      Author: bhlangonijr
 */

#include "SearchAgent.h"


SearchAgent* SearchAgent::searchAgent = 0;

SearchAgent* SearchAgent::getInstance ()
{
	if (searchAgent == 0)
	{
		searchAgent = new SearchAgent();
	}
	return searchAgent;
}

SearchAgent::SearchAgent() :
	searchMode(SEARCH_TIME), hashSize(64*1024*1024/sizeof(HashData)), threadNumber(1), whiteTime(0), whiteIncrement(0), blackTime(0),
	blackIncrement(0), depth(5), movesToGo(0), moveTime(0), infinite(false), searchInProgress(false)
{

	transTable = TranspositionTable(hashSize);
}

// start a new game
void SearchAgent::newGame() {
	board.setInitialPosition();
	this->clearHash();

	this->setSearchMode(SEARCH_TIME);
	this->setWhiteTime(0);
	this->setWhiteIncrement(0);
	this->setBlackTime(0);
	this->setBlackIncrement(0);

	this->setDepth(0);
	this->setMovesToGo(0);
	this->setMoveTime(0);
	this->setInfinite(false);
	this->setSearchInProgress(false);


}

// clear hash table
void SearchAgent::clearHash() {

	transTable.clear();

}

// get board
const Board SearchAgent::getBoard() const {
	return board;
}

// set board
void SearchAgent::setBoard(Board _board) {
	board = _board;
}

// set position from SAN moves
void SearchAgent::setPositionFromSAN(std::string startPosMoves) {
	board.loadFromString(startPosMoves);
}

// set position from FEN moves
void SearchAgent::setPositionFromFEN(std::string fenMoves) {
	// TODO create loadFromFEN in class Board
}

// start search
void SearchAgent::startSearch() {

	if (getSearchInProgress()) {
		Uci::getInstance()->text("Search in progress...");
		return;
	}

	setSearchInProgress(true);

	clearHash();

	Board tmp(board);

	SimplePVSearch search(tmp, getDepth());

	search.search();

	//std::cout << "score " << search.getScore()<<std::endl;


}

// stop search
void SearchAgent::stopSearch() {
	setSearchInProgress(false);
}











