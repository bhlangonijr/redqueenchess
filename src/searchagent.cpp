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
		blackIncrement(0), depth(defaultDepth), movesToGo(0), moveTime(0), ponder(false), mainSearcher(0),
		freeThreads(0), threadShouldWait(false) {
	// creates initial hashtables
	createHash();
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
	lock(&threadPool[MAIN_THREAD].mutex);
	while (!quit) {
		wait(&threadPool[MAIN_THREAD].waitCond, &threadPool[MAIN_THREAD].mutex);
		if (quit) {
			break;
		}
		mainSearcher->setThreadId(MAIN_THREAD);
		setThreadsShouldWait(false);
		awakeWaitingThreads();
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
		prepareThreadPool();
		mainSearcher->search(board);
		setSearchInProgress(false);
		setThreadsShouldWait(true);
		wakeUp(&waitCond2);
	}
	unlock(&threadPool[MAIN_THREAD].mutex);
	pthread_exit(NULL);
	return NULL;
}

// worker threads loop
void* SearchAgent::executeThread(const int threadId) {
	while (!quit) {
		ThreadPool& thread = threadPool[threadId];
		if (getThreadsShouldWait()) {
			lock(&thread.mutex);
			thread.status = THREAD_STATUS_WAITING;
			wait(&thread.waitCond, &thread.mutex);
			if (thread.status == THREAD_STATUS_WAITING) {
				thread.status = THREAD_STATUS_AVAILABLE;
			}
			unlock(&thread.mutex);
		}
		ThreadPool& master = threadPool[thread.masterThreadId];
		if (quit) {
			break;
		}
		if (thread.status==THREAD_STATUS_WORK_ASSIGNED) {
			thread.status = THREAD_STATUS_WORKING;
//			lock(&mutex1);
//			long time = thread.ss->getTickCount();
//			std::cout << "Launching thread - workers " << master.workers << " freethreads: " << freeThreads <<
//					" id.master " << thread.masterThreadId << " id.slave " << threadId << std::endl;
//			unlock(&mutex1);
			smpPVSearch(*thread.board,master.ss,thread.ss,master);
			lock(&mutex1);
//			std::cout << "Launched thread " << master.workers << " freethreads: " << freeThreads <<
//					" id.master " << thread.masterThreadId << " id.slave " << threadId << " time " <<
//					(thread.ss->getTickCount()-time) << " nodes = " << thread.ss->getSearchedNodes() << std::endl;

			master.workers--;
			freeThreads++;
			if (thread.isMaster) {
				thread.status=THREAD_STATUS_AVAILABLE;
			} else {
				master.nodes+=thread.ss->getSearchedNodes();
				master.ss->mergeHistory(thread.ss->getKillerArray(),thread.ss->getHistoryArray());
				resetThread(threadId);
				thread.ss->resetStats();
			}
			unlock(&mutex1);
		}
		if (thread.isMaster && master.workers<=0) {
			master.ss->updateSearchedNodes(master.nodes);
			return NULL;
		}
	}
	pthread_exit(NULL);
	return NULL;
}
// Spawn a new search thread
const bool SearchAgent::spawnThreads(Board& board, void* data, const int threadGroup, const int masterThreadId,
		MoveIterator* moves, MoveIterator::Move* move, MoveIterator::Move* hashMove, int* bestScore,
		int* currentAlpha, int* currentScore, int* moveCounter, bool* nmMateScore) {
	lock(&mutex1);
	if (singleProcessor || threadPool[masterThreadId].status!=THREAD_STATUS_AVAILABLE ||
			getFreeThreads()<1 || getThreadNumber() < 2 || getRequestStop()) {
		unlock(&mutex1);
		return false;
	}
	int threadId=0;
	resetThreadStop(masterThreadId);
	resetThread(masterThreadId);
	SearchTypes::SearchInfo* si = static_cast<SearchTypes::SearchInfo*>(data);
	for(int i=0;i<threadPoolSize&&i<=maxWorkersPerSplitPoint;i++) {
		ThreadPool& thread = threadPool[i];
		if (thread.status==THREAD_STATUS_AVAILABLE) {
			threadId=i;
			ThreadPool& thread = threadPool[threadId];
			thread.isMaster = masterThreadId == i;
			if (i!=0) {
				thread.ss->resetStats();
				thread.ss->clearHistory();
				thread.ss->setSearchFixedDepth(mainSearcher->isSearchFixedDepth());
				thread.ss->setTimeToSearch(mainSearcher->getTimeToSearch());
				thread.ss->setInfinite(mainSearcher->isInfinite());
				thread.ss->setDepth(mainSearcher->getDepth());
				thread.ss->setStartTime(mainSearcher->getStartTime());
				thread.ss->setTimeToStop(mainSearcher->getTimeToStop());
				thread.ss->mergeHistory(threadPool[masterThreadId].ss->getKillerArray(),
						threadPool[masterThreadId].ss->getHistoryArray());
			}
			thread.alpha = si->alpha;
			thread.beta = si->beta;
			thread.depth = si->depth;
			thread.ply = si->ply;
			thread.isPV = si->nodeType==PV_NODE;
			thread.allowNullMove = si->allowNullMove;
			thread.partialSearch = si->partialSearch;
			thread.moves = moves;
			thread.move = si->move;
			thread.threadGroup = threadGroup;
			thread.board = new Board(board);
			thread.bestMove=move;
			thread.bestScore=bestScore;
			thread.currentAlpha=currentAlpha;
			thread.currentScore=currentScore;
			thread.moveCounter=moveCounter;
			thread.nmMateScore=nmMateScore;
			thread.hashMove=hashMove;
			thread.ss->setThreadGroup(thread.threadGroup);
			thread.masterThreadId = masterThreadId;
			threadPool[masterThreadId].workers++;
			thread.status = THREAD_STATUS_WORK_ASSIGNED;
			freeThreads--;
		}
	}
	unlock(&mutex1);
	if (threadId>0) {
		executeThread(masterThreadId);
	}
	return threadId>0;
};

// parallel search
void SearchAgent::smpPVSearch(Board& board, SimplePVSearch* master,
		SimplePVSearch* ss, SearchAgent::ThreadPool& thread) {
	const bool isKingAttacked = board.isInCheck();
	const bool isPV = thread.isPV;
	int score = 0;
	MoveIterator::Move move;
	while (true) {
		master->lock();
		move = master->selectMove<false>(board, *thread.moves, *thread.hashMove, thread.ply, thread.depth);
		(*thread.moveCounter)++;
		master->unlock();
		if (move.none()) {
			break;
		}
		MoveBackup backup;
		board.doMove(move,backup);
		const bool givingCheck = board.setInCheck(board.getSideToMove());
		const bool passedPawn = ss->isPawnPush(board,move.to);
		const bool pawnOn7thExtension = move.promotionPiece!=EMPTY;
		//futility
		if  (	!isPV &&
				move.type == MoveIterator::NON_CAPTURE &&
				thread.depth < futilityDepth &&
				!isKingAttacked &&
				!givingCheck &&
				!pawnOn7thExtension &&
				!passedPawn &&
				!*thread.nmMateScore) {
			if (moveCountMargin(thread.depth) < *thread.moveCounter
					&& !master->isMateScore(*thread.bestScore) ) {
				board.undoMove(backup);
				continue;
			}
			const int futilityScore = *thread.currentScore + futilityMargin(thread.depth);
			master->lock();
			if (futilityScore < thread.beta) {
				if (futilityScore>*thread.bestScore) {
					*thread.bestScore=futilityScore;
				}
				master->unlock();
				board.undoMove(backup);
				continue;
			}
			master->unlock();
		}
		//reductions
		int reduction=0;
		int extension=0;
		if (isKingAttacked || pawnOn7thExtension) {
			extension++;
		} else if (thread.depth>lmrDepthThreshold && !givingCheck && !passedPawn &&
				!(*thread.nmMateScore) &&	move.type == MoveIterator::NON_CAPTURE) {
			reduction=ss->getReduction(isPV,thread.depth,*thread.moveCounter);
		}
		int newDepth=thread.depth-1+extension;
		SearchInfo newSi(true,move,thread.beta,1-thread.beta, newDepth-reduction, thread.ply+1, NONPV_NODE);
		if (isPV) {
			newSi.update(newDepth-reduction,NONPV_NODE,-thread.beta,-(*thread.currentAlpha),move);
		}
		score = -ss->zwSearch(board, newSi);
		bool research=isPV?score > *thread.currentAlpha &&
				score < thread.beta:score >= thread.beta && reduction>0;
		if (research) {
			if (reduction>2) {
				newSi.update(newDepth-1,NONPV_NODE);
				score = -ss->zwSearch(board, newSi);
				research=(score >= thread.beta);
			}
			if (research) {
				newSi.update(newDepth,NONPV_NODE);
				score = -ss->zwSearch(board, newSi);
			}
			if (isPV && score > *thread.currentAlpha && score < thread.beta) {
				newSi.update(newDepth,PV_NODE);
				score = -ss->pvSearch(board, newSi);
			}
		}
		board.undoMove(backup);
		master->lock();
		if (!(master->stop() || threadShouldStop(thread.threadGroup))) {
			if (score>=thread.beta) {
				*thread.bestScore=score;
				*thread.bestMove=move;
				requestThreadStop(master->getThreadId());
				master->updateHistory(board,*thread.bestMove,thread.depth);
				master->updateKillers(board,*thread.bestMove,thread.ply);
				TranspositionTable::NodeFlag flag = *thread.currentScore!=-maxScore?
						TranspositionTable::LOWER_EVAL:TranspositionTable::LOWER ;
				hashPut(board.getKey(),*thread.bestScore,*thread.currentScore,
						thread.depth,thread.ply,flag,*thread.bestMove);
				master->unlock();
				return;
			}
			if (score>*thread.bestScore) {
				*thread.bestScore=score;
				if(score>*thread.currentAlpha ) {
					*thread.currentAlpha=score;
					*thread.bestMove=move;
				}
			}
		}
		master->unlock();
	}
}

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
		lock(&mutex2);
		std::cout << "Position[" << (i+1) << "]: " << benchPositions[i] << std::endl;
		newGame();
		setPositionFromFEN(benchPositions[i]);
		setSearchMode(SearchAgent::SEARCH_DEPTH);
		setDepth(benchDepth);
		startSearch();
		wait(&waitCond2, &mutex2);
		time+=mainSearcher->getSearchedTime();
		nodes+=mainSearcher->getSearchedNodes();
		unlock(&mutex2);
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
	void *threadResult = object->executeThread(object->getAndIncCurrentThread());
	return threadResult;
}

void SearchAgent::initializeThreadPool(const int size) {
	const int newSize=singleProcessor?1:size;
	for (int i=0;i<newSize;i++) {
		threadPool[i].threadId=i;
		threadPool[i].threadType=ThreadType(i);
		if (!threadPool[i].ss) {
			delete threadPool[i].ss;
			threadPool[i].ss=0;
		}
		threadPool[i].ss=new SimplePVSearch();
		threadPool[i].ss->setSearchAgent(this);
		threadPool[i].ss->setThreadId(i);

	}
	mainSearcher = threadPool[0].ss;
	threadPoolSize=newSize;
	freeThreads=newSize;
	currentThread=MAIN_THREAD;
	setThreadsShouldWait(true);
	initThreads();
}

void SearchAgent::awakeWaitingThreads() {
	for(int i=1;i<threadPoolSize;i++) {
		ThreadPool& thread = threadPool[i];
		if (thread.status==THREAD_STATUS_WAITING) {
			wakeUp(&thread.waitCond);
		}
	}
}

void SearchAgent::prepareThreadPool() {
	for(int i=1;i<threadPoolSize;i++) {
		ThreadPool& thread = threadPool[i];
		thread.ss->resetStats();
		thread.ss->cleanUp();
		thread.ss->setSearchFixedDepth(mainSearcher->isSearchFixedDepth());
		thread.ss->setTimeToSearch(mainSearcher->getTimeToSearch());
		thread.ss->setInfinite(mainSearcher->isInfinite());
		thread.ss->setDepth(mainSearcher->getDepth());
		thread.ss->setStartTime(thread.ss->getTickCount());
		thread.ss->setTimeToStop();
	}
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
