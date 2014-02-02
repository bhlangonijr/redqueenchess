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
#include <string.h>
#include "stringutil.h"
#include "mersenne.h"
#include "bitboard.h"
#include "moveiterator.h"
#include "magicmoves.h"
#include "parameter.h"

class Parameter;

namespace BoardTypes{

//used for hashing
typedef uint64_t Key;
const int ALL_CASTLE_RIGHT = 4;		// all castle rights
const int MAX_GAME_LENGTH =	1024;	// Max game lenght
// castle types
enum CastleRight {
	NO_CASTLE, KING_SIDE_CASTLE, QUEEN_SIDE_CASTLE, BOTH_SIDE_CASTLE
};
// castle squares
const Bitboard castleSquare[ALL_PIECE_COLOR][ALL_CASTLE_RIGHT]={
		/*WHITE*/{EMPTY_BB, Sq2Bb(F1)|Sq2Bb(G1), Sq2Bb(D1)|Sq2Bb(C1)|Sq2Bb(B1), Sq2Bb(F1)|Sq2Bb(G1)|Sq2Bb(D1)|Sq2Bb(C1)|Sq2Bb(B1)},
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
const MoveIterator::Move whiteoo = MoveIterator::Move(E1,G1,EMPTY);
const MoveIterator::Move whiteooo = MoveIterator::Move(E1,C1,EMPTY);
const MoveIterator::Move blackoo = MoveIterator::Move(E8,G8,EMPTY);
const MoveIterator::Move blackooo = MoveIterator::Move(E8,C8,EMPTY);
// Backup move
struct MoveBackup {
	MoveBackup() {}

	bool hasWhiteKingCastle;
	bool hasWhiteQueenCastle;
	bool hasBlackKingCastle;
	bool hasBlackQueenCastle;
	bool hasCapture;
	bool hasPromotion;
	Key key;
	Key pawnKey;
	PieceTypeByColor capturedPiece;
	Square capturedSquare;
	CastleRight whiteCastleRight;
	CastleRight blackCastleRight;
	Square enPassant;
	PieceTypeByColor movingPiece;
	Square from;
	Square to;
	int halfMoveCounter;
	int halfNullMoveCounter;
	GamePhase phase;
	bool inCheck;
};

// the board node representation
struct Node {

	Node () :key(0ULL), pawnKey(0ULL), piece(),  enPassant(NONE), sideToMove(WHITE), moveCounter(0),
			halfMoveCounter(0), halfNullMoveCounter(0), gamePhase(OPENING), inCheck(false) {}

	Node (const Node& node) : key(node.key), pawnKey(node.pawnKey), piece( node.piece ),
			enPassant( node.enPassant ), sideToMove(node.sideToMove), moveCounter(node.moveCounter),
			halfMoveCounter(node.halfMoveCounter), halfNullMoveCounter(node.halfNullMoveCounter),
			gamePhase(node.gamePhase), inCheck(node.inCheck) {

		memcpy(square, node.square, ALL_SQUARE * sizeof(PieceTypeByColor));
		memcpy(keyHistory, node.keyHistory, MAX_GAME_LENGTH * sizeof(Key));
		memcpy(pieceCount, node.pieceCount, ALL_PIECE_TYPE_BY_COLOR * sizeof(int));

		psq[WHITE]=node.psq[WHITE];
		psq[BLACK]=node.psq[BLACK];
		fullMaterial[WHITE]=node.fullMaterial[WHITE];
		fullMaterial[BLACK]=node.fullMaterial[BLACK];
		pieceColorCount[WHITE]=node.pieceColorCount[WHITE];
		pieceColorCount[BLACK]=node.pieceColorCount[BLACK];
		pieceColor[WHITE]=node.pieceColor[WHITE];
		pieceColor[BLACK]=node.pieceColor[BLACK];
		castleRight[WHITE]=node.castleRight[WHITE];
		castleRight[BLACK]=node.castleRight[BLACK];
		castleDone[WHITE]=node.castleDone[WHITE];
		castleDone[BLACK]=node.castleDone[BLACK];
		kingSquare[WHITE]=node.kingSquare[WHITE];
		kingSquare[BLACK]=node.kingSquare[BLACK];
		kingSquare[COLOR_NONE]=node.kingSquare[COLOR_NONE];
	}

	Key key;
	Key pawnKey;

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

	} piece;

	CastleRight castleRight[ALL_PIECE_COLOR];
	bool castleDone[ALL_PIECE_COLOR];
	Square enPassant;
	PieceColor sideToMove;
	PieceTypeByColor square[ALL_SQUARE];
	Bitboard pieceColor[ALL_PIECE_COLOR];
	int pieceCount[ALL_PIECE_TYPE_BY_COLOR];
	int pieceColorCount[ALL_PIECE_COLOR];
	int fullMaterial[ALL_PIECE_COLOR];
	int psq[ALL_PIECE_COLOR];
	Key keyHistory[MAX_GAME_LENGTH];
	int moveCounter;
	int halfMoveCounter;
	int halfNullMoveCounter;
	GamePhase gamePhase;
	Square kingSquare[ALL_PIECE_COLOR];
	bool inCheck;

	// clear structure node
	void clear()
	{
		key=0ULL;
		pawnKey=0ULL;
		moveCounter=0;
		halfMoveCounter=0;
		halfNullMoveCounter=0;
		gamePhase=OPENING;
		memset(piece.array, 0, sizeof(Bitboard)*ALL_PIECE_TYPE_BY_COLOR);
		memset(pieceCount, 0, sizeof(int)*ALL_PIECE_TYPE_BY_COLOR);
		memset(keyHistory, 0, sizeof(Key)*MAX_GAME_LENGTH);
		for(register int x=0;x<ALL_SQUARE;x++){
			square[x]=EMPTY;
		}
		for(register int x=0;x<MAX_GAME_LENGTH;x++){
			keyHistory[x]=0x0ULL;
		}
		fullMaterial[WHITE]=0;
		fullMaterial[BLACK]=0;
		psq[WHITE]=0;
		psq[BLACK]=0;
		pieceColorCount[WHITE]=0;
		pieceColorCount[BLACK]=0;
		pieceColor[WHITE]=0ULL;
		pieceColor[BLACK]=0ULL;
		pieceColor[COLOR_NONE]=FULL_BB;
		castleRight[WHITE]=NO_CASTLE;
		castleRight[BLACK]=NO_CASTLE;
		castleDone[WHITE]=false;
		castleDone[BLACK]=false;
		enPassant=NONE;
		sideToMove=COLOR_NONE;
		kingSquare[WHITE]=NONE;
		kingSquare[BLACK]=NONE;
		kingSquare[COLOR_NONE]=NONE;
		inCheck=false;
	}

};

// Zobrist keys for hashing
struct NodeZobrist {
	Key pieceSquare[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE];
	Key castleRight[ALL_CASTLE_RIGHT*ALL_CASTLE_RIGHT];
	Key enPassant[ALL_FILE];
	Key sideToMove[ALL_PIECE_COLOR];
	Key ignoreMove;
};

}

using namespace BoardTypes;

static int fullPst[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE];

class Board
{

public:

	Board();
	Board(const Board& board);
	virtual ~Board();

	const void printBoard();
	const void printBoard(const std::string pad);
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
	const PieceColor getPieceColor(const Square square) const;
	const PieceType getPieceType(const PieceTypeByColor piece) const;
	const PieceType getPieceType(const Square square) const;
	const Square makeSquare(const Rank rank, const File file) const;
	const Rank getSquareRank(const Square square) const;
	const File getSquareFile(const Square square) const;
	const bool isAttacked(const PieceColor color, const Square from);
	const bool setInCheck(const PieceColor color);
	const bool isNotLegal();
	const bool isInCheck();
	void setInCheck(const bool check);

	template <bool isMoveFromCache>
	const bool isMoveLegal(MoveIterator::Move& move);
	const bool isAttackedBy(const MoveIterator::Move& move);
	const bool isDraw();
	const bool isCastleDone(const PieceColor color);
	const bool isCaptureMove(MoveIterator::Move& move);

	const Bitboard getPieces(const PieceColor color) const;
	const Bitboard getAllPieces() const;
	const Bitboard getEmptySquares() const;
	const Bitboard getPieces(const PieceTypeByColor piece) const;
	const Bitboard getPieces(const PieceColor color, const PieceType pieceType) const;
	const PieceTypeByColor getPiece(const Square square) const;
	const int getPieceCount(const PieceColor color, const PieceType pieceType) const;
	const int getPieceCount(const PieceTypeByColor piece) const;
	const int getPieceCount(const PieceColor color ) const;
	const int getMaterial(const PieceColor color) const;
	const int getPieceSquareValue(const PieceColor color) const;
	const int getMaterialPst(const PieceColor color) const;

	void generateCaptures(MoveIterator& moves, const PieceColor side);
	void generateQuiesMoves(MoveIterator& moves, const PieceColor side);
	void generateNonCaptures(MoveIterator& moves, const PieceColor side);
	Bitboard getDCPieces(const Bitboard pseudoAttacks, const Bitboard attackingPieces,
			const Bitboard attacks, const Bitboard discoveredCandidates, const Bitboard sidePieces, const bool diagonalAttacks);
	void generateNonCaptureChecks(MoveIterator& moves, const PieceColor side);
	void generateEvasions(MoveIterator& moves, const PieceColor side);
	void generateAllMoves(MoveIterator& moves, const PieceColor side);

	void generatePawnCaptures(MoveIterator& moves, const PieceColor side, const Bitboard mask);
	void generatePawnMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask);
	void generatePromotion(MoveIterator& moves, const PieceColor side, const Bitboard mask);
	void generateKnightMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask);
	void generateBishopMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask);
	void generateRookMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask);
	void generateQueenMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask);
	void generateKingMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask);
	void generateCastleMoves(MoveIterator& moves, const PieceColor side);

	void generatePawnCaptures(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard piece);
	void generatePawnMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard piece);
	void generateKnightMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard piece);
	void generateBishopMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard piece);
	void generateRookMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard piece);
	void generateQueenMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard piece);
	void generateCastleMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask);

	const Bitboard getRookAttacks(const Square square);
	const Bitboard getRookAttacks(const Square square, const Bitboard occupied);
	const Bitboard getBishopAttacks(const Square square);
	const Bitboard getBishopAttacks(const Square square, const Bitboard occupied);
	const Bitboard getQueenAttacks(const Square square);
	const Bitboard getQueenAttacks(const Square square, const Bitboard occupied);
	const Bitboard getKnightAttacks(const Square square);
	const Bitboard getKnightAttacks(const Square square, const Bitboard occupied);
	const Bitboard getPawnAttacks(const Square square);
	const Bitboard getPawnAttacks(const Square square,  const PieceColor color);
	const Bitboard getPawnMoves(const Square square);
	const Bitboard getPawnMoves(const Square square, const Bitboard occupied);
	const Bitboard getPawnCaptures(const Square square);
	const Bitboard getPawnCaptures(const Square square, const Bitboard occupied);
	const Bitboard getKingAttacks(const Square square);
	const Bitboard getKingAttacks(const Square square, const Bitboard occupied);

	static void initialize();
	const GamePhase predictGamePhase();
	const void calcFullPieceSquareValue();
	const int getZobristCastleIndex();
	const Key getKey() const;
	const Key getPartialSearchKey() const;
	const Key getPawnKey() const;
	void setKey(Key key);
	void setPawnKey(Key key);
	const Key generateKey();
	const Key generatePawnKey();

	void increaseMoveCounter();
	void decreaseMoveCounter();
	const int getMoveCounter() const;

	void increaseHalfMoveCounter();
	void resetHalfMoveCounter();
	const int getHalfMoveCounter() const;

	void increaseHalfNullMoveCounter();
	void resetHalfNullMoveCounter();
	const int getHalfNullMoveCounter() const;

	void updateKeyHistory();

	const GamePhase getGamePhase();
	void setGamePhase(const GamePhase phase);

	Square getKingSquare(const PieceColor color);
	bool isPawnFinal();
	bool isPawnFinal(const Square exclude);
	bool isCaptureOrPromotion(MoveIterator::Move& move);
	bool isPawnPromoting();
	bool isPawnOn6th();
	static const int interpolate(const int first, const int second, const int position);
	const int getPieceSquareValue(const PieceTypeByColor piece, const Square square, GamePhase phase);
	static const int calcPieceSquareValue(const PieceTypeByColor piece, const Square square, GamePhase phase);
	void clearBoard();

private:

	Node& getBoard();
	void putPiece(const PieceTypeByColor piece, const Square square);
	void removePiece(const PieceTypeByColor piece, const Square square);

	Node currentBoard;

};
// get the board structure
inline Node& Board::getBoard() {
	return currentBoard;
}

inline void Board::clearBoard() {
	currentBoard.clear();
}
// put a piece in the board and store piece info
inline void Board::putPiece(const PieceTypeByColor piece, const Square square) {
	const PieceColor color = pieceColor[piece];
	currentBoard.piece.array[piece] |= squareToBitboard[square];
	currentBoard.pieceColor[color] |= squareToBitboard[square];
	currentBoard.square[square] = piece;
	currentBoard.pieceCount[piece]++;
	currentBoard.fullMaterial[color]+=materialValues[piece];
	currentBoard.psq[color]+=fullPst[piece][square];
	currentBoard.pieceColorCount[color]++;

	if (piece==WHITE_KING || piece==BLACK_KING) {
		currentBoard.kingSquare[color]=square;
	}
}
// remove a piece from the board and erase piece info
inline void Board::removePiece(const PieceTypeByColor piece, const Square square) {
	const PieceColor color = pieceColor[piece];
	currentBoard.piece.array[piece] ^= squareToBitboard[square];
	currentBoard.pieceColor[color] ^= squareToBitboard[square];
	currentBoard.square[square] = EMPTY;
	currentBoard.pieceCount[piece]--;
	currentBoard.fullMaterial[color]-=materialValues[piece];
	currentBoard.psq[color]-=fullPst[piece][square];
	currentBoard.pieceColorCount[color]--;

	if (piece==WHITE_KING || piece==BLACK_KING) {
		currentBoard.kingSquare[color]=NONE;
	}
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
inline const CastleRight Board::getCastleRights(PieceColor color) const {
	return currentBoard.castleRight[color];
}

// remove castle rights passed as params
inline void Board::removeCastleRights(const PieceColor color, const CastleRight castle) {
	currentBoard.castleRight[color]=(CastleRight)((int)(currentBoard.castleRight[color])&(~(int)(castle)));
}

// set castle rights
inline void Board::setCastleRights(const PieceColor color, const CastleRight castle) {
	currentBoard.castleRight[color]=castle;
}

// can castle?
inline bool Board::canCastle(const PieceColor color) {

	return canCastle(color, BOTH_SIDE_CASTLE);
}

// can castle specific?
inline bool Board::canCastle(const PieceColor color, const CastleRight castleRight) {

	// lost the right to castle?
	if (isCastleDone(color)) {
		return false;
	} else if (currentBoard.castleRight[color]!=BOTH_SIDE_CASTLE &&
			currentBoard.castleRight[color]!=castleRight) {
		return false;
	}

	// is king in check?
	if (isInCheck()) {
		return false;
	}

	// pieces interposing king & rooks?
	if (castleSquare[color][castleRight]&getAllPieces()) {
		return false;
	}

	// squares through castle & king destination attacked?
	if (castleRight==BOTH_SIDE_CASTLE || castleRight==KING_SIDE_CASTLE) {
		if (color==WHITE) {
			if (isAttacked(color,F1) || isAttacked(color,G1)) {
				return false;
			}
		} else {
			if (isAttacked(color,F8) || isAttacked(color,G8)) {
				return false;
			}
		}
	}

	if (castleRight==BOTH_SIDE_CASTLE || castleRight==QUEEN_SIDE_CASTLE) {
		if (color==WHITE) {
			if (isAttacked(color,D1) || isAttacked(color,C1)) {
				return false;
			}
		} else {
			if (isAttacked(color,D8) || isAttacked(color,C8)) {
				return false;
			}
		}
	}
	return true;
}

// get
inline const PieceColor Board::getSideToMove() const {
	return currentBoard.sideToMove;
}

// set
inline void Board::setSideToMove(const PieceColor color) {
	currentBoard.sideToMove=color;
}

// get en passant
inline const Square Board::getEnPassant() const {
	return currentBoard.enPassant;
}

// set en passant
inline void Board::setEnPassant(const Square square) {
	currentBoard.enPassant=square;
}

// flip side
inline const PieceColor Board::flipSide(const PieceColor color) {
	return PieceColor((int)(color)^1);
}

// get piece color
inline const PieceColor Board::getPieceColor(const PieceTypeByColor piece) const {
	return pieceColor[piece];
}

// get piece color by square
inline const PieceColor Board::getPieceColor(const Square square) const {
	return pieceColor[currentBoard.square[square]];
}

// get piece type
inline const PieceType Board::getPieceType(const PieceTypeByColor piece) const {
	return pieceType[piece];
}

// get piece type by square
inline const PieceType Board::getPieceType(const Square square) const {
	return pieceType[currentBoard.square[square]];
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

inline const bool Board::isAttacked(const PieceColor color, const Square from) {

	const PieceColor other = flipSide(color);

	if (from!=NONE) {
		return 	(getBishopAttacks(from) & (getPieces(other,BISHOP) |
				getPieces(other,QUEEN))) ||
				(getRookAttacks(from) & (getPieces(other,ROOK) |
						getPieces(other,QUEEN))) ||
						(getKnightAttacks(from) & getPieces(other,KNIGHT)) ||
						(getPawnAttacks(from,color) & getPieces(other,PAWN)) ||
						(getKingAttacks(from) & getPieces(other,KING));
	}

	return false;
}

inline const bool Board::setInCheck(const PieceColor color) {
	setInCheck(isAttacked(color, getKingSquare(color)));
	return isInCheck();
}

// verify board legality
inline const bool Board::isNotLegal() {
	const PieceColor color = flipSide(getSideToMove());
	return setInCheck(color);
}

inline const bool Board::isInCheck() {
	return currentBoard.inCheck;
}

inline void Board::setInCheck(const bool check) {
	currentBoard.inCheck = check;
}

// verify board legality
template <bool isMoveFromCache>
inline const bool Board::isMoveLegal(MoveIterator::Move& move) {

	const PieceTypeByColor fromPiece=getPiece(move.from);
	const PieceTypeByColor toPiece=getPiece(move.to);
	const PieceColor color = getPieceColor(move.from);
	const PieceType fromType = pieceType[fromPiece];

	if (isMoveFromCache) {
		if (move.none() || color==getPieceColor(move.to) ||
				color != getSideToMove() || fromPiece==EMPTY) {
			return false;
		}
		const bool pawnPromoting = fromType==PAWN &&
				(squareToBitboard[move.from] & promoRank[color]);
		const bool hasPromoPiece=move.promotionPiece!=EMPTY;

		if (hasPromoPiece != pawnPromoting) {
			return false;
		}
		if (((fromPiece==WHITE_KING && (move==whiteoo || move==whiteooo)) ||
				(fromPiece==BLACK_KING && (move==blackoo || move==blackooo)))) {
			const CastleRight castleRight =	(move.to==C1 || move.to==C8) ?
					QUEEN_SIDE_CASTLE : KING_SIDE_CASTLE;
			return canCastle(color, castleRight);
		}
		if (!isAttackedBy(move)) {
			return false;
		}
	} else {
		if (move.type==MoveIterator::CASTLE) {
			return true;
		}
	}

	if (fromType==KING) {
		if (isAttacked(color,move.to)) {
			return false;
		}
	}

	const Square kingSq = fromType==KING?move.to:getKingSquare(color);
	const PieceColor other = flipSide(color);
	const Bitboard moveTo = squareToBitboard[move.to];
	const Bitboard moveFrom = squareToBitboard[move.from];
	Bitboard allPieces = (getAllPieces()^moveFrom)|moveTo;

	Bitboard pawns = (getPieces(other,PAWN))&~moveTo;

	if (pawns && fromType==PAWN &&
			toPiece == EMPTY && getEnPassant()!=NONE &&
			getSquareFile(move.from)!=getSquareFile(move.to)) {
		pawns &= ~squareToBitboard[getEnPassant()];
		allPieces &= ~squareToBitboard[getEnPassant()];
	}

	const Bitboard bishopAndQueens = ((getPieces(other,BISHOP) |
			getPieces(other,QUEEN)))&~moveTo;

	if (bishopAndQueens &&
			(getBishopAttacks(kingSq,allPieces)&bishopAndQueens)) {
		return false;
	}

	const Bitboard rookAndQueens = ((getPieces(other,ROOK) |
			getPieces(other,QUEEN)))&~moveTo;

	if (rookAndQueens &&
			(getRookAttacks(kingSq,allPieces)&rookAndQueens)) {
		return false;
	}

	const Bitboard knights = (getPieces(other,KNIGHT))&~moveTo;

	if (knights &&
			(getKnightAttacks(kingSq,allPieces)&knights)) {
		return false;
	}

	if (pawns &&
			(getPawnAttacks(kingSq,color)&pawns)) {
		return false;
	}

	return true;
}

// Get attacks from a given square
inline const bool Board::isAttackedBy(const MoveIterator::Move& move) {
	const PieceType pieceType = getPieceType(move.from);
	Bitboard attacks = EMPTY_BB;
	switch (pieceType) {
	case PAWN:
		if (getSquareFile(move.from)!=getSquareFile(move.to)) {
			attacks = getPawnCaptures(move.from);
		} else {
			attacks = getPawnMoves(move.from);
		}
		break;
	case KNIGHT:
		attacks = getKnightAttacks(move.from);
		break;
	case BISHOP:
		attacks = getBishopAttacks(move.from);
		break;
	case ROOK:
		attacks = getRookAttacks(move.from);
		break;
	case QUEEN:
		attacks = getQueenAttacks(move.from);
		break;
	case KING:
		attacks = getKingAttacks(move.from);
		break;
	default:
		break;
	}
	return attacks & squareToBitboard[move.to];
}

// verify draw by 50th move rule, 3 fold rep and insuficient material
inline const bool Board::isDraw() {
	const Bitboard pawns = getPieces(WHITE_PAWN)|getPieces(BLACK_PAWN);
	if (!pawns) {
		if (getMaterial(WHITE)+getMaterial(BLACK)<=minimalMaterial) {
			return true;
		}
	}

	if (getHalfMoveCounter()>=100 && !isInCheck()) {
		return true;
	}

	const int halfMoveCounter = std::min(getHalfMoveCounter(), getHalfNullMoveCounter());

	for (int x = 4; x <= halfMoveCounter; x += 2) {
		if (currentBoard.keyHistory[getMoveCounter()-x]==getKey()) {
			return true;
		}
	}

	return false;

}

// verify if castle has been made
inline const bool Board::isCastleDone(const PieceColor color) {
	return currentBoard.castleDone[color];
}

// verify if move is capture
inline const bool Board::isCaptureMove(MoveIterator::Move& move) {
	if (getPiece(move.to)!=EMPTY) {
		return true;
	} else if (getPieceType(move.from)==PAWN){
		if (getEnPassant()!=NONE &&
				getSquareFile(move.from)!=getSquareFile(move.to)) {
			return true;
		}
	}
	return false;
}

// get all pieces of a given color
inline const Bitboard Board::getPieces(const PieceColor color) const {
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
inline const Bitboard Board::getPieces(const PieceTypeByColor piece) const {
	return currentBoard.piece.array[piece];
}

// get pieces by type
inline const Bitboard Board::getPieces(const PieceColor color, const PieceType pieceType) const {
	return currentBoard.piece.array[makePiece(color,pieceType)];
}

// get pieces by square
inline const PieceTypeByColor Board::getPiece(const Square square) const {
	return currentBoard.square[square];
}

// get piece count by type
inline const int Board::getPieceCount(const PieceColor color, const PieceType pieceType) const {
	return currentBoard.pieceCount[makePiece(color,pieceType)];
}

// get piece count by type
inline const int Board::getPieceCount(const PieceTypeByColor piece) const {
	return currentBoard.pieceCount[piece];
}

// get piece count by type
inline const int Board::getPieceCount(const PieceColor color) const {
	return currentBoard.pieceColorCount[color];
}

// get full material by side
inline const int Board::getMaterial(const PieceColor color) const {
	return currentBoard.fullMaterial[color];
}

// get pst value
inline const int Board::getPieceSquareValue(const PieceColor color) const {
	return currentBoard.psq[color];
}

// get eval
inline const int Board::getMaterialPst(const PieceColor color) const {
	return currentBoard.psq[color]+currentBoard.fullMaterial[color];
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getRookAttacks(const Square square) {
	return getRookAttacks(square, getAllPieces());
}

// return a bitboard with attacked squares by the rook in the given square
inline const Bitboard Board::getRookAttacks(const Square square, const Bitboard occupied) {
	if (square==NONE) {
		return EMPTY_BB;
	}
	return R_MAGIC(square, occupied);
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getBishopAttacks(const Square square) {
	return getBishopAttacks(square, getAllPieces());
}

// return a bitboard with attacked squares by the bishop in the given square
inline const Bitboard Board::getBishopAttacks(const Square square, const Bitboard occupied) {
	if (square==NONE) {
		return EMPTY_BB;
	}
	return B_MAGIC(square, occupied);
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getQueenAttacks(const Square square) {
	return getQueenAttacks(square, getAllPieces());
}

// return a bitboard with attacked squares by the queen in the given square
inline const Bitboard Board::getQueenAttacks(const Square square, const Bitboard occupied) {
	if (square==NONE) {
		return EMPTY_BB;
	}
	return Q_MAGIC(square, occupied);
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
inline const Bitboard Board::getPawnMoves(const Square square) {
	return getPawnMoves(square, getAllPieces());
}
// overload method - gets current occupied squares in the board
inline const Bitboard Board::getPawnAttacks(const Square square) {
	return getPawnAttacks(square,getPieceColor(square));
}

// return a bitboard with move squares by the pawn in the given square
inline const Bitboard Board::getPawnAttacks(const Square square, const PieceColor color) {
	return color==WHITE?
			whitePawnAttacks[square]:blackPawnAttacks[square];
}

// return a bitboard with move squares by the pawn in the given square
inline const Bitboard Board::getPawnMoves(const Square square, const Bitboard occupied) {
	const PieceColor color = getPieceColor(square);
	Bitboard pawnMoves = color==WHITE?whitePawnMoves[square]:blackPawnMoves[square];
	if (squareRank[square]==RANK_2 && color==WHITE) {
		if (pawnMoves&~occupied) {
			pawnMoves |= pawnMoves<<8; // double move
		}
	} else if (squareRank[square]==RANK_7 && color==BLACK) {
		if (pawnMoves&~occupied) {
			pawnMoves |= pawnMoves>>8; // double move
		}
	}
	return pawnMoves & ~occupied;
}

// overload method - gets current occupied squares in the board
inline const Bitboard Board::getPawnCaptures(const Square square) {
	return getPawnCaptures(square, getAllPieces());
}

// return a bitboard with captures by the pawn in the given square
inline const Bitboard Board::getPawnCaptures(const Square square, const Bitboard occupied) {
	const PieceColor color = getPieceColor(square);
	const Bitboard pawnAttacks = color==WHITE ?
			whitePawnAttacks[square]:blackPawnAttacks[square];
	Bitboard occ = occupied;
	if (getEnPassant()!=NONE) {
		const Bitboard epSquare = squareToBitboard[getEnPassant()];
		occ |= color==WHITE?epSquare<<8:epSquare>>8; // en passant
	}
	return pawnAttacks & occ;
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
	const Bitboard otherPieces = getPieces(otherSide);

	generatePawnCaptures(moves, side, otherPieces);
	generateKnightMoves(moves, side, otherPieces);
	generateBishopMoves(moves, side, otherPieces);
	generateRookMoves(moves, side, otherPieces);
	generateQueenMoves(moves, side, otherPieces);
	generateKingMoves(moves, side, otherPieces);

}

//generate only non capture moves
inline void Board::generateNonCaptures(MoveIterator& moves, const PieceColor side){

	const Bitboard empty = getEmptySquares();

	generatePawnMoves(moves, side, empty);
	generateKnightMoves(moves, side, empty);
	generateBishopMoves(moves, side, empty);
	generateRookMoves(moves, side, empty);
	generateQueenMoves(moves, side, empty);
	generateKingMoves(moves, side, empty);
	generateCastleMoves(moves, side);

}

//generate quiescence moves
inline void Board::generateQuiesMoves(MoveIterator& moves, const PieceColor side){
	generateCaptures(moves, side);
	generateNonCaptureChecks(moves, side);
	generatePromotion(moves, side, getEmptySquares());
}

// Get the discovered check pieces
inline Bitboard Board::getDCPieces(const Bitboard pseudoAttacks,
		const Bitboard attackingPieces,	const Bitboard attacks,
		const Bitboard discoveredCandidates,
		const Bitboard sidePieces, const bool diagonalAttacks) {
	Bitboard dcs = pseudoAttacks&attackingPieces?
			(pseudoAttacks & attacks & sidePieces):EMPTY_BB;
	Bitboard result = EMPTY_BB;
	Square from = extractLSB(dcs);
	while ( from!=NONE ) {
		const Bitboard dc = squareToBitboard[from];
		if (dc&~discoveredCandidates&~attackingPieces&~result) {
			const Bitboard sliderAttacks = diagonalAttacks ?
					getBishopAttacks(from) : getRookAttacks(from);
			const Bitboard attackingDc = pseudoAttacks &
					sliderAttacks & attackingPieces;
			if (attackingDc) {
				result |= dc;
			}
		}
		from = extractLSB(dcs);
	}
	return result;
}

// generate non capture checks
inline void Board::generateNonCaptureChecks(MoveIterator& moves, const PieceColor side) {

	const PieceColor otherSide = flipSide(side);
	const Bitboard empty = getEmptySquares();
	const Square kingSquare = getKingSquare(otherSide);
	const Bitboard sidePieces = getPieces(side);

	const Bitboard pawn = getPieces(side,PAWN);
	const Bitboard king = getPieces(side,KING);
	const Bitboard knight = getPieces(side,KNIGHT);
	const Bitboard bishop = getPieces(side,BISHOP);
	const Bitboard rook = getPieces(side,ROOK);
	const Bitboard queen = getPieces(side,QUEEN);

	const Bitboard bishopAndQueen = (bishop|queen);
	const Bitboard rookAndQueen = (rook|queen);

	const Bitboard bishopAttacks = bishopAndQueen ? getBishopAttacks(kingSquare) : EMPTY_BB;
	const Bitboard rookAttacks = rookAndQueen ? getRookAttacks(kingSquare) : EMPTY_BB;
	const Bitboard knightAttacks = knight ? getKnightAttacks(kingSquare) : EMPTY_BB;
	const Bitboard pawnAttacks = pawn ? getPawnAttacks(kingSquare,otherSide) : EMPTY_BB;
	const Bitboard rookPseudoAttacks = fileAttacks[kingSquare]|rankAttacks[kingSquare];
	const Bitboard bishopPseudoAttacks = diagA1H8Attacks[kingSquare]|diagH1A8Attacks[kingSquare];

	Bitboard discoverCandidate = EMPTY_BB;

	const Bitboard rookAndQueenDc = ~rookAttacks&rookAndQueen&rookPseudoAttacks;
	if (rookAndQueen && rookAttacks && rookAndQueenDc) {
		Bitboard dc = getDCPieces(rookPseudoAttacks,rookAndQueenDc,rookAttacks,
				discoverCandidate,sidePieces, false);
		if (dc) {
			generateBishopMoves(moves, side, empty & ~bishopAttacks, dc&bishop);
			discoverCandidate=dc;
		}
	}
	const Bitboard bishopAndQueenDc = ~bishopAttacks&bishopAndQueen&bishopPseudoAttacks;
	if (bishopAndQueen && bishopAttacks && bishopAndQueenDc) {
		Bitboard dc = getDCPieces(bishopPseudoAttacks,bishopAndQueenDc,bishopAttacks,
				discoverCandidate,sidePieces, true);
		if (dc) {
			generatePawnMoves(moves, side, empty & ~pawnAttacks, discoverCandidate&pawn);
			generateRookMoves(moves, side, empty & ~rookAttacks, discoverCandidate&rook);
			discoverCandidate|=dc;
		}
	}

	if (discoverCandidate) {
		generateKnightMoves(moves, side, empty & ~knightAttacks,
				discoverCandidate&knight);
		generateKingMoves(moves, side, discoverCandidate&king);
	}

	if (bishopAttacks & bishop) {
		generateBishopMoves(moves, side, empty & bishopAttacks);
	}

	if ((bishopAttacks|rookAttacks) & queen) {
		generateQueenMoves(moves, side, empty & (bishopAttacks|rookAttacks));
	}

	if (rookAttacks&rook) {
		generateRookMoves(moves, side, empty & rookAttacks);
	}

	if (knightAttacks&knight) {
		generateKnightMoves(moves, side, empty & knightAttacks);
	}

	if (pawnAttacks&pawn) {
		generatePawnMoves(moves, side, empty & pawnAttacks);
	}

	if (rookAttacks&&rook) {
		generateCastleMoves(moves,side,rookAttacks);
	}
}

// generate check evasions
inline void Board::generateEvasions(MoveIterator& moves, const PieceColor side) {

	const PieceColor otherSide = flipSide(side);
	const Bitboard empty = getEmptySquares();
	const Square kingSquare = getKingSquare(side);
	const Bitboard otherPieces = getPieces(otherSide);

	const Bitboard bishop = getPieces(otherSide,BISHOP);
	const Bitboard rook = getPieces(otherSide,ROOK);
	const Bitboard queen = getPieces(otherSide,QUEEN);
	const Bitboard knight = getPieces(otherSide,KNIGHT);
	const Bitboard pawn = getPieces(otherSide,PAWN);

	const Bitboard bishopAndQueen = (bishop | queen);
	const Bitboard rookAndQueen = (rook | queen);

	const Bitboard bishopAttacks = bishopAndQueen ? getBishopAttacks(kingSquare) : EMPTY_BB;
	const Bitboard rookAttacks = rookAndQueen ? getRookAttacks(kingSquare) : EMPTY_BB;
	const Bitboard knightAttacks = getKnightAttacks(kingSquare);
	const Bitboard pawnAttacks = getPawnAttacks(kingSquare);

	Bitboard kingAttackers = EMPTY_BB;

	if (bishopAttacks & bishopAndQueen) {
		const Bitboard diagA1H8 = (diagA1H8Attacks[kingSquare] & bishopAttacks);
		const Bitboard diagH1A8 = (diagH1A8Attacks[kingSquare] & bishopAttacks);
		if (diagA1H8 & bishopAndQueen) {
			kingAttackers |= diagA1H8;
		}
		if (diagH1A8 & bishopAndQueen) {
			kingAttackers |= diagH1A8;
		}
	}

	if (rookAttacks & rookAndQueen) {
		const Bitboard file = (fileAttacks[kingSquare] & rookAttacks);
		const Bitboard rank = (rankAttacks[kingSquare] & rookAttacks);
		if (file & rookAndQueen) {
			kingAttackers |= file;
		}
		if (rank & rookAndQueen) {
			kingAttackers |= rank;
		}
	}

	kingAttackers |= knightAttacks & knight;
	kingAttackers |= pawnAttacks & pawn;

	const Bitboard attackersAndEmpty = (otherPieces | empty) & kingAttackers;

	generatePawnCaptures(moves, side, otherPieces & kingAttackers);
	generatePawnMoves(moves, side, empty & kingAttackers);
	generateKnightMoves(moves, side, attackersAndEmpty);
	generateBishopMoves(moves, side, attackersAndEmpty);
	generateRookMoves(moves, side, attackersAndEmpty);
	generateQueenMoves(moves, side, attackersAndEmpty);
	generateKingMoves(moves, side, otherPieces|empty);

}

//generate all moves - captures + noncaptures
inline void Board::generateAllMoves(MoveIterator& moves, const PieceColor side) {

	const PieceColor otherSide = flipSide(side);
	const Bitboard mask = getEmptySquares() | getPieces(otherSide);

	generatePawnCaptures(moves, side, getPieces(otherSide));
	generatePawnMoves(moves, side, getEmptySquares());
	generateKnightMoves(moves, side, mask);
	generateBishopMoves(moves, side, mask);
	generateRookMoves(moves, side, mask);
	generateQueenMoves(moves, side, mask);
	generateKingMoves(moves, side, mask);
	generateCastleMoves(moves, side);

}

// generate pawn captures
inline void Board::generatePawnCaptures(MoveIterator& moves, const PieceColor side, const Bitboard mask) {
	return generatePawnCaptures(moves,side,mask,getPieces(side,PAWN));
}
// generate pawn captures
inline void Board::generatePawnCaptures(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard pieces) {

	Bitboard attacks = EMPTY_BB;
	Square from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getPawnCaptures(from,mask) ;
		Square target = extractLSB(attacks);

		bool promotion= squareToBitboard[from] & promoRank[side];

		while ( target!=NONE ) {
			if (promotion) {
				moves.add(from,target,makePiece(side,QUEEN), MoveIterator::PROMO_CAPTURE);
				moves.add(from,target,makePiece(side,KNIGHT), MoveIterator::PROMO_CAPTURE);
				moves.add(from,target,makePiece(side,ROOK), MoveIterator::PROMO_CAPTURE);
				moves.add(from,target,makePiece(side,BISHOP), MoveIterator::PROMO_CAPTURE);
			} else {
				moves.add(from,target,EMPTY);
			}
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}
}

// generate pawn moves
inline void Board::generatePawnMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask) {
	return generatePawnMoves(moves,side,mask,getPieces(side,PAWN));
}

// generate pawn moves
inline void Board::generatePawnMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard pieces) {

	Bitboard attacks = EMPTY_BB;
	Square from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getPawnMoves(from) & mask;
		Square target = extractLSB(attacks);

		bool promotion= squareToBitboard[from] & promoRank[side];

		while ( target!=NONE ) {
			if (promotion) {
				moves.add(from,target,makePiece(side,QUEEN),MoveIterator::PROMO_NONCAPTURE);
				moves.add(from,target,makePiece(side,KNIGHT),MoveIterator::PROMO_NONCAPTURE);
				moves.add(from,target,makePiece(side,ROOK),MoveIterator::PROMO_NONCAPTURE);
				moves.add(from,target,makePiece(side,BISHOP),MoveIterator::PROMO_NONCAPTURE);
			} else {
				moves.add(from,target,EMPTY);
			}
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

}

// generate only quiet promotions
inline void Board::generatePromotion(MoveIterator& moves, const PieceColor side, const Bitboard mask) {

	Bitboard pieces = getPieces(side,PAWN) & promoRank[side];
	Bitboard attacks = EMPTY_BB;
	Square from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getPawnMoves(from) & mask;
		Square target = extractLSB(attacks);

		while ( target!=NONE ) {
			moves.add(from,target,makePiece(side,QUEEN),MoveIterator::PROMO_NONCAPTURE);
			moves.add(from,target,makePiece(side,KNIGHT),MoveIterator::PROMO_NONCAPTURE);
			moves.add(from,target,makePiece(side,ROOK),MoveIterator::PROMO_NONCAPTURE);
			moves.add(from,target,makePiece(side,BISHOP),MoveIterator::PROMO_NONCAPTURE);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

}
// generate knight moves
inline void Board::generateKnightMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask) {
	return generateKnightMoves(moves,side,mask,getPieces(side,KNIGHT));
}

// generate knight moves
inline void Board::generateKnightMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard pieces) {

	Bitboard attacks = EMPTY_BB;
	Square from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getKnightAttacks(from) & mask;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			moves.add(from,target,EMPTY);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

}

// generate bishop moves
inline void Board::generateBishopMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask) {
	return generateBishopMoves(moves,side,mask,getPieces(side,BISHOP));
}

// generate bishop moves
inline void Board::generateBishopMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard pieces) {

	Bitboard attacks = EMPTY_BB;
	Square from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getBishopAttacks(from) & mask;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			moves.add(from,target,EMPTY);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

}
// generate rook moves
inline void Board::generateRookMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask) {
	return generateRookMoves(moves,side,mask,getPieces(side,ROOK));
}

// generate rook moves
inline void Board::generateRookMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard pieces) {

	Bitboard attacks = EMPTY_BB;
	Square from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getRookAttacks(from) & mask;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			moves.add(from,target,EMPTY);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

}

// generate queen moves
inline void Board::generateQueenMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask) {
	return generateQueenMoves(moves,side,mask,getPieces(side,QUEEN));
}

// generate queen moves
inline void Board::generateQueenMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask, Bitboard pieces) {

	Bitboard attacks = EMPTY_BB;
	Square from = extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = getQueenAttacks(from) & mask;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			moves.add(from,target,EMPTY);
			target = extractLSB(attacks);
		}
		from = extractLSB(pieces);
	}

}

// generate queen moves
inline void Board::generateKingMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask) {

	Bitboard attacks = EMPTY_BB;
	Square from = getKingSquare(side);

	if ( from!=NONE ) {
		attacks = getKingAttacks(from) & mask ;
		Square target = extractLSB(attacks);
		while ( target!=NONE ) {
			moves.add(from,target,EMPTY);
			target = extractLSB(attacks);
		}
	}

}

// generate castle moves
inline void Board::generateCastleMoves(MoveIterator& moves, const PieceColor side, const Bitboard mask) {

	if (canCastle(side, KING_SIDE_CASTLE)) {
		if (side==WHITE && (squareToBitboard[F1] & mask)) {
			moves.add(E1,G1,EMPTY,MoveIterator::CASTLE);
		} else if (side==BLACK && (squareToBitboard[F8] & mask)) {
			moves.add(E8,G8,EMPTY,MoveIterator::CASTLE);
		}
	}

	if (canCastle(side, QUEEN_SIDE_CASTLE)) {
		if (side==WHITE && (squareToBitboard[D1] & mask)) {
			moves.add(E1,C1,EMPTY,MoveIterator::CASTLE);
		} else if (side==BLACK && (squareToBitboard[D8] & mask)) {
			moves.add(E8,C8,EMPTY,MoveIterator::CASTLE);
		}
	}
}

// generate castle moves
inline void Board::generateCastleMoves(MoveIterator& moves, const PieceColor side) {

	if (canCastle(side, KING_SIDE_CASTLE)) {
		if (side==WHITE) {
			moves.add(E1,G1,EMPTY,MoveIterator::CASTLE);
		} else {
			moves.add(E8,G8,EMPTY,MoveIterator::CASTLE);
		}
	}

	if (canCastle(side, QUEEN_SIDE_CASTLE)) {
		if (side==WHITE) {
			moves.add(E1,C1,EMPTY,MoveIterator::CASTLE);
		} else {
			moves.add(E8,C8,EMPTY,MoveIterator::CASTLE);
		}
	}
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

// increase the game null half move counter
inline void Board::increaseHalfNullMoveCounter() {
	currentBoard.halfNullMoveCounter++;
}

// decrease the game null half move counter
inline void Board::resetHalfNullMoveCounter() {
	currentBoard.halfNullMoveCounter=0;
}

// get
inline const int Board::getHalfNullMoveCounter() const {
	return currentBoard.halfNullMoveCounter;
}

// update key history
inline void Board::updateKeyHistory() {
	currentBoard.keyHistory[getMoveCounter()]=getKey();
}

// get game phase
inline const GamePhase Board::getGamePhase() {
	return currentBoard.gamePhase;
}

// set game phase
inline void Board::setGamePhase(const GamePhase phase) {
	currentBoard.gamePhase=GamePhase(std::max(0,std::min(maxGamePhase,(int)(phase))));
}

inline Square Board::getKingSquare(const PieceColor color) {
	return currentBoard.kingSquare[color];
}

// remains pawns & kings only?
inline bool Board::isPawnFinal() {
	Bitboard pieces = getPieces(WHITE_PAWN) | getPieces(BLACK_PAWN) |
			getPieces(WHITE_KING) | getPieces(BLACK_KING);
	return !(pieces^getAllPieces());
}

// remains pawns & kings only?
inline bool Board::isPawnFinal(const Square exclude) {
	Bitboard pieces = getPieces(WHITE_PAWN) | getPieces(BLACK_PAWN) |
			getPieces(WHITE_KING) | getPieces(BLACK_KING);
	pieces|=squareToBitboard[exclude];
	return !(pieces^getAllPieces());
}

// capture or promotion
inline bool Board::isCaptureOrPromotion(MoveIterator::Move& move) {
	return isCaptureMove(move) || move.promotionPiece != EMPTY;
}

// pawn promoting
inline bool Board::isPawnPromoting() {
	return (getPieces(WHITE_PAWN) & rankBB[RANK_7]) ||
			(getPieces(BLACK_PAWN) & rankBB[RANK_2]);
}
// pawn close to promotion
inline bool Board::isPawnOn6th() {
	return (getPieces(WHITE_PAWN) & rankBB[RANK_6]) ||
			(getPieces(BLACK_PAWN) & rankBB[RANK_3]);
}

#endif /* BOARD_H_ */
