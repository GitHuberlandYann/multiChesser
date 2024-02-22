#include "Chess.hpp"
#include <iostream>

Chess::Chess( void )
	: _board(PIECES::board_init), _turn(TURN_WHITE)
{

}

Chess::~Chess( void )
{

}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

void Chess::addCapture( int row, int col )
{
	if (row < 0 || row >= 8 || col < 0 || col >= 8) {
		return ;
	}
	int offset = (row << 3) + col, piece = _board[offset];
	if (piece == PIECES::EMPTY || (piece & PIECES::WHITE) != _turn) {
		_captured[offset] = true;
	}
}

void Chess::addKingCaptures( int index )
{
	int row = index >> 3, col = index & 0x7;
	for (int r = -1; r <= 1; ++r) {
		for (int c = -1; c <= 1; ++c) {
			addCapture(row + c, col + c);
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
	for (int row = 0; row < 8; ++row) {
		addCapture(row, index & 0x7);
	}
	for (int col = 0; col < 8; ++col) {
		addCapture(index >> 3, col);
	}
}

void Chess::addBishopCaptures( int index )
{
	int row = index >> 3, col = index & 0x7;
	for (int diag = -7; diag < 7; ++diag) {
		addCapture(row + diag, col + diag);
		addCapture(row + diag, col - diag);
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
	addCapture(row - 1, col + ((_turn == TURN_WHITE) ? 1 : -1));
	addCapture(row + 1, col + ((_turn == TURN_WHITE) ? 1 : -1));
}

// determines whether current board is a legal position, based on whose turn it is to play
// we do that by filling bitboard with captured cases and checking if king is in check
bool Chess::legalBoard( void )
{
	int index = -1, king_pos = -1;
	_captured.fill(false);
	for (auto piece : _board) {
		++index;
		if ((piece & PIECES::WHITE) != _turn) {
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
	if (king_pos == -1) {
		std::cerr << "king missing from board" << std::endl;
		return (true);
	}
	return (_captured[king_pos]);
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

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

std::array<char, 64> Chess::getBoard( void )
{
	return _board;
}

void Chess::setBoard( std::string str )
{
	for (int i = 0; i < 64; ++i) {
		_board[i] = str[i];
	}
}

void Chess::drawBoard( std::vector<int> &vertices, int startX, int startY, int width, int height )
{
	int square_width = width >> 3, square_height = height >> 3;

	for (int row = 0; row < 8; ++row) {
		for (int col = 0; col < 8; ++col) {
			drawSquare(vertices, (row + col) & 0x1, startX + col * square_width, startY + (7 - row) * square_height, square_width, square_height);
			char piece = _board[(row << 3) + col];
			if (piece != PIECES::EMPTY) {
				// std::cout << "row " << row << ", col " << col << ": " << static_cast<int>(piece) << std::endl;
				drawSquare(vertices, 1 + (piece & 0x7) + 6 * ((piece & PIECES::WHITE) == PIECES::WHITE), startX + col * square_width, startY + (7 - row) * square_height, square_width, square_height);
			}
		}
	}
	// std::cout << "OVER\n\n" << std::endl;
}
