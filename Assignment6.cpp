// File Name: Assignment5.cpp
// Author: Baylie Stith
//
// Description: Solves a sudoku puzzle using recursion (depth-first search
//              with backtracking). Finds all solutions and writes them to corresponding
//              output files (solution1.txt through solution5.txt).
// Contributors: ChatGPT
// Orignal AI Used: Claude
//
// Input: Sudoku puzzle text files (digits 1-9, '_' or '0' for a blank)
// Output: Written to solution files: either "No solution found." or each solution grid
//         followed by "Solutions found: n".
// Created: 10/13/2026
// Revised: 10/14/2026 (Baylie Stith & ChatGPT)
// Revised: 10/26/2026 Saylor Statler - Modified for Assignment 5
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
   -------------------------------------------------------------------------------------- */
   bool load() {

       // opens the file and checks if it's valid
       std::ifstream in(fileName_);
       if (!in) return false; // if valid -> return true, otherwise return false
      
       int count = 0; // counts the number of cells filled so far
       char ch;

       // Loops while less than 81 cells are filled and a character can still be read
       while (count < N * N && in.get(ch)) { // doesn't read more than 81 characters
           // if digit is 1-9 store it. Also records the row and column of the cell
           // then records the character as an int
           if (ch >= '1' && ch <= '9')      grid_[count / N][count % N] = ch - '0';
           // Baylie Stith and ChatGPT to 'return false;'
           // _ or 0 stored as empty cell also records row and column and stores int
           else if (ch == '_' || ch == '0') grid_[count / N][count % N] = 0;
           // spaces, tabs, or newlines skipped
           else if (isWhitespace(ch))      continue; // Jumps back to top of the loop
           else                             return false; // invalid character found -> returns false
           ++count; // moves to next cell
       }
       // if file ends before 81 cells returns false -> file too short
       if (count != N * N) return false;

       // Anything left after the 81 cells is whitespace.
       while (in.get(ch)) {
           if (!isWhitespace(ch)) return false; // if cells are after whitespace -> return false
       }
       return true; // return true if everything else is correct
   }

   /* -------------------------------------------------------------------------------------------
   Bool isInitialGridValid()-> Checks that the given clues do not already conflict with each other.
   Input: Grid of the puzzle
   Output: True if the grid is valid
           False if the grid is invalid
   Created by: Baylie Stith (Created the function)
   Edited by: ChatGPT (Fixed logic errors)
   ------------------------------------------------------------------------------------------- */
   bool isInitialGridValid() {
       // Nested loops to check the cell row by row
       for (int row = 0; row < N; ++row) {
           for (int col = 0; col < N; ++col) {
               int value = grid_[row][col]; // save cells value
               if (value == 0) continue; // empty cell -> skip to next cell

               // Temporarily remove the value so isCandidate()
               // can check whether it conflicts with other cells.
               grid_[row][col] = 0;
               // looks for value in the same row and column block
               // if it find a conflict it trys another value until it finds every possible
               // value that could go in that spot
               bool valid = isCandidate(row, col, value);
               grid_[row][col] = value; // stores value

               if (!valid) return false; // if a clue conflicts with another puzzle is unsolvable -> returns false
           }
       }
       return true; // returns true if every clue passes
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
  
   // indicares if areas is considered white space or not
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
   -------------------------------------------------------------------------------- */

   bool isCandidate(int row, int col, int v) const {
       // Checks each row and column if the value v appears
       for (int i = 0; i < N; ++i) {
           if (grid_[row][i] == v) return false;  // row
           if (grid_[i][col] == v) return false;  // column
       }
       // finds top-left corner of cell
       int br = (row / B) * B, bc = (col / B) * B;
       // scans the block for v. If v present -> returns true
       for (int r = br; r < br + B; ++r)
           for (int c = bc; c < bc + B; ++c)
               if (grid_[r][c] == v) return false;  // block
       return true;
   }

   /* ------------------------------------------------------------------------------
   Bool findEmpty() -> Picks the empty cell with the fewest candidates (fewer branches to try)
                       and Returns false if the grid is full (no empty cells)
   Input: Sudoku puzzle
   Output: True -> if an empty cell was found
           False -> if grid full
   Created by: Baylie Stith (Created the function)
   Edited by: ChatGPT (Fixed logic errors)
   ------------------------------------------------------------------------------ */

   bool findEmpty(int& bestRow, int& bestCol) const {
       int minCandidates = N + 1; // starts with a number larger than the maximum possible candidates (9)
       bool found = false; // returns false if no empty cells found

       // visits every cell skipping full ones
       for (int row = 0; row < N; ++row) {
           for (int col = 0; col < N; ++col) {
               if (grid_[row][col] != 0) continue;
              
               // counts how many digits caan go in the cell
               int count = 0;
               for (int v = 1; v <= N; ++v)
                   if (isCandidate(row, col, v)) ++count;

               // if the cell has less options than other previous cells remember it as the best choice
               if (count < minCandidates) {
                   minCandidates = count;
                   bestRow = row;
                   bestCol = col;
                   found = true;
               }

               // No legal values means this branch is a dead end; stop looking.
               if (count == 0) return true;
           }
       }
       return found; // return if empty cell found and then choose the cell with least optons
   }

   /* ---------------------------------------------------------------------------------
   Void search() -> Recursive depth-first search with backtracking.
   Input: Sudoku puzzle
   Output: Solution(s) and how many were found
   Created by: Baylie Stith (Created the function)
   Edited by: ChatGPT (Fixed logic errors)
   --------------------------------------------------------------------------------- */

   void search() {
       int row = 0, col = 0;
       if (!findEmpty(row, col)) {  // no empty cells: a complete solution
           ++solutionCount_;
           // MODIFIED (Assignment 5): Removed "Solution k:" header printing 
           // to adhere strictly to the requirement that each solution is printed 
           // in the pure grid format followed by "Solutions found: n".
           printGrid(*out_, grid_);
           return;
       }
       // trys each value 1-9 in the chosen cell. I digit wokrs, place it, call search()
       // on the rest of the grid and set the cell back to 0 so next digit starts from
       // a clean slate
       for (int v = 1; v <= N; ++v) {
           if (isCandidate(row, col, v)) {
               grid_[row][col] = v;
               search();
               grid_[row][col] = 0;  // backtrack
           }
       }
   }

   /* ---------------------------------------------------------------------------------
   Void printGrid() -> Prints the Sudoku grid to the specified output stream.
   Input: Output stream and Sudoku grid
   Output: Formatted grid representation
   Created by: Claude (Created the function)
   Edited by: Baylie Stith (Created comments)
   --------------------------------------------------------------------------------- */

   static void printGrid(std::ostream& out, const Grid& g) { // takes output stream and grid
       // before 3 and 6 rows print a divider (horizontal)
       for (int r = 0; r < N; ++r) {
           if (r > 0 && r % B == 0) out << "------+-------+------\n";
           // print divider between columns 3 and 6 (vertical)
           for (int c = 0; c < N; ++c) {
               if (c > 0 && c % B == 0) out << "| ";
               // prints '_' for empty cell overwise turns int into char and prints a new line
               out << (g[r][c] == 0 ? '_' : char('0' + g[r][c]));
               out << (c < N - 1 ? " " : "");
           }
           out << '\n';
       }
       out << '\n';
   }
};

/* --------------------------------------------------------------------------
int main() -> Main function to run the Sudoku puzzle solver. It processes
              command-line arguments for puzzle files, loads each puzzle,
              checks for validity, and solves it.
Input: Command-line arguments for puzzle files
Output: Solved puzzle(s) and the number of solutions
Created by: Claude (Created the entire function)
Edited by: Baylie Stith (Created comments, improved error checking)
Edited by: ChatGPT (Fixed logic errors)
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

   // MODIFIED (Assignment 5): Loop through each input puzzle file, create the corresponding
   // output file (solution1.txt through solution5.txt), handle invalid states with
   // "No solution found.", and write solution grids followed by "Solutions found: n".
   for (size_t i = 0; i < files.size(); ++i) {
       const auto& name = files[i];

       // Derive output filename corresponding to input file (e.g. puzzle1.txt -> solution1.txt)
       std::string outName;
       if (name.rfind("puzzle", 0) == 0) {
           outName = "solution" + name.substr(6);
       } else {
           outName = "solution_" + name + ".txt";
       }

       std::ofstream outFile(outName);
       if (!outFile) {
           continue; // Skip if output file cannot be opened
       }

       SudokuPuzzle puzzle(name);

       // If loading fails or initial clues conflict, write exactly "No solution found."
       if (!puzzle.load() || !puzzle.isInitialGridValid()) {
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