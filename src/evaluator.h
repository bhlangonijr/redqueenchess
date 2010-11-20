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
#define BP(COLOR, X) ((FSQUARE((COLOR==WHITE?BLACK:WHITE), (neighborFiles[X]), X)) | (rankBB[squareRank[X]] & adjacentSquares[X]))
#define FQ(COLOR, X) (FSQUARE(COLOR, (fileBB[squareFile[X]]), X))

const int DOUBLED_PAWN_PENALTY =   MS(-10,-20);
const int ISOLATED_PAWN_PENALTY =  MS(-10,-17);
const int BACKWARD_PAWN_PENALTY =  MS(-10,-12);
const int DONE_CASTLE_BONUS=       MS(+20,+1);
const int CONNECTED_PAWN_BONUS =   MS(+5,+7);
const int BISHOP_PAIR_BONUS = 	   MS(+25,+30);
const int CONNECTED_PASSER_BONUS = MS(+20,+25);


const int knightMobility[9] = {
		MS(-8,-6),MS(-6,-4),MS(-4,-2),MS(-2,-1),MS(+0,+0),MS(+2,+1),MS(+4,+2),MS(+6,+3),MS(+8,+4)
};

const int bishopMobility[16] = {
		MS(-30,-15),MS(-25,-12),MS(-20,-10),MS(-15,-7),MS(-10,+5),MS(-5,+2),MS(+0,+1),MS(+5,+2),
		MS(+10,+5),MS(+15,+7),MS(+20,+10),MS(+25,+12),MS(+30,+15),MS(+35,+17),MS(+35,+17),MS(+40,+20)
};

const int rookMobility[16] = {
		MS(-14,-28),MS(-12,-24),MS(-10,-20),MS(-8,-16),MS(-6,-12),MS(-4,-8),MS(-2,-4),MS(+0,+1),
		MS(+2,+4),MS(+4,+8),MS(+6,+12),MS(+8,+16),MS(+10,+20),MS(+12,+24),MS(+12,+24),MS(+16,+26)
};

const int bishopKingBonus[8] = {
		MS(+1,+2),MS(+2,+4),MS(+3,+6),MS(+4,+8),MS(+5,+10),MS(+6,+12),MS(+7,+14),MS(+8,+16)
};

const int rookKingBonus[8] = {
		MS(+2,+4),MS(+4,+8),MS(+6,+12),MS(+8,+16),MS(+10,+20),MS(+12,+24),MS(+14,+28),MS(+16,+28)
};

const int queenKingBonus[8] = {
		MS(+3,+6),MS(+6,+12),MS(+9,+18),MS(+12,+24),MS(+15,+30),MS(+18,+32),MS(+21,+36),MS(+24,+38)
};

const int minorKingZoneAttackBonus[10] = {
		1*MS(+2,+7),2*MS(+2,+7),3*MS(+2,+7),4*MS(+2,+7),5*MS(+2,+7),6*MS(+2,+7),7*MS(+2,+7),8*MS(+2,+7),9*MS(+2,+7),10*MS(+2,+7)
};

const int majorKingZoneAttackBonus[10] = {
		1*MS(+2,+10),2*MS(+2,+10),3*MS(+2,+10),4*MS(+2,+10),5*MS(+2,+10),6*MS(+2,+10),7*MS(+2,+10),8*MS(+2,+10),9*MS(+2,+10),10*MS(+2,+10)
};

const int passedPawnBonus[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,MS(+25,+25),MS(+30,+40),MS(+35,+50),MS(+40,+60),MS(+50,+100),MS(+70,+150),0},
		{0,MS(+70,+150),MS(+50,+100),MS(+40,+60),MS(+35,+50),MS(+30,+40),MS(+25,+25),0},
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

const Bitboard backwardPawnMask[ALL_PIECE_COLOR][ALL_SQUARE]= {
		{
				BP(WHITE,A1), BP(WHITE,B1), BP(WHITE,C1), BP(WHITE,D1), BP(WHITE,E1), BP(WHITE,F1), BP(WHITE,G1), BP(WHITE,H1),
				BP(WHITE,A2), BP(WHITE,B2), BP(WHITE,C2), BP(WHITE,D2), BP(WHITE,E2), BP(WHITE,F2), BP(WHITE,G2), BP(WHITE,H2),
				BP(WHITE,A3), BP(WHITE,B3), BP(WHITE,C3), BP(WHITE,D3), BP(WHITE,E3), BP(WHITE,F3), BP(WHITE,G3), BP(WHITE,H3),
				BP(WHITE,A4), BP(WHITE,B4), BP(WHITE,C4), BP(WHITE,D4), BP(WHITE,E4), BP(WHITE,F4), BP(WHITE,G4), BP(WHITE,H4),
				BP(WHITE,A5), BP(WHITE,B5), BP(WHITE,C5), BP(WHITE,D5), BP(WHITE,E5), BP(WHITE,F5), BP(WHITE,G5), BP(WHITE,H5),
				BP(WHITE,A6), BP(WHITE,B6), BP(WHITE,C6), BP(WHITE,D6), BP(WHITE,E6), BP(WHITE,F6), BP(WHITE,G6), BP(WHITE,H6),
				BP(WHITE,A7), BP(WHITE,B7), BP(WHITE,C7), BP(WHITE,D7), BP(WHITE,E7), BP(WHITE,F7), BP(WHITE,G7), BP(WHITE,H7),
				BP(WHITE,A8), BP(WHITE,B8), BP(WHITE,C8), BP(WHITE,D8), BP(WHITE,E8), BP(WHITE,F8), BP(WHITE,G8), BP(WHITE,H8)
		},
		{
				BP(BLACK,A1), BP(BLACK,B1), BP(BLACK,C1), BP(BLACK,D1), BP(BLACK,E1), BP(BLACK,F1), BP(BLACK,G1), BP(BLACK,H1),
				BP(BLACK,A2), BP(BLACK,B2), BP(BLACK,C2), BP(BLACK,D2), BP(BLACK,E2), BP(BLACK,F2), BP(BLACK,G2), BP(BLACK,H2),
				BP(BLACK,A3), BP(BLACK,B3), BP(BLACK,C3), BP(BLACK,D3), BP(BLACK,E3), BP(BLACK,F3), BP(BLACK,G3), BP(BLACK,H3),
				BP(BLACK,A4), BP(BLACK,B4), BP(BLACK,C4), BP(BLACK,D4), BP(BLACK,E4), BP(BLACK,F4), BP(BLACK,G4), BP(BLACK,H4),
				BP(BLACK,A5), BP(BLACK,B5), BP(BLACK,C5), BP(BLACK,D5), BP(BLACK,E5), BP(BLACK,F5), BP(BLACK,G5), BP(BLACK,H5),
				BP(BLACK,A6), BP(BLACK,B6), BP(BLACK,C6), BP(BLACK,D6), BP(BLACK,E6), BP(BLACK,F6), BP(BLACK,G6), BP(BLACK,H6),
				BP(BLACK,A7), BP(BLACK,B7), BP(BLACK,C7), BP(BLACK,D7), BP(BLACK,E7), BP(BLACK,F7), BP(BLACK,G7), BP(BLACK,H7),
				BP(BLACK,A8), BP(BLACK,B8), BP(BLACK,C8), BP(BLACK,D8), BP(BLACK,E8), BP(BLACK,F8), BP(BLACK,G8), BP(BLACK,H8)
		},
		{}
};

const Bitboard frontSquares[ALL_PIECE_COLOR][ALL_SQUARE]= {
		{
				FQ(WHITE,A1), FQ(WHITE,B1), FQ(WHITE,C1), FQ(WHITE,D1), FQ(WHITE,E1), FQ(WHITE,F1), FQ(WHITE,G1), FQ(WHITE,H1),
				FQ(WHITE,A2), FQ(WHITE,B2), FQ(WHITE,C2), FQ(WHITE,D2), FQ(WHITE,E2), FQ(WHITE,F2), FQ(WHITE,G2), FQ(WHITE,H2),
				FQ(WHITE,A3), FQ(WHITE,B3), FQ(WHITE,C3), FQ(WHITE,D3), FQ(WHITE,E3), FQ(WHITE,F3), FQ(WHITE,G3), FQ(WHITE,H3),
				FQ(WHITE,A4), FQ(WHITE,B4), FQ(WHITE,C4), FQ(WHITE,D4), FQ(WHITE,E4), FQ(WHITE,F4), FQ(WHITE,G4), FQ(WHITE,H4),
				FQ(WHITE,A5), FQ(WHITE,B5), FQ(WHITE,C5), FQ(WHITE,D5), FQ(WHITE,E5), FQ(WHITE,F5), FQ(WHITE,G5), FQ(WHITE,H5),
				FQ(WHITE,A6), FQ(WHITE,B6), FQ(WHITE,C6), FQ(WHITE,D6), FQ(WHITE,E6), FQ(WHITE,F6), FQ(WHITE,G6), FQ(WHITE,H6),
				FQ(WHITE,A7), FQ(WHITE,B7), FQ(WHITE,C7), FQ(WHITE,D7), FQ(WHITE,E7), FQ(WHITE,F7), FQ(WHITE,G7), FQ(WHITE,H7),
				FQ(WHITE,A8), FQ(WHITE,B8), FQ(WHITE,C8), FQ(WHITE,D8), FQ(WHITE,E8), FQ(WHITE,F8), FQ(WHITE,G8), FQ(WHITE,H8)
		},
		{
				FQ(BLACK,A1), FQ(BLACK,B1), FQ(BLACK,C1), FQ(BLACK,D1), FQ(BLACK,E1), FQ(BLACK,F1), FQ(BLACK,G1), FQ(BLACK,H1),
				FQ(BLACK,A2), FQ(BLACK,B2), FQ(BLACK,C2), FQ(BLACK,D2), FQ(BLACK,E2), FQ(BLACK,F2), FQ(BLACK,G2), FQ(BLACK,H2),
				FQ(BLACK,A3), FQ(BLACK,B3), FQ(BLACK,C3), FQ(BLACK,D3), FQ(BLACK,E3), FQ(BLACK,F3), FQ(BLACK,G3), FQ(BLACK,H3),
				FQ(BLACK,A4), FQ(BLACK,B4), FQ(BLACK,C4), FQ(BLACK,D4), FQ(BLACK,E4), FQ(BLACK,F4), FQ(BLACK,G4), FQ(BLACK,H4),
				FQ(BLACK,A5), FQ(BLACK,B5), FQ(BLACK,C5), FQ(BLACK,D5), FQ(BLACK,E5), FQ(BLACK,F5), FQ(BLACK,G5), FQ(BLACK,H5),
				FQ(BLACK,A6), FQ(BLACK,B6), FQ(BLACK,C6), FQ(BLACK,D6), FQ(BLACK,E6), FQ(BLACK,F6), FQ(BLACK,G6), FQ(BLACK,H6),
				FQ(BLACK,A7), FQ(BLACK,B7), FQ(BLACK,C7), FQ(BLACK,D7), FQ(BLACK,E7), FQ(BLACK,F7), FQ(BLACK,G7), FQ(BLACK,H7),
				FQ(BLACK,A8), FQ(BLACK,B8), FQ(BLACK,C8), FQ(BLACK,D8), FQ(BLACK,E8), FQ(BLACK,F8), FQ(BLACK,G8), FQ(BLACK,H8)
		},
		{}
};

const int lazyEvalMargin=200;

class Evaluator {
public:

	Evaluator();
	virtual ~Evaluator();
	const int evaluate(Board& board, const int alpha, const int beta);
	const int evalMaterial(Board& board, PieceColor color);
	const int evalKing(Board& board, PieceColor color);
	const int evalPawns(Board& board, PieceColor color);
	const int evalBishops(Board& board, PieceColor color);
	const int evalBoardControl(Board& board, PieceColor color, int& kingThreat);
	const bool isPawnPassed(Board& board, const Square from);
	const void setGameStage(const GamePhase phase);
	const int see(Board& board, MoveIterator::Move& move);

private:

	Bitboard getLeastValuablePiece(Board& board, Bitboard attackers, PieceColor& color, PieceTypeByColor& piece);
};

// verify if pawn is passer
inline const bool Evaluator::isPawnPassed(Board& board, const Square from) {
	const PieceColor color = board.getPieceColorBySquare(from);
	const PieceColor other = board.flipSide(color);
	const Bitboard otherPawns = board.getPiecesByType(board.makePiece(other,PAWN));
	return !(passedMask[color][from]&otherPawns);
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

