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
 * parameter.h
 *
 *  Created on: Nov 20, 2011
 *      Author: bhlangonijr
 */

#ifndef PARAMETER_H_
#define PARAMETER_H_
#include "bitboard.h"

// middlegame & endgame piece square table
static int pieceSquareTable[ALL_PIECE_TYPE][ALL_SQUARE]={
		{ // pawns
				MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0),
				MS(-21, -5), MS( -9, -7), MS( -3, -9), MS(  4,-11), MS(  4,-11), MS( -3, -9), MS( -9, -7), MS(-21, -5),
				MS(-20, -5), MS( -8, -7), MS( -2, -9), MS(  5,-11), MS(  5,-11), MS( -2, -9), MS( -8, -7), MS(-20, -5),
				MS(-19, -4), MS( -7, -6), MS( -1, -8), MS(  6,-10), MS(  6,-10), MS( -1, -8), MS( -7, -6), MS(-19, -4),
				MS(-17, -3), MS( -5, -5), MS(  1, -7), MS(  8, -9), MS(  8, -9), MS(  1, -7), MS( -5, -5), MS(-17, -3),
				MS(-16, -2), MS( -4, -4), MS(  0, -6), MS(  9, -8), MS(  9, -8), MS(  0, -6), MS( -4, -4), MS(-16, -2),
				MS(-15,  0), MS( -3, -2), MS(  3, -4), MS( 10, -6), MS( 10, -6), MS(  3, -4), MS( -3, -2), MS(-15,  0),
				MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0),
		},
		{ // knights
				MS(-56,-20), MS(-40,-15), MS(-29,-10), MS(-25, -7), MS(-25, -7), MS(-29,-10), MS(-40,-15), MS(-56,-20),
				MS(-34,-13), MS(-18, -6), MS( -7, -2), MS( -3,  0), MS( -3,  0), MS( -7, -2), MS(-18, -6), MS(-34,-13),
				MS(-18, -8), MS( -2, -2), MS(  9,  3), MS( 13,  5), MS( 13,  5), MS(  9,  3), MS( -2, -2), MS(-18, -8),
				MS( -9, -4), MS(  7,  1), MS( 18,  6), MS( 22, 10), MS( 22, 10), MS( 18,  6), MS(  7,  1), MS( -9, -4),
				MS( -3, -2), MS( 13,  3), MS( 24,  8), MS( 28, 12), MS( 28, 12), MS( 24,  8), MS( 13,  3), MS( -3, -2),
				MS( -5, -1), MS( 11,  5), MS( 22, 10), MS( 26, 12), MS( 26, 12), MS( 22, 10), MS( 11,  5), MS( -5, -1),
				MS(-14, -6), MS(  0,  1), MS( 13,  5), MS( 17,  7), MS( 17,  7), MS( 13,  5), MS(  0,  1), MS(-14, -6),
				MS(-118,-13), MS(-19, -8), MS( -8, -3), MS( -4,  0), MS( -4,  0), MS( -8, -3), MS(-19, -8), MS(-118,-13),
		},
		{ // bishops
				MS( -5,  0), MS( -6, -1), MS( -9, -2), MS(-11, -2), MS(-11, -2), MS( -9, -2), MS( -6, -1), MS( -5,  0),
				MS( -1, -1), MS(  5,  1), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  5,  1), MS( -1, -1),
				MS( -4, -2), MS(  0,  0), MS(  9,  5), MS(  8,  4), MS(  8,  4), MS(  9,  5), MS(  0,  0), MS( -4, -2),
				MS( -6, -2), MS(  0,  0), MS(  8,  4), MS( 17,  7), MS( 17,  7), MS(  8,  4), MS(  0,  0), MS( -6, -2),
				MS( -6, -2), MS(  0,  0), MS(  8,  4), MS( 17,  7), MS( 17,  7), MS(  8,  4), MS(  0,  0), MS( -6, -2),
				MS( -4, -2), MS(  0,  0), MS(  9,  5), MS(  8,  4), MS(  8,  4), MS(  9,  5), MS(  0,  0), MS( -4, -2),
				MS( -1, -1), MS(  5,  1), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  0,  0), MS(  5,  1), MS( -1, -1),
				MS(  0,  0), MS( -1, -1), MS( -4, -2), MS( -6, -2), MS( -6, -2), MS( -4, -2), MS( -1, -1), MS(  0,  0),
		},
		{ // rooks
				MS( -2,  0), MS(  0,  0), MS(  6,  0), MS( 10,  0), MS( 10,  0), MS(  6,  0), MS(  0,  0), MS( -2,  0),
				MS( -2,  0), MS(  0,  0), MS(  6,  0), MS( 10,  0), MS( 10,  0), MS(  6,  0), MS(  0,  0), MS( -2,  0),
				MS( -2,  0), MS(  0,  0), MS(  6,  0), MS( 10,  0), MS( 10,  0), MS(  6,  0), MS(  0,  0), MS( -2,  0),
				MS( -2,  0), MS(  0,  0), MS(  6,  0), MS( 10,  0), MS( 10,  0), MS(  6,  0), MS(  0,  0), MS( -2,  0),
				MS( -2,  1), MS(  0,  1), MS(  6,  1), MS( 10,  1), MS( 10,  1), MS(  6,  1), MS(  0,  1), MS( -2,  1),
				MS( -2,  1), MS(  0,  1), MS(  6,  1), MS( 10,  1), MS( 10,  1), MS(  6,  1), MS(  0,  1), MS( -2,  1),
				MS( -2,  1), MS(  0,  1), MS(  6,  1), MS( 10,  1), MS( 10,  1), MS(  6,  1), MS(  0,  1), MS( -2,  1),
				MS( -2, -2), MS(  0, -2), MS(  6, -2), MS( 10, -2), MS( 10, -2), MS(  6, -2), MS(  0, -2), MS( -2, -2),
		},
		{ //queen
				MS(-14,-14), MS(-10, -9), MS( -7, -7), MS( -5, -6), MS( -5, -6), MS( -7, -7), MS(-10, -9), MS(-14,-14),
				MS( -5, -9), MS(  1, -4), MS(  3, -2), MS(  5, -2), MS(  5, -2), MS(  3, -2), MS(  1, -4), MS( -5, -9),
				MS( -2, -7), MS(  3, -2), MS(  7,  0), MS(  8,  3), MS(  8,  3), MS(  7,  0), MS(  3, -2), MS( -2, -7),
				MS(  0, -6), MS(  5, -2), MS(  8,  3), MS( 11,  6), MS( 11,  6), MS(  8,  3), MS(  5, -2), MS(  0, -6),
				MS(  0, -6), MS(  5, -2), MS(  8,  3), MS( 11,  6), MS( 11,  6), MS(  8,  3), MS(  5, -2), MS(  0, -6),
				MS( -2, -7), MS(  3, -2), MS(  7,  0), MS(  8,  3), MS(  8,  3), MS(  7,  0), MS(  3, -2), MS( -2, -7),
				MS( -5, -9), MS(  1, -4), MS(  3, -2), MS(  5, -2), MS(  5, -2), MS(  3, -2), MS(  1, -4), MS( -5, -9),
				MS( -9,-14), MS( -5, -9), MS( -2, -7), MS(  0, -6), MS(  0, -6), MS( -2, -7), MS( -5, -9), MS( -9,-14),
		},
		{	//king
				MS( 46,-72), MS( 51,-49), MS( 21,-33), MS(  1,-27), MS(  1,-27), MS( 21,-33), MS( 51,-49), MS( 46,-72),
				MS( 43,-39), MS( 48,-14), MS( 18, -2), MS( -2,  4), MS( -2,  4), MS( 18, -2), MS( 48,-14), MS( 43,-39),
				MS( 40,-28), MS( 45, -7), MS( 15,  8), MS( -5, 14), MS( -5, 14), MS( 15,  8), MS( 45, -7), MS( 40,-28),
				MS( 37,-22), MS( 42, -2), MS( 12, 14), MS( -8, 23), MS( -8, 23), MS( 12, 14), MS( 42, -2), MS( 37,-22),
				MS( 32,-17), MS( 37,  4), MS(  7, 19), MS(-13, 28), MS(-13, 28), MS(  7, 19), MS( 37,  4), MS( 32,-17),
				MS( 27,-23), MS( 32, -2), MS(  0, 13), MS(-18, 19), MS(-18, 19), MS(  0, 13), MS( 32, -2), MS( 27,-23),
				MS( 17,-34), MS( 22, -9), MS( -8,  3), MS(-28,  9), MS(-28,  9), MS( -8,  3), MS( 22, -9), MS( 17,-34),
				MS(  7,-52), MS( 12,-29), MS(-18,-13), MS(-38, -7), MS(-38, -7), MS(-18,-13), MS( 12,-29), MS(  7,-52),
		},
		{}
};

static int DOUBLED_PAWN_PENALTY =   		 MS(-10,-16);
static int PAWN_MOBILITY_PENALTY =   	 MS(-5,-7);
static int ISOLATED_PAWN_PENALTY =  		 MS(-20,-10);
static int ISOLATED_OPEN_PAWN_PENALTY =   MS(-20,-20);
static int BACKWARD_PAWN_PENALTY =  		 MS(-13,-10);
static int BACKWARD_OPEN_PAWN_PENALTY =   MS(-17,-13);
static int DONE_CASTLE_BONUS=       		 MS(+20,-1);
static int CONNECTED_PAWN_BONUS =   		 MS(+5,-1);
static int BISHOP_PAIR_BONUS = 	   		 MS(+30,+45);
static int UNSTOPPABLE_PAWN_BONUS = 		 MS(+500,+500);
static int UNSTOPPABLE_CANDIDATE_BONUS =	 MS(+50,+50);
static int ROOK_ON_7TH_RANK_BONUS = 		 MS(+15,+25);
static int ROOK_ON_OPEN_FILE_BONUS = 	 MS(+17,+17);
static int ROOK_ON_HALF_OPEN_FILE_BONUS = MS(+10,+10);
static int QUEEN_ON_7TH_RANK_BONUS = 	 MS(+10,+15);
static int PASSER_AND_KING_BONUS = 		 MS(+0,+5);
static int PAWN_END_GAME_BONUS = 		 MS(+0,+15);
static int TRADE_PAWN_PENALTY =           MS(-4,-6);
static int TRADE_PIECE_PENALTY =          MS(-3,-5);
static int TRADE_PAWN_BONUS =             MS(+1,+2);
static int TRADE_PIECE_BONUS =          	 MS(+2,+2);

static int QUEEN_CHECK_BONUS =  	 		 MS(+10,+15);
static int ROOK_CHECK_BONUS =  	 		 MS(+6,+11);
static int INDIRECT_QUEEN_CHECK_BONUS =	 MS(+3,+6);
static int INDIRECT_ROOK_CHECK_BONUS = 	 MS(+2,+4);
static int INDIRECT_KNIGHT_CHECK_BONUS =	 MS(+1,+2);
static int INDIRECT_BISHOP_CHECK_BONUS =  MS(+1,+2);
static int SHELTER_BONUS				  =  MS(+2,+0);
static int SHELTER_OPEN_FILE_PENALTY   =  MS(-9,-0);
static int TEMPO_BONUS				  =  MS(+10,+10);

static int knightMobility[9] = {
		-4*MS(+8,+4),-2*MS(+8,+4),+0*MS(+8,+4),+1*MS(+8,+4),+2*MS(+8,+4),
		+3*MS(+8,+4),+4*MS(+8,+4),+5*MS(+8,+4),+6*MS(+8,+4)
};

static int bishopMobility[16] = {
		-7*MS(+6,+4),-4*MS(+6,+4),-2*MS(+6,+4),+0*MS(+6,+4),+1*MS(+6,+4),+2*MS(+6,+4),+3*MS(+6,+4),+4*MS(+6,+4),
		+5*MS(+6,+4),+6*MS(+6,+4),+7*MS(+6,+4),+8*MS(+6,+4),+9*MS(+6,+4),+10*MS(+6,+4),+10*MS(+6,+4),+10*MS(+6,+4)
};

static int rookMobility[16] = {
		-8*MS(+5,+3),-6*MS(+5,+3),-3*MS(+5,+3),-1*MS(+5,+3),+1*MS(+5,+3),+2*MS(+5,+3),+3*MS(+5,+3),+4*MS(+5,+3),
		+5*MS(+5,+3),+6*MS(+5,+3),+7*MS(+5,+3),+8*MS(+5,+3),+9*MS(+5,+3),+10*MS(+5,+3),+11*MS(+5,+3),+12*MS(+5,+3)
};

static int knightKingBonus[8] = {
		6*MS(+2,+4),6*MS(+2,+4),6*MS(+2,+4),4*MS(+2,+4),3*MS(+2,+4),2*MS(+2,+4),1*MS(+2,+4),0*MS(+2,+4)
};

static int bishopKingBonus[8] = {
		7*MS(+2,+4),6*MS(+2,+4),5*MS(+2,+4),4*MS(+2,+4),3*MS(+2,+4),2*MS(+2,+4),1*MS(+2,+4),0*MS(+2,+4)
};

static int rookKingBonus[8] = {
		7*MS(+4,+6),6*MS(+4,+6),5*MS(+4,+6),4*MS(+4,+6),3*MS(+4,+6),2*MS(+4,+6),1*MS(+4,+6),0*MS(+4,+6)
};

static int queenKingBonus[8] = {
		7*MS(+5,+7),6*MS(+5,+7),5*MS(+5,+7),4*MS(+5,+7),3*MS(+5,+7),2*MS(+5,+7),1*MS(+5,+7),0*MS(+5,+7)
};

static int kingZoneAttackWeight[ALL_PIECE_TYPE][10] = {
		{//pawns
				0*MS(+3,+0),1*MS(+3,+0),2*MS(+3,+0),3*MS(+3,+0),4*MS(+3,+0),
				5*MS(+3,+0),6*MS(+3,+0),7*MS(+3,+0),8*MS(+3,+0),9*MS(+3,+0)
		},
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

static int connectedPasserBonus[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,MS(+20,+25),MS(+25,+35),MS(+30,+40),MS(+35,+45),MS(+50,+60),MS(+60,+90),0},
		{0,MS(+60,+90),MS(+50,+60),MS(+35,+45),MS(+30,+40),MS(+25,+35),MS(+20,+25),0},
		{}
};

static int freePasserBonus[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,MS(+20,+25),MS(+25,+35),MS(+40,+50),MS(+50,+60),MS(+60,+70),MS(+80,+90),0},
		{0,MS(+80,+90),MS(+60,+70),MS(+50,+60),MS(+40,+50),MS(+25,+35),MS(+20,+25),0},
		{}
};

static int passedPawnBonus[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,MS(+5,+10),MS(+10,+20),MS(+20,+40),MS(+40,+80),MS(+60,+100),MS(+90,+120),0},
		{0,MS(+90,+120),MS(+60,+100),MS(+40,+80),MS(+20,+40),MS(+10,+20),MS(+5,+10),0},
		{}
};

static int candidatePasserBonus[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,MS(+5,+7),MS(+10,+15),MS(+17,+25),MS(+30,+40),MS(+53,+71),0,0},
		{0,0,MS(+53,+71),MS(+30,+40),MS(+17,+25),MS(+10,+15),MS(+5,+7),0},
		{}
};

static int pawnWeight[ALL_PIECE_COLOR][ALL_FILE] = {
		{0,MS(-3,-2),MS(-2,-1),MS(+1,+2),MS(+1,+2),MS(-2,-1),MS(-3,-2),0},
		{0,MS(-3,-2),MS(-2,-1),MS(+1,+2),MS(+1,+2),MS(-2,-1),MS(-3,-2),0},
		{}
};

static int spaceBonus[18] = {
		MS(+0,+0),MS(+0,+0),MS(+5,+5),MS(+7,+7),MS(+9,+9),MS(+15,+13),MS(+20,+17),MS(+25,+23),MS(+30,+27),
		MS(+35,+33),MS(+45,+45),MS(+50,+50),MS(+60,+60),MS(+70,+70),MS(+80,+80),MS(+90,+90),MS(+100,+100),MS(+100,+100)
};

static int threatBonus[ALL_PIECE_TYPE][ALL_PIECE_TYPE] = {
		{MS(+0,+0),MS(+2,+3),MS(+2,+3),MS(+3,+5),MS(+15,+20),MS(+20,+25),0},
		{MS(+0,+0),MS(+5,+10),MS(+5,+10),MS(+5,+10),MS(+10,+15),MS(+15,+20),0},
		{MS(+0,+0),MS(+5,+10),MS(+5,+10),MS(+5,+10),MS(+10,+15),MS(+15,+20),0},
		{MS(+0,+0),MS(+3,+5),MS(+3,+5),MS(+0,+0),MS(+5,+10),MS(+10,+15),0},
		{MS(+0,+0),MS(+2,+4),MS(+2,+4),MS(+2,+4),MS(+0,+0),MS(+10,+15),0},
		{},
		{}
};

static int knightOutpostBonus[ALL_PIECE_COLOR][ALL_SQUARE] = {
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

class Parameter {
public:
	Parameter();
	virtual ~Parameter();

	static void readParameters(std::string fileName ) {



	}

};

#endif /* PARAMETER_H_ */
