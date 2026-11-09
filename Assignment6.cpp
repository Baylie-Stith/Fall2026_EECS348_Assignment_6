// File Name: Assignment6.cpp
// Author: Baylie Stith
//
// Description: Solves a sudoku puzzle using recursion (depth-first search
//              with backtracking). Finds all solutions and writes them to a single
//              output file (solutions.txt).
// Contributors: ChatGPT, Claude
// Orignal AI Used: Claude
//
// Input: Sudoku puzzle text files (digits 1-9, '_' or '0' for a blank)
// Output: Written to solutions.txt: for each puzzle, "Puzzle: <file name>" followed by either
//         "No solution found." or each solution grid followed by "Solutions found: n".
// Created: 10/13/2026
// Revised: 10/14/2026 (Baylie Stith & ChatGPT)
// Revised: 10/26/2026 Saylor Statler - Modified for Assignment 5
// Revised by: Baylie Stith 11/09/2026 - Modified for Assignment 6 (single solutions.txt output,
//             puzzle labels, divider grid format) with help from Claude and ChatGPT
//             (Claude: single solutions.txt output and puzzle labels;
//              ChatGPT: bitmask candidate checking, dead-end detection, board reset in load())
//
// Build: g++ -std=c++17 -O2 -o Assignment6.exe Assignment6.cpp
// Run:   ./Assignment6.exe puzzle1.txt puzzle2.txt puzzle3.txt puzzle4.txt puzzle5.txt

// Header files -> Access extra functions
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/* -----------------------------------------------------------------
Class representing a sudoku puzzle
Function: Loads, checks, solves, and prints the puzzle.
Input: File name of the puzzle
Output: Solved puzzle(s) and the number of solutions
Created by: Claude (Created the entire Class)
Edited by: Baylie Stith (Created comments, improved error checking)
Edited by: ChatGPT (Helped improve written code that caused errors)
--------------------------------------------------------------------- */
class SudokuPuzzle {
// Code can be used by main()
public:

   // Constants fixed when first compiled
   static constexpr int N = 9; // Constant for the size of the grid
   static constexpr int B = 3; // Constant for the size of each block

   // Runs when SudokuPuzzle created. Takes file name (makes its constant) and stores it
   explicit SudokuPuzzle(const std::string& fileName) : fileName_(fileName) {}
   /* --------------------------------------------------------------------------------------
   Bool function Load() -> Reads exactly 81 cells: digits 1-9 or '_' or '0' for blank. Whitespace is ignored. Returns false if the file is missing, has a bad
                           character, or the wrong cell count.
   Input: File name of the puzzle
   Output: True -> Puzzle loaded successfully
           False -> Puzzle failed to load
   Created by: Claude (Created the entire function)
   Edited by: Baylie Stith (Created comments, allowed 0 to pass as blank)
   Edited by: ChatGPT (Assignment 6: reset the board at the start of load())
   -------------------------------------------------------------------------------------- */
   bool load() {
       // Open the puzzle file for reading
       std::ifstream in(fileName_);
       // If the file is missing or can't be opened -> puzzle failed to load
       if (!in) return false;

       // MODIFIED (Assignment 6, ChatGPT): Reset the board in case this object is loaded more than once.
       // Sets every cell in every row to 0 (blank) so old values can't carry over
       for (auto& row : grid_) row.fill(0);

       int count = 0; // counts how many cells have been read so far (0-81)
       char ch;       // holds the character currently being read

       // Read one character at a time until 81 cells are filled or the file ends
       while (count < N * N && in.get(ch)) {
           // Digit 1-9: convert the character to an int and store it in the grid.
           // count / N gives the row and count % N gives the column.
           if (ch >= '1' && ch <= '9')
               grid_[count / N][count % N] = ch - '0';
           // '_' or '0' means a blank cell, stored as 0
           else if (ch == '_' || ch == '0')
               grid_[count / N][count % N] = 0;
           // Spaces, tabs, and newlines are not cells, so skip them without
           // increasing count (continue jumps back to the top of the loop)
           else if (isWhitespace(ch))
               continue;
           // Any other character is invalid -> the puzzle failed to load
           else
               return false;
           // A cell was stored, so move on to the next one
           ++count;
       }

       // If the file ended before 81 cells were read, it is too short -> return false
       if (count != N * N) return false;

       // After 81 cells, only whitespace is allowed.
       // Keep reading to make sure the file has no extra characters at the end.
       while (in.get(ch)) {
           // A non-whitespace character here means the file has too many cells -> return false
           if (!isWhitespace(ch)) return false;
       }
       // Exactly 81 valid cells and nothing else -> the puzzle loaded successfully
       return true;
   }

   /* -------------------------------------------------------------------------------------------
   Bool isInitialGridValid()-> Checks that the given clues do not already conflict with each other.
   Input: Grid of the puzzle
   Output: True if the grid is valid
           False if the grid is invalid
   Created by: Baylie Stith (Created the function)
   Edited by: ChatGPT (Fixed logic errors)
   Edited by: ChatGPT (Assignment 6: rewrote to build row/column/box bitmasks from the clues)
   ------------------------------------------------------------------------------------------- */
   bool isInitialGridValid() {
       // MODIFIED (Assignment 6, ChatGPT): Build lookup masks from the starting clues and detect conflicts.
       // Clear the masks so no digits are marked as used before we start checking.
       // Each mask holds one 9-bit number per row, column, and 3x3 box.
       // Bit 0 stands for digit 1, bit 1 for digit 2, ... bit 8 for digit 9.
       rowUsed_.fill(0);
       colUsed_.fill(0);
       boxUsed_.fill(0);

       // Visit every cell in the grid, row by row
       for (int row = 0; row < N; ++row) {
           for (int col = 0; col < N; ++col) {
               // Save the digit in this cell (0 means the cell is blank)
               int value = grid_[row][col];
               // Blank cell -> nothing to check, move on to the next cell
               if (value == 0) continue;

               // Make a number with only this digit's bit turned on.
               // Example: value 3 -> 1u << 2 -> binary 000000100
               unsigned short bit = static_cast<unsigned short>(1u << (value - 1));
               // Find which 3x3 box (0-8) this cell belongs to
               int box = boxIndex(row, col);

               // If this digit's bit is already on in the row, column, or box mask,
               // the same digit was already seen there, so two clues conflict.
               // The puzzle is unsolvable -> return false.
               if ((rowUsed_[row] & bit) || (colUsed_[col] & bit) ||
                   (boxUsed_[box] & bit)) {
                   return false;
               }

               // No conflict: turn on this digit's bit in the row, column, and box
               // masks so later clues are checked against it
               rowUsed_[row] |= bit;
               colUsed_[col] |= bit;
               boxUsed_[box] |= bit;
           }
       }
       // Every clue passed the check -> the starting grid is valid.
       // The masks now hold all the clues, so search() can start from them.
       return true;
   }

   // prints grid to output stream and passes it to private printgrid without making changes
   void print(std::ostream& out) const { printGrid(out, grid_); }

   /* -----------------------------------------------------------------------------
   Int solveAll() -> Finds and prints every solution; returns how many were found.
   Input: Sudoku puzzle
   Output: Solution(s) and how many were found
   Created by: Claude (Created the entire function)
   Edited by: Baylie Stith (Created comments))
   ---------------------------------------------------------------------------- */

   int solveAll(std::ostream& out) {
       solutionCount_ = 0; // resets solution count to 0
       out_ = &out;
       search(); // starts recursive search
       return solutionCount_; // prints number of solutions found
   }

// only accessible through this classes own functions
private:
   using Grid = std::array<std::array<int, N>, N>; // creates name for 9x9 grid

   // member variables which each sudoku puzzle carrys
   std::string fileName_; // holds name of puzzle
   Grid grid_{}; // holds grid size
   int solutionCount_ = 0; // starts solutions at 0
   std::ostream* out_ = nullptr;

   // MODIFIED (Assignment 6, ChatGPT): Each bit represents whether a digit is already used
   // in a row, column, or box.
   std::array<unsigned short, N> rowUsed_{};
   std::array<unsigned short, N> colUsed_{};
   std::array<unsigned short, N> boxUsed_{};

   // MODIFIED (Assignment 6, ChatGPT): Added helper that maps a cell to its 3x3 box number
   static int boxIndex(int row, int col) {
       return (row / B) * B + (col / B);
   }
  
   // Indicates whether a character is whitespace.
   static bool isWhitespace(char ch) {
       return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
   }

   /* --------------------------------------------------------------------------------
   bool isCandidate() -> checks if value v can go at (row, col) without conflicting
                          with other values in the same row, column, or block. Returns
                          true if it can go there, false otherwise.
   Input: Row, Column, and Value
   Output: True if the value can go there
           false otherwise
   Created by: Claude (Created the entire function)
   Edited by: Baylie Stith (Created comments)
   Edited by: ChatGPT (Assignment 6: replaced the row/column/block scans with bitmask lookups)
   -------------------------------------------------------------------------------- */

   bool isCandidate(int row, int col, int v) const {
       // MODIFIED (Assignment 6, ChatGPT): Bitmask lookup instead of scanning the grid
       unsigned short bit = static_cast<unsigned short>(1u << (v - 1));
       int box = boxIndex(row, col);
       return !(rowUsed_[row] & bit) &&
              !(colUsed_[col] & bit) &&
              !(boxUsed_[box] & bit);
   }

   /* ------------------------------------------------------------------------------
   Bool findEmpty() -> Picks the empty cell with the fewest candidates (fewer branches to try)
                       and Returns false if the grid is full (no empty cells)
   Input: Sudoku puzzle
   Output: True -> if an empty cell was found
           False -> if grid full
   Created by: Baylie Stith (Created the function)
   Edited by: ChatGPT (Fixed logic errors)
   Edited by: ChatGPT (Assignment 6: added deadEnd flag and early return on a one-candidate cell)
   ------------------------------------------------------------------------------ */

   // MODIFIED (Assignment 6, ChatGPT): Added deadEnd parameter
   bool findEmpty(int& bestRow, int& bestCol, bool& deadEnd) const {
       int minCandidates = N + 1;
       bool found = false;
       deadEnd = false;

       // Find the empty cell with the fewest legal values (MRV).
       for (int row = 0; row < N; ++row) {
           for (int col = 0; col < N; ++col) {
               if (grid_[row][col] != 0) continue;

               int count = 0;
               for (int v = 1; v <= N; ++v)
                   if (isCandidate(row, col, v)) ++count;

               // MODIFIED (Assignment 6, ChatGPT): No candidates means this branch cannot
               // produce a solution.
               if (count == 0) {
                   deadEnd = true;
                   return true;
               }

               if (count < minCandidates) {
                   minCandidates = count;
                   bestRow = row;
                   bestCol = col;
                   found = true;

                   // MODIFIED (Assignment 6, ChatGPT): A cell with one candidate is the best
                   // possible choice.
                   if (count == 1) return true;
               }
           }
       }
       return found; // false means there are no empty cells
   }

   /* ---------------------------------------------------------------------------------
   Void search() -> Recursive depth-first search with backtracking.
   Input: Sudoku puzzle
   Output: Solution(s) and how many were found
   Created by: Baylie Stith (Created the function)
   Edited by: ChatGPT (Fixed logic errors)
   Edited by: ChatGPT (Assignment 6: updates and undoes the bitmasks, handles deadEnd)
   Edited by: Baylie Stith (Assignment 6: added comments)
   --------------------------------------------------------------------------------- */

   void search() {
    // Records empty cell with fewest candidates and reports if there's no legal digits
       int row = 0, col = 0;
       bool deadEnd = false; // MODIFIED (Assignment 6, ChatGPT)

    // Returns false if no empty cells left, meaning solutions found. Counts it, prints it,
    // then looks for more solutions.
       if (!findEmpty(row, col, deadEnd)) {
           ++solutionCount_;
           printGrid(*out_, grid_);
           return;
       }
       // Empty cell has no candidates so branch can't be completed. Returns and trys a
       // different digit.
       if (deadEnd) return; // MODIFIED (Assignment 6, ChatGPT)

       // MODIFIED (Assignment 6, ChatGPT): Try each candidate, update its masks, recurse,
       // then undo the move.
       // loops through each digit 1-9 to see if it can be placed in the empty cell
       for (int v = 1; v <= N; ++v) {
           if (isCandidate(row, col, v)) {
            // assignes bits for each digit to record what digits are used in each row, column, and box
               unsigned short bit = static_cast<unsigned short>(1u << (v - 1));
               int box = boxIndex(row, col);
            
            // Places a digit then marks it as used in the row, column, and box.
               grid_[row][col] = v;
               rowUsed_[row] |= bit;
               colUsed_[col] |= bit;
               boxUsed_[box] |= bit;

            // Solve the rest of the grid and returns after exploring every possibility.
               search();

            // Undoes the move and marks the digit as unused in the row, column, and box.
               grid_[row][col] = 0;
               rowUsed_[row] &= static_cast<unsigned short>(~bit);
               colUsed_[col] &= static_cast<unsigned short>(~bit);
               boxUsed_[box] &= static_cast<unsigned short>(~bit);
           }
       }
   }

   /* ---------------------------------------------------------------------------------
   Void printGrid() -> Prints the Sudoku grid to the specified output stream.
   Input: Output stream and Sudoku grid
   Output: Formatted grid representation
   Created by: Claude (Created the function)
   Edited by: Baylie Stith (Created comments)
   Edited by: Baylie Stith (Assignment 6: kept the divider-line grid format)
   --------------------------------------------------------------------------------- */

   static void printGrid(std::ostream& out, const Grid& g) { // takes output stream and grid
       // Loop through each row of the grid, top to bottom
       for (int r = 0; r < N; ++r) {
           // Before the 4th and 7th rows (r = 3 and r = 6), print a horizontal divider
           // line to separate the 3x3 blocks. r > 0 skips the very first row.
           if (r > 0 && r % B == 0) out << "------+-------+------\n";

           // Loop through each column in the current row, left to right
           for (int c = 0; c < N; ++c) {
               // Before the 4th and 7th columns (c = 3 and c = 6), print a vertical
               // divider to separate the 3x3 blocks. c > 0 skips the first column.
               if (c > 0 && c % B == 0) out << "| ";

               // Print '_' if the cell is empty (0). Otherwise convert the digit (1-9)
               // to its character by adding it to '0' (e.g. 5 -> '5') and print that.
               out << (g[r][c] == 0 ? '_' : char('0' + g[r][c]));

               // Print a space after every cell except the last one in the row,
               // so there is no trailing space at the end of the line
               out << (c < N - 1 ? " " : "");
           }
           // End the current row with a newline
           out << '\n';
       }
       // Print a blank line after the whole grid to separate it from whatever comes next
       // (another solution or the "Solutions found: n" line)
       out << '\n';
   }
}; // MODIFIED (Assignment 6, Baylie Stith): restored closing brace of class SudokuPuzzle

/* --------------------------------------------------------------------------
int main() -> Main function to run the Sudoku puzzle solver. It processes
              command-line arguments for puzzle files, loads each puzzle,
              checks for validity, and solves it.
Input: Command-line arguments for puzzle files
Output: Solved puzzle(s) and the number of solutions
Created by: Claude (Created the entire function)
Edited by: Baylie Stith (Created comments, improved error checking)
Edited by: ChatGPT (Fixed logic errors)
Edited by: Claude (Assignment 6: single solutions.txt output and "Puzzle: x" labels)
Edited by: Baylie Stith (Assignment 6: split the load/validity checks into separate ifs)
-------------------------------------------------------------------------- */

int main(int argc, char* argv[]) { // argc -> num of command line arguments.argv -> hold them as text
   // Builds list of files to proccess. If file name passed use them, otherwise generate the files
   // by joining text with int converted char
   std::vector<std::string> files;
   if (argc > 1) {
       for (int i = 1; i < argc; ++i) files.push_back(argv[i]);
   } else {
       for (int i = 1; i <= 5; ++i)
           files.push_back("puzzle" + std::to_string(i) + ".txt");
   }

   // MODIFIED (Assignment 6, Claude): Single output file for all puzzles
   std::ofstream outFile("solutions.txt");
   if (!outFile) {
       std::cerr << "Could not open solutions.txt for writing.\n";
       return 1;
   }

   for (size_t i = 0; i < files.size(); ++i) {
       const auto& name = files[i];

       // MODIFIED (Assignment 6, Claude): Blank line between puzzles, then the puzzle label
       if (i > 0) outFile << "\n";
       outFile << "Puzzle: " << name << "\n";

       SudokuPuzzle puzzle(name);

       // MODIFIED (Assignment 6, Baylie Stith): Load and validity checks are separate ifs
       if (!puzzle.load()) {
           outFile << "No solution found.\n";
           continue;
       }
       if (!puzzle.isInitialGridValid()) {
           outFile << "No solution found.\n";
           continue;
       }

       // Run solver and count solutions
       int found = puzzle.solveAll(outFile);

       // Write required summary line or error message
       if (found == 0) {
           outFile << "No solution found.\n";
       } else {
           outFile << "Solutions found: " << found << "\n";
       }
   }
   return 0; // ends program
}