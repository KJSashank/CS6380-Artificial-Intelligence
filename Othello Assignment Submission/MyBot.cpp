/*
* @file botTemplate.cpp
* @author Arun Tejasvi Chaganty <arunchaganty@gmail.com>
* @date 2010-02-04
* Template for users to create their own bots
*/

/* Author - EE19B137 K JOTHIR SASHANK
Date - 28 Nov 2021
Othello Playing Bot Assignment - CS6380
This code is an edit of the problem statement code with the addition of alhpa beta pruning algorithm and an evalution function given below
*/

#include "Othello.h"
#include "OthelloBoard.h"
#include "OthelloPlayer.h"
#include <cstdlib>
#include <ctime>
#include <list>
using namespace std;
using namespace Desdemona;

OthelloBoard univ_board;
clock_t clk_begin,clk_end;
Turn player_colour;
#define INF 1e18

// Evaluation function is derived from multiple sources: kartikkukreja github, Washington DC research paper
double evaluation_fcn(OthelloBoard board)  {
    char grid[8][8];
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
        Coin findTurn = board.get(i,j);
        if(findTurn == player_colour) grid[i][j] = 'x';
        else if(findTurn == other(player_colour)) grid[i][j] = 'o';
        else grid[i][j] = '-';
        }
    }
    char opp_colour = 'o',bot_colour = 'x';
    double m = 0.0,cc = 0.0, ss = 0.0, c = 0.0, ce = 0.0, v = 0.0; // Components of evaluation function

    // Corner occupancy (c)
    int player_corners = 0;
    int opp_corners = 0;
    if(grid[0][0] == bot_colour) player_corners++;
    else if(grid[0][0] == opp_colour) opp_corners++;
    if(grid[0][7] == bot_colour) player_corners++;
    else if(grid[0][7] == opp_colour) opp_corners++;
    if(grid[7][0] == bot_colour) player_corners++;
    else if(grid[7][0] == opp_colour) opp_corners++;
    if(grid[7][7] == bot_colour) player_corners++;
    else if(grid[7][7] == opp_colour) opp_corners++;
    c = 25*850.724 * (player_corners - opp_corners);

    // Corner closeness (ce)
    int player_corneredge = 0;
    int opp_corneredge = 0;
    if(grid[0][0] == '-')   {
        if(grid[0][1] == bot_colour) player_corneredge++;
        else if(grid[0][1] == opp_colour) opp_corneredge++;
        if(grid[1][1] == bot_colour) player_corneredge++;
        else if(grid[1][1] == opp_colour) opp_corneredge++;
        if(grid[1][0] == bot_colour) player_corneredge++;
        else if(grid[1][0] == opp_colour) opp_corneredge++;
    }
    if(grid[0][7] == '-')   {
        if(grid[0][6] == bot_colour) player_corneredge++;
        else if(grid[0][6] == opp_colour) opp_corneredge++;
        if(grid[1][6] == bot_colour) player_corneredge++;
        else if(grid[1][6] == opp_colour) opp_corneredge++;
        if(grid[1][7] == bot_colour) player_corneredge++;
        else if(grid[1][7] == opp_colour) opp_corneredge++;
    }
    if(grid[7][0] == '-')   {
        if(grid[7][1] == bot_colour) player_corneredge++;
        else if(grid[7][1] == opp_colour) opp_corneredge++;
        if(grid[6][1] == bot_colour) player_corneredge++;
        else if(grid[6][1] == opp_colour) opp_corneredge++;
        if(grid[6][0] == bot_colour) player_corneredge++;
        else if(grid[6][0] == opp_colour) opp_corneredge++;
    }
    if(grid[7][7] == '-')   {
        if(grid[6][7] == bot_colour) player_corneredge++;
        else if(grid[6][7] == opp_colour) opp_corneredge++;
        if(grid[6][6] == bot_colour) player_corneredge++;
        else if(grid[6][6] == opp_colour) opp_corneredge++;
        if(grid[7][6] == bot_colour) player_corneredge++;
        else if(grid[7][6] == opp_colour) opp_corneredge++;
    }
    ce = -3820.26 * (player_corneredge - opp_corneredge);

    // Stability & Coin count
    int player_squares = 0, opp_squares = 0, i, j, k, player_vulnerability = 0, opp_vulnerability = 0, x, y;

    int A[] = {-1, -1, 0, 1, 1, 1, 0, -1};                         // Used to scout the 9 neighbouring squares of a given square
    int B[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int V[8][8] = 	{ { 20, -3, 11, 8, 8, 11, -3, 20 },             // Assign values to each square of the board according to their stability
    				{ -3, -7, -4, 1, 1, -4, -7, -3 },
    				{ 11, -4, 2, 2, 2, 2, -4, 11 },
    				{ 8, 1, 2, -3, -3, 2, 1, 8 },
    				{ 8, 1, 2, -3, -3, 2, 1, 8 },
    				{ 11, -4, 2, 2, 2, 2, -4, 11 },
    				{ -3, -7, -4, 1, 1, -4, -7, -3 },
    				{ 20, -3, 11, 8, 8, 11, -3, 20 } };

    for(i = 0; i < 8; i++)
        for(j = 0; j < 8; j++)  {
            if(grid[i][j] == bot_colour)  {                        // Finding the stability value wrt the V matrix given above - static stability (ss)
                ss += 10*V[i][j];
                player_squares++;
            } 
            else if(grid[i][j] == opp_colour)  {
                ss -= 10*V[i][j];
                opp_squares++;
            }

            //  Check how many neighbouring squares exist for the opponent where he can turn around the given coin 
            if(grid[i][j] != '-')   {                       
                for(k = 0; k < 8; k++)  {
                    x = i + A[k]; y = j + B[k];
                    if(x >= 0 && x < 8 && y >= 0 && y < 8 && grid[x][y] == '-') {
                        if(grid[i][j] == bot_colour)  player_vulnerability++;
                        else opp_vulnerability++;
                        break;
                    }
                }
            }
        }

    // Vulnerability (v)
    if(player_vulnerability > opp_vulnerability) v = -(7839.6 * player_vulnerability)/(player_vulnerability + opp_vulnerability);
    else if(player_vulnerability < opp_vulnerability) v = (7839.6 * opp_vulnerability)/(player_vulnerability + opp_vulnerability);

    // Total Coin count (cc)
    if(player_squares > opp_squares) cc = (1190.0 * player_squares)/(player_squares + opp_squares);
    else if(player_squares < opp_squares) cc = -(1190.0 * opp_squares)/(player_squares + opp_squares);

    // Mobility (m)
    int player_moves = board.getValidMoves(player_colour).size();
    int opp_moves = board.getValidMoves(other(player_colour)).size();
    if(player_moves > opp_moves) m = (8692.2 * player_moves)/(player_moves + opp_moves);
    else if(player_moves < opp_moves) m = -8692.2 *(opp_moves)/(player_moves + opp_moves);

    double evaluation_value = 0;
    // final weighted evaluation_value
    evaluation_value = cc + c + ce + m + v + ss;
    return evaluation_value;
}

// Aplha beta algorithm
double Alphabeta(OthelloBoard board, Move move,Turn turn, int k, double alpha, double beta) {
    clk_end = clock();
    int depth;
    depth = 6;
    if(((double)(clk_end-clk_begin)/CLOCKS_PER_SEC)>1.95) {
        if(turn == player_colour) return -INF;
        return INF;
    }
	if(k == depth) {
		return evaluation_fcn(board);
	}
	board.makeMove(turn,move);
	turn = other(turn);
	list<Move> moves = board.getValidMoves(turn);
	list<Move>::iterator move_it = moves.begin();
    if((moves.size())==0) return evaluation_fcn(board);
    if(turn==other(player_colour)) {
        for(;move_it!=moves.end();move_it++) {
            beta = min(beta,Alphabeta(board,*move_it,turn,k+1,alpha,beta));
            if(alpha >= beta) return alpha;
        }
        return beta;
    }
    else {
        for(;move_it!=moves.end();move_it++) {
            alpha = max(alpha,Alphabeta(board,*move_it,turn,k+1,alpha,beta));
            if(alpha >= beta) return beta;
        }
        return alpha;
    }
}

class MyBot: public OthelloPlayer
{
    public:
        /**
         * Initialisation routines here
         * This could do anything from open up a cache of "best moves" to
         * spawning a background processing thread. 
         */
        Turn turn;
        MyBot( Turn turn );

        /**
         * Play something 
         */
        virtual Move play( const OthelloBoard& board );
    private:
};

MyBot::MyBot( Turn turn )
    : OthelloPlayer( turn )
{
    this->turn=turn;
}

Move MyBot::play( const OthelloBoard& board )
{
    clk_begin = clock();
    list<Move> moves = board.getValidMoves(turn);
    player_colour = turn;  
    list<Move>::iterator move_it = moves.begin();
    Move bestMove((*move_it).x,(*move_it).y);
    double old_value = -INF;
    OthelloBoard temp = board;
    for(;move_it!=moves.end();move_it++) {
    	double new_value = Alphabeta(temp,*move_it,player_colour,1,-INF,INF);
    	if(new_value > old_value) {
    		old_value = new_value;
    		bestMove = *move_it;
    	}
    	temp = board;
    }
    return bestMove;
}

// The following lines are _very_ important to create a bot module for Desdemona

extern "C" {
    OthelloPlayer* createBot( Turn turn )
    {
        return new MyBot( turn );
    }

    void destroyBot( OthelloPlayer* bot )
    {
        delete bot;
    }
}
