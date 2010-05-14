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
 * Board.h
 *
 *  Created on: Feb 21, 2009
 *      Author: bhlangonijr
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <iostream>
#include <vector>
#include <inttypes.h>
#include <time.h>
#include <assert.h>

#include "stringutil.h"
#include "mersenne.h"
#include "bitboard.h"
#include "moveiterator.h"
#include "magicmoves.h"

//#include "Evaluator.h"

namespace BoardTypes{


//used for hashing
typedef uint64_t Key;

#define USE_MAGIC_MOVES true			  								// set to true to use magic moves

#define ALL_CASTLE_RIGHT				 4								// all castle rights
#define MAX_GAME_LENGTH					 1024							// Max game lenght
#define St2Sq(F,R)						 (((int)F-96)+((int)R-49)*8)-1	// encode String to Square enum

#define MAX(x,y)						 (x>y?x:y)
#define MIN(x,y)						 (x<y?x:y)

// castle types
enum CastleRight {
	NO_CASTLE, KING_SIDE_CASTLE, QUEEN_SIDE_CASTLE, BOTH_SIDE_CASTLE
};

// castle squares
const Bitboard castleSquare[ALL_PIECE_COLOR][ALL_CASTLE_RIGHT]={
		/*WHITE*/{EMPTY_BB, Sq2Bb(F1)|Sq2Bb(G1), Sq2Bb(D1)|Sq2Bb(C1)|Sq2Bb(B1), Sq2Bb(F1)|Sq2Bb(G1)|Sq2Bb(D1)|Sq2Bb(C1)||Sq2Bb(B1)},
		/*BLACK*/{EMPTY_BB, Sq2Bb(F8)|Sq2Bb(G8), Sq2Bb(D8)|Sq2Bb(C8)|Sq2Bb(B8), Sq2Bb(F8)|Sq2Bb(G8)|Sq2Bb(D8)|Sq2Bb(C8)|Sq2Bb(B8)},
		/*NONE */{EMPTY_BB, EMPTY_BB, EMPTY_BB, EMPTY_BB }
};

// castle zobrist keys index table
const int zobristCastleIndex[ALL_CASTLE_RIGHT][ALL_CASTLE_RIGHT]={
		{0,1,2,3},
		{4,5,6,7},
		{8,9,10,11},
		{12,13,14,15}
};

//start FEN position
const std::string startFENPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

//material values for move classification
const int pieceMaterialValues[ALL_PIECE_TYPE_BY_COLOR] = {100, 318, 325, 520, 975, 10000, 100, 318, 325, 520, 975, 10000, 0};

// Backup move
struct MoveBackup {
	MoveBackup()
	{}

	bool hasWhiteKingCastle;
	bool hasWhiteQueenCastle;
	bool hasBlackKingCastle;
	bool hasBlackQueenCastle;

	bool hasCapture;
	bool hasPromotion;

	Key key;
	PieceTypeByColor capturedPiece;
	Square capturedSquare;

	CastleRight whiteCastleRight;
	CastleRight blackCastleRight;
	Square enPassant;
	PieceTypeByColor movingPiece;
	Square from;
	Square to;
	int halfMoveCounter;

};

// the board node representation
struct Node {

	Node () : key(0ULL), piece(), moveCounter(0), halfMoveCounter(0)
	{}

	Node (const Node& node) : key(node.key), piece( node.piece ), enPassant( node.enPassant ),
			sideToMove( node.sideToMove ), moveCounter(node.moveCounter),
			halfMoveCounter(node.halfMoveCounter)
	{
		for(register int x=0;x<ALL_SQUARE;x++){
			square[x]=node.square[x];
		}
		for(register int x=0;x<MAX_GAME_LENGTH;x++){
			keyHistory[x]=node.keyHistory[x];
		}
		pieceColor[WHITE]=node.pieceColor[WHITE];
		pieceColor[BLACK]=node.pieceColor[BLACK];
		castleRight[WHITE]=node.castleRight[WHITE];
		castleRight[BLACK]=node.castleRight[BLACK];
		castleDone[WHITE]=node.castleDone[WHITE];
		castleDone[BLACK]=node.castleDone[BLACK];

	}

	Key key;

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

	CastleRight castleRight[ALL_PIECE_COLOR];
	bool castleDone[ALL_PIECE_COLOR];
	Square enPassant;
	PieceColor sideToMove;
	PieceTypeByColor square[ALL_SQUARE];
	Bitboard pieceColor[ALL_PIECE_COLOR];

	Key keyHistory[MAX_GAME_LENGTH];
	int moveCounter;
	int halfMoveCounter;

	// clear structure node
	void clear()
	{
		key=0ULL;
		moveCounter=0;
		halfMoveCounter=0;
		for(register int x=0;x<ALL_PIECE_TYPE_BY_COLOR;x++){
			piece.array[x]=0ULL;
		}
		for(register int x=0;x<ALL_SQUARE;x++){
			square[x]=EMPTY;
		}
		for(register int x=0;x<MAX_GAME_LENGTH;x++){
			keyHistory[x]=0x0ULL;
		}

		pieceColor[WHITE]=0ULL;
		pieceColor[BLACK]=0ULL;
		pieceColor[COLOR_NONE]=FULL_BB;
		castleRight[WHITE]=NO_CASTLE;
		castleRight[BLACK]=NO_CASTLE;
		castleDone[WHITE]=false;
		castleDone[BLACK]=false;
		enPassant=NONE;
		sideToMove=COLOR_NONE;

	}

};
// Zobrist keys for hashing
struct NodeZobrist {

	Key pieceSquare[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE];
	Key castleRight[ALL_CASTLE_RIGHT*ALL_CASTLE_RIGHT];
	Key enPassant[ALL_FILE];
	Key sideToMove;

};

}

using namespace BoardTypes;

class Board
{

public:

	Board();
	Board(const Board& board);
	virtual ~Board();

	const void printBoard();
	const void printBoard(const std::string pad);
	void genericTest();
	void doMove(const MoveIterator::Move& move, MoveBackup& backup);
	void doNullMove(MoveBackup& backup);
	void undoMove(MoveBackup& backup);
	void undoNullMove(MoveBackup& backup);
	void setInitialPosition();
	void loadFromString(const std::string startPosMoves);
	void loadFromFEN(const std::string startFENMoves);
	const std::string getFEN();
	const PieceTypeByColor encodePieceChar(char piece);
	const CastleRight getCastleRights(PieceColor color) const;
	void removeCastleRights(const PieceColor color, const CastleRight castle);
	void setCastleRights(const PieceColor color, const CastleRight castle);
	bool canCastle(const PieceColor color);
	bool canCastle(const PieceColor color, const CastleRight castleRight);
	const PieceColor getSideToMove() const;
	void setSideToMove(const PieceColor color);
	const Square getEnPassant() const;
	void setEnPassant(const Square square);
	const PieceColor flipSide(const PieceColor color);
	const PieceColor getPieceColor(const PieceTypeByColor piece) const;
	const PieceColor getPieceColorBySquare(const Square square) const;
	const PieceType getPieceType(const PieceTypeByColor piece) const;
	const PieceType getPieceTypeBySquare(const Square square) const;
	const PieceTypeByColor makePiece(const PieceColor color, const PieceType type) const;
	const Square makeSquare(const Rank rank, const File file) const;
	const Rank getSquareRank(const Square square) const;
	const File getSquareFile(const Square square) const;
	const bool isAttacked(const PieceColor color, const PieceType type);
	const bool isAttacked(const Bitboard occupation, const PieceColor attackingSide);
	const bool isNotLegal();
	const bool isMoveLegal(MoveIterator::Move& move);
	const Bitboard getAttacksFrom(MoveIterator::Move& move);
	const bool isDraw();
	const bool isCastleDone(const PieceColor color);

	const Bitboard getPiecesByColor(const PieceColor color) const;
	const Bitboard getAllPieces() const;
	const Bitboard getEmptySquares() const;
	const Bitboard getPiecesByType(const PieceTypeByColor piece) const;
	const PieceTypeByColor getPieceBySquare(const Square square) const;
	int const getPieceCountByType(const PieceTypeByColor piece) const;

	void generateCaptures(MoveIterator& moves, const PieceColor side);
	void generateNonCaptures(MoveIterator& moves, const PieceColor side);
	void generateAllMoves(MoveIterator& moves, const PieceColor side);

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
	const Bitboard getPawnMoves(const Square square);
	const Bitboard getPawnMoves(const Square square, const Bitboard occupied);
	const Bitboard getPawnCaptures(const Square square);
	const Bitboard getPawnCaptures(const Square square, const Bitboard occupied);
	const Bitboard getKingAttacks(const Square square);
	const Bitboard getKingAttacks(const Square square, const Bitboard occupied);

	static void initializeZobrist();
	const int getZobristCastleIndex();
	const Key getKey() const;
	void setKey(Key key);
	const Key generateKey();

	void increaseMoveCounter();
	void decreaseMoveCounter();
	const int getMoveCounter() const;

	void increaseHalfMoveCounter();
	void resetHalfMoveCounter();
	const int getHalfMoveCounter() const;

	void updateKeyHistory();

	const long getTickCount() {
		return ((clock() * 1000) / CLOCKS_PER_SEC);
	}


private:

	Node& getBoard();
	void clearBoard();
	bool putPiece(const PieceTypeByColor piece, const Square square);
	bool removePiece(const PieceTypeByColor piece, const Square square);

	Node currentBoard;
	static NodeZobrist nodeZobrist;
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

	return true;
}
// remove a piece from the board and erase piece info
inline bool Board::removePiece(const PieceTypeByColor piece, const Square square)
{
	currentBoard.piece.array[piece] ^= squareToBitboard[square];
	currentBoard.pieceColor[pieceColor[piece]] ^= squareToBitboard[square];
	currentBoard.square[square] = EMPTY;

	return true;
}

inline const PieceTypeByColor Board::encodePieceChar(char piece) {

	for(size_t n=WHITE_PAWN;n<=EMPTY;n++) {
		if (pieceChar[n]==piece) {
			return PieceTypeByColor(n);
		}
	}
	return EMPTY;
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

// can castle?
inline bool Board::canCastle(const PieceColor color) {

	return canCastle(color, BOTH_SIDE_CASTLE);
}

// can castle specific?
inline bool Board::canCastle(const PieceColor color, const CastleRight castleRight) {

	if (currentBoard.castleRight[color] == NO_CASTLE) { // lost the right to castle?
		return false;
	} else if (currentBoard.castleRight[color]!=BOTH_SIDE_CASTLE &&
			currentBoard.castleRight[color]!=castleRight) {
		return false;
	}

	if (castleSquare[color][castleRight]&getAllPieces()) { // pieces interposing king & rooks?
		return false;
	}

	Bitboard castle=getPiecesByType(makePiece(color,KING))|(castleSquare[color][castleRight]);

	if (isAttacked(castle,flipSide(color))) { // squares through castle & king destination attacked?
		return false;
	}
	return true;
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

// flip side
inline const PieceColor Board::flipSide(const PieceColor color) {
	return PieceColor((int)color ^ 1);
}

// get piece color
inline const PieceColor Board::getPieceColor(const PieceTypeByColor piece) const {
	return pieceColor[piece];
}

// get piece color by square
inline const PieceColor Board::getPieceColorBySquare(const Square square) const {
	return pieceColor[currentBoard.square[square]];
}

// get piece type
inline const PieceType Board::getPieceType(const PieceTypeByColor piece) const {
	return pieceType[piece];
}

// get piece type by square
inline const PieceType Board::getPieceTypeBySquare(const Square square) const {
	return pieceType[currentBoard.square[square]];
}

// make piece by color and type
inline const PieceTypeByColor Board::makePiece(const PieceColor color, const PieceType type) const {
	return pieceTypeByColor[color][type];
}

// make square by rank & file
inline const Square Board::makeSquare(const Rank rank, const File file) const {
	return encodeSquare[rank][file];
}

// get rank from square
inline const Rank Board::getSquareRank(const Square square) const {
	return squareRank[square];
}

// get file from square
inline const File Board::getSquareFile(const Square square) const {
	return squareFile[square];
}

inline const bool Board::isAttacked(const PieceColor color, const PieceType type) {


	Bitboard piece = getPiecesByType(makePiece(color,type));
	PieceColor other = flipSide(color);

	Square from = extractLSB(piece);

	if ( from!=NONE ) {
		return 	(getBishopAttacks(from) & (getPiecesByType(makePiece(other,BISHOP)) |
				getPiecesByType(makePiece(other,QUEEN)))) ||
				(getRookAttacks(from) & (getPiecesByType(makePiece(other,ROOK)) |
						getPiecesByType(makePiece(other,QUEEN)))) ||
						(getKnightAttacks(from) & getPiecesByType(makePiece(other,KNIGHT))) ||
						(getPawnAttacks(from) & getPiecesByType(makePiece(other,PAWN))) ||
						(getKingAttacks(from) & getPiecesByType(makePiece(other,KING)));
	}
	return false;


}

// verify if the given bitboard occupation is attacked
inline const bool Board::isAttacked(const Bitboard occupation, const PieceColor attackingSide) {

	Bitboard pieces = occupation;
	Square from = extractLSB(pieces);

	while ( from!=NONE ) {

		bool result= (getBishopAttacks(from) & (getPiecesByType(makePiece(attackingSide,BISHOP)) |
				getPiecesByType(makePiece(attackingSide,QUEEN)))) ||
				(getRookAttacks(from) & (getPiecesByType(makePiece(attackingSide,ROOK)) |
						getPiecesByType(makePiece(attackingSide,QUEEN)))) ||
						(getKnightAttacks(from) & getPiecesByType(makePiece(attackingSide,KNIGHT))) ||
						(getPawnAttacks(from) & getPiecesByType(makePiece(attackingSide,PAWN))) ||
						(getKingAttacks(from) & getPiecesByType(makePiece(attackingSide,KING)));

		if (result) {
			return true;
		}
		from = extractLSB(pieces);
	}

	return false;
}

// verify board legality
inline const bool Board::isNotLegal() {

	return isAttacked(flipSide(getSideToMove()),KING);

}

// verify board legality
inline const bool Board::isMoveLegal(MoveIterator::Move& move) {

	PieceTypeByColor fromPiece=this->getPieceBySquare(move.from);
	PieceTypeByColor toPiece=this->getPieceBySquare(move.to);

	if (move.from==NONE || fromPiece==EMPTY) {
		return false;
	}

	if (toPiece != EMPTY &&
			this->getPieceColorBySquare(move.from) == this->getPieceColorBySquare(move.to)) {
		return false;
	}

	if (!(getAttacksFrom(move) & squareToBitboard[move.to])) {
		return false;
	}

	if (getPieceType(fromPiece) == PAWN &&
		getSquareFile(move.from)!=	getSquareFile(move.to) &&
		toPiece == EMPTY &&
		getSquareFile(getEnPassant()) != getSquareFile(move.to)
		) {
		return false;
	}

	if ((getPieceType(fromPiece) == KING) ||
			(move.from==E1 && move.to==G1) ||
			(move.from==E1 && move.to==C1) ||
			(move.from==E8 && move.to==G8) ||
			(move.from==E8 && move.to==C8)) {
		const CastleRight castleRight = (move.to==C1 || move.to==C8) ? QUEEN_SIDE_CASTLE : KING_SIDE_CASTLE;

		if (!canCastle(getSideToMove(), castleRight)) {
			return false;
		}
	}

	return true;

}

// Get attacks from a given square
inline const Bitboard Board::getAttacksFrom(MoveIterator::Move& move) {

	const PieceTypeByColor fromPiece = getPieceBySquare(move.from);
	const PieceType pieceType = getPieceType(fromPiece);

	Bitboard attacks = EMPTY_BB;

	if (pieceType==PAWN) {
		attacks = this->getPawnAttacks(move.from);
	} else if (pieceType==KNIGHT) {
		attacks = this->getKnightAttacks(move.from);
	} else if (pieceType==BISHOP) {
		attacks = this->getBishopAttacks(move.from);
	} else if (pieceType==ROOK) {
		attacks = this->getRookAttacks(move.from);
	} else if (pieceType==QUEEN) {
		attacks = this->getQueenAttacks(move.from);
	} else if (pieceType==KING) {
		attacks = this->getKingAttacks(move.from);
	}

	return attacks;

}



// verify draw by 50th move rule, 3 fold rep and insuficient material
inline const bool Board::isDraw() {

	int repetition = 0;

	for (int x=1;x<getMoveCounter();x++) {
		if (currentBoard.keyHistory[getMoveCounter()]==currentBoard.keyHistory[x]) {
			repetition++;
		}
		if (repetition>=2) {
			return true;
		}
	}

	return getHalfMoveCounter()>=100;

}

// verify if castle has been made
inline const bool Board::isCastleDone(const PieceColor color) {
	return currentBoard.castleDone[color];

}

// get all pieces of a given color
inline const Bitboard Board::getPiecesByColor(const PieceColor color) const {
	return currentBoard.pieceColor[color];
}

// get all pieces
inline const Bitboard Board::getAllPieces() const {
	return currentBoard.pieceColor[WHITE] | currentBoard.pieceColor[BLACK];
}

// get empty squares
inline const Bitboard Board::getEmptySquares() const {
	return ~getAllPieces();
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

	if (piece==EMPTY || !currentBoard.piece.array[piece]) return 0;

	return _BitCount(currentBoard.piece.array[piece]);
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getRookAttacks(const Square square) {
	return getRookAttacks(square, getAllPieces());
}

// return a bitboard with attacked squares by the rook in the given square
inline const Bitboard Board::getRookAttacks(const Square square, const Bitboard occupied) {

#if defined(USE_MAGIC_MOVES)

	return R_MAGIC(square, occupied);

#else
	const Bitboard file = getSliderAttacks(fileAttacks[square], occupied, square);
	const Bitboard rank = getSliderAttacks(rankAttacks[square], occupied, square);

	return file | rank;
#endif

}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getBishopAttacks(const Square square) {
	return getBishopAttacks(square, getAllPieces());
}

// return a bitboard with attacked squares by the bishop in the given square
inline const Bitboard Board::getBishopAttacks(const Square square, const Bitboard occupied) {
#if defined(USE_MAGIC_MOVES)

	return B_MAGIC(square, occupied);

#else

	const Bitboard diagA1H8 = getSliderAttacks(diagA1H8Attacks[square], occupied, square);
	const Bitboard diagH1A8 = getSliderAttacks(diagH1A8Attacks[square], occupied, square);

	return diagA1H8 | diagH1A8;
#endif
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
	if (getPieceBySquare(square)==EMPTY) {
		return EMPTY_BB;
	}
	return knightAttacks[square];
}

// return a bitboard with attacked squares by the pawn in the given square
inline const Bitboard Board::getKnightAttacks(const Square square, const Bitboard occupied) {
	return knightAttacks[square] & occupied;
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getPawnMoves(const Square square) {
	return getPawnMoves(square, getAllPieces());
}
// overload method - gets current occupied squares in the board
inline const Bitboard Board::getPawnAttacks(const Square square) {

	const Bitboard pawnAttacks = getPieceColorBySquare(square)==WHITE?whitePawnAttacks[square]:blackPawnAttacks[square];
	const Bitboard captures = (diagA1H8Attacks[square]|diagH1A8Attacks[square]) & pawnAttacks ;

	return captures;
}

// return a bitboard with move squares by the pawn in the given square
inline const Bitboard Board::getPawnAttacks(const Square square, const Bitboard occupied) {

	const Bitboard pawnAttacks = getPieceColorBySquare(square)==WHITE?whitePawnAttacks[square]:blackPawnAttacks[square];
	const Bitboard captures = (diagA1H8Attacks[square]|diagH1A8Attacks[square]) & pawnAttacks & occupied;

	return captures;
}

// return a bitboard with move squares by the pawn in the given square
inline const Bitboard Board::getPawnMoves(const Square square, const Bitboard occupied) {

	const PieceColor color = getPieceColorBySquare(square);
	const Bitboard pawnAttacks = color==WHITE ? whitePawnAttacks[square] : blackPawnAttacks[square];
	Bitboard occ = occupied;

	if (squareRank[square]==RANK_2 || squareRank[square]==RANK_7) {
		if (squareToBitboard[square+(color==WHITE?8:-8)]&occ) {
			occ |= squareToBitboard[square+(color==WHITE?16:-16)]; // double move
		}
	}

	return (fileAttacks[square] & pawnAttacks) & ~occ ;
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getPawnCaptures(const Square square) {
	return getPawnCaptures(square, getAllPieces());
}

// return a bitboard with captures by the pawn in the given square
inline const Bitboard Board::getPawnCaptures(const Square square, const Bitboard occupied) {

	const PieceColor color = getPieceColorBySquare(square);
	const Bitboard pawnAttacks = color==WHITE ? whitePawnAttacks[square] : blackPawnAttacks[square];
	Bitboard occ = occupied;

	if (getEnPassant()!=NONE) {
		occ |= (squareToBitboard[Square(getEnPassant() + (color==WHITE?8:-8))]); // en passant
	}

	return (diagA1H8Attacks[square]|diagH1A8Attacks[square]) & pawnAttacks & occ ;
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getKingAttacks(const Square square) {
	return adjacentSquares[square];
}

// return a bitboard with attacked squares by the King in the given square
inline const Bitboard Board::getKingAttacks(const Square square, const Bitboard occupied) {
	return adjacentSquares[square] & occupied;
}

// generate only capture moves
inline void Board::generateCaptures(MoveIterator& moves, const PieceColor side) {

	const PieceColor otherSide = flipSide(side);
	const Bitboard otherPieces = getPiecesByColor(otherSide);
	Bitboard pieces = EMPTY_BB;
	Bitboard attacks = EMPTY_BB;
	Square from = NONE;

	pieces = getPiecesByType(makePiece(side,PAWN));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getPawnCaptures(from,otherPieces);
		Square target = extractLSB(attacks);
		bool promotion=((getSquareRank(from)==RANK_7&&side==WHITE) ||
				(getSquareRank(from)==RANK_2&&side==BLACK));

		MoveIterator::MoveType type = promotion ? MoveIterator::PROMO_CAPTURE : MoveIterator::EQUAL_CAPTURE;

		while ( target!=NONE ) {
			if (promotion) {
				moves.add(from,target,makePiece(side,QUEEN), type);
				moves.add(from,target,makePiece(side,ROOK), type);
				moves.add(from,target,makePiece(side,BISHOP), type);
				moves.add(from,target,makePiece(side,KNIGHT), type);
			} else {
				if (pieceMaterialValues[getPieceBySquare(target)]>pieceMaterialValues[getPieceBySquare(from)]) {
					type=MoveIterator::GOOD_CAPTURE;
				} else if (pieceMaterialValues[getPieceBySquare(target)]<pieceMaterialValues[getPieceBySquare(from)]) {
					type=MoveIterator::BAD_CAPTURE;
				}
				moves.add(from,target,EMPTY,type);
			}
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

	pieces = getPiecesByType(makePiece(side,KNIGHT));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getKnightAttacks(from) & otherPieces;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			MoveIterator::MoveType type = MoveIterator::EQUAL_CAPTURE;
			if (pieceMaterialValues[getPieceBySquare(target)]>pieceMaterialValues[getPieceBySquare(from)]) {
				type=MoveIterator::GOOD_CAPTURE;
			} else if (pieceMaterialValues[getPieceBySquare(target)]<pieceMaterialValues[getPieceBySquare(from)]) {
				type=MoveIterator::BAD_CAPTURE;
			}
			moves.add(from,target,EMPTY,type);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

	pieces = getPiecesByType(makePiece(side,BISHOP));
	from = extractLSB(pieces);

	while ( from!=NONE ) {

		attacks = getBishopAttacks(from) & otherPieces;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			MoveIterator::MoveType type = MoveIterator::EQUAL_CAPTURE;
			if (pieceMaterialValues[getPieceBySquare(target)]>pieceMaterialValues[getPieceBySquare(from)]) {
				type=MoveIterator::GOOD_CAPTURE;
			} else if (pieceMaterialValues[getPieceBySquare(target)]<pieceMaterialValues[getPieceBySquare(from)]) {
				type=MoveIterator::BAD_CAPTURE;
			}
			moves.add(from,target,EMPTY,type);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

	pieces = getPiecesByType(makePiece(side,ROOK));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getRookAttacks(from) & otherPieces;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			MoveIterator::MoveType type = MoveIterator::EQUAL_CAPTURE;
			if (pieceMaterialValues[getPieceBySquare(target)]>pieceMaterialValues[getPieceBySquare(from)]) {
				type=MoveIterator::GOOD_CAPTURE;
			} else if (pieceMaterialValues[getPieceBySquare(target)]<pieceMaterialValues[getPieceBySquare(from)]) {
				type=MoveIterator::BAD_CAPTURE;
			}
			moves.add(from,target,EMPTY,type);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

	pieces = getPiecesByType(makePiece(side,QUEEN));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getQueenAttacks(from) & otherPieces;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			MoveIterator::MoveType type = MoveIterator::EQUAL_CAPTURE;
			if (pieceMaterialValues[getPieceBySquare(target)]>pieceMaterialValues[getPieceBySquare(from)]) {
				type=MoveIterator::GOOD_CAPTURE;
			} else if (pieceMaterialValues[getPieceBySquare(target)]<pieceMaterialValues[getPieceBySquare(from)]) {
				type=MoveIterator::BAD_CAPTURE;
			}
			moves.add(from,target,EMPTY,type);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

	pieces = getPiecesByType(makePiece(side,KING));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getKingAttacks(from) & otherPieces;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			MoveIterator::MoveType type = MoveIterator::EQUAL_CAPTURE;
			if (pieceMaterialValues[getPieceBySquare(target)]>pieceMaterialValues[getPieceBySquare(from)]) {
				type=MoveIterator::GOOD_CAPTURE;
			} else if (pieceMaterialValues[getPieceBySquare(target)]<pieceMaterialValues[getPieceBySquare(from)]) {
				type=MoveIterator::BAD_CAPTURE;
			}
			moves.add(from,target,EMPTY,type);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

}

//generate only non capture moves
inline void Board::generateNonCaptures(MoveIterator& moves, const PieceColor side){

	Bitboard pieces = EMPTY_BB;
	Bitboard empty = getEmptySquares();
	Bitboard attacks = EMPTY_BB;
	Square from = NONE;

	pieces = getPiecesByType(makePiece(side,PAWN));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getPawnMoves(from) & empty;
		Square target = extractLSB(attacks);
		bool promotion=((getSquareRank(from)==RANK_7&&side==WHITE) ||
				(getSquareRank(from)==RANK_2&&side==BLACK));
		MoveIterator::MoveType type = promotion ? MoveIterator::PROMO_NONCAPTURE : MoveIterator::NON_CAPTURE;
		while ( target!=NONE ) {
			if (promotion) {
				moves.add(from,target,makePiece(side,QUEEN),type);
				moves.add(from,target,makePiece(side,ROOK),type);
				moves.add(from,target,makePiece(side,BISHOP),type);
				moves.add(from,target,makePiece(side,KNIGHT),type);
			} else {
				moves.add(from,target,EMPTY,type);
			}
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

	pieces = getPiecesByType(makePiece(side,KNIGHT));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getKnightAttacks(from) & empty;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			moves.add(from,target,EMPTY,MoveIterator::NON_CAPTURE);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

	pieces = getPiecesByType(makePiece(side,BISHOP));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getBishopAttacks(from) & empty;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			moves.add(from,target,EMPTY,MoveIterator::NON_CAPTURE);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

	pieces = getPiecesByType(makePiece(side,ROOK));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getRookAttacks(from) & empty;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			moves.add(from,target,EMPTY,MoveIterator::NON_CAPTURE);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

	pieces = getPiecesByType(makePiece(side,QUEEN));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getQueenAttacks(from) & empty;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			moves.add(from,target,EMPTY,MoveIterator::NON_CAPTURE);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

	pieces = getPiecesByType(makePiece(side,KING));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getKingAttacks(from) & empty;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			moves.add(from,target,EMPTY,MoveIterator::NON_CAPTURE);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

	if (canCastle(side, KING_SIDE_CASTLE)) {
		if (side==WHITE) {
			moves.add(E1,G1,EMPTY,MoveIterator::NON_CAPTURE);
		} else {
			moves.add(E8,G8,EMPTY,MoveIterator::NON_CAPTURE);
		}
	}

	if (canCastle(side, QUEEN_SIDE_CASTLE)) {
		if (side==WHITE) {
			moves.add(E1,C1,EMPTY,MoveIterator::NON_CAPTURE);
		} else {
			moves.add(E8,C8,EMPTY,MoveIterator::NON_CAPTURE);
		}
	}

}

//generate all moves - captures + noncaptures
inline void Board::generateAllMoves(MoveIterator& moves, const PieceColor side) {

	generateCaptures(moves, side);
	generateNonCaptures(moves, side);

}

// increase the game move counter
inline void Board::increaseMoveCounter() {
	currentBoard.moveCounter++;
}

// decrease the game move counter
inline void Board::decreaseMoveCounter() {
	currentBoard.moveCounter--;
}

// get
inline const int Board::getMoveCounter() const {
	return currentBoard.moveCounter;
}

// increase the game move counter
inline void Board::increaseHalfMoveCounter() {
	currentBoard.halfMoveCounter++;
}

// decrease the game move counter
inline void Board::resetHalfMoveCounter() {
	currentBoard.halfMoveCounter=0;
}

// get
inline const int Board::getHalfMoveCounter() const {
	return currentBoard.halfMoveCounter;
}

// update key history
inline void Board::updateKeyHistory() {
	currentBoard.keyHistory[getMoveCounter()]=getKey();
}

#endif /* BOARD_H_ */