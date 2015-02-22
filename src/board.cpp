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
 * Board.cpp
 *
 *  Created on: Feb 21, 2009
 *      Author: bhlangonijr
 */
#include "board.h"
using namespace BoardTypes;
static NodeZobrist zobrist;
static int pst[maxGamePhase+1][ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE];

Board::Board() : currentBoard() {
	setInitialPosition();
}

Board::Board(const Board& board) : currentBoard( Node(board.currentBoard) ) {

}

Board::~Board() {
}
// print board for debug
const void Board::printBoard() {
	printBoard("");
}
// print board for debug
const void Board::printBoard(const std::string pad) {
	std::string ranks[8];
	int j=0;
	for(int x=0;x<ALL_SQUARE-1;x++) {
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
// do a move and set backup info into struct MoveBackup
void Board::doMove(const MoveIterator::Move& move, MoveBackup& backup){
	const PieceTypeByColor fromPiece = getPiece(move.from);
	const PieceTypeByColor toPiece = getPiece(move.to);
	const PieceColor otherSide = flipSide(getSideToMove());
	bool enPassant=false;
	bool reversible=true;
	backup.key=getKey();
	backup.pawnKey=getPawnKey();
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
	backup.phase=getGamePhase();
	backup.halfMoveCounter =  getHalfMoveCounter();
	backup.halfNullMoveCounter = getHalfNullMoveCounter();
	backup.inCheck = isInCheck();
	removePiece(fromPiece,move.from);
	setKey(getKey()^zobrist.pieceSquare[fromPiece][move.from]);
	if (toPiece!=EMPTY) {
		removePiece(toPiece,move.to);
		setKey(getKey()^zobrist.pieceSquare[toPiece][move.to]);
		backup.hasCapture=true;
		backup.capturedPiece=toPiece;
		backup.capturedSquare=move.to;
		reversible=false;
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
					reversible=false;
				} else if (move.to==C1) { // castle queen side
					removePiece(WHITE_ROOK,A1);
					putPiece(WHITE_ROOK,D1);
					setKey(getKey()^zobrist.pieceSquare[WHITE_ROOK][A1]);
					setKey(getKey()^zobrist.pieceSquare[WHITE_ROOK][D1]);
					backup.hasWhiteQueenCastle=true;
					currentBoard.castleDone[WHITE]=true;
					reversible=false;
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
					reversible=false;
				} else if (move.to==C8) { // castle queen side
					removePiece(BLACK_ROOK,A8);
					putPiece(BLACK_ROOK,D8);
					setKey(getKey()^zobrist.pieceSquare[BLACK_ROOK][A8]);
					setKey(getKey()^zobrist.pieceSquare[BLACK_ROOK][D8]);
					backup.hasBlackQueenCastle=true;
					currentBoard.castleDone[BLACK]=true;
					reversible=false;
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
		reversible=false;
		setPawnKey(getPawnKey()^zobrist.pieceSquare[fromPiece][move.from]);
		if (getEnPassant()!=NONE) {
			if (getSquareFile(move.from)!=getSquareFile(move.to)&&toPiece==EMPTY) { // en passant
				removePiece(makePiece(otherSide,PAWN),getEnPassant());
				setKey(getKey()^zobrist.pieceSquare[makePiece(otherSide,PAWN)][getEnPassant()]);
				backup.hasCapture=true;
				backup.capturedPiece=makePiece(otherSide,PAWN);
				backup.capturedSquare=getEnPassant();
			}
		}
		const Rank doubleInitialRank = getSideToMove()==WHITE?RANK_2:RANK_7;
		const Rank doubleFinalRank = getSideToMove()==WHITE?RANK_4:RANK_5;
		if (getSquareRank(move.to)==doubleFinalRank&&getSquareRank(move.from)==doubleInitialRank) {
			setEnPassant(move.to);
			enPassant=true;
		}
	}
	if (!enPassant && getEnPassant()!=NONE) {
		setEnPassant(NONE);
	}
	if (getEnPassant()!=NONE) {
		setKey(getKey()^zobrist.enPassant[getSquareFile(getEnPassant())]);
	}
	if (reversible) {
		increaseHalfMoveCounter();
		increaseHalfNullMoveCounter();
	} else {
		resetHalfMoveCounter();
		resetHalfNullMoveCounter();
	}
	if (backup.capturedPiece!=EMPTY) {
		if (pieceType[backup.capturedPiece]==PAWN) {
			setPawnKey(getPawnKey()^zobrist.pieceSquare[backup.capturedPiece][backup.capturedSquare]);
		}
		setGamePhase(GamePhase(currentBoard.gamePhase+
				phaseIncrement[getPieceType(backup.capturedPiece)]));
	}
	if (backup.hasPromotion) {
		setGamePhase(GamePhase(currentBoard.gamePhase-
				phaseIncrement[getPieceType(move.promotionPiece)]));
	} else {
		setPawnKey(getPawnKey()^zobrist.pieceSquare[fromPiece][move.to]);
	}
	setKey(getKey()^zobrist.sideToMove[getSideToMove()]);
	setSideToMove(otherSide);
	setKey(getKey()^zobrist.sideToMove[otherSide]);
	increaseMoveCounter();
	updateKeyHistory();

}
// do a null move and set backup info into struct MoveBackup
void Board::doNullMove(MoveBackup& backup){
	PieceColor otherSide = flipSide(getSideToMove());
	backup.key=getKey();
	backup.pawnKey=getPawnKey();
	backup.enPassant=getEnPassant();
	backup.whiteCastleRight=getCastleRights(WHITE);
	backup.blackCastleRight=getCastleRights(BLACK);
	backup.hasWhiteKingCastle=false;
	backup.hasWhiteQueenCastle=false;
	backup.hasBlackKingCastle=false;
	backup.hasBlackQueenCastle=false;
	backup.phase=getGamePhase();
	backup.halfMoveCounter =  getHalfMoveCounter();
	backup.halfNullMoveCounter =  getHalfNullMoveCounter();

	increaseHalfMoveCounter();
	resetHalfNullMoveCounter();

	if (getEnPassant()!=NONE) {
		setKey(getKey()^zobrist.enPassant[getSquareFile(getEnPassant())]);
		setEnPassant(NONE);
	}

	setKey(getKey()^zobrist.sideToMove[getSideToMove()]);
	setSideToMove(otherSide);
	setKey(getKey()^zobrist.sideToMove[otherSide]);
/*	increaseMoveCounter();
	updateKeyHistory();*/

}
// undo a move based on struct MoveBackup
void Board::undoMove(MoveBackup& backup){
	PieceTypeByColor piece=backup.movingPiece;
	PieceColor sideToMove=flipSide(getSideToMove());
	removePiece(currentBoard.square[backup.to],backup.to);
	putPiece(piece,backup.from);
	currentBoard.halfMoveCounter = backup.halfMoveCounter;
	currentBoard.halfNullMoveCounter = backup.halfNullMoveCounter;
	currentBoard.gamePhase=backup.phase;
	currentBoard.inCheck=backup.inCheck;
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
	setPawnKey(backup.pawnKey);
	setKey(backup.key);
}
// undo a null move based on struct MoveBackup
void Board::undoNullMove(MoveBackup& backup){
	PieceColor sideToMove=flipSide(getSideToMove());
	setCastleRights(WHITE, backup.whiteCastleRight);
	setCastleRights(BLACK, backup.blackCastleRight);
	setEnPassant(backup.enPassant);
	currentBoard.halfMoveCounter = backup.halfMoveCounter;
	currentBoard.halfNullMoveCounter = backup.halfNullMoveCounter;
	currentBoard.gamePhase=backup.phase;
	//decreaseMoveCounter();
	setSideToMove(sideToMove);
	setPawnKey(backup.pawnKey);
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
	StringUtil::normalizeString(moves);
	size_t last = 0;
	size_t position = moves.find(" ");
	MoveBackup backup;
	while ( position != std::string::npos )  {
		std::string move=moves.substr(last,(position-last));
		moveFrom = move.substr(0,2);
		moveTo = move.substr(2,2);
		promotionPiece=EMPTY;
		if (move.length()>4) {
			if (move[4]=='Q' || move[4]=='q'){
				promotionPiece=makePiece(getSideToMove(),QUEEN);
			} else if (move[4]=='R' || move[4]=='r'){
				promotionPiece=makePiece(getSideToMove(),ROOK);
			} else if (move[4]=='N' || move[4]=='n'){
				promotionPiece=makePiece(getSideToMove(),KNIGHT);
			} else if (move[4]=='B' || move[4]=='b'){
				promotionPiece=makePiece(getSideToMove(),BISHOP);
			}
		}
		this->doMove(MoveIterator::Move(Square(St2Sq(moveFrom[0],moveFrom[1])),
				Square(St2Sq(moveTo[0],moveTo[1])), promotionPiece),backup);
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
	if (castleRights.find("K")!=std::string::npos && castleRights.find("Q")!=std::string::npos) {
		this->setCastleRights(WHITE, BOTH_SIDE_CASTLE);
	} else if (castleRights.find("K")!=std::string::npos) {
		this->setCastleRights(WHITE, KING_SIDE_CASTLE);
	} else if (castleRights.find("Q")!=std::string::npos) {
		this->setCastleRights(WHITE, QUEEN_SIDE_CASTLE);
	} else {
		this->setCastleRights(WHITE, NO_CASTLE);
	}
	// black castle rights
	if (castleRights.find("k")!=std::string::npos && castleRights.find("q")!=std::string::npos) {
		this->setCastleRights(BLACK, BOTH_SIDE_CASTLE);
	} else if (castleRights.find("k")!=std::string::npos) {
		this->setCastleRights(BLACK, KING_SIDE_CASTLE);
	} else if (castleRights.find("q")!=std::string::npos) {
		this->setCastleRights(BLACK, QUEEN_SIDE_CASTLE);
	} else {
		this->setCastleRights(BLACK, NO_CASTLE);
	}
	// en passant
	tokens = StringUtil::getMiddleString(tokens, " ");
	std::string enPassant = tokens.substr(0,tokens.find(" "));
	if (enPassant[0]=='-') {
		setEnPassant(NONE);
	} else {
		const int square=St2Sq(enPassant[0],enPassant[1]);
		setEnPassant(this->getSideToMove()==WHITE?Square(square-8):Square(square+8));
	}
	//halfmove clock
	tokens = StringUtil::getMiddleString(tokens, " ");
	std::string halfMove = tokens.substr(0,tokens.find(" "));
	this->currentBoard.halfMoveCounter = StringUtil::toInt(halfMove);
	//fullmove counter
	tokens = StringUtil::getMiddleString(tokens, " ");
	this->currentBoard.moveCounter = StringUtil::toInt(tokens);
	setKey(generateKey());
	setPawnKey(generatePawnKey());
	updateKeyHistory();
	setGamePhase(predictGamePhase());
}
// get FEN from current board
const std::string Board::getFEN() {
	std::string fen="";
	int count=0;
	int rankCounter=1;
	int sqCount=0;
	for (int rank=RANK_8;rank>=RANK_1;rank--) {
		for (int file=FILE_A;file<=FILE_H;file++) {
			PieceTypeByColor piece = getPiece(makeSquare(Rank(rank),File(file)));
			if (piece!=EMPTY) {
				if (count>0) {
					fen+=StringUtil::toStr(count);
				}
				fen+=pieceChar[piece];
				count=0;
			} else {
				count++;
			}
			if ((sqCount+1)%8==0) {
				if (count>0) {
					fen+=StringUtil::toStr(count);
					count=0;
				}
				if (rankCounter<8) {
					fen+="/";
				}
				rankCounter++;
			}
			sqCount++;
		}
	}
	fen+=this->getSideToMove()==WHITE?" w":" b";
	std::string castleRights = "";
	if (this->getCastleRights(WHITE)==BOTH_SIDE_CASTLE) {
		castleRights+="KQ";
	} else if (this->getCastleRights(WHITE)==KING_SIDE_CASTLE) {
		castleRights+="K";
	} if (this->getCastleRights(WHITE)==QUEEN_SIDE_CASTLE) {
		castleRights+="Q";
	}
	if (this->getCastleRights(BLACK)==BOTH_SIDE_CASTLE) {
		castleRights+="kq";
	} else if (this->getCastleRights(BLACK)==KING_SIDE_CASTLE) {
		castleRights+="k";
	} if (this->getCastleRights(BLACK)==QUEEN_SIDE_CASTLE) {
		castleRights+="q";
	}
	castleRights = castleRights.length()==0?"-":castleRights;
	fen+=" "+castleRights;
	fen+=this->getEnPassant()==NONE?" -":" "+squareToString[this->getEnPassant()];
	fen+=" "+StringUtil::toStr(this->getHalfMoveCounter());
	fen+=" "+StringUtil::toStr(this->getMoveCounter());
	return fen;
}

void Board::initialize() {
	// initialize zobrist keys
	for(int piece=0; piece<ALL_PIECE_TYPE_BY_COLOR; piece++) {
		for(int square=0; square<ALL_SQUARE; square++) {
			zobrist.pieceSquare[piece][square]=genRandInt64();
		}
	}
	zobrist.sideToMove[WHITE]=genRandInt64();
	zobrist.sideToMove[BLACK]=genRandInt64();
	zobrist.sideToMove[COLOR_NONE]=0x0;
	for(int file=0; file<ALL_FILE; file++) {
		zobrist.enPassant[file]=genRandInt64();
	}
	for(int castle=0; castle<ALL_CASTLE_RIGHT*ALL_CASTLE_RIGHT; castle++) {
		zobrist.castleRight[castle]=genRandInt64();
	}
	zobrist.ignoreMove=genRandInt64();
	// initialize incremental pst
	for (int phase=0; phase<=maxGamePhase; phase++) {
		for (int piece=0; piece<ALL_PIECE_TYPE_BY_COLOR; piece++) {
			for (int square=0; square<ALL_SQUARE; square++) {
				pst[phase][piece][square]=
						calcPieceSquareValue(PieceTypeByColor(piece),(Square)(square),(GamePhase)(phase));
			}
		}
	}
	// initialize pst
	for (int piece=0; piece<ALL_PIECE_TYPE_BY_COLOR; piece++) {
		for (int square=0; square<ALL_SQUARE; square++) {
			const Square sq = pieceColor[piece]==WHITE?(Square)(square):flip[square];
			fullPst[piece][square]=pieceSquareTable[pieceType[piece]][sq];
		}
	}
}
// game phase full calculation
const GamePhase Board::predictGamePhase() {
	const int pawns = getPieceCount(WHITE_PAWN)+getPieceCount(BLACK_PAWN);
	const int knights = getPieceCount(WHITE_KNIGHT)+getPieceCount(BLACK_KNIGHT);
	const int bishops = getPieceCount(WHITE_BISHOP)+getPieceCount(BLACK_BISHOP);
	const int rooks = getPieceCount(WHITE_ROOK)+getPieceCount(BLACK_ROOK);
	const int queens = getPieceCount(WHITE_QUEEN)+getPieceCount(BLACK_QUEEN);
	return GamePhase(
			(16-pawns)*phaseIncrement[PAWN]+
			(4-knights)*phaseIncrement[KNIGHT]+
			(4-bishops)*phaseIncrement[BISHOP]+
			(4-rooks)*phaseIncrement[ROOK]+
			(2-queens)*phaseIncrement[QUEEN]);
}
// get index for zobrist index
const int Board::getZobristCastleIndex() {
	return zobristCastleIndex[getCastleRights(WHITE)][getCastleRights(BLACK)];
}
// get board zobrist key
const Key Board::getKey() const {
	return currentBoard.key;
}
// get zobrist key for partial searches
const Key Board::getPartialSearchKey() const {
	return currentBoard.key ^ zobrist.ignoreMove;
}
// get pawn zobrist key
const Key Board::getPawnKey() const {
	return currentBoard.pawnKey;
}
// set board zobrist key
void Board::setKey(Key _key) {
	currentBoard.key = _key;
}
// set pawn zobrist key
void Board::setPawnKey(Key _key) {
	currentBoard.pawnKey = _key;
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
	key ^= zobrist.sideToMove[getSideToMove()];
	return key;
}
// generate pawn zobrist key
const Key Board::generatePawnKey() {
	Key key=Key(0x0ULL);
	for(int square=A1; square<=H8; square++) {
		if (pieceType[currentBoard.square[square]]==PAWN) {
			key ^= zobrist.pieceSquare[currentBoard.square[square]][square];
		}
	}
	return key;
}

const int Board::getPieceSquareValue(const PieceTypeByColor piece, const Square square, GamePhase phase) {
	return pst[phase][piece][square];
}

const int Board::interpolate(const int first, const int second, const int position) {
	return (first*position)/maxGamePhase+(second*(maxGamePhase-position))/maxGamePhase;
}

const int Board::calcPieceSquareValue(const PieceTypeByColor piece, const Square square, GamePhase phase) {
	const Square sq = pieceColor[piece]==WHITE?square:flip[square];
	const int egValue = lowerScore(pieceSquareTable[pieceType[piece]][sq]);
	const int mgValue = upperScore(pieceSquareTable[pieceType[piece]][sq]);
	return interpolate(egValue,mgValue,phase);
}
