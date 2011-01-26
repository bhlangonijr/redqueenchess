/*
	Redqueen Chess Engine
    Copyright (C) 2008-2011 Ben-Hur Carlos Vieira Langoni Junior

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
#include <sstream>
#include "board.h"
#include "psqt.h"
#include "bitboard.h"
#define FSQUARE(COLOR,BB,X) (BB&(COLOR==WHITE?upperMaskBitboard[X]:lowerMaskBitboard[X]))&(~(rankBB[squareRank[X]]))
#define PM(COLOR, X) (FSQUARE(COLOR, (fileBB[squareFile[X]] | neighborFiles[X]), X))
#define BP(COLOR, X) ((FSQUARE((COLOR==WHITE?BLACK:WHITE), (neighborFiles[X]), X)) | (rankBB[squareRank[X]] & adjacentSquares[X]))
#define FQ(COLOR, X) (FSQUARE(COLOR, (fileBB[squareFile[X]]), X))
const int DOUBLED_PAWN_PENALTY =   		MS(-10,-16);
const int ISOLATED_PAWN_PENALTY =  		MS(-20,-10);
const int ISOLATED_OPEN_PAWN_PENALTY =  MS(-20,-20);
const int BACKWARD_PAWN_PENALTY =  		MS(-13,-10);
const int BACKWARD_OPEN_PAWN_PENALTY =  MS(-17,-13);
const int DONE_CASTLE_BONUS=       		MS(+20,-1);
const int CONNECTED_PAWN_BONUS =   		MS(+3,+5);
const int BISHOP_PAIR_BONUS = 	   		MS(+25,+25);
const int UNSTOPPABLE_PAWN_BONUS = 		MS(+0,+200);
const int ROOK_ON_7TH_RANK_BONUS = 		MS(+15,+25);
const int QUEEN_ON_7TH_RANK_BONUS = 	MS(+10,+15);
const int PASSER_AND_KING_BONUS = 		MS(0,+5);

const int knightMobility[9] = {
		-4*MS(+8,+4),-2*MS(+8,+4),+0*MS(+8,+4),+1*MS(+8,+4),+2*MS(+8,+4),
		+3*MS(+8,+4),+4*MS(+8,+4),+5*MS(+8,+4),+6*MS(+8,+4)
};
const int bishopMobility[16] = {
		-7*MS(+6,+4),-4*MS(+6,+4),-2*MS(+6,+4),+0*MS(+6,+4),+1*MS(+6,+4),+2*MS(+6,+4),+3*MS(+6,+4),+4*MS(+6,+4),
		+5*MS(+6,+4),+6*MS(+6,+4),+7*MS(+6,+4),+8*MS(+6,+4),+9*MS(+6,+4),+10*MS(+6,+4),+10*MS(+6,+4),+10*MS(+6,+4)
};

const int rookMobility[16] = {
		-8*MS(+5,+3),-6*MS(+5,+3),-3*MS(+5,+3),-1*MS(+5,+3),+1*MS(+5,+3),+2*MS(+5,+3),+3*MS(+5,+3),+4*MS(+5,+3),
		+5*MS(+5,+3),+6*MS(+5,+3),+7*MS(+5,+3),+8*MS(+5,+3),+9*MS(+5,+3),+10*MS(+5,+3),+11*MS(+5,+3),+12*MS(+5,+3)
};

const int knightKingBonus[8] = {
		6*MS(+2,+4),6*MS(+2,+4),6*MS(+2,+4),4*MS(+2,+4),3*MS(+2,+4),2*MS(+2,+4),1*MS(+2,+4),0*MS(+2,+4)
};

const int bishopKingBonus[8] = {
		7*MS(+2,+4),6*MS(+2,+4),5*MS(+2,+4),4*MS(+2,+4),3*MS(+2,+4),2*MS(+2,+4),1*MS(+2,+4),0*MS(+2,+4)
};

const int rookKingBonus[8] = {
		7*MS(+4,+6),6*MS(+4,+6),5*MS(+4,+6),4*MS(+4,+6),3*MS(+4,+6),2*MS(+4,+6),1*MS(+4,+6),0*MS(+4,+6)
};

const int queenKingBonus[8] = {
		7*MS(+5,+7),6*MS(+5,+7),5*MS(+5,+7),4*MS(+5,+7),3*MS(+5,+7),2*MS(+5,+7),1*MS(+5,+7),0*MS(+5,+7)
};

const int kingZoneAttackWeight[ALL_PIECE_TYPE][10] = {
		{},
		{//knight
				0*MS(+4,+7),1*MS(+4,+7),2*MS(+4,+7),3*MS(+4,+7),4*MS(+4,+7),
				5*MS(+4,+7),6*MS(+4,+7),7*MS(+4,+7),8*MS(+4,+7),9*MS(+4,+7)
		},
		{// bishop
				0*MS(+4,+7),1*MS(+4,+7),2*MS(+4,+7),3*MS(+4,+7),4*MS(+4,+7),
				5*MS(+4,+7),6*MS(+4,+7),7*MS(+4,+7),8*MS(+4,+7),9*MS(+4,+7)
		},
		{// rook
				0*MS(+5,+8),1*MS(+5,+8),2*MS(+5,+8),3*MS(+5,+8),4*MS(+5,+8),
				5*MS(+5,+8),6*MS(+5,+8),7*MS(+5,+8),8*MS(+5,+8),9*MS(+5,+8)
		},
		{// queen
				0*MS(+6,+10),1*MS(+6,+10),2*MS(+6,+10),3*MS(+6,+10),4*MS(+6,+10),
				5*MS(+6,+10),6*MS(+6,+10),7*MS(+6,+10),8*MS(+6,+10),9*MS(+6,+10)
		},
		{},
		{}
};

const int connectedPasserBonus[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,MS(+20,+25),MS(+25,+35),MS(+30,+40),MS(+35,+45),MS(+50,+60),MS(+60,+90),0},
		{0,MS(+60,+90),MS(+50,+60),MS(+35,+45),MS(+30,+40),MS(+25,+35),MS(+20,+25),0},
		{}
};

const int freePasserBonus[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,MS(+20,+25),MS(+25,+35),MS(+40,+50),MS(+50,+60),MS(+60,+70),MS(+80,+90),0},
		{0,MS(+80,+90),MS(+60,+70),MS(+50,+60),MS(+40,+50),MS(+25,+35),MS(+20,+25),0},
		{}
};

const int passedPawnBonus[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,MS(+5,+10),MS(+15,+25),MS(+25,+35),MS(+35,+40),MS(+40,+50),MS(+40,+70),0},
		{0,MS(+40,+70),MS(+40,+50),MS(+35,+40),MS(+25,+35),MS(+15,+25),MS(+5,+10),0},
		{}
};

const int candidatePasserBonus[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,MS(+5,+7),MS(+9,+12),MS(+15,+17),MS(+18,+25),MS(+27,+35),0,0},
		{0,0,MS(+27,+35),MS(+18,+25),MS(+15,+17),MS(+9,+12),MS(+5,+7),0},
		{}
};

const int spaceBonus[18] = {
		MS(+0,+0),MS(+0,+0),MS(+5,+5),MS(+7,+7),MS(+9,+9),MS(+15,+13),MS(+20,+17),MS(+25,+23),MS(+30,+27),
		MS(+35,+33),MS(+45,+45),MS(+50,+50),MS(+60,+60),MS(+70,+70),MS(+80,+80),MS(+90,+90),MS(+100,+100),MS(+100,+100)
};

const int threatBonus[ALL_PIECE_TYPE][ALL_PIECE_TYPE] = {
		{MS(+0,+0),MS(+2,+3),MS(+2,+3),MS(+3,+5),MS(+15,+20),MS(+20,+25),0},
		{MS(+0,+0),MS(+5,+10),MS(+5,+10),MS(+5,+10),MS(+10,+15),MS(+15,+20),0},
		{MS(+0,+0),MS(+5,+10),MS(+5,+10),MS(+5,+10),MS(+10,+15),MS(+15,+20),0},
		{MS(+0,+0),MS(+3,+5),MS(+3,+5),MS(+0,+0),MS(+5,+10),MS(+10,+15),0},
		{MS(+0,+0),MS(+2,+4),MS(+2,+4),MS(+2,+4),MS(+0,+0),MS(+10,+15),0},
		{},
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

const int knightOutpostBonus[ALL_PIECE_COLOR][ALL_SQUARE] = {
		{
				MS(+0,+0), MS(+0,+0), MS(+0,+0), MS(+0,+0),   MS(+0,+0),   MS(+0,+0), MS(+0,+0), MS(+0,+0),
				MS(+0,+0), MS(+0,+0), MS(+0,+0), MS(+0,+0),   MS(+0,+0),   MS(+0,+0), MS(+0,+0), MS(+0,+0),
				MS(+0,+0), MS(+0,+0), MS(+5,+3), MS(+7,+5),   MS(+7,+5),   MS(+5,+3), MS(+0,+0), MS(+0,+0),
				MS(+0,+0), MS(+4,+2), MS(+7,+5), MS(+12,+10), MS(+12,+10), MS(+7,+5), MS(+4,+2), MS(+0,+0),
				MS(+0,+0), MS(+4,+2), MS(+7,+5), MS(+12,+10), MS(+12,+10), MS(+7,+5), MS(+4,+2), MS(+0,+0),
				MS(+0,+0), MS(+0,+0), MS(+0,+0), MS(+0,+0),   MS(+0,+0),   MS(+0,+0), MS(+0,+0), MS(+0,+0),
				MS(+0,+0), MS(+0,+0), MS(+0,+0), MS(+0,+0),   MS(+0,+0),   MS(+0,+0), MS(+0,+0), MS(+0,+0),
				MS(+0,+0), MS(+0,+0), MS(+0,+0), MS(+0,+0),   MS(+0,+0),   MS(+0,+0), MS(+0,+0), MS(+0,+0)
		},
		{
				MS(+0,+0), MS(+0,+0), MS(+0,+0), MS(+0,+0),   MS(+0,+0),   MS(+0,+0), MS(+0,+0), MS(+0,+0),
				MS(+0,+0), MS(+0,+0), MS(+0,+0), MS(+0,+0),   MS(+0,+0),   MS(+0,+0), MS(+0,+0), MS(+0,+0),
				MS(+0,+0), MS(+0,+0), MS(+0,+0), MS(+0,+0),   MS(+0,+0),   MS(+0,+0), MS(+0,+0), MS(+0,+0),
				MS(+0,+0), MS(+4,+2), MS(+7,+5), MS(+12,+10), MS(+12,+10), MS(+7,+5), MS(+4,+2), MS(+0,+0),
				MS(+0,+0), MS(+4,+2), MS(+7,+5), MS(+12,+10), MS(+12,+10), MS(+7,+5), MS(+4,+2), MS(+0,+0),
				MS(+0,+0), MS(+0,+0), MS(+5,+3), MS(+7,+5),   MS(+7,+5),   MS(+5,+3), MS(+0,+0), MS(+0,+0),
				MS(+0,+0), MS(+0,+0), MS(+0,+0), MS(+0,+0),   MS(+0,+0),   MS(+0,+0), MS(+0,+0), MS(+0,+0),
				MS(+0,+0), MS(+0,+0), MS(+0,+0), MS(+0,+0),   MS(+0,+0),   MS(+0,+0), MS(+0,+0), MS(+0,+0)
		},
		{}
};

const int lazyEvalMargin=200;
const size_t pawnHashSize=1<<18;

class Evaluator {
public:
	struct EvalInfo {
		EvalInfo(Board& _board) : board(_board) {
			side = board.getSideToMove();
			other = board.flipSide(side);
			all = board.getAllPieces();
			pawns[WHITE] = board.getPieces(makePiece(WHITE,PAWN));
			pawns[BLACK] = board.getPieces(makePiece(BLACK,PAWN));
			evalPieces[WHITE] = board.getPieceSquareValue(WHITE);
			evalPieces[BLACK] = board.getPieceSquareValue(BLACK);
			evalPawns[WHITE] = 0;
			evalPawns[BLACK] = 0;
			mobility[WHITE] = 0;
			mobility[BLACK] = 0;
			pieceThreat[WHITE] = 0;
			pieceThreat[BLACK] = 0;
			kingThreat[WHITE]=0;
			kingThreat[BLACK]=0;
			eval=0;
		}
		Board& board;
		Bitboard all;
		PieceColor side;
		PieceColor other;
		Bitboard attackers[ALL_PIECE_TYPE_BY_COLOR];
		Bitboard attacks[ALL_PIECE_COLOR];
		Bitboard pawns[ALL_PIECE_COLOR];
		int kingThreat[ALL_PIECE_COLOR];
		int evalPieces[ALL_PIECE_COLOR];
		int evalPawns[ALL_PIECE_COLOR];
		int mobility[ALL_PIECE_COLOR];
		int pieceThreat[ALL_PIECE_COLOR];
		int eval;

		inline const int getScore() {
			return (evalPieces[side]-evalPieces[other]) +
					(evalPawns[side]-evalPawns[other]) +
					(mobility[side]-mobility[other]) +
					(pieceThreat[side]-pieceThreat[other]) +
					(kingThreat[side]-kingThreat[other]);
		}

		inline const int getEval() {
			return eval;
		}

		inline void computeEval() {
			eval = interpolate(getScore(),board.getGamePhase())+
					board.getMaterial(side)-board.getMaterial(other);
			normalize();
		}

		inline void normalize() {
			if (eval>maxScore) {
				eval=maxScore;
			} else if (eval<-maxScore) {
				eval=-maxScore;
			}
		}

		inline std::string toString() {
			std::stringstream out;
			const int whiteScore=evalPieces[WHITE]+evalPawns[WHITE]+mobility[WHITE]+
					pieceThreat[WHITE]+kingThreat[WHITE];
			const int blackScore=evalPieces[BLACK]+evalPawns[BLACK]+mobility[BLACK]+
								pieceThreat[BLACK]+kingThreat[BLACK];
			out << "Material[WHITE]:          " << (board.getMaterial(WHITE)-kingValue) << std::endl;
			out << "Material[BLACK]:          " << (board.getMaterial(BLACK)-kingValue) << std::endl;
			out << "Pieces(PST&Other)[WHITE]: " << interpolate(evalPieces[WHITE],board.getGamePhase()) << std::endl;
			out << "Pieces(PST&Other)[BLACK]: " << interpolate(evalPieces[BLACK],board.getGamePhase()) << std::endl;
			out << "Pawns[WHITE]:             " << interpolate(evalPawns[WHITE],board.getGamePhase()) << std::endl;
			out << "Pawns[BLACK]:             " << interpolate(evalPawns[BLACK],board.getGamePhase()) << std::endl;
			out << "Mobility&Space[WHITE]:    " << interpolate(mobility[WHITE],board.getGamePhase()) << std::endl;
			out << "Mobility&Space[BLACK]:    " << interpolate(mobility[BLACK],board.getGamePhase()) << std::endl;
			out << "Pieces threats[WHITE]:    " << interpolate(pieceThreat[WHITE],board.getGamePhase()) << std::endl;
			out << "Pieces threats[BLACK]:    " << interpolate(pieceThreat[BLACK],board.getGamePhase()) << std::endl;
			out << "King threats[WHITE]:      " << interpolate(kingThreat[WHITE],board.getGamePhase()) << std::endl;
			out << "King threats[BLACK]:      " << interpolate(kingThreat[BLACK],board.getGamePhase()) << std::endl;
			out << "Endgame score(WHITE):    " << upperScore(whiteScore) << std::endl;
			out << "Endgame score(BLACK):    " << upperScore(blackScore) << std::endl;
			out << "Middlegame score(WHITE): " << lowerScore(whiteScore) << std::endl;
			out << "Middlegame score(BLACK): " << lowerScore(blackScore) << std::endl;
			std::string sign = eval==0?"":(eval>0 && side==WHITE)||(eval<0 && side==BLACK)?"+":"-";
			out << "Main eval:                " << sign << abs(eval) << std::endl;
			return out.str();
		}
	};

	struct PawnInfo {
		Key key;
		int value[ALL_PIECE_COLOR];
		Bitboard passers[ALL_PIECE_COLOR];
	}__attribute__ ((aligned(64)));

	Evaluator();
	virtual ~Evaluator();
	const int evaluate(Board& board, const int alpha, const int beta);
	void evalKing(PieceColor color, EvalInfo& evalInfo);
	void evalPawnsFromCache(PieceColor color, PawnInfo& info, EvalInfo& evalInfo);
	void evalPawns(PieceColor color, EvalInfo& evalInfo);
	const int evalPassedPawn(EvalInfo& evalInfo, PieceColor color, const Square from,
			const bool isPawnFinal, const bool isChained);
	void evalPieces(PieceColor color, EvalInfo& evalInfo);
	void evalBoardControl(PieceColor color, EvalInfo& evalInfo);
	void evalThreats(PieceColor color, EvalInfo& evalInfo);
	const bool isPawnPassed(Board& board, const Square from);
	const void setGameStage(const GamePhase phase);
	template <bool lazySee>
	const int see(Board& board, MoveIterator::Move& move);

	inline const bool isDebugEnabled() {
		return debug;
	}

	inline const void setDebugEnabled(const bool enabled) {
		this->debug = enabled;
	}

	inline const static int interpolate(const int value, const int gamePhase) {
		const int mgValue = upperScore(value);
		const int egValue = lowerScore(value);
		return (egValue*gamePhase)/maxGamePhase+(mgValue*(maxGamePhase-gamePhase))/maxGamePhase;
	}

	inline const int getKingAttackWeight(const int piece, const int count) {
		return kingZoneAttackWeight[pieceType[piece]][count];
	}

	inline bool getPawnInfo(const Key key, PawnInfo& pawnHash) {
		PawnInfo& entry = pawnInfo[static_cast<size_t>(key) & (pawnHashSize-1)];
		if (entry.key==key) {
			pawnHash.key=entry.key;
			pawnHash.passers[WHITE]=entry.passers[WHITE];
			pawnHash.passers[BLACK]=entry.passers[BLACK];
			pawnHash.value[WHITE]=entry.value[WHITE];
			pawnHash.value[BLACK]=entry.value[BLACK];
			return true;
		}
		return false;
	}

	inline void setPawnInfo(const Key key, const int value, const PieceColor color, const Bitboard passers) {
		PawnInfo& entry = pawnInfo[static_cast<size_t>(key) & (pawnHashSize-1)];
		entry.key=key;
		entry.value[color]=value;
		entry.passers[color]=passers;
	}

	inline void cleanPawnInfo() {
		memset(pawnInfo, 0, pawnHashSize * sizeof(PawnInfo));
	}

private:
	Bitboard getLeastValuablePiece(Board& board, Bitboard attackers, PieceColor& color, PieceTypeByColor& piece);
	PawnInfo pawnInfo[pawnHashSize];
	bool debug;
};

// verify if pawn is passer
inline const bool Evaluator::isPawnPassed(Board& board, const Square from) {
	const PieceColor color = board.getPieceColor(from);
	const PieceColor other = board.flipSide(color);
	const Bitboard otherPawns = board.getPieces(other,PAWN);
	return !(passedMask[color][from]&otherPawns);
}

// static exchange evaluation
template <bool lazySee>
inline const int Evaluator::see(Board& board, MoveIterator::Move& move) {
	const int gainTableSize=32;
	PieceColor side = board.getPieceColor(move.from);
	PieceColor other = board.flipSide(side);
	PieceTypeByColor firstPiece = board.getPiece(move.from);
	PieceTypeByColor secondPiece = board.getPiece(move.to);
	Bitboard fromPiece = squareToBitboard[move.from];
	Bitboard occupied = board.getAllPieces();
	if (secondPiece==EMPTY && board.getPieceType(firstPiece)==PAWN && board.getEnPassant()!=NONE &&
			board.getSquareFile(move.from)!=board.getSquareFile(move.to)) {
		secondPiece=makePiece(other,PAWN);
		occupied^=squareToBitboard[board.getEnPassant()];
	}
	if (lazySee && secondPiece!=EMPTY &&
			materialValues[secondPiece]>=materialValues[firstPiece]) {
		return 1;
	}
	const Bitboard bishopAttacks =  board.getBishopAttacks(move.to,occupied);
	const Bitboard rookAttacks =  board.getRookAttacks(move.to,occupied);
	const Bitboard knightAttacks =  board.getKnightAttacks(move.to);
	const Bitboard pawnAttacks =  board.getPawnAttacks(move.to);
	const Bitboard kingAttacks =  board.getKingAttacks(move.to);
	const Bitboard rooks = board.getPieces(side,ROOK) | board.getPieces(other,ROOK);
	const Bitboard bishops = board.getPieces(side,BISHOP) | board.getPieces(other,BISHOP);
	const Bitboard queens = board.getPieces(side,QUEEN) | board.getPieces(other,QUEEN);
	const Bitboard bishopAndQueen = bishops | queens;
	const Bitboard rookAndQueen = rooks | queens;
	Bitboard attackers =
			(bishopAttacks & bishopAndQueen) |
			(rookAttacks & rookAndQueen) |
			(knightAttacks & (board.getPieces(side,KNIGHT) | board.getPieces(other,KNIGHT))) |
			(pawnAttacks & (board.getPieces(side,PAWN) | board.getPieces(other,PAWN))) |
			(kingAttacks & (board.getPieces(side,KING) | board.getPieces(other,KING)));
	int idx = 0;
	int gain[gainTableSize];
	PieceColor sideToMove = side;
	Bitboard allAttackers = EMPTY_BB;
	gain[0] = materialValues[secondPiece];
	if (board.getPieceType(secondPiece)==KING) {
		return queenValue*10;
	}
	while (fromPiece) {
		allAttackers |= attackers;
		idx++;
		gain[idx]  = materialValues[firstPiece] - gain[idx-1];
		attackers ^= fromPiece;
		occupied  ^= fromPiece;
		Bitboard moreAttackers = (bishopAndQueen | rookAndQueen) & (~allAttackers);
		if (moreAttackers) {
			const Bitboard findMoreAttackers = moreAttackers &
					(board.getBishopAttacks(move.to,occupied) |
							board.getRookAttacks(move.to,occupied)) ;
			if (findMoreAttackers) {
				attackers |= findMoreAttackers;
			}
		}
		sideToMove=board.flipSide(sideToMove);
		fromPiece  = getLeastValuablePiece (board, attackers, sideToMove, firstPiece);
	}
	while (--idx) {
		gain[idx-1]= -std::max(-gain[idx-1], gain[idx]);
	}
	return gain[0];
}

inline Bitboard Evaluator::getLeastValuablePiece(Board& board, Bitboard attackers, PieceColor& color, PieceTypeByColor& piece) {
	const int first = makePiece(color,PAWN);
	const int last = makePiece(color,KING);
	for(register int pieceType = first; pieceType <= last; pieceType++) {
		Bitboard pieces = board.getPieces(static_cast<PieceTypeByColor>(pieceType)) & attackers;
		if (pieces) {
			piece = static_cast<PieceTypeByColor>(pieceType);
			return pieces & -pieces;
		}
	}
	return EMPTY_BB;
}

#endif /* EVALUATOR_H_ */
