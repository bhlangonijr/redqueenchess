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
#include "psqt.h"
#include "bitboard.h"

#define FSQUARE(COLOR,BB,X) (BB&(COLOR==WHITE?upperMaskBitboard[X]:lowerMaskBitboard[X]))&~(rankBB[squareRank[X]])
#define PM(COLOR, X) (FSQUARE(COLOR, (fileBB[squareFile[X]] | neighborFiles[X]), X))
#define VI(LOWER,UPPER,POS) UPPER*POS/(maxGamePhase) + LOWER*((maxGamePhase)-POS)/(maxGamePhase)

#define KM(POS) VI(7,1,POS)
#define BM(POS) VI(5,1,POS)
#define RM(POS) VI(3,1,POS)

#define BK(POS) VI(2,4,POS)
#define RK(POS) VI(3,5,POS)
#define QK(POS) VI(3,6,POS)

#define ZI(POS) VI(3,7,POS)
#define ZA(POS) VI(5,10,POS)

const int DONE_CASTLE_BONUS=       +10;
const int DOUBLED_PAWN_PENALTY =   -10;
const int ISOLATED_PAWN_PENALTY =  -15;
const int BACKWARD_PAWN_PENALTY =  -5;
const int BISHOP_PAIR_BONUS = 	   +15;

const int knightMobilityBonus[maxGamePhase+1] = {
		KM(0),KM(1),KM(2),KM(3),KM(4),KM(5),KM(6),KM(7),KM(8),KM(9),KM(10),KM(11),KM(0),KM(12),KM(13),KM(14),
		KM(15),KM(16),KM(17),KM(18),KM(19),KM(20),KM(21),KM(22),KM(23),KM(24),KM(25),KM(26),KM(27),KM(28),KM(29),KM(30),KM(31)
};

const int bishopMobilityBonus[maxGamePhase+1] = {
		BM(0),BM(1),BM(2),BM(3),BM(4),BM(5),BM(6),BM(7),BM(8),BM(9),BM(10),BM(11),BM(0),BM(12),BM(13),BM(14),
		BM(15),BM(16),BM(17),BM(18),BM(19),BM(20),BM(21),BM(22),BM(23),BM(24),BM(25),BM(26),BM(27),BM(28),BM(29),BM(30),BM(31)
};

const int rookMobilityBonus[maxGamePhase+1] = {
		RM(0),RM(1),RM(2),RM(3),RM(4),RM(5),RM(6),RM(7),RM(8),RM(9),RM(10),RM(11),RM(0),RM(12),RM(13),RM(14),
		RM(15),RM(16),RM(17),RM(18),RM(19),RM(20),RM(21),RM(22),RM(23),RM(24),RM(25),RM(26),RM(27),RM(28),RM(29),RM(30),RM(31)
};

const int bishopKingBonus[maxGamePhase+1] = {
		BK(0),BK(1),BK(2),BK(3),BK(4),BK(5),BK(6),BK(7),BK(8),BK(9),BK(10),BK(11),BK(0),BK(12),BK(13),BK(14),
		BK(15),BK(16),BK(17),BK(18),BK(19),BK(20),BK(21),BK(22),BK(23),BK(24),BK(25),BK(26),BK(27),BK(28),BK(29),BK(30),BK(31)
};

const int rookKingBonus[maxGamePhase+1] =  {
		RK(0),RK(1),RK(2),RK(3),RK(4),RK(5),RK(6),RK(7),RK(8),RK(9),RK(10),RK(11),RK(0),RK(12),RK(13),RK(14),
		RK(15),RK(16),RK(17),RK(18),RK(19),RK(20),RK(21),RK(22),RK(23),RK(24),RK(25),RK(26),RK(27),RK(28),RK(29),RK(30),RK(31)
};

const int queenKingBonus[maxGamePhase+1] = {
		QK(0),QK(1),QK(2),QK(3),QK(4),QK(5),QK(6),QK(7),QK(8),QK(9),QK(10),QK(11),QK(0),QK(12),QK(13),QK(14),
		QK(15),QK(16),QK(17),QK(18),QK(19),QK(20),QK(21),QK(22),QK(23),QK(24),QK(25),QK(26),QK(27),QK(28),QK(29),QK(30),QK(31)
};

const int minorKingZoneAttackBonus[maxGamePhase+1] = {
		ZI(0),ZI(1),ZI(2),ZI(3),ZI(4),ZI(5),ZI(6),ZI(7),ZI(8),ZI(9),ZI(10),ZI(11),ZI(0),ZI(12),ZI(13),ZI(14),
		ZI(15),ZI(16),ZI(17),ZI(18),ZI(19),ZI(20),ZI(21),ZI(22),ZI(23),ZI(24),ZI(25),ZI(26),ZI(27),ZI(28),ZI(29),ZI(30),ZI(31)
};

const int majorKingZoneAttackBonus[maxGamePhase+1] = {
		ZA(0),ZA(1),ZA(2),ZA(3),ZA(4),ZA(5),ZA(6),ZA(7),ZA(8),ZA(9),ZA(10),ZA(11),ZA(0),ZA(12),ZA(13),ZA(14),
		ZA(15),ZA(16),ZA(17),ZA(18),ZA(19),ZA(20),ZA(21),ZA(22),ZA(23),ZA(24),ZA(25),ZA(26),ZA(27),ZA(28),ZA(29),ZA(30),ZA(31)
};

const int passedPawnBonus1[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,25,40,50,60,100,150,0},
		{0,150,80,60,50,40,25,0},
		{}
};

const int passedPawnBonus2[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,25,30,35,40,50,70,0},
		{0,70,50,40,35,30,25,0},
		{}
};

const Bitboard passedMask[ALL_PIECE_COLOR][ALL_SQUARE]= {
		{
				PM(WHITE,A1), PM(WHITE,B1), PM(WHITE,C1), PM(WHITE,D1), PM(WHITE,E1), PM(WHITE,F1), PM(WHITE,G1), PM(WHITE,H1),
				PM(WHITE,A2), PM(WHITE,B2), PM(WHITE,C2), PM(WHITE,D2), PM(WHITE,E2), PM(WHITE,F2), PM(WHITE,G2), PM(WHITE,H2),
				PM(WHITE,A3), PM(WHITE,B3), PM(WHITE,C3), PM(WHITE,D3), PM(WHITE,E3), PM(WHITE,F3), PM(WHITE,G3), PM(WHITE,H3),
				PM(WHITE,A4), PM(WHITE,B4), PM(WHITE,C4), PM(WHITE,D4), PM(WHITE,E4), PM(WHITE,F4), PM(WHITE,G4), PM(WHITE,H4),
				PM(WHITE,A5), PM(WHITE,B5), PM(WHITE,C5), PM(WHITE,D5), PM(WHITE,E5), PM(WHITE,F5), PM(WHITE,G5), PM(WHITE,H5),
				PM(WHITE,A6), PM(WHITE,B6), PM(WHITE,C6), PM(WHITE,D6), PM(WHITE,E6), PM(WHITE,F6), PM(WHITE,G6), PM(WHITE,H6),
				PM(WHITE,A7), PM(WHITE,B7), PM(WHITE,C7), PM(WHITE,D7), PM(WHITE,E7), PM(WHITE,F7), PM(WHITE,G7), PM(WHITE,H7),
				PM(WHITE,A8), PM(WHITE,B8), PM(WHITE,C8), PM(WHITE,D8), PM(WHITE,E8), PM(WHITE,F8), PM(WHITE,G8), PM(WHITE,H8)
		},
		{
				PM(BLACK,A1), PM(BLACK,B1), PM(BLACK,C1), PM(BLACK,D1), PM(BLACK,E1), PM(BLACK,F1), PM(BLACK,G1), PM(BLACK,H1),
				PM(BLACK,A2), PM(BLACK,B2), PM(BLACK,C2), PM(BLACK,D2), PM(BLACK,E2), PM(BLACK,F2), PM(BLACK,G2), PM(BLACK,H2),
				PM(BLACK,A3), PM(BLACK,B3), PM(BLACK,C3), PM(BLACK,D3), PM(BLACK,E3), PM(BLACK,F3), PM(BLACK,G3), PM(BLACK,H3),
				PM(BLACK,A4), PM(BLACK,B4), PM(BLACK,C4), PM(BLACK,D4), PM(BLACK,E4), PM(BLACK,F4), PM(BLACK,G4), PM(BLACK,H4),
				PM(BLACK,A5), PM(BLACK,B5), PM(BLACK,C5), PM(BLACK,D5), PM(BLACK,E5), PM(BLACK,F5), PM(BLACK,G5), PM(BLACK,H5),
				PM(BLACK,A6), PM(BLACK,B6), PM(BLACK,C6), PM(BLACK,D6), PM(BLACK,E6), PM(BLACK,F6), PM(BLACK,G6), PM(BLACK,H6),
				PM(BLACK,A7), PM(BLACK,B7), PM(BLACK,C7), PM(BLACK,D7), PM(BLACK,E7), PM(BLACK,F7), PM(BLACK,G7), PM(BLACK,H7),
				PM(BLACK,A8), PM(BLACK,B8), PM(BLACK,C8), PM(BLACK,D8), PM(BLACK,E8), PM(BLACK,F8), PM(BLACK,G8), PM(BLACK,H8)
		},
		{}
};

const int lazyEvalMargin=200;

class Evaluator {
public:

	Evaluator();
	virtual ~Evaluator();
	const int evaluate(Board& board, const int& alpha, const int& beta);
	const int evalMaterial(Board& board, PieceColor color);
	const int evalPieces(Board& board, PieceColor color);
	const int evalBoardControl(Board& board, PieceColor color, int& kingThreat);
	const int evalImbalances(Board& board, PieceColor color);
	const bool isPawnPassed(Board& board, const Square from);
	const void setGameStage(const GamePhase phase);
	const int seeSign(Board& board, MoveIterator::Move& move);
	const int see(Board& board, MoveIterator::Move& move);

private:

	Bitboard getLeastValuablePiece(Board& board, Bitboard attackers, PieceColor& color, PieceTypeByColor& piece);
};

// verify if pawn is passer
inline const bool Evaluator::isPawnPassed(Board& board, const Square from) {
	const Bitboard pawns = board.getPiecesByType(WHITE_PAWN) |
			board.getPiecesByType(BLACK_PAWN);

	const Bitboard thePawn = squareToBitboard[from];
	const PieceColor color = board.getPieceColorBySquare(from);

	return !(passedMask[color][from]&(pawns^thePawn));
}

// static exchange evaluation sign
inline const int Evaluator::seeSign(Board& board, MoveIterator::Move& move) {

	const int value = defaultMaterialValues[board.getPieceBySquare(move.to)]-
			defaultMaterialValues[board.getPieceBySquare(move.from)];

	if ( value >= 0	&&	board.getPieceType(board.getPieceBySquare(move.from)) != KING) {
		return value;
	}

	return this->see(board,move);
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

	if (board.getPieceType(secondPiece)==KING) {
		return queenValue*10;
	}

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

