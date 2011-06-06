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
 * Bitboard.h
 *
 *  Created on: Dec 10, 2009
 *      Author: bhlangonijr
 */

#ifndef BITBOARD_H_
#define BITBOARD_H_
#include <inttypes.h>
#include <iostream>
#include <stdlib.h>
#include <sys/time.h>
//Bitboard type - unsigned long int (8 bytes)
typedef uint64_t Bitboard;
#define SqBB(S)					(0x1ULL << static_cast<int>(S))										// Encode a square enum to a bitboard
#define Sq2Bb(X)				(squareToBitboard[X]) 												// square to bitboard macro
#define Sq2FA(X)				(fileBB[squareFile[X]]^squareToBitboard[X])							// Encode Square to File Attack
#define Sq2RA(X)				(rankBB[squareRank[X]]^squareToBitboard[X])							// Encode Square to Rank Attack
#define Sq2A1(X)				(diagonalA1H8BB[squareToDiagonalA1H8[X]]^squareToBitboard[X])		// Encode Square to Diagonal A1H1 Attack
#define Sq2H1(X)				(diagonalH1A8BB[squareToDiagonalH1A8[X]]^squareToBitboard[X])			// Encode Square to Diagonal H1A1 Attack
#define Sq2UM(X)				(~(squareToBitboard[X]-1)) 											// Encode Square to BB uppermask
#define Sq2LM(X)				(squareToBitboard[X]-1)												// Encode Square to BB lowermask
#define St2Sq(F,R)				(((static_cast<int>(F)-96)+(static_cast<int>(R)-49)*8)-1)			// encode String to Square enum
#define bitsBetween(BB,S1,S2)		(((squareToBitboard[S2]|(squareToBitboard[S2]-squareToBitboard[S1]))) & BB)
#define NFILE(X) ((squareFile[X]!=FILE_H ? fileBB[squareFile[X]+1] : EMPTY_BB) | \
		(squareFile[X]!=FILE_A ? fileBB[squareFile[X]-1] : EMPTY_BB))
#define MS(X,Y) ((Y)+(X<<16))
#define AVG(X,Y) ((X+Y)/2)
const int ALL_PIECE_TYPE =				7;  														// pawn, knight, bishop, rook, queen, king
const int ALL_PIECE_TYPE_BY_COLOR =		13; 														// (black, white) X (pawn, knight, bishop, rook, queen, king) + empty
const int ALL_PIECE_COLOR	=			3;  														// black, white, none
const int ALL_SQUARE	=				65; 														// all square A1 .. H8
const int ALL_RANK	=					8;															// all ranks
const int ALL_FILE	=					8;															// all files
const int ALL_DIAGONAL	=				15;															// all diagonals
const Bitboard FULL_BB =		0xFFFFFFFFFFFFFFFFULL;
const Bitboard EMPTY_BB	=		0x0ULL;
const Bitboard WHITE_SQUARES =	0x55AA55AA55AA55AAULL;
const Bitboard BLACK_SQUARES =	0xAA55AA55AA55AA55ULL;
const Bitboard MID_BOARD =		0x00FFFFFFFFFFFF00ULL;
const uint64_t debruijn64 = 0x07EDD5E59A4E28C2ULL;
const uint32_t index64[64] = {
		63,  0, 58,  1, 59, 47, 53,  2,
		60, 39, 48, 27, 54, 33, 42,  3,
		61, 51, 37, 40, 49, 18, 28, 20,
		55, 30, 34, 11, 43, 14, 22,  4,
		62, 57, 46, 52, 38, 26, 32, 41,
		50, 36, 17, 19, 29, 10, 13, 21,
		56, 45, 25, 31, 35, 16,  9, 12,
		44, 24, 15,  8, 23,  7,  6,  5
};
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
//ranks - row
enum Rank {
	RANK_1=0, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE
};
//files - column
enum File {
	FILE_A=0, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE
};
// diagonals A1..H8
enum DiagonalA1H8 {
	A8_A8, B8_A7, C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3, G1_H2, H1_H1
};
// diagonals A1..H8
enum DiagonalH1A8 {
	A1_A1, B1_A2, C1_A3,D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6, G8_H7, H8_H8
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
	WHITE_PAWN=0, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING, BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING, EMPTY
};
// flipped board
const Square flip[ALL_SQUARE] = {
		A8, B8, C8, D8, E8, F8, G8, H8,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A1, B1, C1, D1, E1, F1, G1, H1,
		NONE
};
// array with piece codes
const char pieceChar[ALL_PIECE_TYPE_BY_COLOR+1] = "PNBRQKpnbrqk ";
//color of a given piece
const PieceColor pieceColor[ALL_PIECE_TYPE_BY_COLOR] = {
		WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, COLOR_NONE
};
// type of a given piece
const PieceType pieceType[ALL_PIECE_TYPE_BY_COLOR] = {
		PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_EMPTY
};
// make piece color by color and piece type
const PieceTypeByColor pieceTypeByColor[ALL_PIECE_COLOR][ALL_PIECE_TYPE] = {
		{WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING, EMPTY},
		{BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}
};
//Encode square to String
const std::string squareToString[ALL_SQUARE]= {
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};
// Rank of a given square
const Rank squareRank[ALL_SQUARE]={
		RANK_1, RANK_1, RANK_1, RANK_1, RANK_1, RANK_1, RANK_1, RANK_1,
		RANK_2, RANK_2, RANK_2, RANK_2, RANK_2, RANK_2, RANK_2, RANK_2,
		RANK_3, RANK_3, RANK_3, RANK_3, RANK_3, RANK_3, RANK_3, RANK_3,
		RANK_4, RANK_4, RANK_4, RANK_4, RANK_4, RANK_4, RANK_4, RANK_4,
		RANK_5, RANK_5, RANK_5, RANK_5, RANK_5, RANK_5, RANK_5, RANK_5,
		RANK_6, RANK_6, RANK_6, RANK_6, RANK_6, RANK_6, RANK_6, RANK_6,
		RANK_7, RANK_7, RANK_7, RANK_7, RANK_7, RANK_7, RANK_7, RANK_7,
		RANK_8, RANK_8, RANK_8, RANK_8, RANK_8, RANK_8, RANK_8, RANK_8,
		RANK_NONE
};
// File of a given square
const File squareFile[ALL_SQUARE]={
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
		FILE_NONE
};
//encode square by rank & file
const Square encodeSquare[ALL_RANK][ALL_FILE]= {
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
const Bitboard squareToBitboard[ALL_SQUARE]={
		SqBB(A1), SqBB(B1), SqBB(C1), SqBB(D1), SqBB(E1), SqBB(F1), SqBB(G1), SqBB(H1),
		SqBB(A2), SqBB(B2), SqBB(C2), SqBB(D2), SqBB(E2), SqBB(F2), SqBB(G2), SqBB(H2),
		SqBB(A3), SqBB(B3), SqBB(C3), SqBB(D3), SqBB(E3), SqBB(F3), SqBB(G3), SqBB(H3),
		SqBB(A4), SqBB(B4), SqBB(C4), SqBB(D4), SqBB(E4), SqBB(F4), SqBB(G4), SqBB(H4),
		SqBB(A5), SqBB(B5), SqBB(C5), SqBB(D5), SqBB(E5), SqBB(F5), SqBB(G5), SqBB(H5),
		SqBB(A6), SqBB(B6), SqBB(C6), SqBB(D6), SqBB(E6), SqBB(F6), SqBB(G6), SqBB(H6),
		SqBB(A7), SqBB(B7), SqBB(C7), SqBB(D7), SqBB(E7), SqBB(F7), SqBB(G7), SqBB(H7),
		SqBB(A8), SqBB(B8), SqBB(C8), SqBB(D8), SqBB(E8), SqBB(F8), SqBB(G8), SqBB(H8),
		EMPTY_BB
};
// bitboard for all ranks
const Bitboard rankBB[ALL_RANK]={
		0x00000000000000FFULL,0x000000000000FF00ULL,0x0000000000FF0000ULL,0x00000000FF000000ULL,
		0x000000FF00000000ULL,0x0000FF0000000000ULL,0x00FF000000000000ULL,0xFF00000000000000ULL
};
// bitboard for all files
const Bitboard fileBB[ALL_FILE]={
		0x0101010101010101ULL,0x0202020202020202ULL,0x0404040404040404ULL,0x0808080808080808ULL,
		0x1010101010101010ULL,0x2020202020202020ULL,0x4040404040404040ULL,0x8080808080808080ULL,

};
// bitboard for black and white space
const Bitboard colorSpaceBB[ALL_PIECE_COLOR] ={
		0x00000000FFFFFFFFULL, 0xFFFFFFFF00000000ULL, EMPTY_BB
};
// bitboard for all diagonal A1..H8
const Bitboard diagonalA1H8BB[ALL_DIAGONAL]={
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
		Sq2Bb(H1)
};
// square to enum diagonal A1..H8
const DiagonalA1H8 squareToDiagonalA1H8[ALL_SQUARE]={
		H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3, G1_H2, H1_H1,
		G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3, G1_H2,
		F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3,
		E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4,
		D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5,
		C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6,
		B8_A7, C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7,
		A8_A8, B8_A7, C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1
};
// bitboard for all diagonal H1..A8
const Bitboard diagonalH1A8BB[ALL_DIAGONAL]={
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
		Sq2Bb(H8)
};
// square to enum diagonal A1..H8
const DiagonalH1A8 squareToDiagonalH1A8[ALL_SQUARE]={
		A1_A1, B1_A2, C1_A3, D1_A4, E1_A5, F1_A6, G1_A7, H1_A8,
		B1_A2, C1_A3, D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2,
		C1_A3, D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3,
		D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4,
		E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5,
		F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6,
		G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6, G8_H7,
		H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6, G8_H7, H8_H8
};
// bitboard for rank attacks
const Bitboard rankAttacks[ALL_SQUARE]={
		Sq2RA(A1), Sq2RA(B1), Sq2RA(C1), Sq2RA(D1), Sq2RA(E1), Sq2RA(F1), Sq2RA(G1), Sq2RA(H1),
		Sq2RA(A2), Sq2RA(B2), Sq2RA(C2), Sq2RA(D2), Sq2RA(E2), Sq2RA(F2), Sq2RA(G2), Sq2RA(H2),
		Sq2RA(A3), Sq2RA(B3), Sq2RA(C3), Sq2RA(D3), Sq2RA(E3), Sq2RA(F3), Sq2RA(G3), Sq2RA(H3),
		Sq2RA(A4), Sq2RA(B4), Sq2RA(C4), Sq2RA(D4), Sq2RA(E4), Sq2RA(F4), Sq2RA(G4), Sq2RA(H4),
		Sq2RA(A5), Sq2RA(B5), Sq2RA(C5), Sq2RA(D5), Sq2RA(E5), Sq2RA(F5), Sq2RA(G5), Sq2RA(H5),
		Sq2RA(A6), Sq2RA(B6), Sq2RA(C6), Sq2RA(D6), Sq2RA(E6), Sq2RA(F6), Sq2RA(G6), Sq2RA(H6),
		Sq2RA(A7), Sq2RA(B7), Sq2RA(C7), Sq2RA(D7), Sq2RA(E7), Sq2RA(F7), Sq2RA(G7), Sq2RA(H7),
		Sq2RA(A8), Sq2RA(B8), Sq2RA(C8), Sq2RA(D8), Sq2RA(E8), Sq2RA(F8), Sq2RA(G8), Sq2RA(H8)
};
// bitboard for file attacks
const Bitboard fileAttacks[ALL_SQUARE]={
		Sq2FA(A1), Sq2FA(B1), Sq2FA(C1), Sq2FA(D1), Sq2FA(E1), Sq2FA(F1), Sq2FA(G1), Sq2FA(H1),
		Sq2FA(A2), Sq2FA(B2), Sq2FA(C2), Sq2FA(D2), Sq2FA(E2), Sq2FA(F2), Sq2FA(G2), Sq2FA(H2),
		Sq2FA(A3), Sq2FA(B3), Sq2FA(C3), Sq2FA(D3), Sq2FA(E3), Sq2FA(F3), Sq2FA(G3), Sq2FA(H3),
		Sq2FA(A4), Sq2FA(B4), Sq2FA(C4), Sq2FA(D4), Sq2FA(E4), Sq2FA(F4), Sq2FA(G4), Sq2FA(H4),
		Sq2FA(A5), Sq2FA(B5), Sq2FA(C5), Sq2FA(D5), Sq2FA(E5), Sq2FA(F5), Sq2FA(G5), Sq2FA(H5),
		Sq2FA(A6), Sq2FA(B6), Sq2FA(C6), Sq2FA(D6), Sq2FA(E6), Sq2FA(F6), Sq2FA(G6), Sq2FA(H6),
		Sq2FA(A7), Sq2FA(B7), Sq2FA(C7), Sq2FA(D7), Sq2FA(E7), Sq2FA(F7), Sq2FA(G7), Sq2FA(H7),
		Sq2FA(A8), Sq2FA(B8), Sq2FA(C8), Sq2FA(D8), Sq2FA(E8), Sq2FA(F8), Sq2FA(G8), Sq2FA(H8)
};
// bitboard for diagonal attacks
const Bitboard diagA1H8Attacks[ALL_SQUARE]={
		Sq2A1(A1), Sq2A1(B1), Sq2A1(C1), Sq2A1(D1), Sq2A1(E1), Sq2A1(F1), Sq2A1(G1), Sq2A1(H1),
		Sq2A1(A2), Sq2A1(B2), Sq2A1(C2), Sq2A1(D2), Sq2A1(E2), Sq2A1(F2), Sq2A1(G2), Sq2A1(H2),
		Sq2A1(A3), Sq2A1(B3), Sq2A1(C3), Sq2A1(D3), Sq2A1(E3), Sq2A1(F3), Sq2A1(G3), Sq2A1(H3),
		Sq2A1(A4), Sq2A1(B4), Sq2A1(C4), Sq2A1(D4), Sq2A1(E4), Sq2A1(F4), Sq2A1(G4), Sq2A1(H4),
		Sq2A1(A5), Sq2A1(B5), Sq2A1(C5), Sq2A1(D5), Sq2A1(E5), Sq2A1(F5), Sq2A1(G5), Sq2A1(H5),
		Sq2A1(A6), Sq2A1(B6), Sq2A1(C6), Sq2A1(D6), Sq2A1(E6), Sq2A1(F6), Sq2A1(G6), Sq2A1(H6),
		Sq2A1(A7), Sq2A1(B7), Sq2A1(C7), Sq2A1(D7), Sq2A1(E7), Sq2A1(F7), Sq2A1(G7), Sq2A1(H7),
		Sq2A1(A8), Sq2A1(B8), Sq2A1(C8), Sq2A1(D8), Sq2A1(E8), Sq2A1(F8), Sq2A1(G8), Sq2A1(H8)
};
// bitboard for diagonal attacks
const Bitboard diagH1A8Attacks[ALL_SQUARE]={
		Sq2H1(A1), Sq2H1(B1), Sq2H1(C1), Sq2H1(D1), Sq2H1(E1), Sq2H1(F1), Sq2H1(G1), Sq2H1(H1),
		Sq2H1(A2), Sq2H1(B2), Sq2H1(C2), Sq2H1(D2), Sq2H1(E2), Sq2H1(F2), Sq2H1(G2), Sq2H1(H2),
		Sq2H1(A3), Sq2H1(B3), Sq2H1(C3), Sq2H1(D3), Sq2H1(E3), Sq2H1(F3), Sq2H1(G3), Sq2H1(H3),
		Sq2H1(A4), Sq2H1(B4), Sq2H1(C4), Sq2H1(D4), Sq2H1(E4), Sq2H1(F4), Sq2H1(G4), Sq2H1(H4),
		Sq2H1(A5), Sq2H1(B5), Sq2H1(C5), Sq2H1(D5), Sq2H1(E5), Sq2H1(F5), Sq2H1(G5), Sq2H1(H5),
		Sq2H1(A6), Sq2H1(B6), Sq2H1(C6), Sq2H1(D6), Sq2H1(E6), Sq2H1(F6), Sq2H1(G6), Sq2H1(H6),
		Sq2H1(A7), Sq2H1(B7), Sq2H1(C7), Sq2H1(D7), Sq2H1(E7), Sq2H1(F7), Sq2H1(G7), Sq2H1(H7),
		Sq2H1(A8), Sq2H1(B8), Sq2H1(C8), Sq2H1(D8), Sq2H1(E8), Sq2H1(F8), Sq2H1(G8), Sq2H1(H8)
};
const Bitboard midBoardNoFileA = MID_BOARD ^ fileBB[FILE_A];
const Bitboard midBoardNoFileH = MID_BOARD ^ fileBB[FILE_H];
// bitboard for all knight attacks
const Bitboard knightAttacks[ALL_SQUARE]={
		0x0000000000020400ULL,0x0000000000050800ULL,0x00000000000a1100ULL,0x0000000000142200ULL,0x0000000000284400ULL,0x0000000000508800ULL,0x0000000000a01000ULL,0x0000000000402000ULL,
		0x0000000002040004ULL,0x0000000005080008ULL,0x000000000a110011ULL,0x0000000014220022ULL,0x0000000028440044ULL,0x0000000050880088ULL,0x00000000a0100010ULL,0x0000000040200020ULL,
		0x0000000204000402ULL,0x0000000508000805ULL,0x0000000a1100110aULL,0x0000001422002214ULL,0x0000002844004428ULL,0x0000005088008850ULL,0x000000a0100010a0ULL,0x0000004020002040ULL,
		0x0000020400040200ULL,0x0000050800080500ULL,0x00000a1100110a00ULL,0x0000142200221400ULL,0x0000284400442800ULL,0x0000508800885000ULL,0x0000a0100010a000ULL,0x0000402000204000ULL,
		0x0002040004020000ULL,0x0005080008050000ULL,0x000a1100110a0000ULL,0x0014220022140000ULL,0x0028440044280000ULL,0x0050880088500000ULL,0x00a0100010a00000ULL,0x0040200020400000ULL,
		0x0204000402000000ULL,0x0508000805000000ULL,0x0a1100110a000000ULL,0x1422002214000000ULL,0x2844004428000000ULL,0x5088008850000000ULL,0xa0100010a0000000ULL,0x4020002040000000ULL,
		0x0400040200000000ULL,0x0800080500000000ULL,0x1100110a00000000ULL,0x2200221400000000ULL,0x4400442800000000ULL,0x8800885000000000ULL,0x100010a000000000ULL,0x2000204000000000ULL,
		0x0004020000000000ULL,0x0008050000000000ULL,0x00110a0000000000ULL,0x0022140000000000ULL,0x0044280000000000ULL,0x0088500000000000ULL,0x0010a00000000000ULL,0x0020400000000000ULL
};
// bitboard for all white pawn attacks
const Bitboard whitePawnAttacks[ALL_SQUARE]={
		0x0000000000000200ULL,0x0000000000000500ULL,0x0000000000000a00ULL,0x0000000000001400ULL,0x0000000000002800ULL,0x0000000000005000ULL,0x000000000000a000ULL,0x0000000000004000ULL,
		0x0000000000020000ULL,0x0000000000050000ULL,0x00000000000a0000ULL,0x0000000000140000ULL,0x0000000000280000ULL,0x0000000000500000ULL,0x0000000000a00000ULL,0x0000000000400000ULL,
		0x0000000002000000ULL,0x0000000005000000ULL,0x000000000a000000ULL,0x0000000014000000ULL,0x0000000028000000ULL,0x0000000050000000ULL,0x00000000a0000000ULL,0x0000000040000000ULL,
		0x0000000200000000ULL,0x0000000500000000ULL,0x0000000a00000000ULL,0x0000001400000000ULL,0x0000002800000000ULL,0x0000005000000000ULL,0x000000a000000000ULL,0x0000004000000000ULL,
		0x0000020000000000ULL,0x0000050000000000ULL,0x00000a0000000000ULL,0x0000140000000000ULL,0x0000280000000000ULL,0x0000500000000000ULL,0x0000a00000000000ULL,0x0000400000000000ULL,
		0x0002000000000000ULL,0x0005000000000000ULL,0x000a000000000000ULL,0x0014000000000000ULL,0x0028000000000000ULL,0x0050000000000000ULL,0x00a0000000000000ULL,0x0040000000000000ULL,
		0x0200000000000000ULL,0x0500000000000000ULL,0x0a00000000000000ULL,0x1400000000000000ULL,0x2800000000000000ULL,0x5000000000000000ULL,0xa000000000000000ULL,0x4000000000000000ULL,
		0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL
};
// bitboard for all black pawn attacks
const Bitboard blackPawnAttacks[ALL_SQUARE]={
		0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,
		0x0000000000000002ULL,0x0000000000000005ULL,0x000000000000000aULL,0x0000000000000014ULL,0x0000000000000028ULL,0x0000000000000050ULL,0x00000000000000a0ULL,0x0000000000000040ULL,
		0x0000000000000200ULL,0x0000000000000500ULL,0x0000000000000a00ULL,0x0000000000001400ULL,0x0000000000002800ULL,0x0000000000005000ULL,0x000000000000a000ULL,0x0000000000004000ULL,
		0x0000000000020000ULL,0x0000000000050000ULL,0x00000000000a0000ULL,0x0000000000140000ULL,0x0000000000280000ULL,0x0000000000500000ULL,0x0000000000a00000ULL,0x0000000000400000ULL,
		0x0000000002000000ULL,0x0000000005000000ULL,0x000000000a000000ULL,0x0000000014000000ULL,0x0000000028000000ULL,0x0000000050000000ULL,0x00000000a0000000ULL,0x0000000040000000ULL,
		0x0000000200000000ULL,0x0000000500000000ULL,0x0000000a00000000ULL,0x0000001400000000ULL,0x0000002800000000ULL,0x0000005000000000ULL,0x000000a000000000ULL,0x0000004000000000ULL,
		0x0000020000000000ULL,0x0000050000000000ULL,0x00000a0000000000ULL,0x0000140000000000ULL,0x0000280000000000ULL,0x0000500000000000ULL,0x0000a00000000000ULL,0x0000400000000000ULL,
		0x0002000000000000ULL,0x0005000000000000ULL,0x000a000000000000ULL,0x0014000000000000ULL,0x0028000000000000ULL,0x0050000000000000ULL,0x00a0000000000000ULL,0x0040000000000000ULL
};
// bitboard for all white pawn moves
const Bitboard whitePawnMoves[ALL_SQUARE]={
		0x0000000000000100ULL,0x0000000000000200ULL,0x0000000000000400ULL,0x0000000000000800ULL,0x0000000000001000ULL,0x0000000000002000ULL,0x0000000000004000ULL,0x0000000000008000ULL,
		0x0000000001010000ULL,0x0000000002020000ULL,0x0000000004040000ULL,0x0000000008080000ULL,0x0000000010100000ULL,0x0000000020200000ULL,0x0000000040400000ULL,0x0000000080800000ULL,
		0x0000000001000000ULL,0x0000000002000000ULL,0x0000000004000000ULL,0x0000000008000000ULL,0x0000000010000000ULL,0x0000000020000000ULL,0x0000000040000000ULL,0x0000000080000000ULL,
		0x0000000100000000ULL,0x0000000200000000ULL,0x0000000400000000ULL,0x0000000800000000ULL,0x0000001000000000ULL,0x0000002000000000ULL,0x0000004000000000ULL,0x0000008000000000ULL,
		0x0000010000000000ULL,0x0000020000000000ULL,0x0000040000000000ULL,0x0000080000000000ULL,0x0000100000000000ULL,0x0000200000000000ULL,0x0000400000000000ULL,0x0000800000000000ULL,
		0x0001000000000000ULL,0x0002000000000000ULL,0x0004000000000000ULL,0x0008000000000000ULL,0x0010000000000000ULL,0x0020000000000000ULL,0x0040000000000000ULL,0x0080000000000000ULL,
		0x0100000000000000ULL,0x0200000000000000ULL,0x0400000000000000ULL,0x0800000000000000ULL,0x1000000000000000ULL,0x2000000000000000ULL,0x4000000000000000ULL,0x8000000000000000ULL,
		0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL
};
// bitboard for all black pawn moves
const Bitboard blackPawnMoves[ALL_SQUARE]={
		0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,0x0000000000000000ULL,
		0x0000000000000001ULL,0x0000000000000002ULL,0x0000000000000004ULL,0x0000000000000008ULL,0x0000000000000010ULL,0x0000000000000020ULL,0x0000000000000040ULL,0x0000000000000080ULL,
		0x0000000000000100ULL,0x0000000000000200ULL,0x0000000000000400ULL,0x0000000000000800ULL,0x0000000000001000ULL,0x0000000000002000ULL,0x0000000000004000ULL,0x0000000000008000ULL,
		0x0000000000010000ULL,0x0000000000020000ULL,0x0000000000040000ULL,0x0000000000080000ULL,0x0000000000100000ULL,0x0000000000200000ULL,0x0000000000400000ULL,0x0000000000800000ULL,
		0x0000000001000000ULL,0x0000000002000000ULL,0x0000000004000000ULL,0x0000000008000000ULL,0x0000000010000000ULL,0x0000000020000000ULL,0x0000000040000000ULL,0x0000000080000000ULL,
		0x0000000100000000ULL,0x0000000200000000ULL,0x0000000400000000ULL,0x0000000800000000ULL,0x0000001000000000ULL,0x0000002000000000ULL,0x0000004000000000ULL,0x0000008000000000ULL,
		0x0000010100000000ULL,0x0000020200000000ULL,0x0000040400000000ULL,0x0000080800000000ULL,0x0000101000000000ULL,0x0000202000000000ULL,0x0000404000000000ULL,0x0000808000000000ULL,
		0x0001000000000000ULL,0x0002000000000000ULL,0x0004000000000000ULL,0x0008000000000000ULL,0x0010000000000000ULL,0x0020000000000000ULL,0x0040000000000000ULL,0x0080000000000000ULL
};
// bitboard for all adjacent squares
const Bitboard adjacentSquares[ALL_SQUARE]={
		0x0000000000000302ULL,0x0000000000000705ULL,0x0000000000000e0aULL,0x0000000000001c14ULL,0x0000000000003828ULL,0x0000000000007050ULL,0x000000000000e0a0ULL,0x000000000000c040ULL,
		0x0000000000030203ULL,0x0000000000070507ULL,0x00000000000e0a0eULL,0x00000000001c141cULL,0x0000000000382838ULL,0x0000000000705070ULL,0x0000000000e0a0e0ULL,0x0000000000c040c0ULL,
		0x0000000003020300ULL,0x0000000007050700ULL,0x000000000e0a0e00ULL,0x000000001c141c00ULL,0x0000000038283800ULL,0x0000000070507000ULL,0x00000000e0a0e000ULL,0x00000000c040c000ULL,
		0x0000000302030000ULL,0x0000000705070000ULL,0x0000000e0a0e0000ULL,0x0000001c141c0000ULL,0x0000003828380000ULL,0x0000007050700000ULL,0x000000e0a0e00000ULL,0x000000c040c00000ULL,
		0x0000030203000000ULL,0x0000070507000000ULL,0x00000e0a0e000000ULL,0x00001c141c000000ULL,0x0000382838000000ULL,0x0000705070000000ULL,0x0000e0a0e0000000ULL,0x0000c040c0000000ULL,
		0x0003020300000000ULL,0x0007050700000000ULL,0x000e0a0e00000000ULL,0x001c141c00000000ULL,0x0038283800000000ULL,0x0070507000000000ULL,0x00e0a0e000000000ULL,0x00c040c000000000ULL,
		0x0302030000000000ULL,0x0705070000000000ULL,0x0e0a0e0000000000ULL,0x1c141c0000000000ULL,0x3828380000000000ULL,0x7050700000000000ULL,0xe0a0e00000000000ULL,0xc040c00000000000ULL,
		0x0203000000000000ULL,0x0507000000000000ULL,0x0a0e000000000000ULL,0x141c000000000000ULL,0x2838000000000000ULL,0x5070000000000000ULL,0xa0e0000000000000ULL,0x40c0000000000000ULL
};
// upper bound bitboard mask
const Bitboard upperMaskBitboard[ALL_SQUARE]={
		Sq2UM(A1), Sq2UM(B1), Sq2UM(C1), Sq2UM(D1), Sq2UM(E1), Sq2UM(F1), Sq2UM(G1), Sq2UM(H1),
		Sq2UM(A2), Sq2UM(B2), Sq2UM(C2), Sq2UM(D2), Sq2UM(E2), Sq2UM(F2), Sq2UM(G2), Sq2UM(H2),
		Sq2UM(A3), Sq2UM(B3), Sq2UM(C3), Sq2UM(D3), Sq2UM(E3), Sq2UM(F3), Sq2UM(G3), Sq2UM(H3),
		Sq2UM(A4), Sq2UM(B4), Sq2UM(C4), Sq2UM(D4), Sq2UM(E4), Sq2UM(F4), Sq2UM(G4), Sq2UM(H4),
		Sq2UM(A5), Sq2UM(B5), Sq2UM(C5), Sq2UM(D5), Sq2UM(E5), Sq2UM(F5), Sq2UM(G5), Sq2UM(H5),
		Sq2UM(A6), Sq2UM(B6), Sq2UM(C6), Sq2UM(D6), Sq2UM(E6), Sq2UM(F6), Sq2UM(G6), Sq2UM(H6),
		Sq2UM(A7), Sq2UM(B7), Sq2UM(C7), Sq2UM(D7), Sq2UM(E7), Sq2UM(F7), Sq2UM(G7), Sq2UM(H7),
		Sq2UM(A8), Sq2UM(B8), Sq2UM(C8), Sq2UM(D8), Sq2UM(E8), Sq2UM(F8), Sq2UM(G8), Sq2UM(H8)
};
// lower bound bitboard mask
const Bitboard lowerMaskBitboard[ALL_SQUARE]={
		Sq2LM(A1), Sq2LM(B1), Sq2LM(C1), Sq2LM(D1), Sq2LM(E1), Sq2LM(F1), Sq2LM(G1), Sq2LM(H1),
		Sq2LM(A2), Sq2LM(B2), Sq2LM(C2), Sq2LM(D2), Sq2LM(E2), Sq2LM(F2), Sq2LM(G2), Sq2LM(H2),
		Sq2LM(A3), Sq2LM(B3), Sq2LM(C3), Sq2LM(D3), Sq2LM(E3), Sq2LM(F3), Sq2LM(G3), Sq2LM(H3),
		Sq2LM(A4), Sq2LM(B4), Sq2LM(C4), Sq2LM(D4), Sq2LM(E4), Sq2LM(F4), Sq2LM(G4), Sq2LM(H4),
		Sq2LM(A5), Sq2LM(B5), Sq2LM(C5), Sq2LM(D5), Sq2LM(E5), Sq2LM(F5), Sq2LM(G5), Sq2LM(H5),
		Sq2LM(A6), Sq2LM(B6), Sq2LM(C6), Sq2LM(D6), Sq2LM(E6), Sq2LM(F6), Sq2LM(G6), Sq2LM(H6),
		Sq2LM(A7), Sq2LM(B7), Sq2LM(C7), Sq2LM(D7), Sq2LM(E7), Sq2LM(F7), Sq2LM(G7), Sq2LM(H7),
		Sq2LM(A8), Sq2LM(B8), Sq2LM(C8), Sq2LM(D8), Sq2LM(E8), Sq2LM(F8), Sq2LM(G8), Sq2LM(H8)
};
// neighbor files bitboards
const Bitboard neighborFiles[ALL_SQUARE]={
		NFILE(A1), NFILE(B1), NFILE(C1), NFILE(D1), NFILE(E1), NFILE(F1), NFILE(G1), NFILE(H1),
		NFILE(A2), NFILE(B2), NFILE(C2), NFILE(D2), NFILE(E2), NFILE(F2), NFILE(G2), NFILE(H2),
		NFILE(A3), NFILE(B3), NFILE(C3), NFILE(D3), NFILE(E3), NFILE(F3), NFILE(G3), NFILE(H3),
		NFILE(A4), NFILE(B4), NFILE(C4), NFILE(D4), NFILE(E4), NFILE(F4), NFILE(G4), NFILE(H4),
		NFILE(A5), NFILE(B5), NFILE(C5), NFILE(D5), NFILE(E5), NFILE(F5), NFILE(G5), NFILE(H5),
		NFILE(A6), NFILE(B6), NFILE(C6), NFILE(D6), NFILE(E6), NFILE(F6), NFILE(G6), NFILE(H6),
		NFILE(A7), NFILE(B7), NFILE(C7), NFILE(D7), NFILE(E7), NFILE(F7), NFILE(G7), NFILE(H7),
		NFILE(A8), NFILE(B8), NFILE(C8), NFILE(D8), NFILE(E8), NFILE(F8), NFILE(G8), NFILE(H8)
};
// middlegame & endgame piece square table
const int pieceSquareTable[ALL_PIECE_TYPE][ALL_SQUARE]={
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

const Bitboard promoRank[ALL_PIECE_COLOR]={rankBB[RANK_7],rankBB[RANK_2],EMPTY_BB};
const Bitboard eighthRank[ALL_PIECE_COLOR]={rankBB[RANK_8],rankBB[RANK_1],EMPTY_BB};
// piece phase increment values
enum PiecePhase {
	PAWN_PHASE_INCREMENT=		0,
	KNIGHT_PHASE_INCREMENT=		2,
	BISHOP_PHASE_INCREMENT=		2,
	ROOT_PHASE_INCREMENT= 		4,
	QUEEN_PHASE_INCREMENT=		8,
	KING_PHASE_INCREMENT=		0
};
//phase increment values
const int phaseIncrement[ALL_PIECE_TYPE] = {PAWN_PHASE_INCREMENT,KNIGHT_PHASE_INCREMENT,BISHOP_PHASE_INCREMENT,
		ROOT_PHASE_INCREMENT,QUEEN_PHASE_INCREMENT,KING_PHASE_INCREMENT};
const int maxGamePhase = 32;
// game phase
enum GamePhase {
	OPENING=		 2,
	MIDDLEGAME=		 maxGamePhase/2,
	ENDGAME=		 26
};
//constants
const int maxScore = 15000;
const int drawScore = 0;
const int winningScore = 2000;
const int maxPieces=32;
const int pawnValue 	= 100;
const int knightValue 	= 415;
const int bishopValue 	= 420;
const int rookValue 	= 640;
const int queenValue 	= 1280;
const int kingValue		= 15000;
const int drawishMinValue = std::min(bishopValue,knightValue);
const int drawishMaxValue = std::max(bishopValue,knightValue);
const int minimalMaterial = kingValue*2+drawishMaxValue;
const int materialValues[ALL_PIECE_TYPE_BY_COLOR] = {
		pawnValue, knightValue, bishopValue, rookValue, queenValue, kingValue,
		pawnValue, knightValue, bishopValue, rookValue, queenValue, kingValue,0
};
extern void printBitboard(Bitboard bb);
extern void initializeBitboards();
extern int squareDistance(const Square from, const Square to);
extern int inverseSquareDistance(const Square from, const Square to);
// return the index of LSB
inline int bitScanForward(int* const index, const uint64_t mask) {
#if defined(__LP64__)
	uint64_t ret;
	asm	("bsfq %[mask], %[ret]" : [ret] "=r" (ret) :[mask] "mr" (mask));
	*index = int(ret);
#else
	*index = int(index64[((mask & -mask) * debruijn64) >> 58]);
#endif
	return mask?1:0;
}
// return the index of MSB
inline int bitScanReverse(int* const index, const uint64_t mask) {
#if defined(__LP64__)
	uint64_t ret;
	asm ("bsrq %[mask], %[ret]" :[ret] "=r" (ret) :[mask] "mr" (mask));
	*index = (unsigned int)ret;
#else
	union {
		double d;
		struct {
			unsigned int mantissal : 32;
			unsigned int mantissah : 20;
			unsigned int exponent : 11;
			unsigned int sign : 1;
		};
	} ud;
	ud.d = (double)(mask & ~(mask >> 32));

	*index = ud.exponent - 1023;
#endif
	return mask?1:0;
}

inline int bitCount(const uint64_t data) {
	if (!data) {
		return 0;
	}
#if defined(__LP64__)
	return __builtin_popcountll( data );
#else
	//from stockfish
	unsigned w = unsigned(data >> 32);
	unsigned v = unsigned(data);
	v -= (v >> 1) & 0x55555555;
	w -= (w >> 1) & 0x55555555;
	v = ((v >> 2) & 0x33333333) + (v & 0x33333333);
	w = ((w >> 2) & 0x33333333) + (w & 0x33333333);
	v = ((v >> 4) + v) & 0x0F0F0F0F;
	v += (((w >> 4) + w) & 0x0F0F0F0F);
	v *= 0x01010101;
	return int(v >> 24);
#endif
}

inline uint32_t bitCount15(const uint64_t data) {
	if (!data) {
		return 0;
	}
#if defined(__LP64__)
	return __builtin_popcountll( data );
#else
	//from stockfish
	unsigned w = unsigned(data >> 32);
	unsigned v = unsigned(data);
	v -= (v >> 1) & 0x55555555;
	w -= (w >> 1) & 0x55555555;
	v = ((v >> 2) & 0x33333333) + (v & 0x33333333);
	w = ((w >> 2) & 0x33333333) + (w & 0x33333333);
	v += w;
	v *= 0x11111111;
	return  int(v >> 28);

#endif
}

// get the bit index from a bitboard
inline Square bitboardToSquare(const Bitboard& bitboard) {
	if (!bitboard) {
		return static_cast<Square>(NONE);
	}
	int square=0;
	if (!bitScanForward(&square, bitboard)) {
		return static_cast<Square>(NONE);
	}
	return static_cast<Square>( square );
}
// extract least significant bit of a bitboard
inline Square extractLSB(Bitboard& bitboard) {
	if (!bitboard) {
		return static_cast<Square>(NONE);
	}
	int square=0;
	if (!bitScanForward(&square, bitboard)) {
		return static_cast<Square>(NONE);
	}
	bitboard &= bitboard - 1;
	return static_cast<Square>(square);
}

inline const PieceTypeByColor makePiece(const PieceColor color, const PieceType type) {
	return pieceTypeByColor[color][type];
}

inline const int upperScore(const int value) {
	return ((static_cast<int16_t>(value >> 15) & 1) + static_cast<int16_t>(value >> 16));
}

inline const int lowerScore(const int value) {
	return  static_cast<int16_t>(value & 0xFFFF);
}

inline const int makeScore(const int upperValue, const int lowerValue) {
	return MS(upperValue,lowerValue);
}

inline const int64_t getTickCount() {
	struct timeval t;
	gettimeofday(&t, NULL);
	const int64_t r = static_cast<uint64_t>(t.tv_sec*1000 + t.tv_usec/1000);
	return r;
}
#endif /* BITBOARD_H_ */
