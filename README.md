# ChessPlusPlus

This is a C++14 Chess AI written for Linux which utilzies OpenMP for multithreading.
It features a 15x15 vector based board as described in [chessprogramming.com](https://chessprogramming.wikispaces.com/Vector+Attacks#Superimposed%20Lookup)

## Dependencies
* g++ version 6.1 or later
* GNU Make

## Use and installation
* Run `make` on the root project directory
* Executable will be created in bin folder
* Follow onscreen prompts or type help for a list of commands
<br></br>
# Features

## Search
* Iterative deepening
* MTD(f)
* Alpha beta
* Transposition table
* Multithreading

## Evaluation
* Material count
* Mobility
* Reduced mobility for knights that move to squares guarded by enemy pawns
* Rook open/half-open files
* Doubled and isolated pawns
* Piece square tables including separate endgame piece square table for kings
* Rooks on the 7th rank
* Pawns on the 6th and 7th rank
* Mate and draw evaluation
