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
 * Evaluator.cpp
 *
 *  Created on: Jan 1, 2010
 *      Author: bhlangonijr
 */

#include "Evaluator.h"

Evaluator::Evaluator() : gamePhase(OPENING) {
	setGameStage(gamePhase);
}

Evaluator::~Evaluator() {

}

const void Evaluator::setGameStage(const GamePhase phase) {

	if (phase==ENDGAME) {
		memcpy(materialValues, endGameMaterialValues, ALL_PIECE_TYPE_BY_COLOR*sizeof(int));
		memcpy(pieceSquareTable, endGamePieceSquareTable, ALL_SQUARE*ALL_PIECE_TYPE_BY_COLOR*sizeof(int));

	} else {
		memcpy(materialValues, defaultMaterialValues, ALL_PIECE_TYPE_BY_COLOR*sizeof(int));
		memcpy(pieceSquareTable, defaultPieceSquareTable, ALL_SQUARE*ALL_PIECE_TYPE_BY_COLOR*sizeof(int));

	}

}

const Evaluator::GamePhase Evaluator::getGameStage(Board& board) {

	// very simple game stage detection
	const int openingMoves=16;
	const int openingPieces=28;
	const int endGameMoves=25;
	const int endGamePieces=14;

	GamePhase phase = MIDDLEGAME;

	int piecesOnBoard =_BitCount(board.getAllPieces());

	if (board.getMoveCounter() >= endGameMoves &&
			piecesOnBoard <= endGamePieces) {
		phase = ENDGAME;
	} else if (board.getMoveCounter() <= openingMoves &&
			piecesOnBoard >= openingPieces) {
		phase = OPENING;
	}

	return phase;
}




