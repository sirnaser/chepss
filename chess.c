#include <ctype.h>
#include <stdio.h>

#define abs(x)      (x > 0? x: -1)

// piece status
#define dead            0
#define didntMoveYet    1
#define movedOnceBefore 2

// pice owner
#define black   0
#define white   1

// piece type
#define none    0
#define pawn    1
#define knight  2
#define bishop  3
#define rook    4
#define queen   5
#define king    6


typedef struct{
    char status;
        // 0: dead, 
        // 1: didn't move yet
    char owner;
        // 0: black, 
        // 1: white
    char type;   
        // 0: empty square,
        // 1: pawn,
        // 2: knight,
        // 3: bishop,
        // 4: rook,
        // 5: queen,
        // 6: king
    char pos[3];    // ex: a1
    char symbol[5]; // ♔♕♗♘♙♖♚♛♝♞♟♜
} piece;

typedef struct{
    piece pieces[16];
    // {king, queen, rook*2, bishop*2, knight*2, pawn*8}
} player;


int term;
char moves[6][1000];
player players[2];
piece* board[8][8];
piece* emptySquare = &(piece){.status=0, .type=0, .symbol=" "};

char standardSymbols[] = " PNBRQK";


void set_piece(char* pos, piece* p){
    board[pos[0]-'a'][pos[1]-'1'] = p;
}

piece* get_piece(char* pos){
    return board[pos[0]-'a'][pos[1]-'1'];
}


// validation
char not_pawn_move(char* move){
    return (move[1] != move[3]) 
        || ((term&1) && move[2] >= move[4])
        || (!(term&1) && move[2] <= move[4])
        || (abs(move[2]-move[4]) > 2)
        || (abs(move[2]-move[4]) > 1 && get_piece(move+1)->status != didntMoveYet);
}

char not_knight_move(char* move){
    return 0;
}

char not_bishop_move(char* move){
    return 0;
}

char not_rook_move(char* move){
    return 0;
}

char not_queen_move(char* move){
    return 0;
}

char not_king_move(char* move){
    return (abs(move[1]-move[3]) > 1)
        || (abs(move[2]-move[4]) > 1);
}

char not_in_board(char* pos){
    return pos[0] < 'a' || 'h' < pos[0] 
        || pos[1] < '1' || '8' < pos[1];
}

char not_valid_move(char* move){
    if (not_in_board(move+1)) return 1;
    if (not_in_board(move+3)) return 2;
    if (get_piece(move+1)->status == dead) return 3;
    if (get_piece(move+1)->owner != (term&1)) return 4;
    if (get_piece(move+3)->owner == (term&1)) return 5;
    
    switch (*move) {
        case 'K':
            if (not_king_move(move+1)) return 6;
            break;
        case 'Q':
            if (not_queen_move(move+1)) return 7;
            break;
        case 'R':
            if (not_rook_move(move+1)) return 8;
            break;
        case 'B':
            if (not_bishop_move(move+1)) return 9;
            break;
        case 'N':
            if (not_knight_move(move+1)) return 10;
            break;
        case 'P':
            if (not_pawn_move(move+1)) return 11;
            break;
        default:
            return 12;
    }

    return 0;
}

char is_checked(){
    char move[6];
    move[3] = players[(term&1)].pieces[0].pos[0];
    move[4] = players[(term&1)].pieces[0].pos[1];

    piece* somePiece;
    for (somePiece=players[!(term&1)].pieces; somePiece<=players[!(term&1)].pieces+15; ++somePiece){
        move[0] = standardSymbols[somePiece->type];
        move[1] = somePiece->pos[0];
        move[2] = somePiece->pos[1];
        
        if (!not_valid_move(move)){
            return somePiece-players[!(term&1)].pieces;
        }
    }

    return 0;
}

char not_possible_move(char* move){
    char errorCode;
    if ((errorCode=not_valid_move(move))) return errorCode;

    piece* pcpt1 = get_piece(move+1);
    piece* pcpt2 = get_piece(move+3);
    
    piece pc1 = *pcpt1;
    piece pc2 = *pcpt2;

    set_piece(move+1, emptySquare);
    set_piece(move+3, pcpt1);

    pcpt1->pos[0] = move[3]; pcpt1->pos[1] = move[4]; 
    pcpt1->status = movedOnceBefore;
    pcpt2->status = dead;


    char isChecked = is_checked();


    *pcpt1 = pc1;
    *pcpt2 = pc2;

    set_piece(move+1, pcpt1);
    set_piece(move+3, pcpt2);

    if (isChecked) return isChecked+13;
    return 0;
}



void reset_game(){
    term = 1;

    players[0] = (player){
        {
            {didntMoveYet,  black, king,   "e8", "♔"}, 
            {didntMoveYet,  black, queen,  "d8", "♕"}, 
            {didntMoveYet,  black, rook,   "a8", "♖"}, 
            {didntMoveYet,  black, rook,   "h8", "♖"}, 
            {didntMoveYet,  black, bishop, "c8", "♙"}, 
            {didntMoveYet,  black, bishop, "f8", "♙"}, 
            {didntMoveYet,  black, knight, "b8", "♘"}, 
            {didntMoveYet,  black, knight, "g8", "♘"}, 
            {didntMoveYet,  black, pawn,   "a7", "♙"}, 
            {didntMoveYet,  black, pawn,   "b7", "♙"}, 
            {didntMoveYet, black, pawn,   "c7", "♙"}, 
            {didntMoveYet, black, pawn,   "d7", "♙"}, 
            {didntMoveYet, black, pawn,   "e7", "♙"}, 
            {didntMoveYet, black, pawn,   "f7", "♙"}, 
            {didntMoveYet, black, pawn,   "g7", "♙"}, 
            {didntMoveYet, black, pawn,   "h7", "♙"}
        }
    };

    players[1] = (player){
        {
            {didntMoveYet,  white, king,   "e1", "♚"}, 
            {didntMoveYet,  white, queen,  "d1", "♛"}, 
            {didntMoveYet,  white, rook,   "a1", "♜"}, 
            {didntMoveYet,  white, rook,   "h1", "♜"}, 
            {didntMoveYet,  white, bishop, "c1", "♝"}, 
            {didntMoveYet,  white, bishop, "f1", "♝"}, 
            {didntMoveYet,  white, knight, "b1", "♞"}, 
            {didntMoveYet,  white, knight, "g1", "♞"}, 
            {didntMoveYet,  white, pawn,   "a2", "♟"}, 
            {didntMoveYet,  white, pawn,   "b2", "♟"}, 
            {didntMoveYet, white, pawn,   "c2", "♟"}, 
            {didntMoveYet, white, pawn,   "d2", "♟"}, 
            {didntMoveYet, white, pawn,   "e2", "♟"}, 
            {didntMoveYet, white, pawn,   "f2", "♟"}, 
            {didntMoveYet, white, pawn,   "g2", "♟"}, 
            {didntMoveYet, white, pawn,   "h2", "♟"}
        }
    };

    piece* somePiece;
    for (char pl=0; pl<2; ++pl){
        for (somePiece=players[pl].pieces; somePiece<=players[pl].pieces+15; ++somePiece){
            set_piece(somePiece->pos, somePiece);
        }
    }
    for (char pos[3]="a3";  pos[1]<='6'; ++pos[1]){
        for (pos[0]='a';  pos[0]<='h'; ++pos[0]){
            set_piece(pos, emptySquare);
        }
    }
}

void do_move(char* move){
    piece* pcpt1 = get_piece(move+1);
    piece* pcpt2 = get_piece(move+3);
    
    set_piece(move+1, emptySquare);
    set_piece(move+3, pcpt1);

    pcpt1->pos[0] = move[3]; pcpt1->pos[1] = move[4];
    pcpt1->status = movedOnceBefore;
    pcpt2->status = dead;
}

void print_pos(int x, int y, char* str){
    printf("e[1;1H");
}

void display(){
    printf("\e[37;40m\e[1;1H\e[2J");

    for (char pos[3]="a8"; pos[1]>='1'; --pos[1]){
        printf( "\e[40m%c ", pos[1]);
        for (pos[0]='a'; pos[0]<='h'; ++pos[0]){
            printf((pos[0]&1)^(pos[1]&1)? "\e[100m": "\e[40m");
            printf(" %s ", get_piece(pos)->symbol);
        }
        printf("\n");
    }
    printf( "\e[40m   a  b  c  d  e  f  g  h \n");
}

void goto_n(int n){

}

int main()
{
    // ♔♕♗♘♙♖♚♛♝♞♟♜

    reset_game();

    char move[20];
    while (1){
        display();

        printf("%i.%i> ", (term+1)/2, !(term&1));
        scanf("%s", move);

        if ('1' <= move[1] && move[1] <= '8'){
            for (char i=3; i >= 0; --i) move[i+1] = move[i];
            move[0] = standardSymbols[get_piece(move+1)->type];
        }
        *move = toupper(*move);
        move[5] = '\0';

        char errorCode;
        if ((errorCode=not_possible_move(move))){
            printf("error code: %i\n", errorCode);
            continue;
        }

        do_move(move);
        if (*move == 'P' && (move[4] == '1' || move[4] == '8')){ // pawn in end of board
            char type;
            printf("which type do you prefer?");
            scanf("%d", &type);
            // transform
        }
        term++;

        char thereIsAPossibleMove = 0;
        piece* somePiece;
        for (somePiece=players[(term&1)].pieces; somePiece<=players[(term&1)].pieces+15; ++somePiece){
            if (thereIsAPossibleMove) break;
            move[0] = standardSymbols[somePiece->type];
            move[1] = somePiece->pos[0];
            move[2] = somePiece->pos[1];
            
            for (move[4]='1';  move[4]<='8'; ++move[4]){
                if (thereIsAPossibleMove) break;
                for (move[3]='a';  move[3]<='h'; ++move[3]){
                    if (!not_possible_move(move)){
                        thereIsAPossibleMove = 1;
                        break;
                    }
                }
            }
        }

        if (!thereIsAPossibleMove){
            if (is_checked()){
                printf("checkmate\n");
            } else {
                printf("stalemate\n");
            }
            break;
        }
    }

    return 0;
}

// TODO:
// check valid moves
// check end of game
// do moves and turn the term
// pawn in end of board
// display borderd board
// display last move

// display dead pieces
// show all valid moves
// invalid move handling
// shah ghalae
// simple move format
// display moves
// save and restore moves
// iterate in moves sequence
