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
 * Evaluator.h
 *
 *  Created on: Jan 1, 2010
 *      Author: bhlangonijr
 */

#ifndef EVALUATOR_H_
#define EVALUATOR_H_

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "inline.h"
#include "board.h"

const int KNIGHT_MOBILITY_BONUS = 6;
const int BISHOP_MOBILITY_BONUS = 4;
const int ROOK_MOBILITY_BONUS = 2;

const int KNIGHT_TROPISM_BONUS = 2;
const int BISHOP_TROPISM_BONUS = 3;
const int ROOK_TROPISM_BONUS = 4;
const int QUEEN_TROPISM_BONUS = 5;

const int KNIGHT_ATTACK_BONUS = 5;
const int BISHOP_ATTACK_BONUS = 7;
const int ROOK_ATTACK_BONUS = 9;
const int QUEEN_ATTACK_BONUS = 11;

const int bishopPairBonus = 15;
const int gameSize=35;
const int maxPieces=32;
const int maxGamePhase=gameSize*maxPieces;

// default material values
const int defaultMaterialValues[ALL_PIECE_TYPE_BY_COLOR] = {100, 325, 325, 500, 975, 10000, 100, 325, 325, 500, 975, 10000, 0};

const int passedPawnBonus[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,0,0,1,20,50,190,1},
		{1,190,50,20,1,0,0,0},
		{0,0,0,0,0,0,0,0}
};

// opening and middlegame piece square table
const int defaultPieceSquareTable[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE]={

		{ // white pawn
				0,  0,  0,  0,  0,  0,  0,  0,
				5,  10, 10,-20,-20, 10, 10, 5,
				5, -5,-10,  0,  0, -10, -5, 5,
				0,  -5,-10, 20, 20, -10,-5,  0,
				0,  0, 0, 5, 	5, 0,  0,  0,
				0,  0,  0,  0,  0,  0,  0,  0,
				5,  5,  5,  5,  5,  5,  5,  5,
				0,  0,  0,  0,  0,  0,  0,  0,
		},
		{//white knight
				-50,-40,-30,-30,-30,-30,-40,-50,
				-40,-20,  0,  5,  5,  0,-20,-40,
				-30,  5, 10, 15, 15, 10,  5,-30,
				-30,  0, 15, 20, 20, 15,  0,-30,
				-30,  5, 15, 20, 20, 15,  5,-30,
				-30,  0, 10, 15, 15, 10,  0,-30,
				-40,-20,  0,  0,  0,  0,-20,-40,
				-50,-40,-30,-30,-30,-30,-40,-50,
		},
		{//white bishop
				-20,-10,-10,-10,-10,-10,-10,-20,
				-10,  5,  0,  0,  0,  0,  5,-10,
				-10, 10, 10, 10, 10, 10, 10,-10,
				-10,  0, 10, 10, 10, 10,  0,-10,
				-10,  5,  5, 10, 10,  5,  5,-10,
				-10,  0,  5, 10, 10,  5,  0,-10,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-20,-10,-10,-10,-10,-10,-10,-20,
		},
		{//white rook
				0,  0,  0,  0,  0,  0,  0,  0,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				5, 10, 10, 10, 10, 10, 10,  5,
				0,  0,  0,  5,  5,  0,  0,  0,
		},
		{//white queen
				-20,-10,-10, -5, -5,-10,-10,-20,
				-10,  0,  5,  0,  0,  0,  0,-10,
				-10,  5,  5,  5,  5,  5,  0,-10,
				0,  0,  5,  5,  5,  5,  0, -5,
				-5,  0,  5,  5,  5,  5,  0, -5,
				-10,  0,  5,  5,  5,  5,  0,-10,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-20,-10,-10, -5, -5,-10,-10,-20,
		},
		{//white king
				20, 30, 10,  0,  0, 10, 30, 20,
				20, 20,  0,  0,  0,  0, 20, 20,
				-10,-20,-20,-20,-20,-20,-20,-10,
				-20,-30,-30,-40,-40,-30,-30,-20,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
		},
		{//black pawn
				0,  0,  0,  0,  0,  0,  0,  0,
				5,  5,  5,  5,  5,  5,  5,  5,
				0,  0,  0,  0,  0,  0,  0,  0,
				0,  0, 0,  5,  5, 0,  0,  0,
				0, -5,-10, 20, 20, -10, -5,  0,
				5, -5,-10,  0,  0,-10, -5,  5,
				5, 10, 10,-20,-20, 10, 10,  5,
				0,  0,  0,  0,  0,  0,  0,  0,
		},
		{//black knight
				-50,-40,-30,-30,-30,-30,-40,-50,
				-40,-20,  0,  0,  0,  0,-20,-40,
				-30,  0, 10, 15, 15, 10,  0,-30,
				-30,  5, 15, 20, 20, 15,  5,-30,
				-30,  0, 15, 20, 20, 15,  0,-30,
				-30,  5, 10, 15, 15, 10,  5,-30,
				-40,-20,  0,  5,  5,  0,-20,-40,
				-50,-40,-30,-30,-30,-30,-40,-50,
		},
		{//black bishop
				-20,-10,-10,-10,-10,-10,-10,-20,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-10,  0,  5, 10, 10,  5,  0,-10,
				-10,  5,  5, 10, 10,  5,  5,-10,
				-10,  0, 10, 10, 10, 10,  0,-10,
				-10, 10, 10, 10, 10, 10, 10,-10,
				-10,  5,  0,  0,  0,  0,  5,-10,
				-20,-10,-10,-10,-10,-10,-10,-20,
		},
		{//black rook
				0,  0,  0,  0,  0,  0,  0,  0,
				5, 10, 10, 10, 10, 10, 10,  5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				0,  0,  0,  5,  5,  0,  0,  0,
		},
		{//black queen:
				-20,-10,-10, -5, -5,-10,-10,-20,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-10,  0,  5,  5,  5,  5,  0,-10,
				-5,  0,  5,  5,  5,  5,  0, -5,
				0,  0,  5,  5,  5,  5,  0, -5,
				-10,  5,  5,  5,  5,  5,  0,-10,
				-10,  0,  5,  0,  0,  0,  0,-10,
				-20,-10,-10, -5, -5,-10,-10,-20,
		},
		{//black king
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-20,-30,-30,-40,-40,-30,-30,-20,
				-10,-20,-20,-20,-20,-20,-20,-10,
				20, 20,  0,  0,  0,  0, 20, 20,
				20, 30, 10,  0,  0, 10, 30, 20,
		},
		{ // empty
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0
		}

};

// end game piece square table
const int endGamePieceSquareTable[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE]={

		{ // white pawn
				0,  0,  0,  0,  0,  0,  0,  0,
				1,  1,  1, -2, -2,  1,  1,  1,
				1, -1, -1,  0,  0, -1, -1,  1,
				0,  5,  5, 10, 10,  5,  5,  0,
				5,  5, 10, 15, 15, 10,  5,  5,
				10, 10, 10, 20, 20, 10, 10, 10,
				11, 15, 15, 25, 25, 15, 15, 11,
				0,  0,  0,  0,  0,  0,  0,  0,
		},
		{//white knight
				-50,-40,-30,-30,-30,-30,-40,-50,
				-40,-20,  0,  5,  5,  0,-20,-40,
				-30,  5, 10, 15, 15, 10,  5,-30,
				-30,  0, 15, 20, 20, 15,  0,-30,
				-30,  5, 15, 20, 20, 15,  5,-30,
				-30,  0, 10, 15, 15, 10,  0,-30,
				-40,-20,  0,  0,  0,  0,-20,-40,
				-50,-40,-30,-30,-30,-30,-40,-50,
		},
		{//white bishop
				-20,-10,-10,-10,-10,-10,-10,-20,
				-10,  5,  0,  0,  0,  0,  5,-10,
				-10, 10, 10, 10, 10, 10, 10,-10,
				-10,  0, 10, 10, 10, 10,  0,-10,
				-10,  5,  5, 10, 10,  5,  5,-10,
				-10,  0,  5, 10, 10,  5,  0,-10,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-20,-10,-10,-10,-10,-10,-10,-20,
		},
		{//white rook
				0,  0,  0,  0,  0,  0,  0,  0,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				5, 10, 10, 10, 10, 10, 10,  5,
				0,  0,  0,  5,  5,  0,  0,  0,
		},
		{//white queen
				-20,-10,-10, -5, -5,-10,-10,-20,
				-10,  0,  5,  0,  0,  0,  0,-10,
				-10,  5,  5,  5,  5,  5,  0,-10,
				0,  0,  5,  5,  5,  5,  0, -5,
				-5,  0,  5,  5,  5,  5,  0, -5,
				-10,  0,  5,  5,  5,  5,  0,-10,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-20,-10,-10, -5, -5,-10,-10,-20,
		},
		{//white king
				0,  10, 20, 30, 30, 20, 10,  0,
				10, 20, 30, 40, 40, 30, 20, 10,
				20, 30, 40, 50, 50, 40, 30, 20,
				30, 40, 50, 60, 60, 50, 40, 30,
				30, 40, 50, 60, 60, 50, 40, 30,
				20, 30, 40, 50, 50, 40, 30, 20,
				10, 20, 30, 40, 40, 30, 20, 10,
				0, 10, 20, 30, 30, 20, 10,  0,
		},
		{//black pawn
				0,  0,  0,  0,  0,  0,  0,  0,
				11, 15, 15, 25, 25, 15, 15, 11,
				10, 10, 10, 20, 20, 10, 10, 10,
				5,  5, 10, 15, 15, 10,  5,  5,
				0,  5,  5, 10, 10,  5,  5,  0,
				1, -1, -1,  0,  0,-1, -1,  1,
				1,  1,  1, -2, -2, 1,  1,  1,
				0,  0,  0,  0,  0, 0,  0,  0,
		},
		{//black knight
				-50,-40,-30,-30,-30,-30,-40,-50,
				-40,-20,  0,  0,  0,  0,-20,-40,
				-30,  0, 10, 15, 15, 10,  0,-30,
				-30,  5, 15, 20, 20, 15,  5,-30,
				-30,  0, 15, 20, 20, 15,  0,-30,
				-30,  5, 10, 15, 15, 10,  5,-30,
				-40,-20,  0,  5,  5,  0,-20,-40,
				-50,-40,-30,-30,-30,-30,-40,-50,
		},
		{//black bishop
				-20,-10,-10,-10,-10,-10,-10,-20,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-10,  0,  5, 10, 10,  5,  0,-10,
				-10,  5,  5, 10, 10,  5,  5,-10,
				-10,  0, 10, 10, 10, 10,  0,-10,
				-10, 10, 10, 10, 10, 10, 10,-10,
				-10,  5,  0,  0,  0,  0,  5,-10,
				-20,-10,-10,-10,-10,-10,-10,-20,
		},
		{//black rook
				0,  0,  0,  0,  0,  0,  0,  0,
				5, 10, 10, 10, 10, 10, 10,  5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				0,  0,  0,  5,  5,  0,  0,  0,
		},
		{//black queen:
				-20,-10,-10, -5, -5,-10,-10,-20,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-10,  0,  5,  5,  5,  5,  0,-10,
				-5,  0,  5,  5,  5,  5,  0, -5,
				0,  0,  5,  5,  5,  5,  0, -5,
				-10,  5,  5,  5,  5,  5,  0,-10,
				-10,  0,  5,  0,  0,  0,  0,-10,
				-20,-10,-10, -5, -5,-10,-10,-20,
		},
		{//black king
				0,  10, 20, 30, 30, 20, 10,  0,
				10, 20, 30, 40, 40, 30, 20, 10,
				20, 30, 40, 50, 50, 40, 30, 20,
				30, 40, 50, 60, 60, 50, 40, 30,
				30, 40, 50, 60, 60, 50, 40, 30,
				20, 30, 40, 50, 50, 40, 30, 20,
				10, 20, 30, 40, 40, 30, 20, 10,
				0, 10, 20, 30, 30, 20, 10,  0
		},
		{ // empty
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0
		}

};

class Evaluator {
public:

	enum GamePhase {
		OPENING=int(maxGamePhase*0.3), MIDDLEGAME=int(maxGamePhase*0.6), ENDGAME=int(maxGamePhase*0.6+1)
	};

	Evaluator();
	virtual ~Evaluator();
	const int evaluate(Board& board);
	const int quickEvaluate(Board& board);
	const int evalMaterial(Board& board, PieceColor color);
	const int evalPieces(Board& board, PieceColor color);
	const int evalMobility(Board& board, PieceColor color);
	const int evalDevelopment(Board& board, PieceColor color);
	const int evalImbalances(Board& board, PieceColor color);
	const bool isPawnPassed(Board& board, const PieceColor color, const Square from);
	const void setGameStage(const GamePhase phase);
	const GamePhase getGameStage();
	const GamePhase predictGameStage(Board& board);
	const int see(Board& board, MoveIterator::Move& move);

	inline const int interpolate(const int first, const int second, const int position) {

		if (position > gameSize) {
			return second;
		}

		return (first*position)/maxGamePhase+(second*(maxGamePhase-position)/maxGamePhase);

	}

	inline const int getPieceMaterialValue(const PieceTypeByColor piece) {
		return defaultMaterialValues[piece];
	}

	inline const int getPieceSquareValue(const PieceTypeByColor piece, const Square square) {

		const int egValue = endGamePieceSquareTable[piece][square];
		const int mgValue = defaultPieceSquareTable[piece][square];

		return interpolate(mgValue,egValue,this->getGameStage());
	}

private:
	Bitboard getLeastValuablePiece(Board& board, Bitboard attackers, PieceColor& color, PieceTypeByColor& piece);
	GamePhase gamePhase;
};

// main eval function
inline const int Evaluator::evaluate(Board& board) {

	const PieceColor side = board.getSideToMove();
	const PieceColor other = board.flipSide(board.getSideToMove());

	this->setGameStage(this->predictGameStage(board));

	int material = evalMaterial(board, side) - evalMaterial(board, other);
	int pieces = evalPieces(board, side) - evalPieces(board, other);
	int development = evalDevelopment(board, side) - evalDevelopment(board, other);
	int imbalances = evalImbalances(board, side) - evalImbalances(board, other);
	int mobility = evalMobility(board, side) - evalMobility(board, other);

	return material+mobility+pieces+development+imbalances;
}

// quick eval function
inline const int Evaluator::quickEvaluate(Board& board) {

	const PieceColor side = board.getSideToMove();
	const PieceColor other = board.flipSide(board.getSideToMove());

	int material = evalMaterial(board, side) - evalMaterial(board, other);
	int pieces = evalPieces(board, side) - evalPieces(board, other);

	return material+pieces;
}

// material eval function
inline const int Evaluator::evalMaterial(Board& board, PieceColor color) {

	const int first = board.makePiece(color,PAWN);
	const int last = board.makePiece(color,KING);
	int material = 0;

	for(int pieceType = first; pieceType <= last; pieceType++) {
		int count = board.getPieceCountByType(PieceTypeByColor(pieceType));
		if (count > 0) {
			material += count * getPieceMaterialValue(PieceTypeByColor(pieceType));
		}
	}

	return material;
}


// king eval function
inline const int Evaluator::evalPieces(Board& board, PieceColor color) {

	const PieceColor other = board.flipSide(color);
	const int DONE_CASTLE_BONUS=       +(board.getPiecesByType(board.makePiece(other,QUEEN))) ? 20 : 10;
	const int DOUBLED_PAWN_PENALTY =   -10;
	const int ISOLATED_PAWN_PENALTY =  -15;
	const int BACKWARD_PAWN_PENALTY =  -5;
	int count=0;

	// king
	if (board.isCastleDone(color)) {
		count= DONE_CASTLE_BONUS;
	}

	const Bitboard pawns = board.getPiecesByType(board.makePiece(color,PAWN));

	//pawns - penalyze doubled & isolated pawns
	if (pawns) {
		Bitboard pieces=pawns;
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {
			const Bitboard pawn = squareToBitboard[from];
			const Bitboard allButThePawn =(pawns^pawn);

			if (fileAttacks[squareFile[from]]&allButThePawn) {
				count += DOUBLED_PAWN_PENALTY;
			}

			if (isPawnPassed(board,color,from)) {
				count += passedPawnBonus[color][squareRank[from]];
			}

			if (!(neighborFiles[from]&allButThePawn)) {
				count += ISOLATED_PAWN_PENALTY;
			} else {
				if (!(adjacentSquares[from]&allButThePawn)) {
					count += BACKWARD_PAWN_PENALTY;
				}
			}

			from = extractLSB(pieces);
		}
	}

	return count;
}

// mobility eval function
inline const int Evaluator::evalMobility(Board& board, PieceColor color) {

	const Bitboard otherKingBB = board.getPiecesByType(board.makePiece(board.flipSide(color),KING));
	const Square otherKingSq = bitboardToSquare(otherKingBB);
	const Bitboard knights = board.getPiecesByType(board.makePiece(color,KNIGHT));
	const Bitboard bishops = board.getPiecesByType(board.makePiece(color,BISHOP));
	const Bitboard rooks = board.getPiecesByType(board.makePiece(color,ROOK));
	const Bitboard queens = board.getPiecesByType(board.makePiece(color,QUEEN));

	Bitboard pieces = EMPTY_BB;
	Bitboard knightAttacks = EMPTY_BB;
	Bitboard bishopAttacks = EMPTY_BB;
	Bitboard rookAttacks = EMPTY_BB;
	Bitboard queenAttacks = EMPTY_BB;

	Square from = NONE;
	int count=0;
	int kingThreat=0;

	pieces = knights;
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const int delta = inverseSquareDistance(from,otherKingSq);
		const Bitboard attacks = board.getKnightAttacks(from);
		const int kingAttacked=attacks&otherKingBB;
		kingThreat += delta*(kingAttacked?KNIGHT_ATTACK_BONUS:KNIGHT_TROPISM_BONUS);
		knightAttacks |= attacks;
		from = extractLSB(pieces);
	}

	pieces = bishops;
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const int delta = inverseSquareDistance(from,otherKingSq);
		const Bitboard attacks = board.getBishopAttacks(from);
		const int kingAttacked=attacks&otherKingBB;
		kingThreat += delta*(kingAttacked?BISHOP_ATTACK_BONUS:BISHOP_TROPISM_BONUS);
		bishopAttacks |= attacks;
		from = extractLSB(pieces);
	}

	pieces = rooks;
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const int delta = inverseSquareDistance(from,otherKingSq);
		const Bitboard attacks = board.getRookAttacks(from);
		const int kingAttacked=attacks&otherKingBB;
		kingThreat += delta*(kingAttacked?ROOK_ATTACK_BONUS:ROOK_TROPISM_BONUS);
		rookAttacks |= attacks;
		from = extractLSB(pieces);
	}

	pieces = queens;
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const int delta = inverseSquareDistance(from,otherKingSq);
		const Bitboard attacks = board.getQueenAttacks(from);
		const int kingAttacked=attacks&otherKingBB;
		kingThreat += delta*(kingAttacked?QUEEN_ATTACK_BONUS:QUEEN_TROPISM_BONUS);
		queenAttacks |= attacks;
		from = extractLSB(pieces);
	}

	count+=_BitCount(knightAttacks)*KNIGHT_MOBILITY_BONUS;
	count+=_BitCount(bishopAttacks)*BISHOP_MOBILITY_BONUS;
	count+=_BitCount(rookAttacks)*ROOK_MOBILITY_BONUS;
	count+=_BitCount(queenAttacks);

	return count+kingThreat;
}

// piece-square eval function
inline const int Evaluator::evalDevelopment(Board& board, PieceColor color) {

	const int first = board.makePiece(color,PAWN);
	const int last = board.makePiece(color,KING);
	int bonus = 0;

	for(int pieceType = first; pieceType <= last; pieceType++) {
		Bitboard pieces = board.getPiecesByType(PieceTypeByColor(pieceType));
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {
			bonus += getPieceSquareValue(PieceTypeByColor(pieceType),Square(from));
			from = extractLSB(pieces);
		}
	}

	return bonus;
}

// imbalances eval function
inline const int Evaluator::evalImbalances(Board& board, PieceColor color) {

	int count=0;

	Bitboard bishop = board.getPiecesByType(board.makePiece(color,BISHOP));

	if ((bishop & WHITE_SQUARES) && (bishop & BLACK_SQUARES)) {
		count += bishopPairBonus;
	} else {
		count -= bishopPairBonus;
	}

	// TODO implement more imbalances
	return count;
}

// verify if pawn is passer
inline const bool Evaluator::isPawnPassed(Board& board, const PieceColor color, const Square from) {
	const Bitboard pawns = board.getPieceType(board.makePiece(board.flipSide(color),PAWN)) |
			board.getPieceType(board.makePiece(color,PAWN));

	if ((color==WHITE && squareRank[from]<RANK_3) ||
			(color==BLACK && squareRank[from]>RANK_6)) {
		return false;
	}

	return !(passedMask[color][from]&pawns);
}

// static exchange evaluation
inline const int Evaluator::see(Board& board, MoveIterator::Move& move) {

	const int gainTableSize=32;
	PieceColor side = board.getPieceColorBySquare(move.from);
	PieceColor other = board.flipSide(side);
	PieceTypeByColor firstPiece = board.getPieceBySquare(move.from);
	PieceTypeByColor secondPiece = board.getPieceBySquare(move.to);

	if (secondPiece==EMPTY) {
		return 0;
	}

	const Bitboard sidePawn =  board.getPiecesByType(board.makePiece(side,PAWN));
	const Bitboard sideKnight =  board.getPiecesByType(board.makePiece(side,KNIGHT));
	const Bitboard sideBishop =  board.getPiecesByType(board.makePiece(side,BISHOP));
	const Bitboard sideRook =  board.getPiecesByType(board.makePiece(side,ROOK));
	const Bitboard sideQueen =  board.getPiecesByType(board.makePiece(side,QUEEN));
	const Bitboard sideKing =  board.getPiecesByType(board.makePiece(side,KING));

	const Bitboard otherPawn =  board.getPiecesByType(board.makePiece(other,PAWN));
	const Bitboard otherKnight =  board.getPiecesByType(board.makePiece(other,KNIGHT));
	const Bitboard otherBishop =  board.getPiecesByType(board.makePiece(other,BISHOP));
	const Bitboard otherRook =  board.getPiecesByType(board.makePiece(other,ROOK));
	const Bitboard otherQueen =  board.getPiecesByType(board.makePiece(other,QUEEN));
	const Bitboard otherKing =  board.getPiecesByType(board.makePiece(other,KING));

	const Bitboard bishopAttacks =  board.getBishopAttacks(move.to);
	const Bitboard rookAttacks =  board.getRookAttacks(move.to);
	const Bitboard knightAttacks =  board.getKnightAttacks(move.to);
	const Bitboard pawnAttacks =  board.getPawnAttacks(move.to);
	const Bitboard kingAttacks =  board.getKingAttacks(move.to);

	const Bitboard rooks = sideRook | otherRook;
	const Bitboard bishops = sideBishop | otherBishop;
	const Bitboard queens = sideQueen | otherQueen;

	const Bitboard bishopAndQueen =  rooks | queens;
	const Bitboard rookAndQueen =  bishops | queens;

	Bitboard occupied = board.getAllPieces();

	Bitboard attackers =
			(bishopAttacks & bishopAndQueen) |
			(rookAttacks & rookAndQueen) |
			(knightAttacks & (sideKnight | otherKnight)) |
			(pawnAttacks & (sidePawn | otherPawn)) |
			(kingAttacks & (sideKing | otherKing));


	int idx = 0;
	int gain[gainTableSize];
	Bitboard fromPiece = squareToBitboard[move.from];
	PieceColor sideToMove = side;
	Bitboard allAttackers = EMPTY_BB;

	gain[idx] = defaultMaterialValues[secondPiece];

	while (true) {

		allAttackers |= attackers;
		idx++;
		gain[idx]  = defaultMaterialValues[firstPiece] - gain[idx-1];
		attackers ^= fromPiece;
		occupied  ^= fromPiece;
		Bitboard moreAttackers = (bishopAndQueen | rookAndQueen) & (~allAttackers);

		if (moreAttackers) {
			Bitboard findMoreAttackers =
					(board.getBishopAttacks(move.to,occupied) |
							board.getRookAttacks(move.to,occupied));
			findMoreAttackers &= moreAttackers;

			if (findMoreAttackers) {
				attackers |= findMoreAttackers;
			}
		}

		sideToMove=board.flipSide(sideToMove);
		fromPiece  = getLeastValuablePiece (board, attackers, sideToMove, firstPiece);

		if (!fromPiece) {
			break;
		}
	}

	while (--idx) {
		gain[idx-1]= -MAX(-gain[idx-1], gain[idx]);
	}

	return gain[0];
}

inline Bitboard Evaluator::getLeastValuablePiece(Board& board, Bitboard attackers, PieceColor& color, PieceTypeByColor& piece) {

	const int first = board.makePiece(color,PAWN);
	const int last = board.makePiece(color,KING);

	for(register int pieceType = first; pieceType <= last; pieceType++) {
		Bitboard pieces = board.getPiecesByType(PieceTypeByColor(pieceType)) & attackers;
		if (pieces) {
			piece = PieceTypeByColor(pieceType);
			return pieces & -pieces;
		}
	}

	return EMPTY_BB;
}

#endif /* EVALUATOR_H_ */

