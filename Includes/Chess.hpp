#ifndef CHESS_HPP
# define CHESS_HPP

# include <vector>
# include <array>

# define TURN_WHITE true
# define TURN_BLACK false

namespace PIECES
{
	enum {
		EMPTY,
		KING,
		QUEEN,
		ROOK,
		BISHOP,
		KNIGHT,
		PAWN,
		WHITE = (1 << 3)
	};

	/* board indexes are
	8	56 57 58 59 60 61 62 63
	7	48 49 50 51 52 53 54 55
	6	40 41 42 43 44 45 46 47
	5	32 33 34 35 36 37 38 39
	4	24 25 26 27 28 29 30 31
	3	16 17 18 19 20 21 22 23
	2	08 09 10 11 12 13 14 15
	1	00 01 02 03 04 05 06 07
		A  B  C  D  E  F  G  H
	*/
	const std::array<char, 64> board_init = {
		WHITE + ROOK, WHITE + KNIGHT, WHITE + BISHOP, WHITE + QUEEN, WHITE + KING, WHITE + BISHOP, WHITE + KNIGHT, WHITE + ROOK,
		WHITE + PAWN, WHITE + PAWN,   WHITE + PAWN,   WHITE + PAWN,  WHITE + PAWN, WHITE + PAWN,   WHITE + PAWN,   WHITE + PAWN,
		EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
		EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
		EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
		EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
		PAWN,         PAWN,           PAWN,           PAWN,          PAWN,         PAWN,           PAWN,           PAWN,
		ROOK,         KNIGHT,         BISHOP,         QUEEN,         KING,         BISHOP,         KNIGHT,         ROOK
	};
}

class Chess
{
	private:
		std::array<char, 64> _board;
		std::array<bool, 64> _captured;
		std::vector<std::array<char, 64>> _history;
		bool _turn;

		void addCapture( int row, int col );
		void addKingCaptures( int index );
		void addQueenCaptures( int index );
		void addRookCaptures( int index );
		void addBishopCaptures( int index );
		void addKnightCaptures( int index );
		void addPawnCaptures( int index );
		bool legalBoard( void );

		void drawSquare( std::vector<int> &vertices, int type, int startX, int startY, int width, int height);

	public:
		Chess( void );
		~Chess( void );

		std::array<char, 64> getBoard( void );
		void setBoard( std::string str );
		void drawBoard( std::vector<int> &vertices, int startX, int startY, int width, int height );
};

#endif
