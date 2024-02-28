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

namespace TEXTURE
{
	enum {
		BLACK_SQUARE,
		WHITE_SQUARE,
		WHITE_KING,
		WHITE_QUEEN,
		WHITE_ROOK,
		WHITE_BISHOP,
		WHITE_KNIGHT,
		WHITE_PAWN,
		BLACK_KING,
		BLACK_QUEEN,
		BLACK_ROOK,
		BLACK_BISHOP,
		BLACK_KNIGHT,
		BLACK_PAWN,
		MOVE_HIGHLIGHT,
		HIGHLIGHT,
		PREMOVE,
		SIZE
	};
}

class Chess
{
	private:
		std::string _board, _castle_state, _en_passant;
		int _half_moves, _full_moves, _current_board;
		std::array<bool, 64> _captured;
		std::vector<std::string> _game_history;
		std::vector<std::array<int, 2>> _premoves;
		std::vector<int> _highlights;
		std::array<int, 2> _last_move;
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
		bool legalBoard( bool debug = true );
		void movePiece( int src, int dst, char src_piece, char dst_piece );

		std::string indexToStr( int index );
		std::string getFENPrefix( void );
		void updateHistoric( void );
		bool premovedSquare( int square );
		bool highlightedSquare( int square );

	public:
		Chess( void );
		~Chess( void );

		int texIndex( char piece );
		std::string getFEN( bool check_ended );
		void setColor( char color );
		std::array<int, 2> setBoard( std::string fen );
		void navigateHistory( bool right, bool once = true );
		void setCaptures( int index );
		void setHighlight( int src, int dst );
		void resetHighlights( void );
		void resetPremoves( void );
		void applyPremoves( void );

		void drawSquare( std::vector<int> &vertices, int type, int startX, int startY, int square_size );
		void drawWaitingRoom( std::vector<int> &vertices, int mouseX, int mouseY, int square_size, int win_width, int win_height );
		void drawBoard( std::vector<int> &vertices, int except, int square_size );
		std::array<int, 3> getSelectedSquare( double mouseX, double mouseY, int square_size );
		bool forceMovePiece( int src, int dst );
		bool tryMovePiece( int src, int dst );
};

#endif
