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
 * Evaluator.h
 *
 *  Created on: Jan 1, 2010
 *      Author: bhlangonijr
 */

#ifndef EVALUATOR_H_
#define EVALUATOR_H_

#include <string.h>
#include "Board.h"

static const int defaultMaterialValues[ALL_PIECE_TYPE_BY_COLOR] = {100, 325, 325, 500, 975, 10000, 100, 325, 325, 500, 975, 10000, 0};

class Evaluator {
public:

	Evaluator();
	virtual ~Evaluator();
	const int evaluate(const Board& board);
	const int evalMaterial(const Board& board);

	const int getPieceMaterialValue(const PieceTypeByColor piece) {
		return materialValues[piece];
	}

	void setPieceMaterialValue(const PieceTypeByColor piece, const int value) {
		materialValues[piece]=value;
	}


private:
	int materialValues[ALL_PIECE_TYPE_BY_COLOR];

};

#endif /* EVALUATOR_H_ */
