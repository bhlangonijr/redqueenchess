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
 * Board.cpp
 *
 *  Created on: Feb 21, 2009
 *      Author: bhlangonijr
 */
#include <iostream>
#include <vector>

#include "Board.h"
#include "StringUtil.h"

using namespace BoardTypes;

Board::Board() : currentBoard(*(new Node()))
{
	this->setInitialPosition();
	setAttackedSquares(getSideToMove(), generateAttackedSquares(getSideToMove()));
	setAttackedSquares(flipSide(getSideToMove()), generateAttackedSquares(flipSide(getSideToMove())));

}

Board::Board(const Board& board) : currentBoard( *(new Node(board.currentBoard)) )
{
}

Board::~Board()
{
	delete &currentBoard;
}

// print board for debug
const void Board::printBoard()
{

	std::vector< std::string> ranks(8);
	int j=0;

	for(int x=0;x<ALL_SQUARE;x++) {

		if (currentBoard.piece.array[currentBoard.square[x]]&squareToBitboard[x]) {
			ranks[j]+= " ";
			ranks[j]+= pieceChar[currentBoard.square[x]];
			ranks[j]+= " ";

		} else {
			ranks[j]+= " ";
			ranks[j]+= pieceChar[12];
			ranks[j]+= " ";
		}
		if ((x+1)%8==0) {
			j++;
		}
	}
	std::cout << "__________________________" << std::endl;
	for(int x=7;x>=0;x--) {
		std::cout << (x+1) << "|" << ranks[x] << std::endl;
		std::cout << "__________________________" << std::endl;
	}

	std::cout << "   a  b  c  d  e  f  g  h " << std::endl;

}

// testing method
void Board::genericTest() {
	//testing code

	//this->printBitboard((fileBB[squareFile[C7]] & (currentBoard.pieceColor[WHITE] | currentBoard.pieceColor[BLACK])) ^ squareToBitboard[C7]);
	this->printBitboard(this->getPiecesByColor(WHITE));
	this->printBitboard(this->getPiecesByColor(flipSide(WHITE)));
	Bitboard empty = EMPTY_BB;
	//for(int x=A2;x<=H7;x++) {
	//this->printBitboard( getPawnAttacks(Square(x), empty ));
	//printBitboard( whitePawnAttacks[A1] );
	//}
	uint32_t start = this->getTickCount();
	PieceColor color = getSideToMove();
	int counter=0;
	//for (int x=0;x<1000000;x++)
	{

		MovePool movePool;
		Move* move=this->generateAllMoves(movePool,color);

		color = flipSide(color);


		while (move) {
			//std::cout << counter << " - " << move->from << " to " << move->to << std::endl;
			std::cout << counter << " - " << squareToString[move->from] << " to " << squareToString[move->to] << std::endl;
			MoveBackup backup;
			doMove(*move,backup);
			printBoard();
			undoMove(backup);
			counter++;
			move = move->next;

		}

		movePool.~object_pool();
	}
	std::cout << "Time: " << (this->getTickCount()-start) << std::endl;
	std::cout << "Perft: " << counter << std::endl;

	this->printBitboard(this->getAttackedSquares(WHITE)&(this->getPiecesByColor(BLACK)|this->getEmptySquares()));

	// end tests

}

// do a move and set backup info into struct MoveBackup
void Board::doMove(const Move move, MoveBackup& backup){

	PieceTypeByColor fromPiece=this->getPieceBySquare(move.from);
	PieceTypeByColor toPiece=this->getPieceBySquare(move.to);
	bool enPassant=false;

	backup.move=move;
	backup.enPassant=getEnPassant();
	backup.whiteCastleRight=getCastleRights(WHITE);
	backup.blackCastleRight=getCastleRights(BLACK);
	backup.hasWhiteKingCastle=false;
	backup.hasWhiteQueenCastle=false;
	backup.hasBlackKingCastle=false;
	backup.hasBlackQueenCastle=false;
	backup.attackedSquares[WHITE]=getAttackedSquares(WHITE);
	backup.attackedSquares[BLACK]=getAttackedSquares(BLACK);

	removePiece(fromPiece,move.from);

	if (toPiece!=EMPTY) {
		removePiece(toPiece,move.to);
		backup.hasCapture=true;
		backup.capturedPiece=toPiece;
		backup.capturedSquare=move.to;
	} else {
		backup.hasCapture=false;
	}

	if (move.promotionPiece==EMPTY) {
		putPiece(fromPiece,move.to);
		backup.hasPromotion=false;
	} else {
		putPiece(move.promotionPiece,move.to);
		backup.hasPromotion=true;
	}

	if (getCastleRights(getSideToMove())!=NO_CASTLE) {
		if (fromPiece==WHITE_KING) {
			if (move.from==E1) {
				if (move.to==G1) { // castle king side
					removePiece(WHITE_ROOK,H1);
					putPiece(WHITE_ROOK,F1);
					backup.hasWhiteKingCastle=true;
				} else if (move.to==C1) { // castle queen side
					removePiece(WHITE_ROOK,A1);
					putPiece(WHITE_ROOK,D1);
					backup.hasWhiteQueenCastle=true;
				}
				removeCastleRights(WHITE,BOTH_SIDE_CASTLE);
			}
		} else if (fromPiece==BLACK_KING) {
			if (move.from==E8) {
				if (move.to==G8) { // castle king side
					removePiece(BLACK_ROOK,H8);
					putPiece(BLACK_ROOK,F8);
					backup.hasBlackKingCastle=true;
				} else if (move.to==C8) { // castle queen side
					removePiece(BLACK_ROOK,A8);
					putPiece(BLACK_ROOK,D8);
					backup.hasBlackQueenCastle=true;
				}
				removeCastleRights(BLACK,BOTH_SIDE_CASTLE);
			}
		}
	}

	if (fromPiece==makePiece(getSideToMove(),ROOK)) {
		if (getCastleRights(getSideToMove())!=NO_CASTLE) {
			if (getSideToMove()==WHITE) {
				if (move.from==A1) {
					removeCastleRights(WHITE,QUEEN_SIDE_CASTLE);
				} else if (move.from==H1) {
					removeCastleRights(WHITE,KING_SIDE_CASTLE);
				}
			} else {
				if (move.from==A8) {
					removeCastleRights(getSideToMove(),QUEEN_SIDE_CASTLE);
				} else if (move.from==H8) {
					removeCastleRights(getSideToMove(),KING_SIDE_CASTLE);
				}
			}
		}
	}

	int signal = getSideToMove()==WHITE?-1:+1;
	Rank doubleInitialRank = getSideToMove()==WHITE?RANK_2:RANK_7;
	Rank doubleFinalRank = getSideToMove()==WHITE?RANK_4:RANK_5;

	if (fromPiece==makePiece(getSideToMove(),PAWN)){
		if (getEnPassant()!=NONE) {
			if (getSquareFile(move.from)!=getSquareFile(move.to)&&toPiece==EMPTY) { // en passant
				Square capturedSquare=makeSquare(Rank(getSquareRank(move.to)+signal), getSquareFile(move.to));
				removePiece(makePiece(flipSide(getSideToMove()),PAWN),capturedSquare);
				backup.hasCapture=true;
				backup.capturedPiece=toPiece;
				backup.capturedSquare=capturedSquare;
			}
		}
		if (getSquareRank(move.to)==doubleFinalRank&&getSquareRank(move.from)==doubleInitialRank) {
			setEnPassant(move.to);
			enPassant=true;
		}
	}

	if (!enPassant) {
		if (getEnPassant()!=NONE)
		{
			setEnPassant(NONE);
		}
	}


	//TODO formulate better solution to Attacked Squares Table
	// the code below invalidates generated attackedSquares table control flag:
	// so in case some method needs attackedSquares, it will be regenerated.
	// This is done that way to avoid unnecessary calls to generateAttackedSquares - it is a performance killer
	// currently attackedSquares table is only used by canCastle() method
	backup.hasAttackedSquares=hasAttackedSquaresTable();
	setAttackedSquaresTable(false);

	setSideToMove(flipSide(getSideToMove()));

}

// undo a move based on struct MoveBackup
void Board::undoMove(MoveBackup& backup)
{

	PieceTypeByColor piece=currentBoard.square[backup.move.to];

	removePiece(piece,backup.move.to);

	if (!backup.hasPromotion) {
		putPiece(piece,backup.move.from);
	} else {
		if (pieceColor[piece]==WHITE) {
			putPiece(WHITE_PAWN,backup.move.from);
		} else {
			putPiece(BLACK_PAWN,backup.move.from);
		}
	}

	if (backup.hasCapture) {
		putPiece(backup.capturedPiece,backup.capturedSquare);
	}

	if (backup.hasWhiteKingCastle) {
		removePiece(WHITE_ROOK,F1);
		putPiece(WHITE_ROOK,H1);
	} else if (backup.hasWhiteQueenCastle) {
		removePiece(WHITE_ROOK,D1);
		putPiece(WHITE_ROOK,A1);
	} else if (backup.hasBlackKingCastle) {
		removePiece(BLACK_ROOK,F8);
		putPiece(BLACK_ROOK,H8);
	} else if (backup.hasBlackQueenCastle) {
		removePiece(BLACK_ROOK,D8);
		putPiece(BLACK_ROOK,A8);
	}

	setEnPassant(backup.enPassant);
	setCastleRights(WHITE, backup.whiteCastleRight);
	setCastleRights(BLACK, backup.blackCastleRight);

	setAttackedSquares(WHITE, backup.attackedSquares[WHITE]);
	setAttackedSquares(BLACK, backup.attackedSquares[BLACK]);
	setAttackedSquaresTable(backup.hasAttackedSquares);

	setSideToMove(flipSide(getSideToMove()));

}

// set initial classic position to the board
void Board::setInitialPosition()
{

	clearBoard();

	putPiece(WHITE_ROOK, A1);
	putPiece(WHITE_KNIGHT, B1);
	putPiece(WHITE_BISHOP, C1);
	putPiece(WHITE_QUEEN, D1);
	putPiece(WHITE_KING, E1);
	putPiece(WHITE_BISHOP, F1);
	putPiece(WHITE_KNIGHT, G1);
	putPiece(WHITE_ROOK, H1);

	putPiece(BLACK_ROOK, A8);
	putPiece(BLACK_KNIGHT, B8);
	putPiece(BLACK_BISHOP, C8);
	putPiece(BLACK_QUEEN, D8);
	putPiece(BLACK_KING, E8);
	putPiece(BLACK_BISHOP, F8);
	putPiece(BLACK_KNIGHT, G8);
	putPiece(BLACK_ROOK, H8);

	for(register int x=A2;x<=H2;x++){
		putPiece(WHITE_PAWN, Square(x));
	}

	for(register int x=A7;x<=H7;x++){
		putPiece(BLACK_PAWN, Square(x));
	}

	//initial side to move
	setSideToMove(WHITE);

	//initial castle rights
	setCastleRights(WHITE, BOTH_SIDE_CASTLE);
	setCastleRights(BLACK, BOTH_SIDE_CASTLE);

	//en passant
	setEnPassant(NONE);

}
// load an specific chess position ex.: d2d4 g8f6 c2c4 e7e6 g1f3 b7b6 b1c3 c8b7 ...
void Board::loadFromString(const std::string startPosMoves) {

	std::string moves = startPosMoves+" ";
	std::string moveFrom = "";
	std::string moveTo = "";
	PieceTypeByColor promotionPiece=EMPTY;

	StringUtil::normalizeString(moves);
	int last = 0;
	int position = moves.find(" ");
	MoveBackup backup;

	while ( position != std::string::npos )  {

		std::string move=moves.substr(last,(position-last));

		moveFrom = move.substr(0,2);
		moveTo = move.substr(2,2);

		if (move.length()>4) {
			if (move[4]=='q'){
				if (getSideToMove()==WHITE){
					promotionPiece=WHITE_QUEEN;
				} else {
					promotionPiece=BLACK_QUEEN;
				}
			} else if (move[4]=='r'){
				if (getSideToMove()==WHITE){
					promotionPiece=WHITE_ROOK;
				} else {
					promotionPiece=BLACK_ROOK;
				}
			} else if (move[4]=='n'){
				if (getSideToMove()==WHITE){
					promotionPiece=WHITE_KNIGHT;
				} else {
					promotionPiece=BLACK_KNIGHT;
				}
			} else if (move[4]=='b'){
				if (getSideToMove()==WHITE){
					promotionPiece=WHITE_BISHOP;
				} else {
					promotionPiece=BLACK_BISHOP;
				}
			}

			//std::cout << "PromotionPiece: " << move[4] << " - " << promotionPiece << std::endl;

		}

		//std::cout << "moveFrom: " << moveFrom << " - " << St2Sq(moveFrom[0],moveFrom[1]) << std::endl;
		//std::cout << "moveTo:   " << moveTo << " - " << St2Sq(moveTo[0],moveTo[1]) << std::endl << std::endl;

		this->doMove(Move(Square(St2Sq(moveFrom[0],moveFrom[1])), Square(St2Sq(moveTo[0],moveTo[1])), promotionPiece),backup);

		last=position+1;
		position = moves.find(" ", position+1);

	}

}

// generate only capture moves
Move* Board::generateCaptures(MovePool& movePool, const PieceColor side) {
	Move* move=NULL;
	PieceColor otherSide = flipSide(side);
	Bitboard pieces = this->getPiecesByColor(side)^this->getPiecesByType(makePiece(side,PAWN));
	Bitboard otherPieces = this->getPiecesByColor(otherSide);
	Bitboard attacks = EMPTY_BB;
	Square from = this->extractLSB(pieces);

	while ( from!=NONE ) {
		attacks = this->getAttacksFrom(from)&otherPieces;
		Square target = this->extractLSB(attacks);
		while ( target!=NONE ) {
			move = movePool.construct(Move(move,from,target,EMPTY));
			target = this->extractLSB(attacks);
		}
		from = this->extractLSB(pieces);
	}

	pieces = this->getPiecesByType(makePiece(side,PAWN));
	from = this->extractLSB(pieces);
	while ( from!=NONE ) {
		attacks = this->getAttacksFrom(from)&otherPieces;
		Square target = this->extractLSB(attacks);
		bool promotion=((getSquareRank(from)==RANK_7&&side==WHITE) || (getSquareRank(from)==RANK_2&&side==BLACK));
		while ( target!=NONE ) {
			if (promotion) {
				move = movePool.construct(Move(move,from,target,makePiece(side,QUEEN)));
				move = movePool.construct(Move(move,from,target,makePiece(side,ROOK)));
				move = movePool.construct(Move(move,from,target,makePiece(side,BISHOP)));
				move = movePool.construct(Move(move,from,target,makePiece(side,KNIGHT)));
			} else {
				move = movePool.construct(Move(move,from,target,EMPTY));
			}
			target = this->extractLSB(attacks);
		}
		from = this->extractLSB(pieces);
	}

	return move;
}

//generate only non capture moves
Move* Board::generateNonCaptures(MovePool& movePool, const PieceColor side){
	Move* move=NULL;
	PieceColor otherSide = flipSide(side);
	Bitboard pieces = this->getPiecesByColor(side)^this->getPiecesByType(makePiece(side,PAWN));
	Bitboard empty = this->getEmptySquares();
	Bitboard attacks = EMPTY_BB;

	Square from = this->extractLSB(pieces);
	while ( from!=NONE ) {
		attacks = this->getAttacksFrom(from)&empty;
		Square target = this->extractLSB(attacks);
		while ( target!=NONE ) {
			move = movePool.construct(Move(move,from,target,EMPTY));
			target = this->extractLSB(attacks);
		}
		from = this->extractLSB(pieces);
	}

	pieces = this->getPiecesByType(makePiece(side,PAWN));
	from = this->extractLSB(pieces);
	while ( from!=NONE ) {
		attacks = this->getAttacksFrom(from)&empty;
		Square target = this->extractLSB(attacks);
		bool promotion=((getSquareRank(from)==RANK_7&&side==WHITE) || (getSquareRank(from)==RANK_2&&side==BLACK));
		while ( target!=NONE ) {
			if (promotion) {
				move = movePool.construct(Move(move,from,target,makePiece(side,QUEEN)));
				move = movePool.construct(Move(move,from,target,makePiece(side,ROOK)));
				move = movePool.construct(Move(move,from,target,makePiece(side,BISHOP)));
				move = movePool.construct(Move(move,from,target,makePiece(side,KNIGHT)));
			} else {
				move = movePool.construct(Move(move,from,target,EMPTY));
			}
			target = this->extractLSB(attacks);
		}
		from = this->extractLSB(pieces);
	}

	if (canCastle(side, KING_SIDE_CASTLE)) {
		if (side==WHITE) {
			move = movePool.construct(Move(move,E1,G1,makePiece(side,KING)));
		} else {
			move = movePool.construct(Move(move,E8,G8,makePiece(side,KING)));
		}
	}

	if (canCastle(side, QUEEN_SIDE_CASTLE)) {
		if (side==WHITE) {
			move = movePool.construct(Move(move,E1,C1,makePiece(side,KING)));
		} else {
			move = movePool.construct(Move(move,E8,C8,makePiece(side,KING)));
		}
	}

	return move;
}

//generate check evasions: move to non attacked square / interpose king / capture cheking piece
Move* Board::generateCheckEvasions(MovePool& movePool, const PieceColor side) {
	//TODO work in progress
	return NULL;
}

//generate all moves - captures + noncaptures
Move* Board::generateAllMoves(MovePool& movePool, const PieceColor side) {

	Move* moves;

	if (isAttacked(side, KING)) {
		moves = generateCheckEvasions(movePool, side);
	} else {
		moves = generateCaptures(movePool, side);
		Move* nonCaptures = generateNonCaptures(movePool, side);
		Move* tmp = moves;
		if (moves) {
			while (tmp->next) {
				tmp = tmp->next;
			}
			tmp->next=nonCaptures;
		} else {
			moves = nonCaptures;
		}
	}

	return moves;
}







