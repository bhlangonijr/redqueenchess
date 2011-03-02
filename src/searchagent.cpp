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
extern "C" void *mainThreadRun(void *);
extern "C" void *workerThreadRun(void *);
SearchAgent* SearchAgent::searchAgent = 0;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waitCond2 = PTHREAD_COND_INITIALIZER;

SearchAgent* SearchAgent::getInstance () {
	if (searchAgent == 0) {
		searchAgent = new SearchAgent();
	}
	return searchAgent;
}

SearchAgent::SearchAgent() : searchMode(SEARCH_TIME), searchInProgress(false), requestStop(false), quit(false),
		hashSize(defaultHashSize),	threadNumber(1), whiteTime(0), whiteIncrement(0), blackTime(0),
		blackIncrement(0), depth(defaultDepth), movesToGo(0), moveTime(0), ponder(false), mainSearcher(0) {
	// creates initial hashtables
	createHash();
	initializeThreadPool(getThreadNumber());
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
	mainSearcher->cleanUp();
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
	pthread_mutex_lock(&threadPool[MAIN_THREAD].mutex);
	while (!quit) {
		pthread_cond_wait(&threadPool[MAIN_THREAD].waitCond, &threadPool[MAIN_THREAD].mutex);
		if (quit) {
			break;
		}
		setRequestStop(false);
		setSearchInProgress(true);
		newSearchHash();
		mainSearcher->setInfinite(getPonder());
		if (getSearchMode()==SearchAgent::SEARCH_DEPTH) {
			mainSearcher->setSearchFixedDepth(true);
			mainSearcher->setDepth(getDepth());
		} else if (getSearchMode()==SearchAgent::SEARCH_TIME ||
				getSearchMode()==SearchAgent::SEARCH_MOVETIME) {
			mainSearcher->setSearchFixedDepth(false);
			mainSearcher->setTimeToSearch(getTimeToSearch());
		} else if (getSearchMode()==SearchAgent::SEARCH_INFINITE) {
			mainSearcher->setSearchFixedDepth(false);
			mainSearcher->setInfinite(true);
			mainSearcher->setDepth(maxSearchDepth);
		}
		mainSearcher->search(board);
		setSearchInProgress(false);
		pthread_cond_signal(&waitCond2);
	}
	pthread_mutex_unlock(&threadPool[MAIN_THREAD].mutex);
	pthread_exit(NULL);
	return NULL;
}

// worker threads loop
void* SearchAgent::executeThread(const int threadId) {
	pthread_mutex_lock(&threadPool[threadId].mutex);
	while (!quit) {
		pthread_cond_wait(&threadPool[threadId].waitCond, &threadPool[threadId].mutex);
		if (quit) {
			break;
		}
	}
	pthread_mutex_unlock(&threadPool[threadId].mutex);
	pthread_exit(NULL);
	return NULL;
}

// start search
void SearchAgent::startSearch() {
	pthread_mutex_lock(&threadPool[MAIN_THREAD].mutex);
	setRequestStop(true);
	pthread_cond_signal(&threadPool[MAIN_THREAD].waitCond);
	pthread_mutex_unlock(&threadPool[MAIN_THREAD].mutex);
}

// start perft
void SearchAgent::doPerft() {
	std::cout << "info Executing perft..." << std::endl;
	int64_t nodes = mainSearcher->perft(board,this->getDepth(),1);
	std::cout << "info Finished perft: " << nodes << std::endl;
}

// bench test
void SearchAgent::doBench() {
	std::cout << "Executing benchmark..." << std::endl;
	int64_t time = 0;
	int64_t nodes = 0;
	mainSearcher->setUpdateUci(false);
	for (int i=0;i<benchSize;i++) {
		pthread_mutex_lock(&mutex2);
		std::cout << "Position[" << (i+1) << "]: " << benchPositions[i] << std::endl;
		newGame();
		setPositionFromFEN(benchPositions[i]);
		setSearchMode(SearchAgent::SEARCH_DEPTH);
		setDepth(benchDepth);
		startSearch();
		pthread_cond_wait(&waitCond2, &mutex2);
		time+=mainSearcher->getSearchedTime();
		nodes+=mainSearcher->getSearchedNodes();
		pthread_mutex_unlock(&mutex2);
	}
	mainSearcher->setUpdateUci(true);
	std::cout << std::endl << "Finished benchmark:  " << std::endl;
	std::cout << "Total time(seconds): " << (time/1000) << std::endl;
	std::cout << "Total nodes:         " << (nodes) << std::endl;
	std::cout << "Nodes/Seconds:       " << (nodes/(std::max(int64_t(1),time/1000))) << std::endl;
}
// eval test
void SearchAgent::doEval() {
	mainSearcher->getEvaluator().setDebugEnabled(true);
	mainSearcher->getEvaluator().evaluate(board,-maxScore,maxScore);
	mainSearcher->getEvaluator().setDebugEnabled(false);
}

// stop search
void SearchAgent::stopSearch() {
	if (getSearchInProgress()) {
		setRequestStop(true);
	}
}

const int64_t SearchAgent::getTimeToSearch(const int64_t usedTime) {
	if (getSearchMode()==SearchAgent::SEARCH_MOVETIME) {
		return getMoveTime();
	}
	int64_t time=board.getSideToMove()==WHITE? getWhiteTime():getBlackTime();
	int64_t incTime=board.getSideToMove()==WHITE?getWhiteIncrement():getBlackIncrement();
	int64_t movesLeft = defaultGameSize;
	time-=usedTime;
	if (movesToGo>0) {
		movesLeft = std::min(movesToGo,25);
		if (movesToGo==1) {
			time=time*60/100;
		} else {
			time=time*99/100;
		}
	} else {
		for (int x=0;x<timeTableSize;x++) {
			if (time<timeTable[x][1] && time >= timeTable[x][2]) {
				movesLeft=timeTable[x][0]+
						(maxGamePhase-board.getGamePhase())/2;
				break;
			}
		}
		time=time*97/100;
	}
	return time/movesLeft+incTime;
}

const int64_t SearchAgent::getTimeToSearch() {
	return getTimeToSearch(0);
}

int64_t SearchAgent::addExtraTime(const int iteration, int* iterationPVChange) {
	const int64_t timeThinking = mainSearcher->getTickCount()-mainSearcher->getStartTime();
	const int64_t weight = std::min(int64_t(90), int64_t(iterationPVChange[iteration]*15+
			iterationPVChange[iteration-1]*5));
	const int64_t newSearchTime = std::max(int64_t(10),mainSearcher->getTimeToSearch()-timeThinking) +
			getTimeToSearch(timeThinking)*weight/100;
	mainSearcher->setTimeToSearch(newSearchTime);
	return newSearchTime;
}

void  SearchAgent::ponderHit() {
	const int64_t timeThinking = mainSearcher->getTickCount()-mainSearcher->getStartTime();
	mainSearcher->setSearchFixedDepth(false);
	mainSearcher->setInfinite(false);
	mainSearcher->setTimeToSearch(getTimeToSearch()-std::max(int64_t(10),timeThinking));
	mainSearcher->setTimeToStop();
}

void SearchAgent::initThreads() {
	currentThread=MAIN_THREAD;
	int rCode = pthread_create( &(threadPool[MAIN_THREAD].executor), NULL, mainThreadRun, this);
	if (rCode) {
		std::cerr << "Failed to created main thread. pthread_create return code:  " << rCode << std::endl;
		exit(EXIT_FAILURE);
	}
	for (int i=1;i<threadPoolSize;i++) {
		currentThread=i;
		int rCode = pthread_create( &(threadPool[i].executor), NULL, workerThreadRun, this);
		if (rCode) {
			std::cerr << "Failed to created worker thread. pthread_create return code:  " << rCode << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

void *mainThreadRun(void *_object) {
	SearchAgent *object = (SearchAgent *)_object;
	void *threadResult = object->startThreadSearch();
	return threadResult;
}

void *workerThreadRun(void *_object) {
	SearchAgent *object = (SearchAgent *)_object;
	void *threadResult = object->executeThread(object->getCurrentThreadId());
	return threadResult;
}

void SearchAgent::initializeThreadPool(const int size) {
	for (int i=0;i<size;i++) {
		threadPool[i].threadId=i;
		threadPool[i].threadType=ThreadType(i);
		if (!threadPool[i].ss) {
			delete threadPool[i].ss;
		}
		threadPool[i].ss=new SimplePVSearch();
		threadPool[i].ss->setSearchAgent(this);

	}
	mainSearcher = threadPool[0].ss;
	threadPoolSize=size;
}

void SearchAgent::shutdown() {
	for (int i=0;i<threadPoolSize;i++) {
		if (!threadPool[i].ss) {
			delete threadPool[i].ss;
		}
	}
	destroyHash();
	setQuit(true);
}
