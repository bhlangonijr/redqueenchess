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
		PV_MOVE, TT_MOVE, GOOD_CAPTURE, PROMO_CAPTURE, PROMO_NONCAPTURE, EQUAL_CAPTURE, KILLER1, KILLER2, NON_CAPTURE, BAD_CAPTURE
	};

	// Move representation
	struct Move {

		Move() : from(NONE), to(NONE), score(DEFAULT_SCORE), type(PV_MOVE)
		{}

		Move(const Square fromSquare, const Square toSquare, const PieceTypeByColor piece) :
			from(fromSquare), to(toSquare), promotionPiece(piece), score(DEFAULT_SCORE), type(PV_MOVE)
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

		Data(): size(0), idx(0) {};
		Data(Data& data) : list(data.list), size(data.size), idx(data.idx){};
		Move list[MOVE_LIST_MAX_SIZE];
		size_t size;
		size_t idx;

	};

	const void add(const Move& move) ;

	const void add(const Square from, const Square to, const PieceTypeByColor piece);

	const void add(const Square from, const Square to, const PieceTypeByColor piece, const MoveType type);

	const void remove(const size_t index);

	const bool hasNext();

	Move& next();

	const void first();

	const size_t size();

	void sort();

	const Move& get(const size_t index);

	const void bringToTop();

	const void sortOne();

	void operator()(Data& data) {
		_data=data;
	}

	void operator()(MoveIterator& moves) {
		_data=moves.getData();
	}

	MoveIterator(Data& data);

	MoveIterator();

	inline Data& getData() {
		return _data;
	}

	virtual ~MoveIterator();
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

inline const bool MoveIterator::hasNext() {
	return _data.idx<_data.size;
}

inline MoveIterator::Move& MoveIterator::next() {
	return _data.list[_data.idx++];
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

inline const void MoveIterator::bringToTop() {

	if (_data.idx==0) {
		return;
	}

	Move tmp=_data.list[0];
	_data.list[0]=_data.list[1];
	_data.list[1]=tmp;

	if (_data.idx>1) {
		const size_t index = _data.idx-1;
		tmp=_data.list[0];
		_data.list[0]=_data.list[index];
		_data.list[index]=tmp;
	};

}

inline const void MoveIterator::sortOne() {

	const size_t index = _data.idx-1;
	for(size_t x=0;x<_data.idx;x++) {

		if (_data.list[index].score > _data.list[x].score) {
			Move tmp=_data.list[x];
			_data.list[x]=_data.list[index];
			_data.list[index]=tmp;
			break;

		}

	}

}



#endif /* MOVEITERATOR_H_ */
