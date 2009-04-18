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
 * Board.h
 *
 *  Created on: Feb 21, 2009
 *      Author: bhlangonijr
 */

#ifndef BOARD_H_
#define BOARD_H_
#include <inttypes.h>
#include <cmath>
#include <boost/pool/object_pool.hpp>

#include "Inline.h"

namespace BoardTypes{

//Bitboard type - unsigned long int (8 bytes)
typedef uint64_t Bitboard;

#define ALL_PIECE_TYPE 			6   																// pawn, knight, bishop, rook, queen, king
#define ALL_PIECE_TYPE_BY_COLOR 13  																// (black, white) X (pawn, knight, bishop, rook, queen, king) + empty
#define ALL_PIECE_COLOR			3   																// black, white, none
#define ALL_SQUARE				64  																// all square A1 .. H8
#define ALL_RANK				8																	// all ranks
#define ALL_FILE				8																	// all files
#define ALL_DIAGONAL			15																	// all diagonals

#define SqBB(S)					0x1ULL << (int)S													// Encode a square enum to a bitboard
#define Sq2Bb(X)				squareToBitboard[X] 												// square to bitboard macro
#define St2Sq(F,R)				(((int)F-96)+((int)R-49)*8)-1										// encode String to Square enum

#define Sq2FA(X)				fileBB[squareFile[X]]												// Encode Square to File Attack
#define Sq2RA(X)				rankBB[squareRank[X]]												// Encode Square to Rank Attack

#define Sq2A1(X)				diagonalA1H8BB[SquareToDiagonalA1H8[X]]								// Encode Square to Diagonal A1H1 Attack
#define Sq2H1(X)				diagonalH1A8BB[SquareToDiagonalH1A8[X]]								// Encode Square to Diagonal H1A1 Attack

#define Sq2UM(X)				~(squareToBitboard[X]-1) 											// Encode Square to BB uppermask
#define Sq2LM(X)				squareToBitboard[X]-1												// Encode Square to BB lowermask

#define FULL_BB						 0xFFFFFFFFFFFFFFFFULL
#define EMPTY_BB					 0x0ULL
#define INITIAL_WHITE_BITBOARD  	 0xFFFFULL
#define INITIAL_BLACK_BITBOARD  	 0xFFFF000000000000ULL
#define INITIAL_WHITE_PAWN_BITBOARD  0xFF00ULL
#define INITIAL_BLACK_PAWN_BITBOARD  0xFF000000000000ULL

#define bitsBetween(BB,S1,S2)		(squareToBitboard[S2]|(squareToBitboard[S2]-squareToBitboard[S1])) & BB

// squares
enum Square {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	NONE
};

//colors
enum PieceColor {
	WHITE, BLACK, COLOR_NONE
};

//piece types
enum PieceType {
	PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_EMPTY
};

// piece type by color
enum PieceTypeByColor {
	WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING, BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING, EMPTY
};

// castle types
enum CastleRight {
	NO_CASTLE=0, KING_SIDE_CASTLE, QUEEN_SIDE_CASTLE, BOTH_SIDE_CASTLE
};

//ranks - row
enum Rank { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };

//files - column
enum File { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };

// diagonals A1..H8
enum DiagonalA1H8 { A8_A8, B8_A7, C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3, G1_H2, H1_H1 };

// diagonals A1..H8
enum DiagonalH1A8 { A1_A1, B1_A2, C1_A3,D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6, G8_H7, H8_H8 };

//color of a given piece
static const PieceColor pieceColor[ALL_PIECE_TYPE_BY_COLOR] = {WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,COLOR_NONE};

// type of a given piece
static const PieceType pieceType[ALL_PIECE_TYPE_BY_COLOR] = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_EMPTY};

// Rank of a given square
static const Rank squareRank[ALL_SQUARE]={
		RANK_1, RANK_1, RANK_1, RANK_1, RANK_1, RANK_1, RANK_1, RANK_1,
		RANK_2, RANK_2, RANK_2, RANK_2, RANK_2, RANK_2, RANK_2, RANK_2,
		RANK_3, RANK_3, RANK_3, RANK_3, RANK_3, RANK_3, RANK_3, RANK_3,
		RANK_4, RANK_4, RANK_4, RANK_4, RANK_4, RANK_4, RANK_4, RANK_4,
		RANK_5, RANK_5, RANK_5, RANK_5, RANK_5, RANK_5, RANK_5, RANK_5,
		RANK_6, RANK_6, RANK_6, RANK_6, RANK_6, RANK_6, RANK_6, RANK_6,
		RANK_7, RANK_7, RANK_7, RANK_7, RANK_7, RANK_7, RANK_7, RANK_7,
		RANK_8, RANK_8, RANK_8, RANK_8, RANK_8, RANK_8, RANK_8, RANK_8
};

// File of a given square
static const File squareFile[ALL_SQUARE]={
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

//encode square by rank & file
static const Square encodeSquare[ALL_RANK][ALL_FILE]= {
		{A1, B1, C1, D1, E1, F1, G1, H1},
		{A2, B2, C2, D2, E2, F2, G2, H2},
		{A3, B3, C3, D3, E3, F3, G3, H3},
		{A4, B4, C4, D4, E4, F4, G4, H4},
		{A5, B5, C5, D5, E5, F5, G5, H5},
		{A6, B6, C6, D6, E6, F6, G6, H6},
		{A7, B7, C7, D7, E7, F7, G7, H7},
		{A8, B8, C8, D8, E8, F8, G8, H8}
};

// represents square location within the bitboard - it's simply a power of 2 to distinguish the squares
static const Bitboard squareToBitboard[ALL_SQUARE]={
		SqBB(A1), SqBB(B1), SqBB(C1), SqBB(D1), SqBB(E1), SqBB(F1), SqBB(G1), SqBB(H1),
		SqBB(A2), SqBB(B2), SqBB(C2), SqBB(D2), SqBB(E2), SqBB(F2), SqBB(G2), SqBB(H2),
		SqBB(A3), SqBB(B3), SqBB(C3), SqBB(D3), SqBB(E3), SqBB(F3), SqBB(G3), SqBB(H3),
		SqBB(A4), SqBB(B4), SqBB(C4), SqBB(D4), SqBB(E4), SqBB(F4), SqBB(G4), SqBB(H4),
		SqBB(A5), SqBB(B5), SqBB(C5), SqBB(D5), SqBB(E5), SqBB(F5), SqBB(G5), SqBB(H5),
		SqBB(A6), SqBB(B6), SqBB(C6), SqBB(D6), SqBB(E6), SqBB(F6), SqBB(G6), SqBB(H6),
		SqBB(A7), SqBB(B7), SqBB(C7), SqBB(D7), SqBB(E7), SqBB(F7), SqBB(G7), SqBB(H7),
		SqBB(A8), SqBB(B8), SqBB(C8), SqBB(D8), SqBB(E8), SqBB(F8), SqBB(G8), SqBB(H8)
};

// bitboard for all ranks
static const Bitboard rankBB[ALL_RANK]={
		0x00000000000000FFULL,0x000000000000FF00ULL,0x0000000000FF0000ULL,0x00000000FF000000ULL,
		0x000000FF00000000ULL,0x0000FF0000000000ULL,0x00FF000000000000ULL,0xFF00000000000000ULL
};

// bitboard for all files
static const Bitboard fileBB[ALL_FILE]={
		0x0101010101010101ULL,0x0202020202020202ULL,0x0404040404040404ULL,0x0808080808080808ULL,
		0x1010101010101010ULL,0x2020202020202020ULL,0x4040404040404040ULL,0x8080808080808080ULL
};

// bitboard for all diagonal A1..H8
static const Bitboard diagonalA1H8BB[ALL_DIAGONAL]={
		Sq2Bb(A8),
		Sq2Bb(B8)|Sq2Bb(A7),
		Sq2Bb(C8)|Sq2Bb(B7)|Sq2Bb(A6),
		Sq2Bb(D8)|Sq2Bb(C7)|Sq2Bb(B6)|Sq2Bb(A5),
		Sq2Bb(E8)|Sq2Bb(D7)|Sq2Bb(C6)|Sq2Bb(B5)|Sq2Bb(A4),
		Sq2Bb(F8)|Sq2Bb(E7)|Sq2Bb(D6)|Sq2Bb(C5)|Sq2Bb(B4)|Sq2Bb(A3),
		Sq2Bb(G8)|Sq2Bb(F7)|Sq2Bb(E6)|Sq2Bb(D5)|Sq2Bb(C4)|Sq2Bb(B3)|Sq2Bb(A2),
		Sq2Bb(H8)|Sq2Bb(G7)|Sq2Bb(F6)|Sq2Bb(E5)|Sq2Bb(D4)|Sq2Bb(C3)|Sq2Bb(B2)|Sq2Bb(A1),
		Sq2Bb(B1)|Sq2Bb(C2)|Sq2Bb(D3)|Sq2Bb(E4)|Sq2Bb(F5)|Sq2Bb(G6)|Sq2Bb(H7),
		Sq2Bb(C1)|Sq2Bb(D2)|Sq2Bb(E3)|Sq2Bb(F4)|Sq2Bb(G5)|Sq2Bb(H6),
		Sq2Bb(D1)|Sq2Bb(E2)|Sq2Bb(F3)|Sq2Bb(G4)|Sq2Bb(H5),
		Sq2Bb(E1)|Sq2Bb(F2)|Sq2Bb(G3)|Sq2Bb(H4),
		Sq2Bb(F1)|Sq2Bb(G2)|Sq2Bb(H3),
		Sq2Bb(G1)|Sq2Bb(H2),
		Sq2Bb(H1) };

// square to enum diagonal A1..H8
static const DiagonalA1H8 SquareToDiagonalA1H8[ALL_SQUARE]={
		H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3, G1_H2, H1_H1,
		G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3, G1_H2,
		F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3,
		E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4,
		D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5,
		C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6,
		B8_A7, C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7,
		A8_A8, B8_A7, C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1};

// bitboard for all diagonal H1..A8
static const Bitboard diagonalH1A8BB[ALL_DIAGONAL]={
		Sq2Bb(A1),
		Sq2Bb(B1)|Sq2Bb(A2),
		Sq2Bb(C1)|Sq2Bb(B2)|Sq2Bb(A3),
		Sq2Bb(D1)|Sq2Bb(C2)|Sq2Bb(B3)|Sq2Bb(A4),
		Sq2Bb(E1)|Sq2Bb(D2)|Sq2Bb(C3)|Sq2Bb(B4)|Sq2Bb(A5),
		Sq2Bb(F1)|Sq2Bb(E2)|Sq2Bb(D3)|Sq2Bb(C4)|Sq2Bb(B5)|Sq2Bb(A6),
		Sq2Bb(G1)|Sq2Bb(F2)|Sq2Bb(E3)|Sq2Bb(D4)|Sq2Bb(C5)|Sq2Bb(B6)|Sq2Bb(A7),
		Sq2Bb(H1)|Sq2Bb(G2)|Sq2Bb(F3)|Sq2Bb(E4)|Sq2Bb(D5)|Sq2Bb(C6)|Sq2Bb(B7)|Sq2Bb(A8),
		Sq2Bb(B8)|Sq2Bb(C7)|Sq2Bb(D6)|Sq2Bb(E5)|Sq2Bb(F4)|Sq2Bb(G3)|Sq2Bb(H2),
		Sq2Bb(C8)|Sq2Bb(D7)|Sq2Bb(E6)|Sq2Bb(F5)|Sq2Bb(G4)|Sq2Bb(H3),
		Sq2Bb(D8)|Sq2Bb(E7)|Sq2Bb(F6)|Sq2Bb(G5)|Sq2Bb(H4),
		Sq2Bb(E8)|Sq2Bb(F7)|Sq2Bb(G6)|Sq2Bb(H5),
		Sq2Bb(F8)|Sq2Bb(G7)|Sq2Bb(H6),
		Sq2Bb(G8)|Sq2Bb(H7),
		Sq2Bb(H8) };

// square to enum diagonal A1..H8
static const DiagonalH1A8 SquareToDiagonalH1A8[ALL_SQUARE]={
		A1_A1, B1_A2, C1_A3, D1_A4, E1_A5, F1_A6, G1_A7, H1_A8,
		B1_A2, C1_A3, D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2,
		C1_A3, D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3,
		D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4,
		E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5,
		F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6,
		G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6, G8_H7,
		H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6, G8_H7, H8_H8};

// bitboard for rank attacks
static const Bitboard rankAttacks[ALL_SQUARE]={
		Sq2RA(A1), Sq2RA(B1), Sq2RA(C1), Sq2RA(D1), Sq2RA(E1), Sq2RA(F1), Sq2RA(G1), Sq2RA(H1),
		Sq2RA(A2), Sq2RA(B2), Sq2RA(C2), Sq2RA(D2), Sq2RA(E2), Sq2RA(F2), Sq2RA(G2), Sq2RA(H2),
		Sq2RA(A3), Sq2RA(B3), Sq2RA(C3), Sq2RA(D3), Sq2RA(E3), Sq2RA(F3), Sq2RA(G3), Sq2RA(H3),
		Sq2RA(A4), Sq2RA(B4), Sq2RA(C4), Sq2RA(D4), Sq2RA(E4), Sq2RA(F4), Sq2RA(G4), Sq2RA(H4),
		Sq2RA(A5), Sq2RA(B5), Sq2RA(C5), Sq2RA(D5), Sq2RA(E5), Sq2RA(F5), Sq2RA(G5), Sq2RA(H5),
		Sq2RA(A6), Sq2RA(B6), Sq2RA(C6), Sq2RA(D6), Sq2RA(E6), Sq2RA(F6), Sq2RA(G6), Sq2RA(H6),
		Sq2RA(A7), Sq2RA(B7), Sq2RA(C7), Sq2RA(D7), Sq2RA(E7), Sq2RA(F7), Sq2RA(G7), Sq2RA(H7),
		Sq2RA(A8), Sq2RA(B8), Sq2RA(C8), Sq2RA(D8), Sq2RA(E8), Sq2RA(F8), Sq2RA(G8), Sq2RA(H8) };

// bitboard for file attacks
static const Bitboard fileAttacks[ALL_SQUARE]={
		Sq2FA(A1), Sq2FA(B1), Sq2FA(C1), Sq2FA(D1), Sq2FA(E1), Sq2FA(F1), Sq2FA(G1), Sq2FA(H1),
		Sq2FA(A2), Sq2FA(B2), Sq2FA(C2), Sq2FA(D2), Sq2FA(E2), Sq2FA(F2), Sq2FA(G2), Sq2FA(H2),
		Sq2FA(A3), Sq2FA(B3), Sq2FA(C3), Sq2FA(D3), Sq2FA(E3), Sq2FA(F3), Sq2FA(G3), Sq2FA(H3),
		Sq2FA(A4), Sq2FA(B4), Sq2FA(C4), Sq2FA(D4), Sq2FA(E4), Sq2FA(F4), Sq2FA(G4), Sq2FA(H4),
		Sq2FA(A5), Sq2FA(B5), Sq2FA(C5), Sq2FA(D5), Sq2FA(E5), Sq2FA(F5), Sq2FA(G5), Sq2FA(H5),
		Sq2FA(A6), Sq2FA(B6), Sq2FA(C6), Sq2FA(D6), Sq2FA(E6), Sq2FA(F6), Sq2FA(G6), Sq2FA(H6),
		Sq2FA(A7), Sq2FA(B7), Sq2FA(C7), Sq2FA(D7), Sq2FA(E7), Sq2FA(F7), Sq2FA(G7), Sq2FA(H7),
		Sq2FA(A8), Sq2FA(B8), Sq2FA(C8), Sq2FA(D8), Sq2FA(E8), Sq2FA(F8), Sq2FA(G8), Sq2FA(H8) };

// bitboard for diagonal attacks
static const Bitboard diagA1H8Attacks[ALL_SQUARE]={
		Sq2A1(A1), Sq2A1(B1), Sq2A1(C1), Sq2A1(D1), Sq2A1(E1), Sq2A1(F1), Sq2A1(G1), Sq2A1(H1),
		Sq2A1(A2), Sq2A1(B2), Sq2A1(C2), Sq2A1(D2), Sq2A1(E2), Sq2A1(F2), Sq2A1(G2), Sq2A1(H2),
		Sq2A1(A3), Sq2A1(B3), Sq2A1(C3), Sq2A1(D3), Sq2A1(E3), Sq2A1(F3), Sq2A1(G3), Sq2A1(H3),
		Sq2A1(A4), Sq2A1(B4), Sq2A1(C4), Sq2A1(D4), Sq2A1(E4), Sq2A1(F4), Sq2A1(G4), Sq2A1(H4),
		Sq2A1(A5), Sq2A1(B5), Sq2A1(C5), Sq2A1(D5), Sq2A1(E5), Sq2A1(F5), Sq2A1(G5), Sq2A1(H5),
		Sq2A1(A6), Sq2A1(B6), Sq2A1(C6), Sq2A1(D6), Sq2A1(E6), Sq2A1(F6), Sq2A1(G6), Sq2A1(H6),
		Sq2A1(A7), Sq2A1(B7), Sq2A1(C7), Sq2A1(D7), Sq2A1(E7), Sq2A1(F7), Sq2A1(G7), Sq2A1(H7),
		Sq2A1(A8), Sq2A1(B8), Sq2A1(C8), Sq2A1(D8), Sq2A1(E8), Sq2A1(F8), Sq2A1(G8), Sq2A1(H8) };

// bitboard for diagonal attacks
static const Bitboard diagH1A8Attacks[ALL_SQUARE]={
		Sq2H1(A1), Sq2H1(B1), Sq2H1(C1), Sq2H1(D1), Sq2H1(E1), Sq2H1(F1), Sq2H1(G1), Sq2H1(H1),
		Sq2H1(A2), Sq2H1(B2), Sq2H1(C2), Sq2H1(D2), Sq2H1(E2), Sq2H1(F2), Sq2H1(G2), Sq2H1(H2),
		Sq2H1(A3), Sq2H1(B3), Sq2H1(C3), Sq2H1(D3), Sq2H1(E3), Sq2H1(F3), Sq2H1(G3), Sq2H1(H3),
		Sq2H1(A4), Sq2H1(B4), Sq2H1(C4), Sq2H1(D4), Sq2H1(E4), Sq2H1(F4), Sq2H1(G4), Sq2H1(H4),
		Sq2H1(A5), Sq2H1(B5), Sq2H1(C5), Sq2H1(D5), Sq2H1(E5), Sq2H1(F5), Sq2H1(G5), Sq2H1(H5),
		Sq2H1(A6), Sq2H1(B6), Sq2H1(C6), Sq2H1(D6), Sq2H1(E6), Sq2H1(F6), Sq2H1(G6), Sq2H1(H6),
		Sq2H1(A7), Sq2H1(B7), Sq2H1(C7), Sq2H1(D7), Sq2H1(E7), Sq2H1(F7), Sq2H1(G7), Sq2H1(H7),
		Sq2H1(A8), Sq2H1(B8), Sq2H1(C8), Sq2H1(D8), Sq2H1(E8), Sq2H1(F8), Sq2H1(G8), Sq2H1(H8) };

// bitboard for all knight attacks
static const Bitboard knightAttacks[ALL_SQUARE]={
		0x0000000000020400ULL,0x0000000000050800ULL,0x00000000000a1100ULL,0x0000000000142200ULL,0x0000000000284400ULL,0x0000000000508800ULL,0x0000000000a01000ULL,0x0000000000402000ULL,
		0x0000000002040004ULL,0x0000000005080008ULL,0x000000000a110011ULL,0x0000000014220022ULL,0x0000000028440044ULL,0x0000000050880088ULL,0x00000000a0100010ULL,0x0000000040200020ULL,
		0x0000000204000402ULL,0x0000000508000805ULL,0x0000000a1100110aULL,0x0000001422002214ULL,0x0000002844004428ULL,0x0000005088008850ULL,0x000000a0100010a0ULL,0x0000004020002040ULL,
		0x0000020400040200ULL,0x0000050800080500ULL,0x00000a1100110a00ULL,0x0000142200221400ULL,0x0000284400442800ULL,0x0000508800885000ULL,0x0000a0100010a000ULL,0x0000402000204000ULL,
		0x0002040004020000ULL,0x0005080008050000ULL,0x000a1100110a0000ULL,0x0014220022140000ULL,0x0028440044280000ULL,0x0050880088500000ULL,0x00a0100010a00000ULL,0x0040200020400000ULL,
		0x0204000402000000ULL,0x0508000805000000ULL,0x0a1100110a000000ULL,0x1422002214000000ULL,0x2844004428000000ULL,0x5088008850000000ULL,0xa0100010a0000000ULL,0x4020002040000000ULL,
		0x0400040200000000ULL,0x0800080500000000ULL,0x1100110a00000000ULL,0x2200221400000000ULL,0x4400442800000000ULL,0x8800885000000000ULL,0x100010a000000000ULL,0x2000204000000000ULL,
		0x0004020000000000ULL,0x0008050000000000ULL,0x00110a0000000000ULL,0x0022140000000000ULL,0x0044280000000000ULL,0x0088500000000000ULL,0x0010a00000000000ULL,0x0020400000000000ULL};

// bitboard for all white pawn attacks
static const Bitboard whitePawnAttacks[ALL_SQUARE]={
		0x0000000000000300ULL,0x0000000000000700ULL,0x0000000000000e00ULL,0x0000000000001c00ULL,0x0000000000003800ULL,0x0000000000007000ULL,0x000000000000e000ULL,0x000000000000c000ULL,
		0x0000000001030000ULL,0x0000000002070000ULL,0x00000000040e0000ULL,0x00000000081c0000ULL,0x0000000010380000ULL,0x0000000020700000ULL,0x0000000040e00000ULL,0x0000000080c00000ULL,
		0x0000000003000000ULL,0x0000000007000000ULL,0x000000000e000000ULL,0x000000001c000000ULL,0x0000000038000000ULL,0x0000000070000000ULL,0x00000000e0000000ULL,0x00000000c0000000ULL,
		0x0000000300000000ULL,0x0000000700000000ULL,0x0000000e00000000ULL,0x0000001c00000000ULL,0x0000003800000000ULL,0x0000007000000000ULL,0x000000e000000000ULL,0x000000c000000000ULL,
		0x0000030000000000ULL,0x0000070000000000ULL,0x00000e0000000000ULL,0x00001c0000000000ULL,0x0000380000000000ULL,0x0000700000000000ULL,0x0000e00000000000ULL,0x0000c00000000000ULL,
		0x0003000000000000ULL,0x0007000000000000ULL,0x000e000000000000ULL,0x001c000000000000ULL,0x0038000000000000ULL,0x0070000000000000ULL,0x00e0000000000000ULL,0x00c0000000000000ULL,
		0x0300000000000000ULL,0x0700000000000000ULL,0x0e00000000000000ULL,0x1c00000000000000ULL,0x3800000000000000ULL,0x7000000000000000ULL,0xe000000000000000ULL,0xc000000000000000ULL,
		0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL};

// bitboard for all black pawn attacks
static const Bitboard blackPawnAttacks[ALL_SQUARE]={
		0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,
		0x0000000000000003ULL,0x0000000000000007ULL,0x000000000000000eULL,0x000000000000001cULL,0x0000000000000038ULL,0x0000000000000070ULL,0x00000000000000e0ULL,0x00000000000000c0ULL,
		0x0000000000000300ULL,0x0000000000000700ULL,0x0000000000000e00ULL,0x0000000000001c00ULL,0x0000000000003800ULL,0x0000000000007000ULL,0x000000000000e000ULL,0x000000000000c000ULL,
		0x0000000000030000ULL,0x0000000000070000ULL,0x00000000000e0000ULL,0x00000000001c0000ULL,0x0000000000380000ULL,0x0000000000700000ULL,0x0000000000e00000ULL,0x0000000000c00000ULL,
		0x0000000003000000ULL,0x0000000007000000ULL,0x000000000e000000ULL,0x000000001c000000ULL,0x0000000038000000ULL,0x0000000070000000ULL,0x00000000e0000000ULL,0x00000000c0000000ULL,
		0x0000000300000000ULL,0x0000000700000000ULL,0x0000000e00000000ULL,0x0000001c00000000ULL,0x0000003800000000ULL,0x0000007000000000ULL,0x000000e000000000ULL,0x000000c000000000ULL,
		0x0000030100000000ULL,0x0000070200000000ULL,0x00000e0400000000ULL,0x00001c0800000000ULL,0x0000381000000000ULL,0x0000702000000000ULL,0x0000e04000000000ULL,0x0000c08000000000ULL,
		0x0003000000000000ULL,0x0007000000000000ULL,0x000e000000000000ULL,0x001c000000000000ULL,0x0038000000000000ULL,0x0070000000000000ULL,0x00e0000000000000ULL,0x00c0000000000000ULL};

// bitboard for all adjacent squares
static const Bitboard adjacentSquares[ALL_SQUARE]={
		0x0000000000000302ULL,0x0000000000000705ULL,0x0000000000000e0aULL,0x0000000000001c14ULL,0x0000000000003828ULL,0x0000000000007050ULL,0x000000000000e0a0ULL,0x000000000000c040ULL,
		0x0000000000030203ULL,0x0000000000070507ULL,0x00000000000e0a0eULL,0x00000000001c141cULL,0x0000000000382838ULL,0x0000000000705070ULL,0x0000000000e0a0e0ULL,0x0000000000c040c0ULL,
		0x0000000003020300ULL,0x0000000007050700ULL,0x000000000e0a0e00ULL,0x000000001c141c00ULL,0x0000000038283800ULL,0x0000000070507000ULL,0x00000000e0a0e000ULL,0x00000000c040c000ULL,
		0x0000000302030000ULL,0x0000000705070000ULL,0x0000000e0a0e0000ULL,0x0000001c141c0000ULL,0x0000003828380000ULL,0x0000007050700000ULL,0x000000e0a0e00000ULL,0x000000c040c00000ULL,
		0x0000030203000000ULL,0x0000070507000000ULL,0x00000e0a0e000000ULL,0x00001c141c000000ULL,0x0000382838000000ULL,0x0000705070000000ULL,0x0000e0a0e0000000ULL,0x0000c040c0000000ULL,
		0x0003020300000000ULL,0x0007050700000000ULL,0x000e0a0e00000000ULL,0x001c141c00000000ULL,0x0038283800000000ULL,0x0070507000000000ULL,0x00e0a0e000000000ULL,0x00c040c000000000ULL,
		0x0302030000000000ULL,0x0705070000000000ULL,0x0e0a0e0000000000ULL,0x1c141c0000000000ULL,0x3828380000000000ULL,0x7050700000000000ULL,0xe0a0e00000000000ULL,0xc040c00000000000ULL,
		0x0203000000000000ULL,0x0507000000000000ULL,0x0a0e000000000000ULL,0x141c000000000000ULL,0x2838000000000000ULL,0x5070000000000000ULL,0xa0e0000000000000ULL,0x40c0000000000000ULL};

// upper bound bitboard mask
static const Bitboard upperMaskBitboard[ALL_SQUARE]={
		Sq2UM(A1), Sq2UM(B1), Sq2UM(C1), Sq2UM(D1), Sq2UM(E1), Sq2UM(F1), Sq2UM(G1), Sq2UM(H1),
		Sq2UM(A2), Sq2UM(B2), Sq2UM(C2), Sq2UM(D2), Sq2UM(E2), Sq2UM(F2), Sq2UM(G2), Sq2UM(H2),
		Sq2UM(A3), Sq2UM(B3), Sq2UM(C3), Sq2UM(D3), Sq2UM(E3), Sq2UM(F3), Sq2UM(G3), Sq2UM(H3),
		Sq2UM(A4), Sq2UM(B4), Sq2UM(C4), Sq2UM(D4), Sq2UM(E4), Sq2UM(F4), Sq2UM(G4), Sq2UM(H4),
		Sq2UM(A5), Sq2UM(B5), Sq2UM(C5), Sq2UM(D5), Sq2UM(E5), Sq2UM(F5), Sq2UM(G5), Sq2UM(H5),
		Sq2UM(A6), Sq2UM(B6), Sq2UM(C6), Sq2UM(D6), Sq2UM(E6), Sq2UM(F6), Sq2UM(G6), Sq2UM(H6),
		Sq2UM(A7), Sq2UM(B7), Sq2UM(C7), Sq2UM(D7), Sq2UM(E7), Sq2UM(F7), Sq2UM(G7), Sq2UM(H7),
		Sq2UM(A8), Sq2UM(B8), Sq2UM(C8), Sq2UM(D8), Sq2UM(E8), Sq2UM(F8), Sq2UM(G8), Sq2UM(H8) };

// lower bound bitboard mask
static const Bitboard lowerMaskBitboard[ALL_SQUARE]={
		Sq2LM(A1), Sq2LM(B1), Sq2LM(C1), Sq2LM(D1), Sq2LM(E1), Sq2LM(F1), Sq2LM(G1), Sq2LM(H1),
		Sq2LM(A2), Sq2LM(B2), Sq2LM(C2), Sq2LM(D2), Sq2LM(E2), Sq2LM(F2), Sq2LM(G2), Sq2LM(H2),
		Sq2LM(A3), Sq2LM(B3), Sq2LM(C3), Sq2LM(D3), Sq2LM(E3), Sq2LM(F3), Sq2LM(G3), Sq2LM(H3),
		Sq2LM(A4), Sq2LM(B4), Sq2LM(C4), Sq2LM(D4), Sq2LM(E4), Sq2LM(F4), Sq2LM(G4), Sq2LM(H4),
		Sq2LM(A5), Sq2LM(B5), Sq2LM(C5), Sq2LM(D5), Sq2LM(E5), Sq2LM(F5), Sq2LM(G5), Sq2LM(H5),
		Sq2LM(A6), Sq2LM(B6), Sq2LM(C6), Sq2LM(D6), Sq2LM(E6), Sq2LM(F6), Sq2LM(G6), Sq2LM(H6),
		Sq2LM(A7), Sq2LM(B7), Sq2LM(C7), Sq2LM(D7), Sq2LM(E7), Sq2LM(F7), Sq2LM(G7), Sq2LM(H7),
		Sq2LM(A8), Sq2LM(B8), Sq2LM(C8), Sq2LM(D8), Sq2LM(E8), Sq2LM(F8), Sq2LM(G8), Sq2LM(H8) };


// Move representation
struct Move {

	Move* next;

	Move()
	{}
	Move(Square fromSquare, Square toSquare, PieceTypeByColor piece) :
		from(fromSquare), to(toSquare), promotionPiece(piece)
		{}
	Move(Move* nextMove, Square fromSquare, Square toSquare, PieceTypeByColor piece) :
		next(nextMove), from(fromSquare), to(toSquare), promotionPiece(piece)
		{}

	Square from;
	Square to;
	PieceTypeByColor promotionPiece;
};

struct MoveBackup {
	MoveBackup()
	{}

	bool hasWhiteKingCastle;
	bool hasWhiteQueenCastle;
	bool hasBlackKingCastle;
	bool hasBlackQueenCastle;

	bool hasCapture;
	bool hasPromotion;

	PieceTypeByColor capturedPiece;
	Square capturedSquare;

	CastleRight whiteCastleRight;
	CastleRight blackCastleRight;
	Square enPassant;

	Move move;

};

// the board node representation
struct Node {

	Node () : key(0ULL), piece(), pieceCount()
	{}

	Node (const Node& node) : key(0ULL), piece( node.piece ), pieceCount( node.pieceCount )
	{
		for(register int x=0;x<ALL_SQUARE;x++){
			square[x]=node.square[x];
		}
		pieceColor[WHITE]=node.pieceColor[WHITE];
		pieceColor[BLACK]=node.pieceColor[BLACK];
		castleRight[WHITE]=node.castleRight[WHITE];
		castleRight[BLACK]=node.castleRight[BLACK];
		enPassant=node.enPassant;
		sideToMove=node.sideToMove;

	}

	Bitboard key;

	union Piece
	{
		Bitboard array[ALL_PIECE_TYPE_BY_COLOR];
		struct Pieces {
			Bitboard whitePawn;
			Bitboard whiteKnight;
			Bitboard whiteBishop;
			Bitboard whiteRook;
			Bitboard whiteQueen;
			Bitboard whiteKing;
			Bitboard blackPawn;
			Bitboard blackKnight;
			Bitboard blackBishop;
			Bitboard blackRook;
			Bitboard blackQueen;
			Bitboard blackKing;
		} data;

	}piece;

	union PieceCount
	{
		int array[ALL_PIECE_TYPE_BY_COLOR];
		struct Pieces {
			int whitePawn;
			int whiteKnight;
			int whiteBishop;
			int whiteRook;
			int whiteQueen;
			int whiteKing;
			int blackPawn;
			int blackKnight;
			int blackBishop;
			int blackRook;
			int blackQueen;
			int blackKing;
		}data;
	}pieceCount;

	CastleRight castleRight[ALL_PIECE_COLOR];
	Square enPassant;
	PieceColor sideToMove;
	PieceTypeByColor square[ALL_SQUARE];
	Bitboard pieceColor[ALL_PIECE_COLOR];

	// clear structure node
	void clear()
	{
		key=0ULL;
		for(register int x=0;x<ALL_PIECE_TYPE_BY_COLOR;x++){
			piece.array[x]=0ULL;
			pieceCount.array[x]=0;
		}

		for(register int x=0;x<ALL_SQUARE;x++){
			square[x]=EMPTY;
		}
		pieceColor[WHITE]=0ULL;
		pieceColor[BLACK]=0ULL;
		castleRight[WHITE]=NO_CASTLE;
		castleRight[BLACK]=NO_CASTLE;
		enPassant=NONE;
		sideToMove=COLOR_NONE;
	}

};

typedef boost::object_pool<Move> MovePool;

static const char pieceChar[ALL_PIECE_TYPE_BY_COLOR+1] = "pnbrqkPNBRQK ";

}

using namespace BoardTypes;

class Board
{

public:

	Board();
	Board(const Board& board);
	virtual ~Board();

	const void printBoard();
	void doMove(const Move move, MoveBackup& backup);
	void undoMove(MoveBackup& backup);
	void setInitialPosition();
	void loadFromString(const std::string startPosMoves);
	const CastleRight getCastleRights(PieceColor color) const;
	void removeCastleRights(const PieceColor color, const CastleRight castle);
	void setCastleRights(const PieceColor color, const CastleRight castle);
	const PieceColor getSideToMove() const;
	void setSideToMove(const PieceColor color);
	const Square getEnPassant() const;
	void setEnPassant(const Square square);
	const Square bitboardToSquare(const Bitboard bitboard) const;
	const PieceColor flipSide(const PieceColor color);
	const PieceColor getPieceColor(const PieceTypeByColor piece) const;
	const PieceType getPieceType(const PieceTypeByColor piece) const;


	const Bitboard getPiecesByColor(const PieceColor color) const;
	const Bitboard getAllPieces() const;
	const Bitboard getPiecesByType(const PieceTypeByColor piece) const;
	inline const PieceTypeByColor getPieceBySquare(const Square square) const;
	const int getPieceCountByType(const PieceTypeByColor piece) const;

	const Bitboard getAttacksTo(const Square square);

	const Move* generateCaptures(MovePool& movePool);
	const Move* generateNonCaptures(MovePool& movePool);
	const Move* generateCheckEvasions(MovePool& movePool);

	const Bitboard getRookAttacks(const Square square);
	const Bitboard getRookAttacks(const Square square, const Bitboard occupied);
	const Bitboard getBishopAttacks(const Square square);
	const Bitboard getBishopAttacks(const Square square, const Bitboard occupied);
	const Bitboard getQueenAttacks(const Square square);
	const Bitboard getQueenAttacks(const Square square, const Bitboard occupied);
	const Bitboard getKnightAttacks(const Square square);
	const Bitboard getKnightAttacks(const Square square, const Bitboard occupied);
	const Bitboard getPawnAttacks(const Square square);
	const Bitboard getPawnAttacks(const Square square, const Bitboard occupied);
	const Bitboard getKingAttacks(const Square square);
	const Bitboard getKingAttacks(const Square square, const Bitboard occupied);
	const Bitboard getAttacksFrom(const Square square);
	const Bitboard getAttacksFrom(const Square square, const Bitboard occupied);

	const Bitboard getAttackedSquares(const PieceColor color);

private:

	Node& getBoard();
	void clearBoard();
	bool putPiece(const PieceTypeByColor piece, const Square square);
	bool removePiece(const PieceTypeByColor piece, const Square square);

	const void printBitboard(Bitboard bb) const;

	void setOccupiedNeighbor(const Bitboard mask, const Square start, Square& minor, Square& major);

	const Square extractLSB(Bitboard& bitboard);

	Node& currentBoard;
};
// get the board structure
inline Node& Board::getBoard()
{
	return currentBoard;
}

inline void Board::clearBoard()
{
	currentBoard.clear();
}
// put a piece in the board and store piece info
inline bool Board::putPiece(const PieceTypeByColor piece, const Square square)
{

	currentBoard.piece.array[piece] |= squareToBitboard[square];
	currentBoard.pieceColor[pieceColor[piece]] |= squareToBitboard[square];
	currentBoard.square[square] = piece;
	currentBoard.pieceCount.array[piece]++;

	return true;
}
// remove a piece from the board and erase piece info
inline bool Board::removePiece(const PieceTypeByColor piece, const Square square)
{

	currentBoard.piece.array[piece] ^= squareToBitboard[square];
	currentBoard.pieceColor[pieceColor[piece]] ^= squareToBitboard[square];
	currentBoard.square[square] = EMPTY;
	currentBoard.pieceCount.array[piece]--;

	return true;
}

// get castle rights
inline const CastleRight Board::getCastleRights(PieceColor color) const
{
	return currentBoard.castleRight[color];
}

// remove castle rights passed as params
inline void Board::removeCastleRights(const PieceColor color, const CastleRight castle)
{
	currentBoard.castleRight[color]=CastleRight((int)currentBoard.castleRight[color]&(~(int)castle));
}

// set castle rights
inline void Board::setCastleRights(const PieceColor color, const CastleRight castle)
{
	currentBoard.castleRight[color]=castle;
}

// get
inline const PieceColor Board::getSideToMove() const
{
	return currentBoard.sideToMove;
}

// set
inline void Board::setSideToMove(const PieceColor color)
{
	currentBoard.sideToMove=color;
}

// get en passant
inline const Square Board::getEnPassant() const
{
	return currentBoard.enPassant;
}

// set en passant
inline void Board::setEnPassant(const Square square)
{
	currentBoard.enPassant=square;
}

// get the bit index from a bitboard
inline const Square Board::bitboardToSquare(const Bitboard bitboard) const {

	unsigned int square = 0;
	unsigned char ret;

	ret = _BitScanForward(&square, bitboard);
	if (!ret) {
		return Square(NONE);
	}

	return Square( square );

}

// flip side
inline const PieceColor Board::flipSide(const PieceColor color) {
	return PieceColor((int)color ^ 1);
}

// get piece color
inline const PieceColor Board::getPieceColor(const PieceTypeByColor piece) const {
	return pieceColor[piece];
}

// get piece type
inline const PieceType Board::getPieceType(const PieceTypeByColor piece) const {
	return pieceType[piece];
}

// get all pieces of a given color
inline const Bitboard Board::getPiecesByColor(const PieceColor color) const {
	return currentBoard.pieceColor[color];
}

// get all pieces
inline const Bitboard Board::getAllPieces() const {
	return currentBoard.pieceColor[WHITE] | currentBoard.pieceColor[BLACK];
}
// get pieces by type
inline const Bitboard Board::getPiecesByType(const PieceTypeByColor piece) const {
	return currentBoard.piece.array[piece];
}

// get pieces by square
inline const PieceTypeByColor Board::getPieceBySquare(const Square square) const {
	return currentBoard.square[square];
}

// get piece count by type
inline const int Board::getPieceCountByType(const PieceTypeByColor piece) const {
	return currentBoard.pieceCount.array[piece];
}

// print a bitboard in a readble form
inline const void Board::printBitboard(Bitboard bb) const {

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
// lookup and set the nearest bits given a starting square index in the bitboard - downside and upside
inline void Board::setOccupiedNeighbor(const Bitboard mask, const Square start, Square& minor, Square& major)
{

	unsigned int minorInt=A1;
	unsigned int majorInt=H8;
	unsigned char ret;

	if (!mask) {
		minor=A1;
		major=H8;
		return;
	}

	Bitboard lowerMask= mask & lowerMaskBitboard[start];
	Bitboard upperMask= mask & upperMaskBitboard[start];

	ret = _BitScanReverse(&minorInt, lowerMask);
	if (!ret) {
		minorInt=A1;
	}
	ret = _BitScanForward(&majorInt, upperMask);
	if (!ret) {
		majorInt=H8;
	}

	if (minorInt==-1023) {
		minorInt=A1;
	}
	minor = Square(minorInt);
	major = Square(majorInt);
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getRookAttacks(const Square square) {
	return getRookAttacks(square, getAllPieces());
}

// return a bitboard with attacked squares by the rook in the given square
inline const Bitboard Board::getRookAttacks(const Square square, const Bitboard occupied) {

	Square minor;
	Square major;

	this->setOccupiedNeighbor(((fileAttacks[square] ^ squareToBitboard[square]) & occupied) , square, minor, major);
	Bitboard file = bitsBetween(fileAttacks[square], minor, major) ^ squareToBitboard[square];
	this->setOccupiedNeighbor(((rankAttacks[square]^ squareToBitboard[square]) & occupied) , square, minor, major);
	Bitboard rank = bitsBetween(rankAttacks[square], minor, major) ^ squareToBitboard[square];

	return file | rank;
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getBishopAttacks(const Square square) {
	return getBishopAttacks(square, getAllPieces());
}

// return a bitboard with attacked squares by the bishop in the given square
inline const Bitboard Board::getBishopAttacks(const Square square, const Bitboard occupied) {

	Square minor;
	Square major;

	this->setOccupiedNeighbor(((diagA1H8Attacks[square]^ squareToBitboard[square]) & occupied), square, minor, major);
	Bitboard diagA1H8 = bitsBetween(diagA1H8Attacks[square], minor, major) ^ squareToBitboard[square];
	this->setOccupiedNeighbor(((diagH1A8Attacks[square]^ squareToBitboard[square]) & occupied), square, minor, major);
	Bitboard diagH1A8 = bitsBetween(diagH1A8Attacks[square], minor, major) ^ squareToBitboard[square];

	return diagA1H8 | diagH1A8;
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getQueenAttacks(const Square square) {
	return getQueenAttacks(square, getAllPieces());
}

// return a bitboard with attacked squares by the queen in the given square
inline const Bitboard Board::getQueenAttacks(const Square square, const Bitboard occupied) {
	return getBishopAttacks(square, occupied) | getRookAttacks(square, occupied);
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getKnightAttacks(const Square square) {
	return knightAttacks[square];
}

// return a bitboard with attacked squares by the pawn in the given square
inline const Bitboard Board::getKnightAttacks(const Square square, const Bitboard occupied) {
	return knightAttacks[square] & occupied;
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getPawnAttacks(const Square square) {
	return getPawnAttacks(square, getAllPieces());
}

// return a bitboard with attacked squares by the pawn in the given square
inline const Bitboard Board::getPawnAttacks(const Square square, const Bitboard occupied) {

	Bitboard moves;
	Bitboard captures;
	Bitboard occ = occupied;
	Bitboard pawnAttacks;
	int move=8;
	int doubleMove=16;

	if (getPieceBySquare(square)==EMPTY) {
		return EMPTY_BB;
	}

	if (getPieceColor(getPieceBySquare(square))==WHITE) {
		move=8;
		doubleMove=16;
		pawnAttacks=whitePawnAttacks[square];
	} else {
		move=-8;
		doubleMove=-16;
		pawnAttacks=blackPawnAttacks[square];
	}

	if (getEnPassant()!=NONE) {
		occ |= (squareToBitboard[getEnPassant()]+move)&adjacentSquares[square]; // en passant
	}
	if (squareRank[square]==RANK_2 || squareRank[square]==RANK_7) {
		if (squareToBitboard[square+move]&occ) {
			occ |= squareToBitboard[square+doubleMove]; // double move
		}
	}

	moves = (fileAttacks[square] & pawnAttacks) & ~occ ;
	captures = (diagA1H8Attacks[square]|diagH1A8Attacks[square]) & pawnAttacks & occ ;

	return moves | captures;
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getKingAttacks(const Square square) {
	return adjacentSquares[square];
}

// return a bitboard with attacked squares by the King in the given square
inline const Bitboard Board::getKingAttacks(const Square square, const Bitboard occupied) {
	return adjacentSquares[square] & occupied;
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getAttacksFrom(const Square square) {
	return getAttacksFrom(square, getAllPieces());
}

// return a bitboard with attacked squares by the piece in the given square
inline const Bitboard Board::getAttacksFrom(const Square square, const Bitboard occupied) {

	PieceType type = getPieceType(getPieceBySquare(square));

	switch (type) {
	case PIECE_EMPTY:
		return EMPTY_BB;
		break;
	case PAWN:
		return getPawnAttacks(square);
		break;
	case KNIGHT:
		return getKnightAttacks(square);
		break;
	case BISHOP:
		return getBishopAttacks(square);
		break;
	case ROOK:
		return getRookAttacks(square);
		break;
	case QUEEN:
		return getQueenAttacks(square);
		break;
	case KING:
		return getKingAttacks(square);
		break;
	default:
		break;
	}

	return EMPTY_BB;
}

// extract least significant bit of a bitboard
inline const Square Board::extractLSB(Bitboard& bitboard) {

	if (!bitboard) {
		return Square(NONE);
	}

	unsigned int square = 0;
	unsigned char ret;

	ret = _BitScanForward(&square, bitboard);
	bitboard &= bitboard - 1;

	if (!ret) {
		return Square(NONE);
	}

	return Square( square );

}

// get a bitboard with pieces attacking the give square
inline const Bitboard Board::getAttacksTo(const Square square){


	Bitboard all = this->getAllPieces();
	Bitboard attacks = EMPTY_BB;

	Square from = this->extractLSB(all);

	while ( from!=NONE ) {
		if (this->getAttacksFrom(from) & squareToBitboard[square]) {
			attacks |= squareToBitboard[from];
		}
		from = this->extractLSB(all);
	}

	return attacks;
}

// get the set of attacked squares
inline const Bitboard Board::getAttackedSquares(const PieceColor color) {

	Bitboard all = this->getPiecesByColor(color);
	Bitboard attacks = EMPTY_BB;

	Square from = this->extractLSB(all);

	while ( from!=NONE ) {
		//std::cout << from << std::endl;
		//this->printBitboard(this->getAttacksFrom(from));
		attacks |= this->getAttacksFrom(from);
		from = this->extractLSB(all);
	}

	return attacks;

}



#endif /* BOARD_H_ */
