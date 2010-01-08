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
 * SimplePVSearch.cpp
 *
 *  Created on: 18/05/2009
 *      Author: bhlangonijr
 */


#include "SimplePVSearch.h"
// If set to true, will check move integrity - used for trace purposes
#define CHECK_MOVE_GEN_ERRORS false
// Exit the program if the count of errors in CHECK_MOVE_GEN_ERRORS exceed the threshold - used for trace purposes
#define EXIT_PROGRAM_THRESHOLD 100
// If true uses Principal Variation Search
#define PV_SEARCH true
// debug iterative d search
#define DEBUG_ID false
// debug alpha-beta search
#define DEBUG_AB false
// debug qs search
#define DEBUG_QS false
// show stats info
#define SHOW_STATS false

using namespace SimplePVSearchTypes;

// root search
void SimplePVSearch::search() {

	Board board(_board);
#if SHOW_STATS
	stats.clear();
#endif
	errorCount=0;
	_startTime = getTickCount();
	_score = idSearch(board);
	SearchAgent::getInstance()->setSearchInProgress(false);
	_time = getTickCount() - _startTime;

#if CHECK_MOVE_GEN_ERRORS
	// checks for data corruption
	Key oldKey = _board.generateKey();
	Key newKey = board.generateKey();

	if (oldKey!=newKey) {
		std::cout << "old key " << oldKey <<std::endl;
		std::cout << "new key " << newKey <<std::endl;
		_board.printBoard();
		board.printBoard();
	}

	assert(oldKey==newKey);
#endif

}

// get current score
int SimplePVSearch::getScore() {
	return this->_score;
}

// iterative deepening
int SimplePVSearch::idSearch(Board& board) {

	MoveIterator moves;
	board.generateAllMoves(moves, board.getSideToMove());
	MoveIterator::Move bestMove = MoveIterator::Move(NONE,NONE,EMPTY);
	bestMove.score = -maxScore;

#if DEBUG_ID
	board.printBoard();
	std::cout << "Eval: " << evaluator.evaluate(board) << std::endl;
#endif
	_nodes = 0;
	uint32_t totalTime = 0;

	uint32_t searchDepth = 0;
	for (uint32_t depth = 1; depth <= _depth; depth++) {
		if (stop()) {
			break;
		}

		uint32_t time = getTickCount();
		uint64_t nodes = _nodes;
		int moveCounter=0;
		int bestScore = -maxScore;
		PvLine pv = PvLine();
		pv.index=0;
		moves.first();

		while (moves.hasNext()) {
			if (stop()) {
				break;
			}
			MoveIterator::Move& move = moves.next();
			_nodes++;
			MoveBackup backup;
			board.doMove(move,backup);

			if (board.isNotLegal()) {
				board.undoMove(backup);
				move.score=-maxScore;
				continue; // not legal
			}

			moveCounter++;
			if (isUpdateUci() && totalTime > 1000) {
				std::cout << "info currmove " << move.toString() << " currmovenumber " << moveCounter << std::endl;
			}

			int score = -pvSearch(board, -maxScore, -bestScore, depth-1, &pv, true);
			move.score=score;

#if DEBUG_ID
			std::cout << "score: " << score << " / move: " << move.toString() << " / depth " << depth << std::endl;
			board.printBoard();
#endif

			board.undoMove(backup);

			if (score > bestScore && !stop()) {
				bestScore = score;
				bestMove = move;
				searchDepth=depth;
#if SHOW_STATS
				stats.pvChanges++;
#endif
			}

		}

		moves.sort();

		time = getTickCount()-time;
		totalTime += time;
#if SHOW_STATS
		stats.searchTime=totalTime;
		stats.searchNodes=_nodes;
#endif
		if (isUpdateUci()) {

			uint64_t nps = time>1000 ?  ((_nodes-nodes)/(time/1000)) : _nodes;
			std::cout << "info depth "<< depth << std::endl;
			std::cout << "info depth "<< depth << " score cp " << bestMove.score << " time " << time << " nodes " << (_nodes-nodes) << " nps " << nps << " pv " << bestMove.toString() << pvLineToString(&pv) << std::endl;
			std::cout << "info nodes " << (_nodes-nodes) << " time " << time << " nps " << nps << " hashfull " << SearchAgent::getInstance()->hashFull() << std::endl;

		}
		if (moveCounter==1) {
			break;
		}
#if SHOW_STATS
		std::cout << "Search stats: " << std::endl;
		std::cout << stats.toString() << std::endl;
#endif
	}

	if (bestMove.from!=NONE) {
		if (isUpdateUci()) {
			std::cout << "bestmove " << bestMove.toString() << std::endl;
		}
	} else {
		std::cout << "bestmove (none)" << std::endl;
	}

	return bestMove.score;
}

// principal variation search
int SimplePVSearch::pvSearch(Board& board, int alpha, int beta, uint32_t depth, PvLine* pv, const bool allowNullMove) {

#if PV_SEARCH
	bool bSearch = true;
#endif
	PvLine line = PvLine();
#if CHECK_MOVE_GEN_ERRORS
	Board old(board);
#endif

	if (board.isDraw()) {
		return 0;
	}

	if (depth==0||stop()) {
		//return evaluator.evaluate(board);
		int score = qSearch(board, alpha, beta, maxQuiescenceSearchDepth, &line);
		SearchAgent::getInstance()->hashPut(board,score,depth,0,SearchAgent::LOWER,MoveIterator::Move());
#if SHOW_STATS
		stats.ttLower++;
#endif
		return score;
	}

	_nodes++;
	int oldAlpha = alpha;
	int score = 0;
	SearchAgent::HashData hashData;
	if (SearchAgent::getInstance()->hashGet(board.getKey(), hashData)) {
		if (hashData.depth>=depth) {
			if ((hashData.flag == SearchAgent::UPPER && hashData.value <= alpha) ||
					(hashData.flag == SearchAgent::LOWER && hashData.value >= beta) ||
					(hashData.flag == SearchAgent::EXACT)) {
#if SHOW_STATS
				stats.ttHits++;
#endif
				score = hashData.value;
				if (score >= maxScore) {
					score -= (_depth-depth);
				} else if (score <= -maxScore) {
					score += (_depth-depth);
				}
				return score;
			}
		}
	}

	if (!board.isNotLegal() && beta < maxScore &&
			(board.getPiecesByType(WHITE_PAWN)|board.getPiecesByType(BLACK_PAWN)) &&
			allowNullMove ) {

		MoveBackup backup;
		board.doNullMove(backup);
		score = -pvSearch(board, -beta, 1-beta, depth-1, &line, false);
		board.undoNullMove(backup);

		if (score >= beta) {
#if SHOW_STATS
			stats.nullMoveHits++;
#endif
			if (score >= maxScore) {
				score -= (_depth-depth);
			} else if (score <= -maxScore) {
				score += (_depth-depth);
			}

			SearchAgent::getInstance()->hashPut(board,score,depth,0,SearchAgent::LOWER,MoveIterator::Move());
			return score;
		}
	}

	MoveIterator moves;
	board.generateAllMoves(moves, board.getSideToMove());
	moves.first();
#if CHECK_MOVE_GEN_ERRORS
	Key key1 = old.generateKey();
#endif

	while (moves.hasNext()) {
		if (stop()) {
			break;
		}

		MoveIterator::Move& move = moves.next();
		MoveBackup backup;
		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

#if CHECK_MOVE_GEN_ERRORS
		Board newBoard(board);
#endif

#if PV_SEARCH
		if ( bSearch ) {
#endif
			score = -pvSearch(board, -beta, -alpha, depth-1, &line, true);
#if PV_SEARCH
		} else {
			score = -pvSearch(board, -alpha-1, -alpha, depth-1, &line, true);
			if ( score > alpha ) {
				score = -pvSearch(board, -beta, -alpha, depth-1, &line, true);
			}
		}
#endif
#if DEBUG_AB
		std::cout << "(AB) score: " << score << " / move: " << move.toString() << " / depth " << depth << std::endl;
		std::string pad=" ";
		pad.append((20-depth)*4, ' ');
		board.printBoard(pad);
#endif

		board.undoMove(backup);

#if CHECK_MOVE_GEN_ERRORS
		Key key2 = board.generateKey();
		if (key1!=key2) {
			errorCount++;
			std::cout << "CRITICAL - Error in undoMove: restored board differs from original " << std::endl;
			std::cout << "Original board: " << std::endl;
			old.printBoard();
			std::cout << "Board with move: " << std::endl;
			newBoard.printBoard();
			std::cout << "Board with undone move: " << std::endl;
			board.printBoard();
			std::cout << move.toString() << std::endl;
			std::cout << "End - Error in undoMove " << std::endl;
			if (errorCount>EXIT_PROGRAM_THRESHOLD) {
				std::cout << "Error count exceed threshold: " << errorCount << std::endl;
				exit(1);
			}
		}
#endif
		if( score >= beta ) {
#if SHOW_STATS
			stats.ttLower++;
#endif

			if (score >= maxScore) {
				score -= (_depth-depth);
			} else if (score <= -maxScore) {
				score += (_depth-depth);
			}

			SearchAgent::getInstance()->hashPut(board,score,depth,0,SearchAgent::LOWER,move);
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
#if PV_SEARCH
			bSearch = false;
#endif
			updatePv(pv, line, move);

		}
	}
#if SHOW_STATS
	if (alpha>oldAlpha) {
		stats.ttExact++;
	} else {
		stats.ttUpper++;
	}
#endif

	score = alpha;
	if (score >= maxScore) {
		score -= (_depth-depth);
	} else if (score <= -maxScore) {
		score += (_depth-depth);
	}

	SearchAgent::getInstance()->hashPut(board,score,depth,0,(alpha>oldAlpha ? SearchAgent::EXACT : SearchAgent::UPPER),pv->moves[0]);

	return alpha;

}

//quiescence search
int SimplePVSearch::qSearch(Board& board, int alpha, int beta, uint32_t depth, PvLine* pv) {

	_nodes++;

	if (board.isDraw()) {
		return 0;
	}

	int standPat = evaluator.evaluate(board);

	if(standPat>=beta||depth==0||stop()) {
#if DEBUG_QS
		std::cout << "(QS) beta: " << beta << " / eval: " << standPat << " / depth " << depth << std::endl;
		std::string pad=" ";
		pad.append((maxQuiescenceSearchDepth-depth+_depth)*4, ' ');
		board.printBoard(pad);
#endif
		pv->index=0;
		return standPat;
	}

	if( alpha < standPat ) {
		alpha = standPat;
	}

	PvLine line = PvLine();
	MoveIterator moves;
	board.generateCaptures(moves, board.getSideToMove());
	moves.first();
	while (moves.hasNext())  {
		if (stop()) {
			break;
		}
		MoveIterator::Move& move = moves.next();
		MoveBackup backup;

		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

		int score = -qSearch(board, -beta, -alpha, depth-1, &line);

#if DEBUG_QS
		std::cout << "(QS) score: " << alpha << " / move: " << move.toString() << " / depth " << depth << std::endl;
		std::string pad=" ";
		pad.append((depth)*4, ' ');
		board.printBoard(pad);
#endif

		board.undoMove(backup);

		if( score >= beta ) {
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			updatePv(pv, line, move);
		}

	}

	return alpha;
}

const bool SimplePVSearch::stop() {

	return !SearchAgent::getInstance()->getSearchInProgress() || timeIsUp();

}

const bool SimplePVSearch::timeIsUp() {

	if ( _searchFixedDepth || _infinite ) {
		return false;
	}

	return !SearchAgent::getInstance()->getSearchInProgress() || ((getTickCount()-_startTime)>=_timeToSearch);

}

const std::string SimplePVSearch::pvLineToString(const PvLine* pv) {
	std::string result="";
	for(int x=0;x<pv->index;x++) {
		result += " ";
		result += pv->moves[x].toString();
	}
	return result;
}

void SimplePVSearch::updatePv(PvLine* pv, PvLine& line, MoveIterator::Move& move) {
	pv->moves[0] = move;
	memcpy(pv->moves + 1, line.moves, line.index * sizeof(MoveIterator::Move));
	pv->index = line.index + 1;
}





