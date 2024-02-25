#ifndef CHESS_HPP
# define CHESS_HPP

# include <string>
# include <vector>
# include <array>

# define TURN_WHITE 'w'
# define TURN_BLACK 'b'

namespace PIECES
{
	const char EMPTY  = '0';
	const char KING   = 'k';
	const char QUEEN  = 'q';
	const char ROOK   = 'r';
	const char BISHOP = 'b';
	const char KNIGHT = 'n';
	const char PAWN   = 'p';

	/* board indexes are
	8	00 01 02 03 04 05 06 07
	7	08 09 10 11 12 13 14 15
	6	16 17 18 19 20 21 22 23
	5	24 25 26 27 28 29 30 31
	4	32 33 34 35 36 37 38 39
	3	40 41 42 43 44 45 46 47
	2	48 49 50 51 52 53 54 55
	1	56 57 58 59 60 61 62 63
		A  B  C  D  E  F  G  H
	*/
	const std::string board_init = "rnbqkbnrpppppppp00000000000000000000000000000000PPPPPPPPRNBQKBNR";
}

class Chess
{
	private:
		std::string _board, _castle_state, _en_passant;
		int _half_moves, _full_moves;
		std::array<bool, 64> _captured;
		std::vector<std::array<char, 64>> _history;
		char _turn, _color;

		bool addCapture( int row, int col, bool empty_allowed = true, bool capture_allowed = true );
		void addKingCaptures( int index );
		void addKingLegalMoves( int index );
		void addQueenCaptures( int index );
		void addRookCaptures( int index );
		void addBishopCaptures( int index );
		void addKnightCaptures( int index );
		void addPawnCaptures( int index );
		void addPawnLegalMoves( int index );
		bool legalBoard( void );
		void movePiece( int src, int dst, char src_piece, char dst_piece );

		std::string indexToStr( int index );

	public:
		Chess( void );
		~Chess( void );

		int texIndex( char piece );
		std::string getFEN( void );
		void setColor( char color );
		void setBoard( std::string fen );
		void setCaptures( int index );
		void drawSquare( std::vector<int> &vertices, int type, int startX, int startY, int square_size );
		void drawWaitingRoom( std::vector<int> &vertices, int mouseX, int mouseY, int square_size );
		void drawBoard( std::vector<int> &vertices, int except, int square_size );
		std::array<int, 3> getSelectedSquare( double mouseX, double mouseY, int square_size );
		bool forceMovePiece( int src, int dst );
		void tryMovePiece( int src, int dst );
};

#endif
