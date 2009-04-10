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

namespace BoardTypes{

//Bitboard type - unsigned long int (8 bytes)
typedef uint64_t Bitboard;

#define USE_INTRINSIC_BITSCAN																	    // will use hardware's bitscan
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
#define Sq2RA(X)				(fileBB[squareFile[X]]|rankBB[squareRank[X]])^squareToBitboard[X]	// Encode Square to Rook Attack
#define Sq2BA(X)				(diagonalA1H8BB[SquareToDiagonalA1H8[X]]| \
								 diagonalH1A8BB[SquareToDiagonalH1A8[X]])^squareToBitboard[X]		// Encode Square to Bishop Attack

#define FULL_BB						 0xFFFFFFFFFFFFFFFFULL
#define INITIAL_WHITE_BITBOARD  	 0xFFFFULL
#define INITIAL_BLACK_BITBOARD  	 0xFFFF000000000000ULL
#define INITIAL_WHITE_PAWN_BITBOARD  0xFF00ULL
#define INITIAL_BLACK_PAWN_BITBOARD  0xFF000000000000ULL

#define bitsBetween(BB,S1,S2)		(squareToBitboard[S2]|(squareToBitboard[S2]-squareToBitboard[S1])) & BB

static const uint64_t debruijn64 = 0x07EDD5E59A4E28C2ULL;

static const uint32_t index64[64] = {
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

// represents square location within the bitboard - it's simply a power of 2 to distinguish the squares
static const Bitboard squareToBitboard[ALL_SQUARE]={SqBB(A1), SqBB(B1), SqBB(C1), SqBB(D1), SqBB(E1), SqBB(F1), SqBB(G1), SqBB(H1),
													SqBB(A2), SqBB(B2), SqBB(C2), SqBB(D2), SqBB(E2), SqBB(F2), SqBB(G2), SqBB(H2),
													SqBB(A3), SqBB(B3), SqBB(C3), SqBB(D3), SqBB(E3), SqBB(F3), SqBB(G3), SqBB(H3),
													SqBB(A4), SqBB(B4), SqBB(C4), SqBB(D4), SqBB(E4), SqBB(F4), SqBB(G4), SqBB(H4),
													SqBB(A5), SqBB(B5), SqBB(C5), SqBB(D5), SqBB(E5), SqBB(F5), SqBB(G5), SqBB(H5),
													SqBB(A6), SqBB(B6), SqBB(C6), SqBB(D6), SqBB(E6), SqBB(F6), SqBB(G6), SqBB(H6),
													SqBB(A7), SqBB(B7), SqBB(C7), SqBB(D7), SqBB(E7), SqBB(F7), SqBB(G7), SqBB(H7),
													SqBB(A8), SqBB(B8), SqBB(C8), SqBB(D8), SqBB(E8), SqBB(F8), SqBB(G8), SqBB(H8) };

//colors
enum PieceColor {
	WHITE, BLACK, COLOR_NONE
};

//piece types
enum PieceType {
	PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

// piece type by color
enum PieceTypeByColor {
	WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING, BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING, EMPTY
};

// castle types
enum CastleRight {
	NO_CASTLE, KING_SIDE_CASTLE, QUEEN_SIDE_CASTLE, BOTH_SIDE_CASTLE
};

//color of a given piece
static const PieceColor pieceColor[ALL_PIECE_TYPE_BY_COLOR] = {WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,COLOR_NONE};

//ranks - row
enum Rank { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };

//files - column
enum File { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };

// diagonals A1..H8
enum DiagonalA1H8 { A8_A8, B8_A7, C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3, G1_H2, H1_H1 };

// diagonals A1..H8
enum DiagonalH1A8 { A1_A1, B1_A2, C1_A3,D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6, G8_H7, H8_H8 };

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


// bitboard for all ranks
static const Bitboard rankBB[ALL_RANK]={0x00000000000000FFULL,0x000000000000FF00ULL,0x0000000000FF0000ULL,0x00000000FF000000ULL,
							  		    0x000000FF00000000ULL,0x0000FF0000000000ULL,0x00FF000000000000ULL,0xFF00000000000000ULL};

// bitboard for all files
static const Bitboard fileBB[ALL_FILE]={0x0101010101010101ULL,0x0202020202020202ULL,0x0404040404040404ULL,0x0808080808080808ULL,
										0x1010101010101010ULL,0x2020202020202020ULL,0x4040404040404040ULL,0x8080808080808080ULL};

// bitboard for all diagonal A1..H8
static const Bitboard diagonalA1H8BB[ALL_DIAGONAL]={Sq2Bb(A8),
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
static const DiagonalA1H8 SquareToDiagonalA1H8[ALL_SQUARE]={H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3, G1_H2, H1_H1,
															G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3, G1_H2,
															F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4, F1_H3,
															E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5, E1_H4,
															D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6, D1_H5,
															C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7, C1_H6,
															B8_A7, C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1, B1_H7,
															A8_A8, B8_A7, C8_A6, D8_A5, E8_A4, F8_A3, G8_A2, H8_A1};

// bitboard for all diagonal H1..A8
static const Bitboard diagonalH1A8BB[ALL_DIAGONAL]={Sq2Bb(A1),
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
static const DiagonalH1A8 SquareToDiagonalH1A8[ALL_SQUARE]={A1_A1, B1_A2, C1_A3, D1_A4, E1_A5, F1_A6, G1_A7, H1_A8,
															B1_A2, C1_A3, D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2,
															C1_A3, D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3,
															D1_A4, E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4,
															E1_A5, F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5,
															F1_A6, G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6,
															G1_A7, H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6, G8_H7,
															H1_A8, B8_H2, C8_H3, D8_H4, E8_H5, F8_H6, G8_H7, H8_H8};

// bitboard for all rook attacks
static const Bitboard rookAttacks[ALL_SQUARE]={	Sq2RA(A1), Sq2RA(B1), Sq2RA(C1), Sq2RA(D1), Sq2RA(E1), Sq2RA(F1), Sq2RA(G1), Sq2RA(H1),
												Sq2RA(A2), Sq2RA(B2), Sq2RA(C2), Sq2RA(D2), Sq2RA(E2), Sq2RA(F2), Sq2RA(G2), Sq2RA(H2),
												Sq2RA(A3), Sq2RA(B3), Sq2RA(C3), Sq2RA(D3), Sq2RA(E3), Sq2RA(F3), Sq2RA(G3), Sq2RA(H3),
												Sq2RA(A4), Sq2RA(B4), Sq2RA(C4), Sq2RA(D4), Sq2RA(E4), Sq2RA(F4), Sq2RA(G4), Sq2RA(H4),
												Sq2RA(A5), Sq2RA(B5), Sq2RA(C5), Sq2RA(D5), Sq2RA(E5), Sq2RA(F5), Sq2RA(G5), Sq2RA(H5),
												Sq2RA(A6), Sq2RA(B6), Sq2RA(C6), Sq2RA(D6), Sq2RA(E6), Sq2RA(F6), Sq2RA(G6), Sq2RA(H6),
												Sq2RA(A7), Sq2RA(B7), Sq2RA(C7), Sq2RA(D7), Sq2RA(E7), Sq2RA(F7), Sq2RA(G7), Sq2RA(H7),
												Sq2RA(A8), Sq2RA(B8), Sq2RA(C8), Sq2RA(D8), Sq2RA(E8), Sq2RA(F8), Sq2RA(G8), Sq2RA(H8) };

// bitboard for all bishop attacks
static const Bitboard bishopAttacks[ALL_SQUARE]={Sq2BA(A1), Sq2BA(B1), Sq2BA(C1), Sq2BA(D1), Sq2BA(E1), Sq2BA(F1), Sq2BA(G1), Sq2BA(H1),
												 Sq2BA(A2), Sq2BA(B2), Sq2BA(C2), Sq2BA(D2), Sq2BA(E2), Sq2BA(F2), Sq2BA(G2), Sq2BA(H2),
												 Sq2BA(A3), Sq2BA(B3), Sq2BA(C3), Sq2BA(D3), Sq2BA(E3), Sq2BA(F3), Sq2BA(G3), Sq2BA(H3),
												 Sq2BA(A4), Sq2BA(B4), Sq2BA(C4), Sq2BA(D4), Sq2BA(E4), Sq2BA(F4), Sq2BA(G4), Sq2BA(H4),
												 Sq2BA(A5), Sq2BA(B5), Sq2BA(C5), Sq2BA(D5), Sq2BA(E5), Sq2BA(F5), Sq2BA(G5), Sq2BA(H5),
												 Sq2BA(A6), Sq2BA(B6), Sq2BA(C6), Sq2BA(D6), Sq2BA(E6), Sq2BA(F6), Sq2BA(G6), Sq2BA(H6),
												 Sq2BA(A7), Sq2BA(B7), Sq2BA(C7), Sq2BA(D7), Sq2BA(E7), Sq2BA(F7), Sq2BA(G7), Sq2BA(H7),
												 Sq2BA(A8), Sq2BA(B8), Sq2BA(C8), Sq2BA(D8), Sq2BA(E8), Sq2BA(F8), Sq2BA(G8), Sq2BA(H8) };

// Move representation
struct Move {
	Move()
	{}
	Move(Square fromSquare, Square toSquare, PieceTypeByColor piece) : from(fromSquare), to(toSquare), promotionPiece(piece)
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

static const char pieceChar[ALL_PIECE_TYPE_BY_COLOR+1] = "pnbrqkPNBRQK ";

}

using namespace BoardTypes;

class Board
{

public:

	Board();
	Board(const Board& board);
	virtual ~Board();

	const void printBoard() const;
	void doMove(const Move move, MoveBackup& backup);
	void undoMove(MoveBackup& backup);
	void setInitialPosition();
	void loadFromString(const std::string startPosMoves);
	const CastleRight getCastleRights(PieceColor color) const;
	const PieceColor getSideToMove() const;
	const Square getEnPassant() const;

private:

	const Node& get() const;
	bool putPiece(const PieceTypeByColor piece, const Square square);
	bool removePiece(const PieceTypeByColor piece, const Square square);
	void removeCastleRights(const PieceColor color, const CastleRight castle);
	void setEnPassant(const Square square);
	const Square bitboardToSquare(Bitboard bitboard) const;

	void setOccupiedNeighbor(const Bitboard mask, const Square start, Square& minor, Square& major);

	const Bitboard getRookAttacks(const Square square);
	const Bitboard getRookAttacks(const Square square, const Bitboard occupied);

	const Bitboard getBishopAttacks(const Square square);
	const Bitboard getBishopAttacks(const Square square, const Bitboard occupied);

	const Bitboard getQueenAttacks(const Square square);
	const Bitboard getQueenAttacks(const Square square, const Bitboard occupied);

	Node& currentBoard;
};

#endif /* BOARD_H_ */
