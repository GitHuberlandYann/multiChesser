#include "Chess.hpp"
#include <iostream>

Chess::Chess( void )
	: _board(PIECES::board_init), _castle_state("KQkq"), _en_passant("-"), _half_moves(0), _full_moves(1), _turn(TURN_WHITE)
{
	_captured.fill(false);
}

Chess::~Chess( void )
{

}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

// set _captured array to true if piece can move to square
// then return whether piece can move further (used by rooks and bishops)
bool Chess::addCapture( int row, int col, bool empty_allowed, bool capture_allowed )
{
	if (row < 0 || row >= 8 || col < 0 || col >= 8) {
		return (false);
	}
	int offset = (row << 3) + col, piece = _board[offset];
	if ((piece == PIECES::EMPTY && empty_allowed) || (capture_allowed && ((_turn == TURN_WHITE) ? islower(piece) : isupper(piece)))) {
		_captured[offset] = true;
		return (piece == PIECES::EMPTY);
	}
	return (false);
}

void Chess::addKingCaptures( int index )
{
	int row = index >> 3, col = index & 0x7;
	for (int r = -1; r <= 1; ++r) {
		for (int c = -1; c <= 1; ++c) {
			addCapture(row + r, col + c);
		}
	}
}

void Chess::addKingLegalMoves( int index )
{
	addKingCaptures(index);
	char king_side = (_turn == TURN_WHITE) ? 'K' : 'k';
	if (_castle_state.find(king_side) != std::string::npos) {
		if (_board[index + 1] == PIECES::EMPTY && _board[index + 2] == PIECES::EMPTY) {
			_captured[index + 2] = true;
		}
	}
	char queen_side = (_turn == TURN_WHITE) ? 'Q' : 'q';
	if (_castle_state.find(queen_side) != std::string::npos) {
		if (_board[index - 1] == PIECES::EMPTY && _board[index - 2] == PIECES::EMPTY) {
			_captured[index - 2] = true;
		}
	}
}

void Chess::addQueenCaptures( int index )
{
	addRookCaptures(index);
	addBishopCaptures(index);
}

void Chess::addRookCaptures( int index )
{
	for (int row = (index >> 3) - 1; row >= 0; --row) {
		if (!addCapture(row, index & 0x7)) break ;
	}
	for (int row = (index >> 3) + 1; row < 8; ++row) {
		if (!addCapture(row, index & 0x7)) break ;
	}
	for (int col = (index & 0x7) - 1; col >= 0; --col) {
		if (!addCapture(index >> 3, col)) break ;
	}
	for (int col = (index & 0x7) + 1; col < 8; ++col) {
		if (!addCapture(index >> 3, col)) break ;
	}
}

void Chess::addBishopCaptures( int index )
{
	int rowi = index >> 3, coli = index & 0x7;
	for (int row = rowi - 1, col = coli - 1; row >= 0 && col >= 0; --row, --col) {
		if (!addCapture(row, col)) break ;
	}
	for (int row = rowi - 1, col = coli + 1; row >= 0 && col < 8; --row, ++col) {
		if (!addCapture(row, col)) break ;
	}
	for (int row = rowi + 1, col = coli - 1; row < 8 && col >= 0; ++row, --col) {
		if (!addCapture(row, col)) break ;
	}
	for (int row = rowi + 1, col = coli + 1; row < 8 && col < 8; ++row, ++col) {
		if (!addCapture(row, col)) break ;
	}
}

void Chess::addKnightCaptures( int index )
{
	int row = index >> 3, col = index & 0x7;
	addCapture(row - 1, col + 2);
	addCapture(row + 1, col + 2);
	addCapture(row - 2, col + 1);
	addCapture(row + 2, col + 1);
	addCapture(row - 2, col - 1);
	addCapture(row + 2, col - 1);
	addCapture(row - 1, col - 2);
	addCapture(row + 1, col - 2);
}

void Chess::addPawnCaptures( int index )
{
	int row = index >> 3, col = index & 0x7;
	addCapture(row + ((_turn == TURN_WHITE) ? -1 : 1), col - 1);
	addCapture(row + ((_turn == TURN_WHITE) ? -1 : 1), col + 1);
}

void Chess::addPawnLegalMoves( int index )
{
	int row = index >> 3, col = index & 0x7;
	bool front = addCapture(row + ((_turn == TURN_WHITE) ? -1 : 1), col, true, false); // 1 forward
	if (row == ((_turn == TURN_WHITE) ? 6 : 1)) {
		if (front) {
			addCapture(row + ((_turn == TURN_WHITE) ? -2 : 2), col, true, false); // 2 forward if first move
		}
	} else if (row == ((_turn == TURN_WHITE) ? 3 : 4)) { // en passant
		if (_en_passant == indexToStr(index + 1 + ((_turn == TURN_WHITE) ? -8 : 8))) {
			addCapture(row + ((_turn == TURN_WHITE) ? -1 : 1), col + 1);
		} else if (_en_passant == indexToStr(index - 1 + ((_turn == TURN_WHITE) ? -8 : 8))) {
			addCapture(row + ((_turn == TURN_WHITE) ? -1 : 1), col - 1);
		}
	}
	addCapture(row + ((_turn == TURN_WHITE) ? -1 : 1), col - 1, false); // diag, only if enemy piece there
	addCapture(row + ((_turn == TURN_WHITE) ? -1 : 1), col + 1, false);
}

// determines whether current board is a legal position, based on whose turn it is to play
// we do that by filling bitboard with captured cases and checking if king is in check
bool Chess::legalBoard( void )
{
	int index = -1, king_pos = -1;
	_captured.fill(false);
	_turn = (_turn == TURN_WHITE) ? TURN_BLACK : TURN_WHITE; // we tmp switch turn to simulate enemy's movement
	for (auto piece : _board) {
		++index;
		if ((_turn == TURN_WHITE) ? islower(piece) : isupper(piece)) {
			if (tolower(piece) == PIECES::KING) {
				king_pos = index;
			}
			continue ;
		}
		switch (tolower(piece)) {
			case PIECES::EMPTY:
				continue ;
			case PIECES::KING:
				addKingCaptures(index);
				break ;
			case PIECES::QUEEN:
				addQueenCaptures(index);
				break ;
			case PIECES::ROOK:
				addRookCaptures(index);
				break ;
			case PIECES::BISHOP:
				addBishopCaptures(index);
				break ;
			case PIECES::KNIGHT:
				addKnightCaptures(index);
				break ;
			case PIECES::PAWN:
				addPawnCaptures(index);
				break ;
		}
	}
	_turn = (_turn == TURN_WHITE) ? TURN_BLACK : TURN_WHITE;
	if (king_pos == -1) {
		std::cerr << "king missing from board" << std::endl;
		return (false);
	}
	std::cout << "king at " << indexToStr(king_pos) << std::endl << "cap: ";
	for (int i = 0; i < 64; ++i) {
		if (_captured[i]) std::cout << " " << indexToStr(i);
	}
	std::cout << std::endl;
	return (!_captured[king_pos]);
}

std::string Chess::indexToStr( int index )
{
	if (index < 0 || index >= 64) {
		return ("xx");
	}
	std::string res = "a0";
	res[0] = static_cast<char>((index & 0x7) + 'a');
	res[1] = static_cast<char>((7 - (index >> 3)) + '1');
	return (res);
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

int Chess::texIndex( char piece )
{
	int res = isupper(piece) ? 2 : 8;
	switch (tolower(piece)) {
		case PIECES::KING:
			return (res + 0);
		case PIECES::QUEEN:
			return (res + 1);
		case PIECES::ROOK:
			return (res + 2);
		case PIECES::BISHOP:
			return (res + 3);
		case PIECES::KNIGHT:
			return (res + 4);
		case PIECES::PAWN:
			return (res + 5);
	}
	return (0);
}

// Forsyth-Edwards Notation rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
std::string Chess::getFEN( void )
{
	std::string res;
	int cnt = 0;

	for (int i = 0; i < 64; ++i) {
		// std::cout << "in get board " << i << ": " << static_cast<int>(_board[i]) << std::endl;
		if (_board[i] == PIECES::EMPTY) {
			++cnt;
		} else {
			if (cnt) {
				res += static_cast<char>('0' + cnt);
				cnt = 0;
			}
			res += _board[i];
		}
		if ((i & 0x7) == 7) {
			if (cnt) {
				res += static_cast<char>('0' + cnt);
				cnt = 0;
			}
			if (i != 63) res += '/';
		}
	}
	res += ' ';
	res += _turn;
	res += ' ';
	res += _castle_state + ' ' + _en_passant + ' ' + std::to_string(_half_moves) + ' ' + std::to_string(_full_moves);
	return (res + '\n');
}

void Chess::setBoard( std::string fen )
{
	int i = 0, bi = 0;
	for (; fen[i] != ' '; ++i) {
		if (isdigit(fen[i])) {
			for (int j = 0; j < fen[i] - '0'; ++j) {
				_board[bi++] = PIECES::EMPTY;
			}
		} else if (fen[i] != '/') {
			_board[bi++] = fen[i];
		}
	}
	_turn = fen[i + 1];
	i += 3;
	_castle_state = "";
	for (; fen[i] != ' '; ++i) _castle_state += fen[i];
	++i;
	_en_passant = "";
	for (; fen[i] != ' '; ++i) _en_passant += fen[i];
	++i;
	_half_moves = 0;
	for (; fen[i] != ' '; ++i) _half_moves = _half_moves * 10 + fen[i] - '0';
	++i;
	_full_moves = 0;
	for (; fen[i] != '\n'; ++i) _full_moves = _full_moves * 10 + fen[i] - '0';
}

void Chess::setCaptures( int index )
{
	_captured.fill(false);
	if (index < 0 || index >= 64) return ;
	char piece = _board[index];
	_turn = (isupper(piece)) ? TURN_WHITE : TURN_BLACK;
	switch (tolower(piece)) {
		case PIECES::EMPTY:
			return ;
		case PIECES::KING:
			addKingLegalMoves(index);
			break ;
		case PIECES::QUEEN:
			addQueenCaptures(index);
			break ;
		case PIECES::ROOK:
			addRookCaptures(index);
			break ;
		case PIECES::BISHOP:
			addBishopCaptures(index);
			break ;
		case PIECES::KNIGHT:
			addKnightCaptures(index);
			break ;
		case PIECES::PAWN:
			addPawnLegalMoves(index);
			break ;
	}
}

void Chess::drawSquare( std::vector<int> &vertices, int type, int startX, int startY, int width, int height)
{
	// std::cout << "square with layer " << type << std::endl;
	vertices.push_back(0 + (0 << 1) + (type << 2));
	vertices.push_back(startX);
	vertices.push_back(startY);
	vertices.push_back(1 + (0 << 1) + (type << 2));
	vertices.push_back(startX + width);
	vertices.push_back(startY);
	vertices.push_back(0 + (1 << 1) + (type << 2));
	vertices.push_back(startX);
	vertices.push_back(startY + height);

	vertices.push_back(1 + (0 << 1) + (type << 2));
	vertices.push_back(startX + width);
	vertices.push_back(startY);
	vertices.push_back(1 + (1 << 1) + (type << 2));
	vertices.push_back(startX + width);
	vertices.push_back(startY + height);
	vertices.push_back(0 + (1 << 1) + (type << 2));
	vertices.push_back(startX);
	vertices.push_back(startY + height);
}

void Chess::drawBoard( std::vector<int> &vertices, int except, int startX, int startY, int width, int height )
{
	int square_width = width >> 3, square_height = height >> 3;

	for (int row = 0; row < 8; ++row) {
		for (int col = 0; col < 8; ++col) {
			drawSquare(vertices, !((row + col) & 0x1), startX + col * square_width, startY + row * square_height, square_width, square_height);
			// std::cout << "row " << row << ", col " << col << ": " << static_cast<int>(_board[(row << 3) + col]) << std::endl;
			if ((row << 3) + col == except) {
				continue ; // we skip square at index except
			}
			char piece = _board[(row << 3) + col];
			if (piece != PIECES::EMPTY) {
				drawSquare(vertices, texIndex(piece), startX + col * square_width, startY + row * square_height, square_width, square_height);
			}

			if (_captured[(row << 3) + col]) {
				drawSquare(vertices, (row + col) & 0x1, startX + col * square_width + square_width / 4, startY + row * square_height + square_height / 4, square_width / 2, square_height / 2);
			}
		}
	}
	// std::cout << "OVER\n\n" << std::endl;
}

// return {piece at, index of square} from mouse position on screen
std::array<int, 2> Chess::getSelectedSquare( double mouseX, double mouseY)
{
	int row = (mouseY - 30) / 30;
	if (row < 0 || row >= 8) {
		return {PIECES::EMPTY, -1};
	}
	int col = (mouseX - 30) / 30;
	if (col < 0 || col >= 8) {
		return {PIECES::EMPTY, -1};
	}
	return {_board[(row << 3) + col], (row << 3) + col};
}

void Chess::movePiece( int src, int dst )
{
	char piece = _board[src];
	std::cout << _turn << " move " << piece << " from " << indexToStr(src) << " to " << indexToStr(dst) << std::endl;
	if ((_turn == TURN_WHITE) ? !isupper(piece) : !islower(piece)) {
		std::cout << "not your turn to move" << std::endl;
		return ;
	}
	_captured.fill(false);
	switch (tolower(piece)) {
		case PIECES::EMPTY:
			return ;
		case PIECES::KING:
			addKingLegalMoves(src);
			break ;
		case PIECES::QUEEN:
			addQueenCaptures(src);
			break ;
		case PIECES::ROOK:
			addRookCaptures(src);
			break ;
		case PIECES::BISHOP:
			addBishopCaptures(src);
			break ;
		case PIECES::KNIGHT:
			addKnightCaptures(src);
			break ;
		case PIECES::PAWN:
			addPawnLegalMoves(src);
			break ;
	}

	if (!_captured[dst]) {
		std::cout << "piece can't move there, captured where";
		for (int i = 0; i < 64; ++i) {
			if (_captured[i]) std::cout << " " << indexToStr(i);
		}
		std::cout << std::endl;
		return ;
	}

	char tmp = _board[dst], tmpep = PIECES::EMPTY;
	_board[dst] = piece;
	_board[src] = PIECES::EMPTY;
	if (tolower(piece) == PIECES::PAWN && ((dst >> 3) == 7 || !(dst >> 3))) { // pawn promotion to a queen
		_board[dst] = piece - 'P' + 'Q';
	} else if (tolower(piece) == PIECES::PAWN && tmp == PIECES::EMPTY && (src >> 3) != (dst >> 3)) { // en passant
		tmpep = _board[dst + ((_turn == TURN_WHITE) ? 8 : -8)];
		_board[dst + ((_turn == TURN_WHITE) ? 8 : -8)] = PIECES::EMPTY;
	} else if (tolower(piece) == PIECES::KING && (src - dst == 2 || dst - src == 2)) { // castle
		_board[dst + ((dst > src) ? -1 : 1)] = piece - 'K' + 'R';
		_board[dst + ((dst > src) ? 1 : -2)] = PIECES::EMPTY;
	}
	if (!legalBoard() && !(tolower(piece) == PIECES::KING && (src - dst == 2 || dst - src == 2))) { // if invalid move, restore pos
		std::cout << "your king is in check" << std::endl;
		_board[src] = piece;
		_board[dst] = tmp;
		if (tmpep != PIECES::EMPTY) _board[dst + ((_turn == TURN_WHITE) ? 8 : -8)] = tmpep; // restore en passant
	} else if (tolower(piece) == PIECES::KING && (src - dst == 2 || dst - src == 2)
			&& (_captured[src] || _captured[dst + ((dst > src) ? -1 : 1)] || _captured[dst])) { // casting would go through a checked square
		std::cout << "casting would go through a checked square" << std::endl;
		_board[src] = piece;
		_board[dst] = tmp;
		_board[dst + ((dst > src) ? -1 : 1)] = PIECES::EMPTY;
		_board[dst + ((dst > src) ? 1 : -2)] = piece - 'K' + 'R';
	} else {
		std::cout << "legal" << std::endl;
		_turn = (_turn == TURN_WHITE) ? TURN_BLACK : TURN_WHITE;
		if (tolower(piece) != PIECES::PAWN && tmp == PIECES::EMPTY) { // incr half moves if not pawn move and not capture (used for 50 moves rule)
			++_half_moves;
		} else {
			_half_moves = 0;
		}
		_full_moves += _turn == TURN_WHITE;
		if (tolower(piece) == PIECES::PAWN && (dst - src == 16 || dst - src == -16)) { // set en passant square for next move
			_en_passant = indexToStr(dst + ((_turn == TURN_WHITE) ? -8 : 8));
		} else {
			_en_passant = "-";
		}
		if (tolower(piece) == PIECES::KING) { // rm castle privilieges
			std::string castle;
			for (auto c : _castle_state) {
				if (c == piece || c == piece - 'K' + 'Q');
				else castle += c;
			}
			_castle_state = castle;
			if (!_castle_state[0]) _castle_state = "-";
		} else if (tolower(piece) == PIECES::ROOK && (src == 0 || src == 7 || src == 63 || src == 56)) {
			char rm = piece - 'R' + (!(src & 0x7) ? 'Q' : 'K');
			std::cout << "rm " << rm << " from castle" << std::endl;
			std::string castle;
			for (auto c : _castle_state) {
				if (c == rm);
				else castle += c;
			}
			_castle_state = castle;
			if (!_castle_state[0]) _castle_state = "-";
		}
	}
}
