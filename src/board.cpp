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
 * Board.cpp
 *
 *  Created on: Feb 21, 2009
 *      Author: bhlangonijr
 */

#include "board.h"

using namespace BoardTypes;

NodeZobrist zobrist;

Board::Board() : currentBoard()
{
	setInitialPosition();

}

Board::Board(const Board& board) : currentBoard( Node(board.currentBoard) ) {

}

Board::~Board(){
}

// print board for debug
const void Board::printBoard() {
	printBoard("");
}

// print board for debug
const void Board::printBoard(const std::string pad) {

	std::string ranks[8];
	int j=0;

	for(int x=0;x<ALL_SQUARE;x++) {

		if (currentBoard.piece.array[currentBoard.square[x]]&squareToBitboard[x]) {
			ranks[j]+= " ";
			ranks[j]+= pieceChar[currentBoard.square[x]];
			ranks[j]+= " |";

		} else {
			ranks[j]+= " ";
			ranks[j]+= pieceChar[12];
			ranks[j]+= " |";
		}
		if ((x+1)%8==0) {
			j++;
		}
	}
	std::cout << pad << "__________________________________" << std::endl;
	for(int x=7;x>=0;x--) {
		std::cout << pad << (x+1) << "|" << ranks[x] << std::endl;
		std::cout << pad << "__________________________________" << std::endl;
	}

	std::cout << pad << "    a   b   c   d   e   f   g   h " << std::endl;

}

// testing method
void Board::genericTest() {
	//testing code

	printBoard();
	int start = getTickCount();
	PieceColor color = getSideToMove();
	int counter=0;
	for (int x=0;x<1000000;x++)
	{
		MoveIterator::Data moveData;

		MoveIterator moves(moveData);
		this->generateAllMoves(moves,color);

		moves.first();
		while (moves.hasNext()) {

			MoveIterator::Move move = moves.next();
			//std::cout << counter << " - " << move.toString() << std::endl;
			MoveBackup backup;
			doMove(move,backup);
			//printBoard();
			undoMove(backup);
			counter++;

		}

	}

	//Evaluator evaluator;
	//std::cout << "Eval:        " << evaluator.evaluate(this) << std::endl;
	std::cout << "sideToMove:  " << color << std::endl;
	std::cout << "Key inc:     " << getKey() << std::endl;
	std::cout << "Key gen:     " << generateKey() << std::endl;
	std::cout << "Time:        " << (this->getTickCount()-start) << std::endl;
	std::cout << "Perft:       " << (counter) << std::endl;
	std::cout << "MoveCounter: " << getMoveCounter() << std::endl;
	std::cout << "NPS:         " << (counter/((this->getTickCount()-start)/1000) ) << std::endl;


}

// do a move and set backup info into struct MoveBackup
void Board::doMove(const MoveIterator::Move& move, MoveBackup& backup){

	PieceTypeByColor fromPiece=this->getPieceBySquare(move.from);
	PieceTypeByColor toPiece=this->getPieceBySquare(move.to);
	PieceColor otherSide = flipSide(getSideToMove());
	bool enPassant=false;

	backup.key=getKey();
	backup.from=move.from;
	backup.to=move.to;
	backup.movingPiece=fromPiece;
	backup.enPassant=getEnPassant();
	backup.whiteCastleRight=getCastleRights(WHITE);
	backup.blackCastleRight=getCastleRights(BLACK);
	backup.hasWhiteKingCastle=false;
	backup.hasWhiteQueenCastle=false;
	backup.hasBlackKingCastle=false;
	backup.hasBlackQueenCastle=false;
	backup.halfMoveCounter =  getHalfMoveCounter();

	increaseHalfMoveCounter();
	removePiece(fromPiece,move.from);
	setKey(getKey()^zobrist.pieceSquare[fromPiece][move.from]);

	if (toPiece!=EMPTY) {
		removePiece(toPiece,move.to);
		setKey(getKey()^zobrist.pieceSquare[toPiece][move.to]);
		backup.hasCapture=true;
		backup.capturedPiece=toPiece;
		backup.capturedSquare=move.to;
		resetHalfMoveCounter();
	} else {
		backup.capturedPiece=EMPTY;
		backup.capturedSquare=NONE;
		backup.hasCapture=false;
	}

	if (move.promotionPiece==EMPTY) {
		putPiece(fromPiece,move.to);
		setKey(getKey()^zobrist.pieceSquare[fromPiece][move.to]);
		backup.hasPromotion=false;
	} else {
		putPiece(move.promotionPiece,move.to);
		setKey(getKey()^zobrist.pieceSquare[move.promotionPiece][move.to]);
		backup.hasPromotion=true;
	}

	setKey(getKey() ^ zobrist.castleRight[getZobristCastleIndex()]);

	if (!isCastleDone(getSideToMove()) && getCastleRights(getSideToMove())!=NO_CASTLE) {
		if (fromPiece==WHITE_KING) {
			if (move.from==E1) {
				if (move.to==G1) { // castle king side
					removePiece(WHITE_ROOK,H1);
					putPiece(WHITE_ROOK,F1);
					setKey(getKey()^zobrist.pieceSquare[WHITE_ROOK][H1]);
					setKey(getKey()^zobrist.pieceSquare[WHITE_ROOK][F1]);
					backup.hasWhiteKingCastle=true;
					currentBoard.castleDone[WHITE]=true;
				} else if (move.to==C1) { // castle queen side
					removePiece(WHITE_ROOK,A1);
					putPiece(WHITE_ROOK,D1);
					setKey(getKey()^zobrist.pieceSquare[WHITE_ROOK][A1]);
					setKey(getKey()^zobrist.pieceSquare[WHITE_ROOK][D1]);
					backup.hasWhiteQueenCastle=true;
					currentBoard.castleDone[WHITE]=true;
				}
			}
			removeCastleRights(WHITE,BOTH_SIDE_CASTLE);
		} else if (fromPiece==BLACK_KING) {
			if (move.from==E8) {
				if (move.to==G8) { // castle king side
					removePiece(BLACK_ROOK,H8);
					putPiece(BLACK_ROOK,F8);
					setKey(getKey()^zobrist.pieceSquare[BLACK_ROOK][H8]);
					setKey(getKey()^zobrist.pieceSquare[BLACK_ROOK][F8]);
					backup.hasBlackKingCastle=true;
					currentBoard.castleDone[BLACK]=true;
				} else if (move.to==C8) { // castle queen side
					removePiece(BLACK_ROOK,A8);
					putPiece(BLACK_ROOK,D8);
					setKey(getKey()^zobrist.pieceSquare[BLACK_ROOK][A8]);
					setKey(getKey()^zobrist.pieceSquare[BLACK_ROOK][D8]);
					backup.hasBlackQueenCastle=true;
					currentBoard.castleDone[BLACK]=true;
				}
			}
			removeCastleRights(BLACK,BOTH_SIDE_CASTLE);
		} else if (fromPiece==makePiece(getSideToMove(),ROOK)) {
			if (getSideToMove()==WHITE) {
				if (move.from==A1) {
					removeCastleRights(WHITE,QUEEN_SIDE_CASTLE);
				} else if (move.from==H1) {
					removeCastleRights(WHITE,KING_SIDE_CASTLE);
				}
			} else {
				if (move.from==A8) {
					removeCastleRights(BLACK,QUEEN_SIDE_CASTLE);
				} else if (move.from==H8) {
					removeCastleRights(BLACK,KING_SIDE_CASTLE);

				}
			}
		}
	}
	if (!isCastleDone(otherSide) && getCastleRights(otherSide)!=NO_CASTLE) {
		if (toPiece==makePiece(otherSide,ROOK)) {
			if (getSideToMove()==WHITE) {
				if (move.to==A8) {
					removeCastleRights(BLACK,QUEEN_SIDE_CASTLE);
				} else if (move.to==H8) {
					removeCastleRights(BLACK,KING_SIDE_CASTLE);
				}
			} else {
				if (move.to==A1) {
					removeCastleRights(WHITE,QUEEN_SIDE_CASTLE);
				} else if (move.to==H1) {
					removeCastleRights(WHITE,KING_SIDE_CASTLE);

				}
			}
		}
	}

	setKey(getKey() ^ zobrist.castleRight[getZobristCastleIndex()]);

	if (getEnPassant()!=NONE) {
		setKey(getKey()^zobrist.enPassant[getSquareFile(getEnPassant())]);
	}

	if (fromPiece==makePiece(getSideToMove(),PAWN)){
		resetHalfMoveCounter();
		if (getEnPassant()!=NONE) {
			if (getSquareFile(move.from)!=getSquareFile(move.to)&&toPiece==EMPTY) { // en passant
				removePiece(makePiece(otherSide,PAWN),getEnPassant());
				setKey(getKey()^zobrist.pieceSquare[makePiece(otherSide,PAWN)][getEnPassant()]);
				backup.hasCapture=true;
				backup.capturedPiece=makePiece(otherSide,PAWN);
				backup.capturedSquare=getEnPassant();
			}
		}

		Rank doubleInitialRank = getSideToMove()==WHITE?RANK_2:RANK_7;
		Rank doubleFinalRank = getSideToMove()==WHITE?RANK_4:RANK_5;

		if (getSquareRank(move.to)==doubleFinalRank&&getSquareRank(move.from)==doubleInitialRank) {
			setEnPassant(move.to);
			enPassant=true;
		}
	}

	if (getEnPassant()!=NONE) {
		setKey(getKey()^zobrist.enPassant[getSquareFile(getEnPassant())]);
	}

	if (!enPassant) {
		if (getEnPassant()!=NONE)
		{
			setEnPassant(NONE);
		}
	}

	increaseMoveCounter();
	updateKeyHistory();

	setKey(getKey()^zobrist.sideToMove);
	setSideToMove(otherSide);

}

// do a null move and set backup info into struct MoveBackup
void Board::doNullMove(MoveBackup& backup){

	PieceColor otherSide = flipSide(getSideToMove());

	backup.key=getKey();
	backup.enPassant=getEnPassant();
	backup.whiteCastleRight=getCastleRights(WHITE);
	backup.blackCastleRight=getCastleRights(BLACK);
	backup.hasWhiteKingCastle=false;
	backup.hasWhiteQueenCastle=false;
	backup.hasBlackKingCastle=false;
	backup.hasBlackQueenCastle=false;

	if (getEnPassant()!=NONE) {
		setKey(getKey()^zobrist.enPassant[getSquareFile(getEnPassant())]);
		setEnPassant(NONE);
	}

	increaseMoveCounter();
	updateKeyHistory();

	setKey(getKey()^zobrist.sideToMove);
	setSideToMove(otherSide);


}

// undo a move based on struct MoveBackup
void Board::undoMove(MoveBackup& backup){

	PieceTypeByColor piece=backup.movingPiece;
	PieceColor sideToMove=flipSide(getSideToMove());
	removePiece(currentBoard.square[backup.to],backup.to);
	putPiece(piece,backup.from);
	currentBoard.halfMoveCounter = backup.halfMoveCounter;

	if (backup.hasCapture) {
		putPiece(backup.capturedPiece,backup.capturedSquare);
	}
	if (backup.hasWhiteKingCastle) {
		removePiece(WHITE_ROOK,F1);
		putPiece(WHITE_ROOK,H1);
		currentBoard.castleDone[WHITE]=false;
	} else if (backup.hasWhiteQueenCastle) {
		removePiece(WHITE_ROOK,D1);
		putPiece(WHITE_ROOK,A1);
		currentBoard.castleDone[WHITE]=false;
	} else if (backup.hasBlackKingCastle) {
		removePiece(BLACK_ROOK,F8);
		putPiece(BLACK_ROOK,H8);
		currentBoard.castleDone[BLACK]=false;
	} else if (backup.hasBlackQueenCastle) {
		removePiece(BLACK_ROOK,D8);
		putPiece(BLACK_ROOK,A8);
		currentBoard.castleDone[BLACK]=false;
	}

	setCastleRights(WHITE, backup.whiteCastleRight);
	setCastleRights(BLACK, backup.blackCastleRight);
	setEnPassant(backup.enPassant);
	decreaseMoveCounter();
	setSideToMove(sideToMove);
	setKey(backup.key);

}

// undo a null move based on struct MoveBackup
void Board::undoNullMove(MoveBackup& backup){

	PieceColor sideToMove=flipSide(getSideToMove());
	setCastleRights(WHITE, backup.whiteCastleRight);
	setCastleRights(BLACK, backup.blackCastleRight);
	setEnPassant(backup.enPassant);
	decreaseMoveCounter();
	setSideToMove(sideToMove);
	setKey(backup.key);

}

// set initial classic position to the board
void Board::setInitialPosition() {

	this->loadFromFEN(startFENPosition);

}
// load an specific chess position ex.: d2d4 g8f6 c2c4 e7e6 g1f3 b7b6 b1c3 c8b7 ...
void Board::loadFromString(const std::string startPosMoves) {

	std::string moves = startPosMoves+" ";
	std::string moveFrom = "";
	std::string moveTo = "";
	PieceTypeByColor promotionPiece=EMPTY;

	setInitialPosition();

	StringUtil::normalizeString(moves);
	size_t last = 0;
	size_t position = moves.find(" ");
	MoveBackup backup;

	while ( position != std::string::npos )  {

		std::string move=moves.substr(last,(position-last));

		moveFrom = move.substr(0,2);
		moveTo = move.substr(2,2);

		if (move.length()>4) {
			if (move[4]=='q'){
				promotionPiece=makePiece(getSideToMove(),QUEEN);
			} else if (move[4]=='r'){
				promotionPiece=makePiece(getSideToMove(),ROOK);
			} else if (move[4]=='n'){
				promotionPiece=makePiece(getSideToMove(),KNIGHT);
			} else if (move[4]=='b'){
				promotionPiece=makePiece(getSideToMove(),BISHOP);
			}
		}

		this->doMove(MoveIterator::Move(Square(St2Sq(moveFrom[0],moveFrom[1])), Square(St2Sq(moveTo[0],moveTo[1])), promotionPiece),backup);
		promotionPiece=EMPTY;
		last=position+1;
		position = moves.find(" ", position+1);
	}

}

// load an specific chess position using FEN notation ex.: rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1
void Board::loadFromFEN(const std::string startFENMoves) {

	std::string ranks = startFENMoves.substr(0,startFENMoves.find(" "))+"/";
	std::string states = StringUtil::getMiddleString(startFENMoves," ");

	clearBoard();

	size_t last = 0;
	size_t position = ranks.find("/");
	size_t square=63;

	//process piece positions
	while ( position != std::string::npos )  {
		std::string rank=ranks.substr(last,(position-last));
		size_t length = rank.length()-1;

		for (int idx=length;idx>=0;idx--) {

			if (std::isalpha(rank[idx])) {
				this->putPiece(encodePieceChar(rank[idx]), Square(square));
				square--;
			} else if (std::isdigit(rank[idx])) {
				size_t count = StringUtil::toInt(rank[idx]);
				square -= count;
			}

		}

		last=position+1;
		position = ranks.find("/", position+1);
	}

	// process board states
	// side to move
	if (states[0]=='w') {
		this->setSideToMove(WHITE);
	} else {
		this->setSideToMove(BLACK);
	}

	std::string tokens = StringUtil::getMiddleString(states, " ");
	std::string castleRights = tokens.substr(0,tokens.find(" "));

	// white castle rights
	if (castleRights.find("KQ")!=std::string::npos) {
		this->setCastleRights(WHITE, BOTH_SIDE_CASTLE);
	} else {
		if (castleRights.find("K")!=std::string::npos) {
			this->setCastleRights(WHITE, KING_SIDE_CASTLE);
		} else if (castleRights.find("Q")!=std::string::npos) {
			this->setCastleRights(WHITE, QUEEN_SIDE_CASTLE);
		} else {
			this->setCastleRights(WHITE, NO_CASTLE);
		}
	}
	// black castle rights
	if (castleRights.find("kq")!=std::string::npos) {
		this->setCastleRights(BLACK, BOTH_SIDE_CASTLE);
	} else {
		if (castleRights.find("k")!=std::string::npos) {
			this->setCastleRights(BLACK, KING_SIDE_CASTLE);
		} else if (castleRights.find("q")!=std::string::npos) {
			this->setCastleRights(BLACK, QUEEN_SIDE_CASTLE);
		} else {
			this->setCastleRights(BLACK, NO_CASTLE);
		}
	}
	// en passant
	tokens = StringUtil::getMiddleString(tokens, " ");
	std::string enPassant = tokens.substr(0,tokens.find(" "));

	if (enPassant[0]=='-') {
		setEnPassant(NONE);
	} else {
		setEnPassant(Square(St2Sq(enPassant[0],enPassant[1])));
	}

	//halfmove clock
	tokens = StringUtil::getMiddleString(tokens, " ");
	std::string halfMove = tokens.substr(0,tokens.find(" "));

	this->currentBoard.halfMoveCounter = StringUtil::toInt(halfMove);

	//fullmove counter
	tokens = StringUtil::getMiddleString(tokens, " ");
	this->currentBoard.moveCounter = StringUtil::toInt(tokens);

	setKey(generateKey());
	updateKeyHistory();

}

// get FEN from current board
const std::string Board::getFEN() {

	std::string fen="";
	size_t count=0;
	size_t rank=1;
	for(size_t sq=A1;sq<=H8;sq++) {

		PieceTypeByColor piece = this->getPieceBySquare(Square(sq));
		if (piece!=EMPTY) {
			if (count>0) {
				fen+=StringUtil::toStr(count);
			}
			fen+=pieceChar[piece];
			count=0;
		} else {
			count++;
		}
		if ((sq+1)%8==0) {
			if (count>0) {
				fen+=StringUtil::toStr(count);
				count=0;
			}
			if (rank<8) {
				fen+="/";
			}
			rank++;
		}
	}

	fen+=this->getSideToMove()==WHITE?" w":" b";

	std::string castleRights = "";

	if (this->getCastleRights(BLACK)==BOTH_SIDE_CASTLE) {
		castleRights+="kq";
	} else if (this->getCastleRights(BLACK)==KING_SIDE_CASTLE) {
		castleRights+="k";
	} if (this->getCastleRights(BLACK)==QUEEN_SIDE_CASTLE) {
		castleRights+="q";
	}

	if (this->getCastleRights(WHITE)==BOTH_SIDE_CASTLE) {
		castleRights+="KQ";
	} else if (this->getCastleRights(WHITE)==KING_SIDE_CASTLE) {
		castleRights+="K";
	} if (this->getCastleRights(WHITE)==QUEEN_SIDE_CASTLE) {
		castleRights+="Q";
	}

	castleRights = castleRights.length()==0?"-":castleRights;

	fen+=" "+castleRights;

	fen+=this->getEnPassant()==NONE?" -":" "+squareToString[this->getEnPassant()];

	fen+=" "+StringUtil::toStr(this->getHalfMoveCounter());

	fen+=" "+StringUtil::toStr(this->getMoveCounter());

	return fen;
}

// initialize zobrist keys
void Board::initializeZobrist() {
	for(int piece=0; piece<ALL_PIECE_TYPE_BY_COLOR; piece++) {
		for(int square=0; square<ALL_SQUARE; square++) {
			zobrist.pieceSquare[piece][square]=genrand_int64();
		}
	}
	zobrist.sideToMove=genrand_int64();
	for(int file=0; file<ALL_FILE; file++) {
		zobrist.enPassant[file]=genrand_int64();
	}
	for(int castle=0; castle<ALL_CASTLE_RIGHT*ALL_CASTLE_RIGHT; castle++) {
		zobrist.castleRight[castle]=genrand_int64();
	}
}

// get index for zobrist index
const int Board::getZobristCastleIndex() {
	return zobristCastleIndex[getCastleRights(WHITE)][getCastleRights(BLACK)];
}


// get board zobrist key
const Key Board::getKey() const {
	return currentBoard.key;
}

// set board zobrist key
void Board::setKey(Key _key) {
	currentBoard.key = _key;
}

// generate board zobrist key
const Key Board::generateKey() {
	Key key=Key(0x0ULL);

	for(int square=A1; square<=H8; square++) {
		if (currentBoard.square[square]!=EMPTY) {
			key ^= zobrist.pieceSquare[currentBoard.square[square]][square];
		}
	}
	key ^= zobrist.castleRight[getZobristCastleIndex()];

	if (currentBoard.enPassant != NONE) {
		key ^= zobrist.enPassant[getSquareFile(currentBoard.enPassant)];
	}
	if (getSideToMove()==BLACK) {
		key ^= zobrist.sideToMove;
	}

	return key;
}





