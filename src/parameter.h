/*
	Redqueen Chess Engine
    Copyright (C) 2008-2012 Ben-Hur Carlos Vieira Langoni Junior

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
#include <iostream>
#include <fstream>
#include "bitboard.h"
#include "stringutil.h"

#define MAX_PARAMS 90
const int DELTA_MAX = 7;

using namespace StringUtil;

// middlegame & endgame piece square table
static int pieceSquareTable[ALL_PIECE_TYPE][ALL_SQUARE]={
		{ // pawns
				 MS(0, 0),  MS(0, 0),  MS(0, 0),  MS(0, 0),  MS(0, 0),  MS(0, 0),  MS(0, 0),  MS(0, 0),
				 MS(-1, -17),  MS(-7, -17),  MS(-11, -17),  MS(-35, -17),  MS(-13, -17),  MS(5, -17),  MS(3, -17),  MS(-5, -17),
				 MS(1, -11),  MS(1, -11),  MS(-6, -11),  MS(-19, -11),  MS(-6, -11),  MS(-7, -11),  MS(-4, -11),  MS(10, -11),
				 MS(1, -7),  MS(14, -7),  MS(8, -7),  MS(4, -7),  MS(5, -7),  MS(4, -7),  MS(10, -7),  MS(7, -7),
				 MS(9, 16),  MS(30, 16),  MS(23, 16),  MS(31, 16),  MS(31, 16),  MS(23, 16),  MS(17, 16),  MS(11, 16),
				 MS(21, 55),  MS(54, 55),  MS(72, 55),  MS(56, 55),  MS(77, 55),  MS(95, 55),  MS(71, 55),  MS(11, 55),
				 MS(118, 82),  MS(121, 82),  MS(173, 82),  MS(168, 82),  MS(107, 82),  MS(82, 82),  MS(-16, 82),  MS(22, 82),
				 MS(0, 0),  MS(0, 0),  MS(0, 0),  MS(0, 0),  MS(0, 0),  MS(0, 0),  MS(0, 0),  MS(0, 0)
		},
		{ // knights
				 MS(-99, -99),  MS(-30, -99),  MS(-66, -94),  MS(-64, -88),  MS(-29, -88),  MS(-19, -94),  MS(-61, -99),  MS(-81, -99),
				 MS(-56, -81),  MS(-31, -62),  MS(-28, -49),  MS(-1, -43),  MS(-7, -43),  MS(-20, -49),  MS(-42, -62),  MS(-11, -81),
				 MS(-38, -46),  MS(-16, -27),  MS(0, -15),  MS(14, -9),  MS(8, -9),  MS(3, -15),  MS(3, -27),  MS(-42, -46),
				 MS(-14, -22),  MS(0, -3),  MS(2, 10),  MS(3, 16),  MS(19, 16),  MS(12, 10),  MS(33, -3),  MS(-7, -22),
				 MS(-14, -7),  MS(-4, 12),  MS(25, 25),  MS(33, 31),  MS(10, 31),  MS(33, 25),  MS(14, 12),  MS(43, -7),
				 MS(-22, -2),  MS(18, 17),  MS(60, 30),  MS(64, 36),  MS(124, 36),  MS(143, 30),  MS(55, 17),  MS(6, -2),
				 MS(-34, -7),  MS(24, 12),  MS(54, 25),  MS(74, 31),  MS(60, 31),  MS(122, 25),  MS(2, 12),  MS(29, -7),
				 MS(-60, -21),  MS(0, -3),  MS(0, 10),  MS(0, 16),  MS(0, 16),  MS(0, 10),  MS(0, -3),  MS(0, -21),
		},
		{ // bishops
				 MS(-7, -27),  MS(12, -21),  MS(-8, -17),  MS(-37, -15),  MS(-31, -15),  MS(-8, -17),  MS(-45, -21),  MS(-67, -27),
				 MS(15, -10),  MS(5, -4),  MS(13, 0),  MS(-10, 2),  MS(1, 2),  MS(2, 0),  MS(0, -4),  MS(15, -10),
				 MS(5, 2),  MS(12, 8),  MS(14, 12),  MS(13, 14),  MS(10, 14),  MS(-1, 12),  MS(3, 8),  MS(4, 2),
				 MS(1, 11),  MS(5, 17),  MS(23, 21),  MS(32, 23),  MS(21, 23),  MS(8, 21),  MS(17, 17),  MS(4, 11),
				 MS(-1, 14),  MS(16, 20),  MS(29, 24),  MS(27, 26),  MS(37, 26),  MS(27, 24),  MS(17, 20),  MS(4, 14),
				 MS(7, 13),  MS(27, 19),  MS(20, 23),  MS(56, 25),  MS(91, 25),  MS(108, 23),  MS(53, 19),  MS(44, 13),
				 MS(-24, 8),  MS(-23, 14),  MS(30, 18),  MS(58, 20),  MS(65, 20),  MS(61, 18),  MS(69, 14),  MS(11, 8),
				 MS(0, -2),  MS(0, 4),  MS(0, 8),  MS(0, 10),  MS(0, 10),  MS(0, 8),  MS(0, 4),  MS(0, -2),
		},
		{ // rooks
				 MS(-2, -32),  MS(-1, -31),  MS(3, -30),  MS(1, -29),  MS(2, -29),  MS(1, -30),  MS(4, -31),  MS(-8, -32),
				 MS(-26, -27),  MS(-6, -25),  MS(2, -24),  MS(-2, -24),  MS(2, -24),  MS(-10, -24),  MS(-1, -25),  MS(-29, -27),
				 MS(-16, -15),  MS(0, -13),  MS(3, -12),  MS(-3, -12),  MS(8, -12),  MS(-1, -12),  MS(12, -13),  MS(3, -15),
				 MS(-9, 1),  MS(-5, 2),  MS(8, 3),  MS(14, 4),  MS(18, 4),  MS(-17, 3),  MS(13, 2),  MS(-13, 1),
				 MS(19, 15),  MS(33, 17),  MS(46, 18),  MS(57, 18),  MS(53, 18),  MS(39, 18),  MS(53, 17),  MS(16, 15),
				 MS(24, 25),  MS(83, 27),  MS(54, 28),  MS(75, 28),  MS(134, 28),  MS(144, 28),  MS(85, 27),  MS(75, 25),
				 MS(46, 27),  MS(33, 28),  MS(64, 29),  MS(62, 30),  MS(91, 30),  MS(89, 29),  MS(70, 28),  MS(104, 27),
				 MS(84, 16),  MS(0, 17),  MS(0, 18),  MS(37, 19),  MS(124, 19),  MS(0, 18),  MS(0, 17),  MS(153, 16),
		},
		{ //queen
				 MS(1, -61),  MS(-10, -55),  MS(-11, -52),  MS(3, -50),  MS(-15, -50),  MS(-51, -52),  MS(-83, -55),  MS(-13, -61),
				 MS(-7, -31),  MS(3, -26),  MS(2, -22),  MS(5, -21),  MS(-1, -21),  MS(-10, -22),  MS(-7, -26),  MS(-2, -31),
				 MS(-11, -8),  MS(0, -3),  MS(12, 1),  MS(2, 3),  MS(8, 3),  MS(11, 1),  MS(7, -3),  MS(-6, -8),
				 MS(-9, 9),  MS(5, 14),  MS(7, 17),  MS(9, 19),  MS(18, 19),  MS(17, 17),  MS(26, 14),  MS(4, 9),
				 MS(-6, 19),  MS(0, 24),  MS(15, 28),  MS(25, 30),  MS(32, 30),  MS(9, 28),  MS(26, 24),  MS(12, 19),
				 MS(-16, 23),  MS(10, 28),  MS(13, 32),  MS(25, 34),  MS(37, 34),  MS(30, 32),  MS(15, 28),  MS(26, 23),
				 MS(1, 21),  MS(11, 26),  MS(35, 30),  MS(0, 31),  MS(16, 31),  MS(55, 30),  MS(39, 26),  MS(57, 21),
				 MS(-13, 12),  MS(6, 17),  MS(-42, 21),  MS(0, 23),  MS(29, 23),  MS(0, 21),  MS(0, 17),  MS(102, 12),
		},
		{	//king
				 MS(0, -34),  MS(0, -30),  MS(0, -28),  MS(-9, -27),  MS(0, -27),  MS(-9, -28),  MS(25, -30),  MS(0, -34),
				 MS(-9, -17),  MS(-9, -13),  MS(-9, -11),  MS(-9, -10),  MS(-9, -10),  MS(-9, -11),  MS(-9, -13),  MS(-9, -17),
				 MS(-9, -2),  MS(-9, 2),  MS(-9, 4),  MS(-9, 5),  MS(-9, 5),  MS(-9, 4),  MS(-9, 2),  MS(-9, -2),
				 MS(-9, 11),  MS(-9, 15),  MS(-9, 17),  MS(-9, 18),  MS(-9, 18),  MS(-9, 17),  MS(-9, 15),  MS(-9, 11),
				 MS(-9, 22),  MS(-9, 26),  MS(-9, 28),  MS(-9, 29),  MS(-9, 29),  MS(-9, 28),  MS(-9, 26),  MS(-9, 22),
				 MS(-9, 31),  MS(-9, 34),  MS(-9, 37),  MS(-9, 38),  MS(-9, 38),  MS(-9, 37),  MS(-9, 34),  MS(-9, 31),
				 MS(-9, 38),  MS(-9, 41),  MS(-9, 44),  MS(-9, 45),  MS(-9, 45),  MS(-9, 44),  MS(-9, 41),  MS(-9, 38),
				 MS(-9, 42),  MS(-9, 46),  MS(-9, 48),  MS(-9, 50),  MS(-9, 50),  MS(-9, 48),  MS(-9, 46),  MS(-9, 42),
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

static int squareDelta[ALL_SQUARE][ALL_SQUARE];

const std::string allParameters[MAX_PARAMS] = {
		"PAWN_PST_MIDDLEGAME",
		"KNIGHT_PST_MIDDLEGAME",
		"BISHOP_PST_MIDDLEGAME",
		"ROOK_PST_MIDDLEGAME",
		"QUEEN_PST_MIDDLEGAME",
		"KING_PST_MIDDLEGAME",
		"PAWN_PST_ENDGAME",
		"KNIGHT_PST_ENDGAME",
		"BISHOP_PST_ENDGAME",
		"ROOK_PST_ENDGAME",
		"QUEEN_PST_ENDGAME",
		"KING_PST_ENDGAME",
		"DOUBLED_PAWN_PENALTY",
		"PAWN_MOBILITY_PENALTY",
		"ISOLATED_PAWN_PENALTY",
		"ISOLATED_OPEN_PAWN_PENALTY",
		"BACKWARD_PAWN_PENALTY",
		"BACKWARD_OPEN_PAWN_PENALTY",
		"DONE_CASTLE_BONUS",
		"CONNECTED_PAWN_BONUS",
		"BISHOP_PAIR_BONUS",
		"UNSTOPPABLE_PAWN_BONUS",
		"UNSTOPPABLE_CANDIDATE_BONUS",
		"ROOK_ON_7TH_RANK_BONUS",
		"ROOK_ON_OPEN_FILE_BONUS",
		"ROOK_ON_HALF_OPEN_FILE_BONUS",
		"QUEEN_ON_7TH_RANK_BONUS",
		"PASSER_AND_KING_BONUS",
		"PAWN_END_GAME_BONUS",
		"TRADE_PAWN_PENALTY",
		"TRADE_PIECE_PENALTY",
		"TRADE_PAWN_BONUS",
		"TRADE_PIECE_BONUS",
		"QUEEN_CHECK_BONUS",
		"ROOK_CHECK_BONUS",
		"INDIRECT_QUEEN_CHECK_BONUS",
		"INDIRECT_ROOK_CHECK_BONUS",
		"INDIRECT_KNIGHT_CHECK_BONUS",
		"INDIRECT_BISHOP_CHECK_BONUS",
		"SHELTER_BONUS",
		"SHELTER_OPEN_FILE_PENALTY",
		"TEMPO_BONUS",
		"KNIGHT_MOBILITY_MIDDLEGAME",
		"BISHOP_MOBILITY_MIDDLEGAME",
		"ROOK_MOBILITY_MIDDLEGAME",
		"KNIGHT_MOBILITY_ENDGAME",
		"BISHOP_MOBILITY_ENDGAME",
		"ROOK_MOBILITY_ENDGAME",
		"KNIGHT_TROPISM_ENDGAME",
		"BISHOP_TROPISM_ENDGAME",
		"ROOK_TROPISM_ENDGAME",
		"QUEEN_TROPISM_ENDGAME",
		"KNIGHT_TROPISM_MIDDLEGAME",
		"BISHOP_TROPISM_MIDDLEGAME",
		"ROOK_TROPISM_MIDDLEGAME",
		"QUEEN_TROPISM_MIDDLEGAME",
		"PAWN_ATTACK_WEIGHT_MIDDLEGAME",
		"KNIGHT_ATTACK_WEIGHT_MIDDLEGAME",
		"BISHOP_ATTACK_WEIGHT_MIDDLEGAME",
		"ROOK_ATTACK_WEIGHT_MIDDLEGAME",
		"QUEEN_ATTACK_WEIGHT_MIDDLEGAME",
		"PAWN_ATTACK_WEIGHT_ENDGAME",
		"KNIGHT_ATTACK_WEIGHT_ENDGAME",
		"BISHOP_ATTACK_WEIGHT_ENDGAME",
		"ROOK_ATTACK_WEIGHT_ENDGAME",
		"QUEEN_ATTACK_WEIGHT_ENDGAME",
		"CONNECTED_PASSER_BONUS_MIDDLEGAME",
		"CONNECTED_PASSER_BONUS_ENDGAME",
		"FREE_PASSER_BONUS_MIDDLEGAME",
		"FREE_PASSER_BONUS_ENDGAME",
		"PASSED_PAWN_BONUS_MIDDLEGAME",
		"PASSED_PAWN_BONUS_ENDGAME",
		"CANDIDATE_PASSER_BONUS_MIDDLEGAME",
		"CANDIDATE_PASSER_BONUS_ENDGAME",
		"PAWN_WEIGHT_MIDDLEGAME",
		"PAWN_WEIGHT_ENDGAME",
		"SPACE_BONUS_MIDDLEGAME",
		"SPACE_BONUS_ENDGAME",
		"PAWN_THREAT_BONUS_MIDDLEGAME",
		"KNIGHT_THREAT_BONUS_MIDDLEGAME",
		"BISHOP_THREAT_BONUS_MIDDLEGAME",
		"ROOK_THREAT_BONUS_MIDDLEGAME",
		"QUEEN_THREAT_BONUS_MIDDLEGAME",
		"PAWN_THREAT_BONUS_ENDGAME",
		"KNIGHT_THREAT_BONUS_ENDGAME",
		"BISHOP_THREAT_BONUS_ENDGAME",
		"ROOK_THREAT_BONUS_ENDGAME",
		"QUEEN_THREAT_BONUS_ENDGAME",
		"KNIGHT_OUTPOST_BONUS_MIDDLEGAME",
		"KNIGHT_OUTPOST_BONUS_ENDGAME"
};

static bool useSettings;
static std::string settingsFile;

inline bool checkParameter(std::string name) {
	bool result=false;
	for(int i=0;i<MAX_PARAMS;i++) {
		if (allParameters[i]==name) {
			result=true;
			break;
		}
	}
	return result;
}

inline std::string readParameter(std::string name, std::string parameters) {
	if (!checkParameter(name)) {
		std::cerr << "Parameter doesn't exist: " << name << std::endl;
		exit(0);
	}
	std::string result = parameters;
	result = getMiddleString(result,name,";");
	size_t n = result.find("=");
	if (n!=std::string::npos) {
		result = result.erase(0,n+1);
	}
	if (result.at(0)==' ') {
		result = result.erase(0,1);
	}
	normalizeString(result);
	return result;
}

//valueType 0 full int, 1 low int, 2 high int
inline void readIntArray(int* array, std::string values, size_t size) {

	size_t last = 0;
	size_t position = values.find(" ");
	size_t p=0;
	while ( position != std::string::npos && p < size)  {
		std::string value = values.substr(last,(position-last));
		int v = toInt(value);
		array[p++] = v;
		last=position+1;
		position = values.find(" ", position+1);
	}
}

inline void updateArray(int* high, int* low, int* array, std::string parameters,
		std::string mid, std::string end, size_t size, bool revert = false) {
	readIntArray(high,readParameter(mid,parameters),size);
	readIntArray(low,readParameter(end,parameters),size);
	for (size_t i=0;i<size;i++) {
		array[(revert?-1:1)*i+(revert?size-1:0)] = MS(high[i],low[i]);
	}
}

inline void updateParameter(int* array, int* param,
		std::string name, std::string parameters,size_t size) {
	readIntArray(array,readParameter(name,parameters),size);
	*param = MS(array[0],array[1]);
}

//setoption name Custom_Settings_File_Path value parameters.txt
inline void readParameters(std::string fileName) {
	std::string line;
	std::ifstream file(fileName.c_str());
	std::string str = "";

	if (file.is_open()) {
		while (file.good()) {
			std::getline (file,line);
			if (line.find("#",0)!=std::string::npos) {
				continue;
			}
			str += line;
		}
		file.close();
		normalizeString(str);

		int low[64];
		int high[64];

		memset(low, 0, sizeof(int)*64);
		memset(high, 0, sizeof(int)*64);
		//update piece square table
		updateArray(high,low,&pieceSquareTable[PAWN][0],str,"PAWN_PST_MIDDLEGAME","PAWN_PST_ENDGAME",64);
		updateArray(high,low,&pieceSquareTable[KNIGHT][0],str,"KNIGHT_PST_MIDDLEGAME","KNIGHT_PST_ENDGAME",64);
		updateArray(high,low,&pieceSquareTable[BISHOP][0],str,"BISHOP_PST_MIDDLEGAME","BISHOP_PST_ENDGAME",64);
		updateArray(high,low,&pieceSquareTable[ROOK][0],str,"ROOK_PST_MIDDLEGAME","ROOK_PST_ENDGAME",64);
		updateArray(high,low,&pieceSquareTable[QUEEN][0],str,"QUEEN_PST_MIDDLEGAME","QUEEN_PST_ENDGAME",64);
		updateArray(high,low,&pieceSquareTable[KING][0],str,"KING_PST_MIDDLEGAME","KING_PST_ENDGAME",64);

		//update evaluation parameters
		updateParameter(high,&DOUBLED_PAWN_PENALTY,"DOUBLED_PAWN_PENALTY",str,2);
		updateParameter(high,&PAWN_MOBILITY_PENALTY,"PAWN_MOBILITY_PENALTY",str,2);
		updateParameter(high,&ISOLATED_PAWN_PENALTY,"ISOLATED_PAWN_PENALTY",str,2);
		updateParameter(high,&ISOLATED_OPEN_PAWN_PENALTY,"ISOLATED_OPEN_PAWN_PENALTY",str,2);
		updateParameter(high,&BACKWARD_PAWN_PENALTY,"BACKWARD_PAWN_PENALTY",str,2);
		updateParameter(high,&BACKWARD_OPEN_PAWN_PENALTY,"BACKWARD_OPEN_PAWN_PENALTY",str,2);
		updateParameter(high,&DONE_CASTLE_BONUS,"DONE_CASTLE_BONUS",str,2);
		updateParameter(high,&CONNECTED_PAWN_BONUS,"CONNECTED_PAWN_BONUS",str,2);
		updateParameter(high,&BISHOP_PAIR_BONUS,"BISHOP_PAIR_BONUS",str,2);
		updateParameter(high,&UNSTOPPABLE_PAWN_BONUS,"UNSTOPPABLE_PAWN_BONUS",str,2);
		updateParameter(high,&UNSTOPPABLE_CANDIDATE_BONUS,"UNSTOPPABLE_CANDIDATE_BONUS",str,2);
		updateParameter(high,&ROOK_ON_7TH_RANK_BONUS,"ROOK_ON_7TH_RANK_BONUS",str,2);
		updateParameter(high,&ROOK_ON_OPEN_FILE_BONUS,"ROOK_ON_OPEN_FILE_BONUS",str,2);
		updateParameter(high,&ROOK_ON_HALF_OPEN_FILE_BONUS,"ROOK_ON_HALF_OPEN_FILE_BONUS",str,2);
		updateParameter(high,&QUEEN_ON_7TH_RANK_BONUS,"QUEEN_ON_7TH_RANK_BONUS",str,2);
		updateParameter(high,&PASSER_AND_KING_BONUS,"PASSER_AND_KING_BONUS",str,2);
		updateParameter(high,&PAWN_END_GAME_BONUS,"PAWN_END_GAME_BONUS",str,2);
		updateParameter(high,&TRADE_PAWN_PENALTY,"TRADE_PAWN_PENALTY",str,2);
		updateParameter(high,&TRADE_PIECE_PENALTY,"TRADE_PIECE_PENALTY",str,2);
		updateParameter(high,&TRADE_PAWN_BONUS,"TRADE_PAWN_BONUS",str,2);
		updateParameter(high,&TRADE_PIECE_BONUS,"TRADE_PIECE_BONUS",str,2);
		updateParameter(high,&QUEEN_CHECK_BONUS,"QUEEN_CHECK_BONUS",str,2);
		updateParameter(high,&ROOK_CHECK_BONUS,"ROOK_CHECK_BONUS",str,2);
		updateParameter(high,&INDIRECT_QUEEN_CHECK_BONUS,"INDIRECT_QUEEN_CHECK_BONUS",str,2);
		updateParameter(high,&INDIRECT_ROOK_CHECK_BONUS,"INDIRECT_ROOK_CHECK_BONUS",str,2);
		updateParameter(high,&INDIRECT_KNIGHT_CHECK_BONUS,"INDIRECT_KNIGHT_CHECK_BONUS",str,2);
		updateParameter(high,&INDIRECT_BISHOP_CHECK_BONUS,"INDIRECT_BISHOP_CHECK_BONUS",str,2);
		updateParameter(high,&SHELTER_BONUS,"SHELTER_BONUS",str,2);
		updateParameter(high,&SHELTER_OPEN_FILE_PENALTY,"SHELTER_OPEN_FILE_PENALTY",str,2);
		updateParameter(high,&TEMPO_BONUS,"TEMPO_BONUS",str,2);

		//update mobility
		updateArray(high,low,&knightMobility[0],str,"KNIGHT_MOBILITY_MIDDLEGAME","KNIGHT_MOBILITY_ENDGAME",9);
		updateArray(high,low,&bishopMobility[0],str,"BISHOP_MOBILITY_MIDDLEGAME","BISHOP_MOBILITY_ENDGAME",9);
		updateArray(high,low,&rookMobility[0],str,"ROOK_MOBILITY_MIDDLEGAME","ROOK_MOBILITY_ENDGAME",9);

		//update king tropism
		updateArray(high,low,&knightKingBonus[0],str,"KNIGHT_TROPISM_MIDDLEGAME","KNIGHT_TROPISM_ENDGAME",8);
		updateArray(high,low,&bishopKingBonus[0],str,"BISHOP_TROPISM_MIDDLEGAME","BISHOP_TROPISM_ENDGAME",8);
		updateArray(high,low,&rookKingBonus[0],str,"ROOK_TROPISM_MIDDLEGAME","ROOK_TROPISM_ENDGAME",8);
		updateArray(high,low,&queenKingBonus[0],str,"QUEEN_TROPISM_MIDDLEGAME","QUEEN_TROPISM_ENDGAME",8);

		//update attack weight by piece
		updateArray(high,low,&kingZoneAttackWeight[PAWN][0],str,"PAWN_ATTACK_WEIGHT_MIDDLEGAME","PAWN_ATTACK_WEIGHT_ENDGAME",10);
		updateArray(high,low,&kingZoneAttackWeight[KNIGHT][0],str,"KNIGHT_ATTACK_WEIGHT_MIDDLEGAME","KNIGHT_ATTACK_WEIGHT_ENDGAME",10);
		updateArray(high,low,&kingZoneAttackWeight[BISHOP][0],str,"BISHOP_ATTACK_WEIGHT_MIDDLEGAME","BISHOP_ATTACK_WEIGHT_ENDGAME",10);
		updateArray(high,low,&kingZoneAttackWeight[ROOK][0],str,"ROOK_ATTACK_WEIGHT_MIDDLEGAME","ROOK_ATTACK_WEIGHT_ENDGAME",10);
		updateArray(high,low,&kingZoneAttackWeight[QUEEN][0],str,"QUEEN_ATTACK_WEIGHT_MIDDLEGAME","QUEEN_ATTACK_WEIGHT_ENDGAME",10);

		//update passed pawn terms
		updateArray(high,low,&connectedPasserBonus[WHITE][0],str,"CONNECTED_PASSER_BONUS_MIDDLEGAME","CONNECTED_PASSER_BONUS_ENDGAME",ALL_RANK);
		updateArray(high,low,&connectedPasserBonus[BLACK][0],str,"CONNECTED_PASSER_BONUS_MIDDLEGAME","CONNECTED_PASSER_BONUS_ENDGAME",ALL_RANK,true);
		updateArray(high,low,&freePasserBonus[WHITE][0],str,"FREE_PASSER_BONUS_MIDDLEGAME","FREE_PASSER_BONUS_ENDGAME",ALL_RANK);
		updateArray(high,low,&freePasserBonus[BLACK][0],str,"FREE_PASSER_BONUS_MIDDLEGAME","FREE_PASSER_BONUS_ENDGAME",ALL_RANK,true);
		updateArray(high,low,&passedPawnBonus[WHITE][0],str,"PASSED_PAWN_BONUS_MIDDLEGAME","PASSED_PAWN_BONUS_ENDGAME",ALL_RANK);
		updateArray(high,low,&passedPawnBonus[BLACK][0],str,"PASSED_PAWN_BONUS_MIDDLEGAME","PASSED_PAWN_BONUS_ENDGAME",ALL_RANK,true);
		updateArray(high,low,&candidatePasserBonus[WHITE][0],str,"CANDIDATE_PASSER_BONUS_MIDDLEGAME","CANDIDATE_PASSER_BONUS_ENDGAME",ALL_RANK);
		updateArray(high,low,&candidatePasserBonus[BLACK][0],str,"CANDIDATE_PASSER_BONUS_MIDDLEGAME","CANDIDATE_PASSER_BONUS_ENDGAME",ALL_RANK,true);

		//pawn weight
		updateArray(high,low,&pawnWeight[WHITE][0],str,"PAWN_WEIGHT_MIDDLEGAME","PAWN_WEIGHT_ENDGAME",ALL_FILE);
		updateArray(high,low,&pawnWeight[BLACK][0],str,"PAWN_WEIGHT_MIDDLEGAME","PAWN_WEIGHT_ENDGAME",ALL_FILE,true);

		//space
		updateArray(high,low,&spaceBonus[0],str,"SPACE_BONUS_MIDDLEGAME","SPACE_BONUS_ENDGAME",18);

		//threat bonus
		updateArray(high,low,&threatBonus[PAWN][0],str,"PAWN_THREAT_BONUS_MIDDLEGAME","PAWN_THREAT_BONUS_ENDGAME",ALL_PIECE_TYPE);
		updateArray(high,low,&threatBonus[KNIGHT][0],str,"KNIGHT_THREAT_BONUS_MIDDLEGAME","KNIGHT_THREAT_BONUS_ENDGAME",ALL_PIECE_TYPE);
		updateArray(high,low,&threatBonus[BISHOP][0],str,"BISHOP_THREAT_BONUS_MIDDLEGAME","BISHOP_THREAT_BONUS_ENDGAME",ALL_PIECE_TYPE);
		updateArray(high,low,&threatBonus[ROOK][0],str,"ROOK_THREAT_BONUS_MIDDLEGAME","ROOK_THREAT_BONUS_ENDGAME",ALL_PIECE_TYPE);
		updateArray(high,low,&threatBonus[QUEEN][0],str,"QUEEN_THREAT_BONUS_MIDDLEGAME","QUEEN_THREAT_BONUS_ENDGAME",ALL_PIECE_TYPE);

		//knight outpost bonus
		updateArray(high,low,&knightOutpostBonus[WHITE][0],str,"KNIGHT_OUTPOST_BONUS_MIDDLEGAME","KNIGHT_OUTPOST_BONUS_ENDGAME",64);
		updateArray(high,low,&knightOutpostBonus[BLACK][0],str,"KNIGHT_OUTPOST_BONUS_MIDDLEGAME","KNIGHT_OUTPOST_BONUS_ENDGAME",64,true);

		std::cout << "info Loaded parameters." << std::endl;

	} else {
		std::cout << "info Unable to open file: " << fileName << std::endl;
	}

}

inline std::string getSettingsFile() {
	return settingsFile;
}

inline bool isUseSettings() {
	return useSettings;
}

inline void setSettingsFile(std::string _settingsFile) {
	settingsFile = _settingsFile;
}

inline void setUseSettings(bool _useSettings) {
	useSettings = _useSettings;
}

// print a bitboard in a readble form
inline void printBitboard(Bitboard bb) {
	for(long x=0;x<64;x++) {
		if ((0x1ULL << x)&bb) {
			std::cout << "1";
		} else {
			std::cout << "0";
		}
		if ((x+1) % 8 == 0) std::cout << std::endl;
	}
	std::cout << std::endl;
}

// initialize bitboards
inline void initializeBitboards() {
	for (int x=0;x<ALL_SQUARE;x++) {
		for (int y=0;y<ALL_SQUARE;y++) {
			const int delta1 = abs(squareRank[x]-squareRank[y]);
			const int delta2 = abs(squareFile[x]-squareFile[y]);
			squareDelta[x][y] = std::max(delta1, delta2);
		}
	}
}

inline int squareDistance(const Square from, const Square to) {
	return squareDelta[from][to];
}


#endif /* PARAMETER_H_ */
