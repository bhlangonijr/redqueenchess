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
 * Evaluator.cpp
 *
 *  Created on: Jan 1, 2010
 *      Author: bhlangonijr
 */

#include "evaluator.h"

Evaluator::Evaluator() : gamePhase(OPENING) {

}

Evaluator::~Evaluator() {

}

const void Evaluator::setGameStage(const GamePhase phase) {

	this->gamePhase = phase;
}

const Evaluator::GamePhase Evaluator::getGameStage() {

	return this->gamePhase;
}

const Evaluator::GamePhase Evaluator::predictGameStage(Board& board) {

	int piecesOnBoard =_BitCount(board.getAllPieces());

	return Evaluator::GamePhase((maxPieces-piecesOnBoard));
}




