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
 * smp.h
 *
 *  Created on: Apr 23, 2011
 *      Author: bhlangonijr
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include "simplepvsearch.h"
#include "moveiterator.h"

const int maxThreads				= 16;
const int maxWorkersPerSplitPoint	= 8;

enum ThreadType {
	MAIN_THREAD=0,
	INACTIVE_THREAD=maxThreads+1
};

enum ThreadStatus {
	THREAD_STATUS_AVAILABLE,
	THREAD_STATUS_WORK_ASSIGNED,
	THREAD_STATUS_WORKING,
	THREAD_STATUS_WAITING
};

struct SplitPoint {
	SplitPoint() : masterThreadId(0), workers(0), nodes(0), moves(0), bestMove(0),
			hashMove(0), bestScore(0), currentAlpha(0), currentScore(0), moveCounter(0), nmMateScore(0),
			masterDone(true), shouldStop(false) {
		init();
	}
	inline void init() {
		clear();
	}
	inline void clear() {
		isPV=false;
		move.clear();
		alpha=0;
		beta=0;
		depth=0;
		ply=0;
		allowNullMove=false;
		partialSearch=false;
		masterThreadId=0;
		workers=0;
		moves=0;
		bestMove=0;
		hashMove=0;
		bestScore=0;
		currentAlpha=0;
		currentScore=0;
		moveCounter=0;
		nmMateScore=0;
		nodes=0;
		masterDone=false;
		shouldStop=false;
	}
	int masterThreadId;
	bool isPV;
	Board board;
	int alpha;
	int beta;
	int depth;
	int ply;
	bool allowNullMove;
	bool partialSearch;
	MoveIterator::Move move;
	int workers;
	int64_t nodes;
	MoveIterator* moves;
	MoveIterator::Move* bestMove;
	MoveIterator::Move* hashMove;
	int* bestScore;
	int* currentAlpha;
	int* currentScore;
	int* moveCounter;
	bool* nmMateScore;
	bool masterDone;
	bool shouldStop;
};

class SearchThread {
public:

	SearchThread() : threadId(0), threadType(INACTIVE_THREAD), status(THREAD_STATUS_AVAILABLE),
	splitPoint(NULL), spNumber(0) {
		init();
	}

	~SearchThread() {
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&waitCond);
	}

	inline void init() {
		pthread_mutex_init(&mutex,NULL);
		pthread_cond_init(&waitCond,NULL);
		clear();
	}

	inline void clear() {
		status=THREAD_STATUS_AVAILABLE;
		splitPoint=NULL;
		spNumber=0;
	}

	int threadId;
	ThreadType threadType;
	ThreadStatus status;
	pthread_t executor;
	pthread_mutex_t mutex;
	pthread_cond_t waitCond;
	SplitPoint* splitPoint;
	int spNumber;
};

#endif /* THREADPOOL_H_ */
