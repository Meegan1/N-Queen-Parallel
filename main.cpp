#include <iostream>
#include "Solver.h"

typedef int chessboard;

/* Finds the number of solutions to the N-queen problem
 * ld: a bit pattern containing ones for positions under attack along left diagonals for this row
 * cols: a bit pattern containing ones for columns that are already accupied
 * rd: a bit pattern containing ones for positions under attack along right diagonals for this row
 * all: a bit pattern where the first N bits are set to one, where N is the number of queens
 *
 * ld, cols, and rd contain sufficient info about the current state of the board.
 * (ld | cols | rd) is a bit pattern containing ones in all positions under attack for this row
 */
int seq_nqueen(chessboard ld, chessboard cols, chessboard rd, const chessboard all) {
    int sol = 0;

    if (cols == all)                            // A solution is found
        return 1;

    chessboard pos = ~(ld | cols | rd) & all;  // Possible posstions for the queen on the current row
    chessboard next;
    while (pos !=
           0) {                          // Iterate over all possible positions and solve the (N-1)-queen in each case
        next = pos & (-pos);                    // next possible position
        pos -= next;                             // update the possible position
        sol += seq_nqueen((ld | next) << 1, cols | next, (rd | next) >> 1,
                          all); // recursive call for the `next' position
    }
    return sol;
}

int nqueen_solver(int n) {
    chessboard all = (1 << n) - 1;            // set N bits on, representing number of columns
    return seq_nqueen(0, 0, 0, all);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "You have to provide: \n 1) Number of Queens \n 2) Number of threads" << std::endl;
        return 0;
    }

    int qn = std::stoi(argv[1]);
    int tn = std::stoi(argv[2]);



    time_t start_time;
    time_t end_time;
    time(&start_time);

    int sol = nqueen_solver(qn);

    time(&end_time);
    std::cout << "Total Time Elapsed: " << end_time - start_time << "s" << std::endl;

    std::cout << "Number of Solutions to " << qn << "-Queen problem is : " << sol << std::endl;

    Solver solver(tn);
    solver.solve(qn);

    return 0;
}