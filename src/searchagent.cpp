/*
	Redqueen Chess Engine
    Copyright (C) 2008-2011 Ben-Hur Carlos Vieira Langoni Junior

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

#include "searchagent.h"
extern "C" void *threadRun(void *);
SearchAgent* SearchAgent::searchAgent = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waitCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t waitCond2 = PTHREAD_COND_INITIALIZER;
pthread_t executor;

SearchAgent* SearchAgent::getInstance () {
	if (searchAgent == 0) {
		searchAgent = new SearchAgent();
	}
	return searchAgent;
}

SimplePVSearch simpleSearcher;

SearchAgent::SearchAgent() : searchMode(SEARCH_TIME), searchInProgress(false), requestStop(false), quit(false),
		hashSize(defaultHashSize),	threadNumber(1), whiteTime(0), whiteIncrement(0), blackTime(0),
		blackIncrement(0), depth(defaultDepth), movesToGo(0), moveTime(0), ponder(false) {
	// creates initial hashtables
	createHash();
	simpleSearcher.setSearchAgent(this);
	initThreads();
}

// start a new game
void SearchAgent::newGame() {
	board.setInitialPosition();
	clearHash();
	setSearchMode(SEARCH_TIME);
	setWhiteTime(0);
	setWhiteIncrement(0);
	setBlackTime(0);
	setBlackIncrement(0);
	setDepth(defaultDepth);
	setMovesToGo(0);
	setMoveTime(0);
	setPonder(false);
	simpleSearcher.cleanUp();
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
	board.loadFromFEN(fenMoves);
}

// start search
void* SearchAgent::startThreadSearch() {
	pthread_mutex_lock(&mutex);
	while (!quit) {
		pthread_cond_wait(&waitCond, &mutex);
		if (quit) {
			break;
		}
		setRequestStop(false);
		setSearchInProgress(true);
		newSearchHash();
		simpleSearcher.setInfinite(getPonder());
		if (getSearchMode()==SearchAgent::SEARCH_DEPTH) {
			simpleSearcher.setSearchFixedDepth(true);
			simpleSearcher.setDepth(getDepth());
		} else if (getSearchMode()==SearchAgent::SEARCH_TIME ||
				getSearchMode()==SearchAgent::SEARCH_MOVETIME) {
			simpleSearcher.setSearchFixedDepth(false);
			simpleSearcher.setTimeToSearch(getTimeToSearch());
		} else if (getSearchMode()==SearchAgent::SEARCH_INFINITE) {
			simpleSearcher.setSearchFixedDepth(false);
			simpleSearcher.setInfinite(true);
			simpleSearcher.setDepth(maxSearchDepth);
		}
		simpleSearcher.search(board);
		setSearchInProgress(false);
		pthread_cond_signal(&waitCond2);
	}
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
	return 0;
}

// start search
void SearchAgent::startSearch() {
	pthread_mutex_lock(&mutex);
	setRequestStop(true);
	pthread_cond_signal(&waitCond);
	pthread_mutex_unlock(&mutex);
}

// start perft
void SearchAgent::doPerft() {
	std::cout << "info Executing perft..." << std::endl;
	long nodes = simpleSearcher.perft(board,this->getDepth(),1);
	std::cout << "info Finished perft: " << nodes << std::endl;
}

// bench test
void SearchAgent::doBench() {
	std::cout << "Executing benchmark..." << std::endl;
	long time = 0;
	long nodes = 0;
	simpleSearcher.setUpdateUci(false);
	for (int i=0;i<benchSize;i++) {
		pthread_mutex_lock(&mutex2);
		std::cout << "Position[" << (i+1) << "]: " << benchPositions[i] << std::endl;
		newGame();
		setPositionFromFEN(benchPositions[i]);
		setSearchMode(SearchAgent::SEARCH_DEPTH);
		setDepth(benchDepth);
		startSearch();
		pthread_cond_wait(&waitCond2, &mutex2);
		time+=simpleSearcher.getSearchedTime();
		nodes+=simpleSearcher.getSearchedNodes();
		pthread_mutex_unlock(&mutex2);
	}
	simpleSearcher.setUpdateUci(true);
	std::cout << std::endl << "Finished benchmark:  " << std::endl;
	std::cout << "Total time(seconds): " << (time/1000) << std::endl;
	std::cout << "Total nodes:         " << (nodes) << std::endl;
	std::cout << "Nodes/Seconds:       " << (nodes/(std::max(1L,time/1000))) << std::endl;
}
// eval test
void SearchAgent::doEval() {
	simpleSearcher.getEvaluator().setDebugEnabled(true);
	simpleSearcher.getEvaluator().evaluate(board,-maxScore,maxScore);
	simpleSearcher.getEvaluator().setDebugEnabled(false);
}

// stop search
void SearchAgent::stopSearch() {
	if (getSearchInProgress()) {
		setRequestStop(true);
	}
}

const long SearchAgent::getTimeToSearch() {
	if (getSearchMode()==SearchAgent::SEARCH_MOVETIME) {
		return getMoveTime();
	}
	long time=board.getSideToMove()==WHITE? getWhiteTime():getBlackTime();
	long incTime=board.getSideToMove()==WHITE?getWhiteIncrement():getBlackIncrement();
	int movesLeft = defaultGameSize;
	if (movesToGo>0) {
		movesLeft = movesToGo;
		if (movesToGo<2) {
			time=(time*95)/100;
		}
	} else {

		for (int x=0;x<timeTableSize;x++) {
			if (time<timeTable[x][1] && time >= timeTable[x][2]) {
				movesLeft=timeTable[x][0]+
						(maxGamePhase-board.getGamePhase())/2;
				break;
			}
		}

	}
	return time/(static_cast<long>(movesLeft))+incTime;
}

void  SearchAgent::ponderHit() {
	const long timeThinking = simpleSearcher.getTickCount()-simpleSearcher.getStartTime();
	simpleSearcher.setSearchFixedDepth(false);
	simpleSearcher.setInfinite(false);
	simpleSearcher.setTimeToSearch(getTimeToSearch()-std::max(10L,timeThinking));
	simpleSearcher.setTimeToStop();
}

void SearchAgent::initThreads() {
	int rCode = pthread_create( &executor, NULL, threadRun, this);
	if (rCode) {
		std::cerr << "Failed to created new thread. pthread_create return code:  " << rCode << std::endl;
		exit(EXIT_FAILURE);
	}
}

void *threadRun(void *_object) {
	SearchAgent *object = (SearchAgent *)_object;
	void *threadResult = object->startThreadSearch();
	return threadResult;
}

void SearchAgent::shutdown() {
	destroyHash();
	setQuit(true);
}
