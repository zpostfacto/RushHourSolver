//
// Solver for the puzzle game "Rush Hour"
//

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
#include <string.h>

#include <set>
#include <vector>

// Assume the board is square
constexpr int BOARD_SIZE = 6;

// The exit ramp is on a particular row
constexpr int BOARD_EXIT_Y = 2;

// Set to true to enable dumping of output to show our thinking
constexpr bool DEBUG_PROGRESS_OUTPUT = false;

// Struct used to describe a particular configuration of cars on the board
struct Board
{

	// We represent the board as a simple 2D grid.  Each cell is
	// a printable-character.  A space character (' ') is used
	// to denote an empty cell.  Each car should be assigned a unique
	// character (e.g. letters or numbers).  The goal car we are trying
	// to get out of the garage must be assigned the character 'X'.
	//
	// The array should be indexed [y][x], where y is the row index
	// and x is the column index
	char cell[BOARD_SIZE][BOARD_SIZE];

	// Define a comparison operator so we can use std::set to
	// optimize lookups.  Hashing would actually be more optimal,
	// but this is simple
	inline bool operator<( const Board &x ) const
	{
		return memcmp( cell, x.cell, sizeof(cell) ) < 0;
	}

	// Return the value of cell[y][x].  Assert if we are out of bounds
	char Cell( int y, int x ) const
	{
		assert( x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE );
		return cell[y][x];
	}

	// Return the value of cell[y][x], but return 0 if the coords
	// are off the board
	char CellSafe( int y, int x ) const
	{
		if ( x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE )
			return 0;
		return cell[y][x];
	}

	// Set a cell value, with bounds checking
	void SetCell( int y, int x, char c )
	{
		assert( x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE );
		cell[y][x] = c;
	}

	// Print this board state.  If there is a next state,
	// then optionally draw an arrow to show shat the move is
	void Print( const char *indent, const Board *next ) const
	{
		if ( !next ) next = this; // !KLUDGE! If not asking to show the move to the next state, just set next to be same as self
		for ( int y = 0 ; y < BOARD_SIZE ; ++y )
		{
			printf( "%s", indent );
			for ( int x = 0 ; x < BOARD_SIZE ; ++x )
			{

				// Assume we won't print an arrow
				char c = Cell( y, x );

				// See if next board state differs here
				char n = next->Cell( y, x );
				if ( c != n )
				{
					if ( c == ' ' )
					{

						// This is where the move happened.  FIgure which
						// direction arrow to draw
						if ( CellSafe( y, x-1 ) == n )
							c = '>';
						else if ( CellSafe( y, x+1 ) == n )
							c = '<';
						else if ( CellSafe( y-1, x ) == n )
							c = 'v';
						else if ( CellSafe( y+1, x ) == n )
							c = '^';
						else
							assert( false ); // Next state is not reachable from this state by a simple move
					}
					else if ( n == ' ' && y == BOARD_EXIT_Y && x < BOARD_SIZE-1 )
					{
						// Check for the special mode where a car leaves the board entirely
						bool bCarLeftBoard = true;
						for ( int xx = x+1 ; xx < BOARD_SIZE ; ++xx )
						{
							if ( next->Cell( y, xx ) != ' ' || ( Cell( y, xx ) != ' ' && Cell( y, xx ) != c ) )
							{
								bCarLeftBoard = false;
								break;
							}
						}
						if ( bCarLeftBoard )
						{
							while ( x < BOARD_SIZE && c == Cell( y, x ) )
							{
								printf( "%c", c );
								++x;
							}
							while ( x < BOARD_SIZE+1 )
							{
								printf( ">" );
								++x;
							}
							break;
						}
					}
				}
				printf( "%c", c );
			}
			printf( "\n" );
		}
	}
};

// List of all board states that we have discovered.
// The initial state is at index 0.  We use breath-first-search
// so all the states reachable with 1 move follow the initial state,
// then all the states reachable with 2 moves, etc.
//
// The second item in the pair is the index into this list
// of the previous state.  This is used to reconstruct the
// optimal path.
std::vector< std::pair<Board,int> > state_list;

// The same set of states as in state_list, but in a map
// so that we can quickly check if a state is already in the list.
std::set<Board> states_in_list; 

// Recursive helper function to print the solution.
int PrintSolutionRecursive( int i, const Board *next )
{
	if ( i < 0 )
		return 0;
	const Board &cur = state_list[i].first;
	int step_number = PrintSolutionRecursive( state_list[i].second, &cur )+1;
	printf( "Solution step %d\n", step_number );
	cur.Print( "  ", next );
	printf( "\n" );
	return step_number;
}

// Called when we have found the solution.  It is assumed to
// be the last state in the list.
void PrintSolutionAndQuit()
{
	PrintSolutionRecursive( state_list.size()-1, nullptr );
	exit(0);
}

// See if we have been in this state before.  If not, add
// it to the table of states, which serves as the queue
// of states we need to explore.  The "from" argument
// is the index of the state we are coming from.
void CheckAddState( const Board &state, int from )
{
	// Attempt insertion in the fast lookup table
	// std::set::insert returns a std::pair, and the
	// "second" memver is a boolean indicating whether
	// insertion actually happened, or whether insertion
	// was not performed because an equivalent item
	// was already in the set.
	if ( !states_in_list.insert( state ).second )
	{

		// We've already seen this state

		// !TEST! Dump it for debugging
		if ( DEBUG_PROGRESS_OUTPUT )
		{
			int idx_found = 0;
			while ( state_list[idx_found].first < state || state < state_list[idx_found].first )
			{
				++idx_found;
				assert( idx_found < (int)states_in_list.size() );
			}
			printf( "  Rejected move, already found state %d\n", idx_found );
			state_list[from].first.Print( "    ", &state );
		}
		return;
	}

	// New board state we haven't seen before.  Add it to the
	// queue, and remember the previous board state we came from
	state_list.emplace_back( state, from );

	// Sanity check invariant that our quick lookup table
	// is the same size as the simple list.
	assert( state_list.size() == states_in_list.size() );

	// !TEST! Dump for debugging
	if ( DEBUG_PROGRESS_OUTPUT && from >= 0 )
	{
		printf( "  Added state %d (previous %d)\n", (int)state_list.size()-1, from );
		state_list[from].first.Print( "    ", &state );
	}
}

// Check if we can move a car one square in a given direction
// into the space at x,y, which must be empty.  dx,dy is the
// direction we will *scan* from x,y.  The car will move in
// the opposite direction, into the empty space.
// 
// Why are dx,dy passed as template arguments rather than ordinary
// function arguments?  This is weird, but it ensures that they are
// compile-time constants and that the compiler treats them as such
// and generates an optimized function for each of the four
// search directions.
//
// Also the board state is passed by a mutable reference, so that
// we can temporarily modify it.  However this function is logically
// constant as we always undo our changes.  We could make a copy,
// but undoing the changes is faster.
template <int dx, int dy>
inline void CheckMove( Board &s, int x, int y, int idx_state )
{
	// #define an expression that will be true if tx,ty is still
	// on the board.  Since dx and dy are constants, we use them to
	// select which of the other checks might actually be necessary.
	// Exactly one check will be needed; the others will be discarded
	// at compile time.
	#define txty_on_board ( \
		( dx >= 0 || tx >= 0 ) && \
		( dx <= 0 || tx < BOARD_SIZE ) && \
		( dy >= 0 || ty >= 0 ) && \
		( dy <= 0 || ty < BOARD_SIZE ) )

	// Step two squares in the scan direction
	int tx = x + dx*2;
	int ty = y + dy*2;

	// Are we still on the board?
	if ( !(txty_on_board) )
		return;

	// Check if the two cells have the same value, and it's not a space
	const char car = s.Cell( ty, tx );
	if ( s.Cell( ty-dy, tx-dx ) != car || car == ' ' )
		return;

	// There's a car here we could move.  Find the end.
	// (The game actually only has cars of 2 or 3, so this
	// loop will only iterate at most one time.  But if there
	// was a car of length 4, this loop would be necessary
	// and work.)
	do {
		tx += dx;
		ty += dy;
	} while ( (txty_on_board) && s.Cell( ty, tx ) == car );
	tx -= dx;
	ty -= dy;

	// Move the car one space, in the opposite direction of dx,dy
	// into the empty space.  This only requires changing two grid
	// cells, no matter how long the car is.
	s.SetCell( y, x, car );
	s.SetCell( ty, tx, ' ' );

	// Check if we just moved a car adjacent to the exit ramp,
	// which is on the right hand side of the board
	if ( dx == -1 && x == BOARD_SIZE-1 && y == BOARD_EXIT_Y )
	{
		// Was it the target car?
		// Then we have solved the puzzle!
		if ( car == 'X' )
		{

			// Add the state.  (This should always succeed!)
			CheckAddState( s, idx_state );

			// And we're done
			PrintSolutionAndQuit();
		}
		else
		{
			// Not target car, but we can still move it
			// completely off the board.  This is always
			// desirable when possible.

			// Erase the car from the board
			for ( int xx = tx+1 ; xx <= x ; ++xx )
				s.SetCell( y, xx, ' ' );

			// Is this a new board state?
			CheckAddState( s, idx_state );

			// Put the car back on the board
			for ( int xx = tx+1 ; xx <= x ; ++xx )
				s.SetCell( y, xx, car );
		}
	}
	else
	{

		// If this is a new state we haven't sen before, add it to
		// the queue to explore
		CheckAddState( s, idx_state );
	}

	// Undo our changes, moving the car back where it was
	s.SetCell( y, x, ' ' );
	s.SetCell( ty, tx, car );

	#undef txty_on_board
}

int main()
{

	//
	// Setup initial board state
	// (Uncomment one of the blocks below)
	//

	Board initial_board;

// Read from STDIN
//	printf( "Enter initial board state:\n" );
//	for ( int y = 0 ; y < BOARD_SIZE ; ++y )
//	{
//		char row[BOARD_SIZE+1];
//		if ( !gets_s(row, sizeof(row)) || strlen(row) != BOARD_SIZE )
//		{
//			fprintf( stderr, "Error parsing board from stdin\n" );
//			return 1;
//		}
//		memcpy( &initial_board.cell[y], row, BOARD_SIZE );
//	}

	// Use hardcoded state for testing

	//// Board #1 (beginner)
	//memcpy( initial_board.cell[0], "AA   O", BOARD_SIZE );
	//memcpy( initial_board.cell[1], "P  Q O", BOARD_SIZE );
	//memcpy( initial_board.cell[2], "PXXQ O", BOARD_SIZE );
	//memcpy( initial_board.cell[3], "P  Q  ", BOARD_SIZE );
	//memcpy( initial_board.cell[4], "B   CC", BOARD_SIZE );
	//memcpy( initial_board.cell[5], "B RRR ", BOARD_SIZE );

	//// Board #93 (expert)
	//memcpy( initial_board.cell[0], " AAB O", BOARD_SIZE );
	//memcpy( initial_board.cell[1], "CD B O", BOARD_SIZE );
	//memcpy( initial_board.cell[2], "CDXXEO", BOARD_SIZE );
	//memcpy( initial_board.cell[3], "FGGHE ", BOARD_SIZE );
	//memcpy( initial_board.cell[4], "F IHJJ", BOARD_SIZE );
	//memcpy( initial_board.cell[5], "  IPPP", BOARD_SIZE );

	// Board #155 (genius)
	memcpy( initial_board.cell[0], "OOOA P", BOARD_SIZE );
	memcpy( initial_board.cell[1], "  BA P", BOARD_SIZE );
	memcpy( initial_board.cell[2], "XXBIIP", BOARD_SIZE );
	memcpy( initial_board.cell[3], " DEEFF", BOARD_SIZE );
	memcpy( initial_board.cell[4], "GDH CC", BOARD_SIZE );
	memcpy( initial_board.cell[5], "G H JJ", BOARD_SIZE );

	//
	// Prepare
	//

	// Print the initial board state
	printf( "Initial board state:\n" );
	initial_board.Print( "  ", nullptr );

	// Add it as the first (and only) state
	CheckAddState( initial_board, -1 );
	assert( state_list.size() == 1 );

	//
	// Search for solution using breadth-first-search
	//

	// Keep exploring the frontier of states, until we hit the end of the list.
	// The list of states also serves as the queue of states to explore.  This
	// looks like a standard for loop, but it's actually a standard breadth-
	// first search, since we add new states to the list as they are discovered.
	for ( int idx_state = 0 ; idx_state < (int)state_list.size() ; ++idx_state )
	{

		// Grab the next state from the frontier.
		Board s = state_list[idx_state].first;

		// !TEST! print status
		if ( DEBUG_PROGRESS_OUTPUT )
		{
			printf( "Exploring state %d\n", idx_state );
			s.Print( "  ", nullptr );
		}
		else if ( idx_state % 100 == 0 )
		{
			printf( "...explored %d board states\n", idx_state );
		}

		// Find all states that are reachable from this state by
		// moving a car a single square.
		for ( int y = 0 ; y < BOARD_SIZE ; ++y )
		{
			for ( int x = 0 ; x < BOARD_SIZE ; ++x )
			{

				// Is this cell empty?
				if ( s.Cell( y, x ) != ' ' )
					continue;
						
				// Check for moving a car into the empty space at x,y
				// from all four directions
				CheckMove<+1, 0>( s, x, y, idx_state );
				CheckMove<-1, 0>( s, x, y, idx_state );
				CheckMove<0, +1>( s, x, y, idx_state );
				CheckMove<0, -1>( s, x, y, idx_state );
			}
		}
	}

	// We've exhausted all possible board states reachable from the
	// initial position and didn't find a solution.  The puzzle
	// is not solvable, or we have a bug!
	printf( "Cannot find solution!\n" );
	return 1;
}

