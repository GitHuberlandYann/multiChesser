#include "Chess.hpp"
#include <iostream>

Chess::Chess( void )
	: _board(PIECES::board_init), _turn(TURN_WHITE)
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
bool Chess::addCapture( int row, int col )
{
	if (row < 0 || row >= 8 || col < 0 || col >= 8) {
		return (false);
	}
	int offset = (row << 3) + col, piece = _board[offset];
	if (piece == PIECES::EMPTY || (piece & PIECES::WHITE) != _turn * PIECES::WHITE) {
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
	// int row = index >> 3, col = index & 0x7;
	// for (int diag = -7; diag < 7; ++diag) {
	// 	addCapture(row + diag, col + diag);
	// 	addCapture(row + diag, col - diag);
	// }
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
	addCapture(row + ((_turn == TURN_WHITE) ? 1 : -1), col - 1);
	addCapture(row + ((_turn == TURN_WHITE) ? 1 : -1), col + 1);
}

// determines whether current board is a legal position, based on whose turn it is to play
// we do that by filling bitboard with captured cases and checking if king is in check
bool Chess::legalBoard( void )
{
	int index = -1, king_pos = -1;
	_captured.fill(false);
	_turn = !_turn; // we tmp switch turn to simulate enemy's movement
	for (auto piece : _board) {
		++index;
		if ((piece & PIECES::WHITE) != _turn * PIECES::WHITE) {
			if ((piece & 0x7) == PIECES::KING) {
				king_pos = index;
			}
			continue ;
		}
		switch (piece & 0x7) {
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
	_turn = !_turn;
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
	std::string res = "a0";
	res[0] = static_cast<char>((index & 0x7) + 'a');
	res[1] = static_cast<char>((index >> 3) + '1');
	return (res);
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

// getBoard and setBoard must be in synch
std::string Chess::getBoard( void )
{
	std::string res;

	for (int i = 0; i < 64; ++i) {
		// std::cout << "in get board " << i << ": " << static_cast<int>(_board[i]) << std::endl;
		res += _board[i] + 32;
	}
	return (res + '\n');
}

void Chess::setBoard( std::string str )
{
	for (int i = 0; i < 64; ++i) {
		// std::cout << "in set board " << i << ": " << static_cast<int>(str[i] - 32) << std::endl;
		_board[i] = str[i] - 32;
	}
}

void Chess::setCaptures( int index )
{
	_captured.fill(false);
	if (index < 0 || index >= 64) return ;
	char piece = _board[index];
	_turn = (piece & PIECES::WHITE) == PIECES::WHITE;
	switch (piece & 0x7) {
		case PIECES::EMPTY:
			return ;
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
			drawSquare(vertices, (row + col) & 0x1, startX + col * square_width, startY + (7 - row) * square_height, square_width, square_height);
			// std::cout << "row " << row << ", col " << col << ": " << static_cast<int>(_board[(row << 3) + col]) << std::endl;
			if ((row << 3) + col == except) {
				continue ; // we skip square at index except
			}
			char piece = _board[(row << 3) + col];
			if (piece != PIECES::EMPTY) {
				drawSquare(vertices, 1 + (piece & 0x7) + 6 * ((piece & PIECES::WHITE) == PIECES::WHITE), startX + col * square_width, startY + (7 - row) * square_height, square_width, square_height);
			}

			if (_captured[(row << 3) + col]) {
				drawSquare(vertices, !((row + col) & 0x1), startX + col * square_width + square_width / 4, startY + (7 - row) * square_height + square_height / 4, square_width / 2, square_height / 2);
			}
		}
	}
	// std::cout << "OVER\n\n" << std::endl;
}

// return {piece at, index of square} from mouse position on screen
std::array<int, 2> Chess::getSelectedSquare( double mouseX, double mouseY)
{
	int row = (270 - mouseY) / 30;
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
	int piece = _board[src];
	std::cout << ((_turn) ? "whites " : "blacks ") << "move " << PIECES::name[piece & 0x7] << " from " << indexToStr(src) << " to " << indexToStr(dst) << std::endl;
	if ((piece & PIECES::WHITE) != _turn * PIECES::WHITE) {
		std::cout << "not your turn to move" << std::endl;
		return ;
	}
	_captured.fill(false);
	switch (piece & 0x7) {
		case PIECES::EMPTY:
			return ;
		case PIECES::KING:
			addKingCaptures(src);
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
			addPawnCaptures(src);
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

	int tmp = _board[dst];
	_board[dst] = _board[src];
	_board[src] = PIECES::EMPTY;
	if (!legalBoard()) { // if invalid move, restore pos
		std::cout << "your king is in check" << std::endl;
		_board[src] = _board[dst];
		_board[dst] = tmp;
	} else {
		std::cout << "legal" << std::endl;
		_turn = !_turn;
	}
}
