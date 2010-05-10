/*
	Redqueen Chess Engine
    Copyright (C) 2008-2010 Ben-Hur Carlos Vieira Langoni Junior

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
 * MoveIterator.h
 *
 *  Created on: Dec 26, 2009
 *      Author: bhlangonijr
 */

#ifndef MOVEITERATOR_H_
#define MOVEITERATOR_H_

#include "bitboard.h"

#define MOVE_LIST_MAX_SIZE 128
#define DEFAULT_SCORE -32000

class MoveIterator {
public:

	enum MoveType {
		UNKNOW, TT_MOVE, GOOD_CAPTURE, PROMO_CAPTURE, PROMO_NONCAPTURE,
		EQUAL_CAPTURE, KILLER1, KILLER2, NON_CAPTURE, BAD_CAPTURE
	};

	enum IteratorStage {
		BEGIN_STAGE, TT_STAGE, INIT_CAPTURE_STAGE, ON_CAPTURE_STAGE,
		KILLER1_STAGE, KILLER2_STAGE, INIT_QUIET_STAGE, ON_QUIET_STAGE, END_STAGE
	};

	// Move representation
	struct Move {

		Move() : from(NONE), to(NONE), score(DEFAULT_SCORE), type(UNKNOW)
		{}

		Move(const Square fromSquare, const Square toSquare, const PieceTypeByColor piece) :
			from(fromSquare), to(toSquare), promotionPiece(piece), score(DEFAULT_SCORE), type(UNKNOW)
		{}

		Move(const Square fromSquare, const Square toSquare, const PieceTypeByColor piece, const MoveType type) :
			from(fromSquare), to(toSquare), promotionPiece(piece), score(DEFAULT_SCORE), type(type)
		{}

		Move(const Move& move) :
			from(move.from), to(move.to), promotionPiece(move.promotionPiece), score(move.score), type(move.type)
		{}

		inline bool operator == (const Move &move) const {
			return ( from==move.from && to==move.to && promotionPiece==move.promotionPiece);
		}

		inline bool operator != (const Move &move) const {
			return (!(from==move.from && to==move.to && promotionPiece==move.promotionPiece));
		}

		Square from;
		Square to;
		PieceTypeByColor promotionPiece;
		int score;
		MoveType type;

		inline const std::string toString() const {
			if (from==NONE || to==NONE ) {
				return "";
			}
			std::string result = squareToString[from]+squareToString[to];
			if (promotionPiece!=EMPTY) {
				result += pieceChar[pieceType[promotionPiece]];
			}
			return result;
		}
	};

	struct Data {

		Data(): size(0), idx(0), saveIdx(0), stage(BEGIN_STAGE) {};
		Data(Data& data) : list(data.list), size(data.size), idx(data.idx), saveIdx(data.saveIdx), stage(BEGIN_STAGE){};
		Move list[MOVE_LIST_MAX_SIZE];
		size_t size;
		size_t idx;
		size_t saveIdx;
		IteratorStage stage;

	};

	const void add(const Move& move) ;

	const void add(const Square from, const Square to, const PieceTypeByColor piece);

	const void add(const Square from, const Square to, const PieceTypeByColor piece, const MoveType type);

	const void remove(const size_t index);

	const void clear();

	const bool hasNext();

	const void bookmark();

	const int goToBookmark();

	const bool end();

	Move& next();

	Move& selectBest();

	const void first();

	const size_t size();

	void sort();

	void sort(const int after);

	const IteratorStage getStage() {
		return _data.stage;
	}

	const void setStage(IteratorStage stage) {
		_data.stage=stage;
	}

	bool goNextStage() {
		if (_data.stage<END_STAGE) {
			_data.stage = IteratorStage(int(_data.stage) + 1 );
			return true;
		}
		return false;
	}

	const Move& get(const size_t index);

	void operator()(Data& data) {
		_data=data;
	}

	void operator()(MoveIterator& moves) {
		_data=moves.getData();
	}

	inline Data& getData() {
		return _data;
	}

	MoveIterator();

	MoveIterator(Data& data);

	MoveIterator(const MoveIterator&);

	virtual ~MoveIterator();

protected:

	MoveIterator& operator= (const MoveIterator&);

private:

	Data& _data;
	Data internal;

};

inline const void MoveIterator::add(const Move& move) {
	_data.list[_data.size++]=move;
}

inline const void MoveIterator::add(const Square from, const Square to, const PieceTypeByColor piece) {
	_data.list[_data.size++]=Move(from,to,piece);
}

inline const void MoveIterator::add(const Square from, const Square to, const PieceTypeByColor piece, const MoveType type) {
	_data.list[_data.size++]=Move(from,to,piece,type);
}

inline const void MoveIterator::remove(const size_t index) {
	for (size_t x=index;x<_data.size-1;x++) {
		_data.list[x]=_data.list[x+1];
	}
	_data.size--;
}

inline const void MoveIterator::clear() {
	_data.size=0;
	_data.idx=0;
}

inline const void MoveIterator::bookmark() {
	_data.saveIdx = _data.idx;
}

inline const int MoveIterator::goToBookmark() {
	_data.idx = _data.saveIdx;
	return _data.idx;
}

inline const bool MoveIterator::hasNext() {
	return _data.idx<_data.size;
}

inline const bool MoveIterator::end() {
	return _data.stage==END_STAGE;
}

inline MoveIterator::Move& MoveIterator::next() {
	return _data.list[_data.idx++];
}

inline MoveIterator::Move& MoveIterator::selectBest() {

	/*size_t idxMax = _data.idx;
	for (size_t x=_data.idx+1;x<_data.size;x++) {
		if (_data.list[x].score > _data.list[idxMax].score) {
			idxMax = x;
		}
	}
	if (idxMax != _data.idx) {
		const Move tmp=_data.list[_data.idx];
		_data.list[_data.idx]=_data.list[idxMax];
		_data.list[idxMax]=tmp;
	}*/

	return next();
}

inline const void MoveIterator::first() {
	_data.idx=0;
}

inline const size_t MoveIterator::size() {
	return _data.size;
}

inline const MoveIterator::Move& MoveIterator::get(const size_t index) {
	return _data.list[index];
}

#endif /* MOVEITERATOR_H_ */
