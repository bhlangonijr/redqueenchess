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
 * SearchAgent.h
 *
 *  Created on: 01/05/2009
 *      Author: bhlangonijr
 */

#ifndef SEARCHAGENT_H_
#define SEARCHAGENT_H_
#include <iostream>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
#endif
#include "board.h"
#include "simplepvsearch.h"
#include "transpositiontable.h"
#include "uci.h"
#include "threadpool.h"

const std::string mainHashName 		= "DefaultHashTable";
const size_t defaultDepth			= 5;
const size_t defaultHashSize		= 128;
const int defaultGameSize			= 30;
const int timeTableSize				= 7;
const int benchSize					= 12;
const int benchDepth				= 14;
const int timeTable [7][3] = {
		{25, 900000, 180000},
		{23, 180000, 60000},
		{21, 30000, 10000},
		{19, 10000, 5000},
		{17, 5000, 1000},
		{15, 1000, 0}
};

const std::string benchPositions[benchSize] = {
		"8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - - 0 1",
		"1q2r1k1/5pb1/2bppnp1/p1p5/Nr2PPP1/1BBP4/1PP4Q/2KR3R w - - 0 1",
		"1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1",
		"rn3rk1/pbppq1pp/1p2pb2/4N2Q/3PN3/3B4/PPP2PPP/R3K2R w KQ - 6 11",
		"1Q6/5kpp/2p4r/b4p2/P2R4/1N4P1/5PKP/4q3 b - - 0 1",
		"r1r2bk1/pp1n1p1p/2pqb1p1/3p4/1P1P4/1QN1PN2/P3BPPP/2RR2K1 w - - 0 1",
		"6k1/8/8/8/8/1p6/6K1/8 w - - 0 1",
		"8/1B3k2/4Rbp1/3Pp1p1/5p2/5P1P/3r2PK/8 b - - 0 1",
		"1r6/r7/2bkp3/1p1p1p1p/p1pP1PpP/P1P1P1B1/1PK4P/1N1B4 b - - 0 150",
		"8/8/8/8/3k4/1r6/6K1/8 w - - 0 1",
		"2n5/2P5/1Pk5/2B5/8/p7/8/1K6 b - - 2 86",
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
};

class SimplePVSearch;

class SearchAgent {

public:

	enum SearchMode {
		SEARCH_TIME, SEARCH_DEPTH, SEARCH_MOVESTOGO, SEARCH_MOVETIME, SEARCH_MOVES, SEARCH_INFINITE, SEARCH_NODES, SEARCH_NONE
	};

	static SearchAgent* getInstance();
	void newGame();
	void clearParameters();
	const Board getBoard() const;
	void setBoard(Board _board);

	const bool shouldStop() const {
		return requestStop || quit;
	}

	inline void lock(pthread_mutex_t* mutex) {
		pthread_mutex_lock(mutex);
	}

	inline void unlock(pthread_mutex_t* mutex) {
		pthread_mutex_unlock(mutex);
	}

	inline void wait(pthread_cond_t* cond, pthread_mutex_t* mutex) {
		pthread_cond_wait(cond, mutex);
	}

	inline void wakeUp(pthread_cond_t* cond) {
		pthread_cond_signal(cond);
	}

	inline const bool getThreadsShouldWait() const {
		return threadShouldWait;
	}

	inline void setThreadsShouldWait(const bool shouldWait) {
		threadShouldWait = shouldWait;
	}

	inline const bool getSearchInProgress() const {
		return searchInProgress;
	}

	inline void setSearchInProgress(bool _searchInProgress) {
		searchInProgress = _searchInProgress;
	}

	inline void setRequestStop(bool shouldStop) {
		requestStop = shouldStop;
	}

	inline bool getRequestStop() {
		return requestStop;
	}

	inline void setQuit(bool shouldQuit) {
		quit = shouldQuit;
	}

	inline const SearchMode getSearchMode() const {
		return searchMode;
	}

	inline void setSearchMode(SearchMode _searchMode) {
		searchMode = _searchMode;
	}

	void setPositionFromSAN(std::string startPosMoves, const bool startPos);
	void setPositionFromFEN(std::string fenMoves);
	void startSearch();
	void doPerft();
	void doBench();
	void doEval();
	void stopSearch();

	inline const size_t getHashSize() const {
		return hashSize;
	}

	inline void setHashSize(size_t _hashSize) {
		hashSize = _hashSize;
	}

	inline const int getThreadNumber() const {
		return threadNumber;
	}

	inline void setThreadNumber(int _threadNumber) {
		threadNumber = _threadNumber;
	}

	inline const int64_t getWhiteTime() const {
		return whiteTime;
	}

	inline void setWhiteTime(int64_t _whiteTime) {
		whiteTime = _whiteTime;
	}

	inline const int64_t getWhiteIncrement() const {
		return whiteIncrement;
	}

	inline void setWhiteIncrement(int64_t _whiteIncrement) {
		whiteIncrement = _whiteIncrement;
	}

	inline const int64_t getBlackTime() const {
		return blackTime;
	}

	inline void setBlackTime(int64_t _blackTime) {
		blackTime = _blackTime;
	}

	inline const int64_t getBlackIncrement() const {
		return blackIncrement;
	}

	inline void setBlackIncrement(int64_t _blackIncrement) {
		blackIncrement = _blackIncrement;
	}

	inline const int getDepth() const {
		return depth;
	}

	inline void setDepth(int _depth) {
		depth = _depth;
	}

	inline const int getMovesToGo() const {
		return movesToGo;
	}

	inline void setMovesToGo(int _movesToGo) {
		movesToGo = _movesToGo;
	}

	inline const int64_t getSearchNodes() const {
		return searchNodes;
	}

	inline void setSearchNodes(int64_t _searchNodes) {
		searchNodes = _searchNodes;
	}

	inline const std::string getSearchMoves() const {
		return searchMoves;
	}

	inline void setSearchMoves(std::string moves) {
		searchMoves = moves;
	}

	inline const int64_t getMoveTime() const {
		return moveTime;
	}

	inline void setMoveTime(int64_t _moveTime) {
		moveTime = _moveTime;
	}

	inline const bool getPonder() const {
		return ponder;
	}

	inline void setPonder(bool _ponder) {
		ponder = _ponder;
	}

	void ponderHit();

	inline void clearHash() {
		transTable->clearHash();
	}

	inline bool hashPut(const Key _key, int value, int evalValue, const int depth, const int ply,
			const TranspositionTable::NodeFlag flag, MoveIterator::Move move) {
		TranspositionTable::HashKey key=(TranspositionTable::HashKey)(_key>>32);
		if (value >= maxScore-100) {
			value += ply;
		} else if (value <= -maxScore+100) {
			value -= ply;
		}
		return transTable->hashPut(key,value,evalValue,flag,move,depth);
	}

	inline bool hashGet(const Key _key, TranspositionTable::HashData& hashData, const int ply) {
		TranspositionTable::HashKey key=(TranspositionTable::HashKey)(_key>>32);
		bool result = transTable->hashGet(key, hashData);
		if (result) {
			int value = hashData.value();
			if (value >= maxScore-100) {
				hashData.setValue(value-ply);
			} else if (value <= -maxScore+100) {
				hashData.setValue(value+ply);
			}
		} else {
			hashData.clear();
		}
		return result;
	}

	inline bool hashGet(bool& okToPrune, const Key _key, TranspositionTable::HashData& hashData, const int ply,
			const int depth, const bool allowNullMove, const int alpha, const int beta) {
		const bool result = hashGet(_key, hashData, ply);
		if (result) {
			okToPrune = hashData.value() != -maxScore &&
					(allowNullMove || !(hashData.flag() & TranspositionTable::NODE_NULL)) &&
					(hashData.depth()>=depth) &&
					(((hashData.flag() & TranspositionTable::LOWER) && hashData.value() >= beta) ||
							((hashData.flag() & TranspositionTable::UPPER) && hashData.value() <= alpha));
		} else {
			okToPrune = false;
		}
		return result;
	}

	inline bool hashGet(bool& okToPrune, const Key _key, TranspositionTable::HashData& hashData, const int ply,
			const int depth) {
		const bool result = hashGet(_key, hashData, ply);
		if (result) {
			okToPrune =	hashData.value() != -maxScore &&
					hashData.depth() >= depth && !(hashData.flag() & TranspositionTable::NODE_NULL) &&
					(hashData.flag() & TranspositionTable::EXACT);
		} else {
			okToPrune = false;
		}
		return result;
	}

	inline void clearHistory() {
		memset(history, 0, sizeof(int)*ALL_PIECE_TYPE_BY_COLOR*ALL_SQUARE);
	}

	inline void updateHistory(Board& board, MoveIterator::Move& move, int depth) {
		if (board.isCaptureOrPromotion(move) || move.none()) {
			return;
		}
		int* h = &history[board.getPiece(move.from)][move.to];
		*h=std::min(*h+depth*depth,INT_MAX);
	}

	inline int getHistory(const PieceTypeByColor pieceFrom, const int squareTo) {
		return history[pieceFrom][squareTo];
	}

#if defined(_SC_NPROCESSORS_ONLN)
	inline int getNumProcs() {
		return std::min((int)(sysconf( _SC_NPROCESSORS_ONLN )), maxThreads);
	}
#else
	inline int getNumProcs() {
		SYSTEM_INFO sysinfo;
		GetSystemInfo( &sysinfo );
		return std::min((int)(sysinfo.dwNumberOfProcessors), maxThreads);
	}
#endif

	inline bool isHashFull() {
		return transTable->isHashFull();
	}

	inline int hashFull() {
		return transTable->hashFull();
	}

	void createHash() {
		transTable = new TranspositionTable(mainHashName, getHashSize());
	}

	void destroyHash() {
		if (transTable) {
			delete transTable;
		}
	}

	void newSearchHash() {
		return transTable->newSearch();
	}

	const int getThreadPoolSize() const {
		return threadPoolSize;
	}

	const SearchThread& getThread(const int index) {
		return threadPool[index];
	}

	const SearchThread& getThread() {
		return threadPool[getCurrentThreadId()];
	}

	const int getCurrentThreadId() const {
		return currentThread;
	}

	const int getAndIncCurrentThread() {
		pthread_mutex_lock(&mutex);
		int t = ++currentThread;
		pthread_mutex_unlock(&mutex);
		return t;
	}

	void setThreadStatus(const int threadId, const ThreadStatus _status) {
		threadPool[threadId].status = _status;
	}

	void resetThread(const int threadId) {
		threadPool[threadId].clear();
	}

	const inline int getFreeThreads() const {
		return freeThreads;
	}

	inline SimplePVSearch* getSearcher(const int threadId) {
		return mainSearcher[threadId];
	}

	int64_t addExtraTime(const int iteration, int* iterationPVChange);
	void initializeThreadPool(const int size);
	void awakeWaitingThreads();
	void prepareThreadPool();
	void *startThreadSearch();
	void *executeThread(const int threadId, SplitPoint* sp);
	const bool spawnThreads(Board& board, void* data, const int currentThreadId,
			MoveIterator* moves, MoveIterator::Move* move, MoveIterator::Move* hashMove, int* bestScore,
			int* currentAlpha, int* currentScore, int* moveCounter, bool* nmMateScore);
	void smpPVSearch(Board board, SimplePVSearch* master, SimplePVSearch* ss, SplitPoint* sp);
	void shutdown();

protected:

	SearchAgent();
	SearchAgent(const SearchAgent&);
	SearchAgent& operator= (const SearchAgent&);

private:

	static SearchAgent* searchAgent;
	Board board;
	SearchMode searchMode;
	volatile bool searchInProgress;
	volatile bool requestStop;
	volatile bool quit;
	size_t hashSize;
	int threadNumber;
	int64_t whiteTime;
	int64_t whiteIncrement;
	int64_t blackTime;
	int64_t blackIncrement;
	int depth;
	int movesToGo;
	int64_t searchNodes;
	std::string searchMoves;
	int64_t moveTime;
	bool ponder;
	TranspositionTable* transTable;
	SearchThread threadPool[maxThreads];
	int history[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE];
	int threadPoolSize;
	SimplePVSearch* mainSearcher[maxThreads];
	int currentThread;
	int freeThreads;
	bool threadShouldWait;
	static pthread_mutex_t mutex;
	static pthread_cond_t waitCond;
	static pthread_mutex_t mutex1;
	static pthread_cond_t waitCond1;
	static pthread_mutex_t mutex2;
	static pthread_cond_t waitCond2;
	static pthread_mutex_t mutex3;
	static pthread_cond_t waitCond3;
	SplitPoint splitPoint[maxThreads][maxThreads];
	const int64_t getTimeToSearch(const int64_t usedTime);
	const int64_t getTimeToSearch();
	void initThreads();
};

#endif /* SEARCHAGENT_H_ */
