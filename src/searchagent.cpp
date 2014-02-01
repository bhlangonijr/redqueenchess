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
 * SearchAgent.cpp
 *
 *  Created on: 01/05/2009
 *      Author: bhlangonijr
 */

#include "searchagent.h"
extern "C" void *mainThreadRun(void *);
extern "C" void *workerThreadRun(void *);
SearchAgent* SearchAgent::searchAgent = 0;
pthread_mutex_t SearchAgent::mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t SearchAgent::waitCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t SearchAgent::mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t SearchAgent::waitCond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t SearchAgent::mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t SearchAgent::waitCond2 = PTHREAD_COND_INITIALIZER;

SearchAgent* SearchAgent::getInstance () {
	if (searchAgent == 0) {
		searchAgent = new SearchAgent();
	}
	return searchAgent;
}

SearchAgent::SearchAgent() : searchMode(SEARCH_TIME), searchInProgress(false), requestStop(false), quit(false),
		hashSize(defaultHashSize),	threadNumber(1), whiteTime(0), whiteIncrement(0), blackTime(0),
		blackIncrement(0), depth(defaultDepth), movesToGo(0), searchNodes(0), searchMoves("none"), moveTime(0), ponder(false),
		freeThreads(0), threadShouldWait(false) {
	memset(mainSearcher,0,sizeof(SimplePVSearch*)*maxThreads);
	// creates initial hashtables
	createHash();
}

// start a new game
void SearchAgent::newGame() {
	board.setInitialPosition();
	clearHash();
	setSearchMode(SEARCH_TIME);
	getSearcher(MAIN_THREAD)->cleanUp();
}

void SearchAgent::clearParameters() {
	setWhiteTime(0);
	setWhiteIncrement(0);
	setBlackTime(0);
	setBlackIncrement(0);
	setDepth(defaultDepth);
	setMovesToGo(0);
	setSearchNodes(0);
	setSearchMoves("none");
	setMoveTime(0);
	setPonder(false);
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
void SearchAgent::setPositionFromSAN(std::string startPosMoves, const bool startPos) {
	if (startPos) {
		board.setInitialPosition();
	}
	board.loadFromString(startPosMoves);
}

// set position from FEN moves
void SearchAgent::setPositionFromFEN(std::string fenMoves) {
	board.loadFromFEN(fenMoves);
}

// start search
void* SearchAgent::startThreadSearch() {
	lock(&threadPool[MAIN_THREAD].mutex);
	while (!quit) {
		wait(&threadPool[MAIN_THREAD].waitCond, &threadPool[MAIN_THREAD].mutex);
		if (quit) {
			break;
		}
		threadPool[MAIN_THREAD].status=THREAD_STATUS_WORKING;
		freeThreads--;
		getSearcher(MAIN_THREAD)->setThreadId(MAIN_THREAD);
		setThreadsShouldWait(false);
		awakeWaitingThreads();
		setRequestStop(false);
		setSearchInProgress(true);
		newSearchHash();
		clearHistory();
		getSearcher(MAIN_THREAD)->setInfinite(getPonder());
		if (getSearchMode()==SearchAgent::SEARCH_DEPTH) {
			getSearcher(MAIN_THREAD)->setSearchFixedDepth(true);
			getSearcher(MAIN_THREAD)->setDepth(getDepth());
		} else if (getSearchMode()==SearchAgent::SEARCH_TIME ||
				getSearchMode()==SearchAgent::SEARCH_MOVETIME) {
			getSearcher(MAIN_THREAD)->setSearchFixedDepth(false);
			getSearcher(MAIN_THREAD)->setTimeToSearch(getTimeToSearch());
		} else if (getSearchMode()==SearchAgent::SEARCH_INFINITE) {
			getSearcher(MAIN_THREAD)->setSearchFixedDepth(false);
			getSearcher(MAIN_THREAD)->setInfinite(true);
			getSearcher(MAIN_THREAD)->setDepth(maxSearchDepth);
		}
		prepareThreadPool();
		getSearcher(MAIN_THREAD)->search(board);
		setSearchInProgress(false);
		setThreadsShouldWait(true);
		wakeUp(&waitCond2);
		threadPool[MAIN_THREAD].status=THREAD_STATUS_AVAILABLE;
	}
	unlock(&threadPool[MAIN_THREAD].mutex);
	pthread_exit(NULL);
	return NULL;
}
/*static int64_t count = 0;
static int64_t sum = 0;*/
// worker threads loop
void* SearchAgent::executeThread(const int threadId, SplitPoint* sp) {
	//int64_t init = getTickCount();
	while (!quit) {
		SearchThread& thread = threadPool[threadId];
		if (getThreadsShouldWait()) {
			if (sp!=NULL) {
				sp->shouldStop=true;
				return NULL;
			}
			lock(&thread.mutex);
			thread.status = THREAD_STATUS_WAITING;
			wait(&thread.waitCond, &thread.mutex);
			if (thread.status == THREAD_STATUS_WAITING) {
				thread.status = THREAD_STATUS_AVAILABLE;
			}
			unlock(&thread.mutex);
		}
		if (quit) {
			break;
		}
		SplitPoint* splitPoint = sp==NULL?thread.splitPoint:sp;
		if (splitPoint==NULL) {
			continue;
		}
		if (thread.status==THREAD_STATUS_WORK_ASSIGNED) {
			thread.status = THREAD_STATUS_WORKING;
		}
		SimplePVSearch* searchMaster = getSearcher(splitPoint->masterThreadId);
		SimplePVSearch* searchSlave = getSearcher(threadId);
		if (thread.status==THREAD_STATUS_WORKING &&
				(sp==NULL || (sp!=NULL && !sp->masterDone))) {
			Board board(splitPoint->board);
			searchMaster->smpPVSearch(board,searchMaster,searchSlave,splitPoint);
			lock(&mutex1);
			if (sp==NULL) {
				splitPoint->workers--;
				splitPoint->nodes+=searchSlave->getSearchedNodes();
				if (thread.spNumber <= 0) {
					resetThread(threadId);
					searchSlave->resetStats();
					freeThreads++;
				}
			} else {
				sp->masterDone = true;
				//init = getTickCount();
			}
			unlock(&mutex1);
		}
		if (sp!=NULL && sp->workers<=0 && sp->masterDone) {
			//int64_t r = (getTickCount()-init);
			searchMaster->updateSearchedNodes(splitPoint->nodes);
			/*if (r > 0) {
				count++; sum += r;
				std::cout << "master waited: " << r << " / count = " << count << " / sum = " << sum <<  std::endl;
			}*/
			return NULL;
		}
	}
	pthread_exit(NULL);
	return NULL;
}
// Spawn a new search thread
const bool SearchAgent::spawnThreads(Board& board, void* data, const int currentThreadId,
		MoveIterator* moves, MoveIterator::Move* move, MoveIterator::Move* hashMove, int* bestScore,
		int* currentAlpha, int* currentScore, int* moveCounter, bool* nmMateScore) {
	lock(&mutex1);
	if (currentThreadId > 0 || getFreeThreads()<1 || getRequestStop() ||
			threadPool[currentThreadId].spNumber >= 1) {
		unlock(&mutex1);
		return false;
	}
	int threadId=-1;
	SearchTypes::SearchInfo* si = static_cast<SearchTypes::SearchInfo*>(data);
	SplitPoint* sp = &splitPoint[currentThreadId][threadPool[currentThreadId].spNumber++];
	sp->clear();
	sp->alpha = si->alpha;
	sp->beta = si->beta;
	sp->depth = si->depth;
	sp->ply = si->ply;
	sp->isPV = si->nodeType==PV_NODE;
	sp->allowNullMove = si->allowNullMove;
	sp->partialSearch = si->partialSearch;
	sp->moves = moves;
	sp->move = si->move;
	sp->board = board;
	sp->bestMove=move;
	sp->bestScore=bestScore;
	sp->currentAlpha=currentAlpha;
	sp->currentScore=currentScore;
	sp->moveCounter=moveCounter;
	sp->nmMateScore=nmMateScore;
	sp->hashMove=hashMove;
	sp->masterThreadId = currentThreadId;
	for(int i=1;i<threadPoolSize;i++) {
		SearchThread& thread = threadPool[i];
		threadId=i;
		SimplePVSearch* searchSlave = getSearcher(threadId);
		if (thread.status==THREAD_STATUS_AVAILABLE &&
				i!=currentThreadId && thread.spNumber <= 0) {
			searchSlave->resetStats();
			searchSlave->setSearchFixedDepth(getSearcher(MAIN_THREAD)->isSearchFixedDepth());
			searchSlave->setTimeToSearch(getSearcher(MAIN_THREAD)->getTimeToSearch());
			searchSlave->setInfinite(getSearcher(MAIN_THREAD)->isInfinite());
			searchSlave->setDepth(getSearcher(MAIN_THREAD)->getDepth());
			searchSlave->setStartTime(getSearcher(MAIN_THREAD)->getStartTime());
			searchSlave->setTimeToStop(getSearcher(MAIN_THREAD)->getTimeToStop());
			sp->workers++;
			thread.splitPoint = sp;
			thread.status = THREAD_STATUS_WORK_ASSIGNED;
			freeThreads--;
			if (sp->workers>=maxWorkersPerSplitPoint) {
				break;
			}
		}
	}
	unlock(&mutex1);
	const bool splitOk = threadId>-1;
	if (splitOk) {
		executeThread(currentThreadId,sp);
	}
	lock(&mutex1);
	threadPool[currentThreadId].spNumber--;
	sp->clear();
	unlock(&mutex1);
	return splitOk;
};

// start search
void SearchAgent::startSearch() {
	lock(&threadPool[MAIN_THREAD].mutex);
	setRequestStop(true);
	wakeUp(&threadPool[MAIN_THREAD].waitCond);
	unlock(&threadPool[MAIN_THREAD].mutex);
}

// start perft
void SearchAgent::doPerft() {
	std::cout << "info Executing perft..." << std::endl;
	int64_t nodes = getSearcher(MAIN_THREAD)->perft(board,this->getDepth(),1);
	std::cout << "info Finished perft: " << nodes << std::endl;
}

// bench test
void SearchAgent::doBench() {
	std::cout << "Executing benchmark..." << std::endl;
	int64_t time = 0;
	int64_t nodes = 0;
	getSearcher(MAIN_THREAD)->setUpdateUci(false);
	for (int i=0;i<benchSize;i++) {
		lock(&mutex2);
		std::cout << "Position[" << (i+1) << "]: " << benchPositions[i] << std::endl;
		newGame();
		setPositionFromFEN(benchPositions[i]);
		setSearchMode(SearchAgent::SEARCH_DEPTH);
		setDepth(benchDepth);
		startSearch();
		wait(&waitCond2, &mutex2);
		time+=getSearcher(MAIN_THREAD)->getSearchedTime();
		nodes+=getSearcher(MAIN_THREAD)->getSearchedNodes();
		unlock(&mutex2);
	}
	getSearcher(MAIN_THREAD)->setUpdateUci(true);
	std::cout << std::endl << "Finished benchmark:  " << std::endl;
	std::cout << "Total time(seconds): " << (time/1000) << std::endl;
	std::cout << "Total nodes:         " << (nodes) << std::endl;
	std::cout << "Nodes/Seconds:       " << (nodes/(std::max(int64_t(1),time/1000))) << std::endl;
}
// eval test
void SearchAgent::doEval() {
	getSearcher(MAIN_THREAD)->getEvaluator().setDebugEnabled(true);
	getSearcher(MAIN_THREAD)->getEvaluator().evaluate(board,-maxScore,maxScore);
	getSearcher(MAIN_THREAD)->getEvaluator().setDebugEnabled(false);
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
		movesLeft = movesToGo;
		if (movesToGo==1) {
			time=time*80/100;
		} else {
			time=time*90/100;
		}
	} else {
		for (int x=0;x<timeTableSize;x++) {
			if (time<timeTable[x][1] && time >= timeTable[x][2]) {
				movesLeft=timeTable[x][0]+
						(maxGamePhase-board.getGamePhase())/2;
				break;
			}
		}
		time=time*95/100;
	}
	return std::min(time/movesLeft+incTime,time);
}

const int64_t SearchAgent::getTimeToSearch() {
	return getTimeToSearch(0);
}

int64_t SearchAgent::addExtraTime(const int iteration, int* iterationPVChange) {
	const int64_t timeThinking = getTickCount()-getSearcher(MAIN_THREAD)->getStartTime();
	const int64_t weight = std::min(int64_t(90), int64_t(iterationPVChange[iteration] * 7 +
			iterationPVChange[iteration-1] * 2));
	const int64_t totalTime=(board.getSideToMove()==WHITE? getWhiteTime():getBlackTime())
			- timeThinking;
	const int64_t maxAllowed = (totalTime/100)*30;
	if (getMovesToGo()==1 || getMovesToGo()==2 || timeThinking > maxAllowed) {
		return 0;
	}
	const int64_t timeLeft = std::max(int64_t(0),
			getSearcher(MAIN_THREAD)->getTimeToSearch()-timeThinking-5);
	const int64_t newSearchTime = std::min(maxAllowed, timeLeft + (getTimeToSearch(timeThinking)*weight)/100);
	getSearcher(MAIN_THREAD)->setTimeToSearch(newSearchTime);
	getSearcher(MAIN_THREAD)->setTimeToStop(getSearcher(MAIN_THREAD)->getStartTime()+newSearchTime);
	return newSearchTime;
}

void  SearchAgent::ponderHit() {
	const int64_t timeThinking = getTickCount()-getSearcher(MAIN_THREAD)->getStartTime();
	getSearcher(MAIN_THREAD)->setSearchFixedDepth(false);
	getSearcher(MAIN_THREAD)->setInfinite(false);
	getSearcher(MAIN_THREAD)->setTimeToSearch(getTimeToSearch()-std::max(int64_t(10),timeThinking));
	getSearcher(MAIN_THREAD)->setTimeToStop();
}

void SearchAgent::initThreads() {
	currentThread=MAIN_THREAD;
	int rCode = pthread_create(&(threadPool[MAIN_THREAD].executor), NULL, mainThreadRun, this);
	if (rCode) {
		std::cerr << "Failed to created main thread. pthread_create return code:  " << rCode << std::endl;
		exit(EXIT_FAILURE);
	}
	for (int i=1;i<threadPoolSize;i++) {
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
	void *threadResult = object->executeThread(object->getAndIncCurrentThread(),NULL);
	return threadResult;
}

void SearchAgent::initializeThreadPool(const int size) {
	const int newSize=size;
	for (int i=0;i<newSize;i++) {
		threadPool[i].threadId=i;
		threadPool[i].threadType=ThreadType(i);
		if (mainSearcher[i]) {
			delete mainSearcher[i];
			mainSearcher[i]=NULL;
		}
		mainSearcher[i]=new SimplePVSearch(&history[0][0]);
		mainSearcher[i]->setSearchAgent(this);
		mainSearcher[i]->setThreadId(i);

	}
	threadPoolSize=newSize;
	freeThreads=newSize;
	currentThread=MAIN_THREAD;
	setThreadsShouldWait(true);
	initThreads();
}

void SearchAgent::awakeWaitingThreads() {
	for(int i=1;i<threadPoolSize;i++) {
		SearchThread& thread = threadPool[i];
		if (thread.status==THREAD_STATUS_WAITING) {
			wakeUp(&thread.waitCond);
		}
	}
}

void SearchAgent::prepareThreadPool() {
	for(int i=1;i<threadPoolSize;i++) {
		SearchThread& thread = threadPool[i];
		thread.clear();
		getSearcher(i)->resetStats();
		getSearcher(i)->clearKillers();
		getSearcher(i)->setSearchFixedDepth(getSearcher(MAIN_THREAD)->isSearchFixedDepth());
		getSearcher(i)->setTimeToSearch(getSearcher(MAIN_THREAD)->getTimeToSearch());
		getSearcher(i)->setInfinite(getSearcher(MAIN_THREAD)->isInfinite());
		getSearcher(i)->setDepth(getSearcher(MAIN_THREAD)->getDepth());
		getSearcher(i)->setStartTime(getSearcher(MAIN_THREAD)->getStartTime());
		getSearcher(i)->setTimeToStop();
	}
	freeThreads=threadPoolSize;
}

void SearchAgent::shutdown() {
	while (getSearchInProgress()) {
		// wait for threads termination
	}
	for (int i=0;i<threadPoolSize;i++) {
		if (mainSearcher[i]) {
			delete mainSearcher[i];
		}
	}
	destroyHash();
	setQuit(true);
}
