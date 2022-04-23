# RushHourSolver
A simple solver for the puzzle game [Rush Hour](https://en.wikipedia.org/wiki/Rush_Hour_(puzzle)).

This code is intended to be a straightforward implementation of a breadth-first search
to exhaustively solve a puzzle.  I tried to comment my code well and make it easy to read
and not do too many "weird" things.  Rush Hour is a particular good puzzle to use as an
example for understanding the basic principle of exploring the state space of a game because
the size of the search space is relatively constrained.  The board is relatively small,
and in any given board position, there are only a few cars on the board that are able to move.

I hope you find the code interesting and useful.