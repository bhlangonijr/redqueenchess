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
 * MoveIterator.h
 *
 *  Created on: Dec 26, 2009
 *      Author: bhlangonijr
 */

#ifndef MOVEITERATOR_H_
#define MOVEITERATOR_H_

#include "Bitboard.h"

#define MOVE_LIST_MAX_SIZE 128

class MoveIterator {
public:

	// Move representation
	struct Move {

		Move() : from(NONE), to(NONE), score(0)
		{}
		Move(const Square fromSquare, const Square toSquare, const PieceTypeByColor piece) :
			from(fromSquare), to(toSquare), promotionPiece(piece), score(0)
			{}

		Move(const Move& move) :
			from(move.from), to(move.to), promotionPiece(move.promotionPiece), score(move.score)
			{}

		Square from;
		Square to;
		PieceTypeByColor promotionPiece;
		int score;

		void copy(const Move& move) {
			from=move.from;
			to=move.to;
			promotionPiece=move.promotionPiece;
			score=move.score;
		}

		const std::string toString() const {
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

	const void add(const Move move) {
		list[_size++]=move;
	}

	const void add(const Square from, const Square to, const PieceTypeByColor piece) {
		list[_size++]=Move(from,to,piece);
	}

	const void remove(const size_t index) {
		for (size_t x=index;x<_size-1;x++) {
			list[x]=list[x+1];
		}
		_size--;
	}

	const bool hasNext() {
		return idx<_size;
	}

	Move& next() {
		return list[idx++];
	}

	Move& get() {
		return list[idx];
	}

	const void first() {
		idx=0;
	}

	const size_t size() {
		return _size;
	}

	void sort();

	const Move& get(const size_t index) {
		return list[index];
	}

	MoveIterator();
	virtual ~MoveIterator();
private:

	Move list[MOVE_LIST_MAX_SIZE];
	size_t _size;
	size_t idx;

};

#endif /* MOVEITERATOR_H_ */
