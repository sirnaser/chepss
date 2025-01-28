#include <ctype.h>
#include <stdio.h>

#define abs(x)      (x>0? x: -x)
#define sgn(x)      (x>0? 1: -1)
#define dif_x(move) move[3]-move[1]
#define dif_y(move) move[4]-move[2]
#define dis_x(move) abs(dif_x(move))
#define dis_y(move) abs(dif_y(move))

// piece.status
#define DEAD                0
#define DID_NOT_MOVE_YET    1
#define MOVED_ONCE_BEFORE   2

// pice.owner
#define BLACK   0
#define WHITE   1

// piece.type
#define NONE    0
#define PAWN    1
#define KNIGHT  2
#define BISHOP  3
#define ROOK    4
#define QUEEN   5
#define KING    6


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


int Term;
char Moves[1000][6];
player Players[2];
piece* Board[8][8];
piece* p_EmptySquare = &(piece){.status=0, .type=0, .symbol=" "};


static const char PIECE_SYMBOLS[2][7][5] = {
    {" ", "♙", "♘", "♗", "♖", "♕", "♔"},
    {" ", "♟", "♞", "♝", "♜", "♛", "♚"}
};
static const char MOVE_SYMBOLS[] = " PNBRQK";


void set_board(char* pos, piece* p){
    Board[pos[0]-'a'][pos[1]-'1'] = p;
}

piece* get_board(char* pos){
    return Board[pos[0]-'a'][pos[1]-'1'];
}


// validation
char not_pawn_move(char* move){
    if (dif_x(move)) return 1;
    if (get_board(move+1)->status == DID_NOT_MOVE_YET){
        if ((Term&1) && dif_y(move) <= 0) return 2;
        if (!(Term&1) && dif_y(move) >= 0) return 3;
        if (dis_y(move) > 2) return 4;
    } else {
        if (dis_y(move) > 1) return 5;
    }

    char sorc[2] = {move[1], move[2]}, dist[2] = {move[3], move[4]};
    char dirc_x = sign(dif_x(move)), dirc_y = sign(dif_y(move));
    return 0;
}

char not_knight_move(char* move){
    if (!(dis_x(move) == 2 && dis_y(move) == 3) 
     && !(dif_x(move) == 3 && dis_y(move) == 2)) return 1;
    return 0;
}

char not_bishop_move(char* move){
    if (dis_x(move) != dis_y(move)) return 1;
    if (move[1] <= 'd'){
        if (dif_x(move) <= 0) return 2;
    } else {
        if (dif_x(move) >= 0) return 3;
    }
    // if (move[2] <= '4'){
    //     if (dif_y(move) <= 0) return 4;
    // } else {
    //     if (dif_y(move) >= 0) return 5;
    // }
    return 0;
}

char not_rook_move(char* move){
    return 0;
}

char not_queen_move(char* move){
    return 0;
}

char not_king_move(char* move){
    if (abs(move[1]-move[3]) > 1) return 1;
    if (abs(move[2]-move[4]) > 1) return 2;
    return 0;

    return (abs(move[1]-move[3]) > 1)
        || (abs(move[2]-move[4]) > 1);
}

char not_in_board(char* pos){
    if (pos[0] < 'a') return 1;
    if ('h' < pos[0]) return 2;
    if (pos[1] < '1') return 3;
    if ('8' < pos[1]) return 4;
    return 0;

    return pos[0] < 'a' || 'h' < pos[0] 
        || pos[1] < '1' || '8' < pos[1];
}

char not_valid_move(char* move){
    if (not_in_board(move+1)) return 1;
    if (not_in_board(move+3)) return 2;
    if (get_board(move+1)->status == DEAD) return 3;
    if (get_board(move+1)->owner != (Term&1)) return 4;
    if (get_board(move+3)->owner == (Term&1)) return 5;
    
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
    move[3] = Players[(Term&1)].pieces[0].pos[0];
    move[4] = Players[(Term&1)].pieces[0].pos[1];

    for (piece* p_pc=Players[!(Term&1)].pieces; p_pc<=Players[!(Term&1)].pieces+15; ++p_pc){
        move[0] = MOVE_SYMBOLS[p_pc->type];
        move[1] = p_pc->pos[0];
        move[2] = p_pc->pos[1];
        
        if (!not_valid_move(move)){
            return p_pc-Players[!(Term&1)].pieces;
        }
    }

    return 0;
}

char not_possible_move(char* move){
    char errorCode;
    if ((errorCode=not_valid_move(move))) return errorCode;

    piece* p_pc1 = get_board(move+1);
    piece* p_pc2 = get_board(move+3);
    
    piece pc1 = *p_pc1;
    piece pc2 = *p_pc2;

    set_board(move+1, p_EmptySquare);
    set_board(move+3, p_pc1);

    p_pc1->pos[0] = move[3]; p_pc1->pos[1] = move[4]; 
    p_pc1->status = MOVED_ONCE_BEFORE;
    p_pc2->status = DEAD;


    char isChecked = is_checked();


    *p_pc1 = pc1;
    *p_pc2 = pc2;

    set_board(move+1, p_pc1);
    set_board(move+3, p_pc2);

    if (isChecked) return isChecked+13;
    return 0;
}

char any_possible_move(){
    char move[6];

    for (piece* p_pc=Players[(Term&1)].pieces; p_pc<=Players[(Term&1)].pieces+15; ++p_pc){
        move[0] = MOVE_SYMBOLS[p_pc->type];
        move[1] = p_pc->pos[0];
        move[2] = p_pc->pos[1];
        
        for (move[4]='1';  move[4]<='8'; ++move[4]){
            for (move[3]='a';  move[3]<='h'; ++move[3]){
                if (!not_possible_move(move)){
                    return 1;
                }
            }
        }
    }

    return 0;
}



void reset_game(){
    Term = 1;

    Players[0] = (player){
        {
            {DID_NOT_MOVE_YET,  BLACK, KING,   "e8", "♔"}, 
            {DID_NOT_MOVE_YET,  BLACK, QUEEN,  "d8", "♕"}, 
            {DID_NOT_MOVE_YET,  BLACK, ROOK,   "a8", "♖"}, 
            {DID_NOT_MOVE_YET,  BLACK, ROOK,   "h8", "♖"}, 
            {DID_NOT_MOVE_YET,  BLACK, BISHOP, "c8", "♙"}, 
            {DID_NOT_MOVE_YET,  BLACK, BISHOP, "f8", "♙"}, 
            {DID_NOT_MOVE_YET,  BLACK, KNIGHT, "b8", "♘"}, 
            {DID_NOT_MOVE_YET,  BLACK, KNIGHT, "g8", "♘"}, 
            {DID_NOT_MOVE_YET,  BLACK, PAWN,   "a7", "♙"}, 
            {DID_NOT_MOVE_YET,  BLACK, PAWN,   "b7", "♙"}, 
            {DID_NOT_MOVE_YET, BLACK, PAWN,   "c7", "♙"}, 
            {DID_NOT_MOVE_YET, BLACK, PAWN,   "d7", "♙"}, 
            {DID_NOT_MOVE_YET, BLACK, PAWN,   "e7", "♙"}, 
            {DID_NOT_MOVE_YET, BLACK, PAWN,   "f7", "♙"}, 
            {DID_NOT_MOVE_YET, BLACK, PAWN,   "g7", "♙"}, 
            {DID_NOT_MOVE_YET, BLACK, PAWN,   "h7", "♙"}
        }
    };

    Players[1] = (player){
        {
            {DID_NOT_MOVE_YET,  WHITE, KING,   "e1", "♚"}, 
            {DID_NOT_MOVE_YET,  WHITE, QUEEN,  "d1", "♛"}, 
            {DID_NOT_MOVE_YET,  WHITE, ROOK,   "a1", "♜"}, 
            {DID_NOT_MOVE_YET,  WHITE, ROOK,   "h1", "♜"}, 
            {DID_NOT_MOVE_YET,  WHITE, BISHOP, "c1", "♝"}, 
            {DID_NOT_MOVE_YET,  WHITE, BISHOP, "f1", "♝"}, 
            {DID_NOT_MOVE_YET,  WHITE, KNIGHT, "b1", "♞"}, 
            {DID_NOT_MOVE_YET,  WHITE, KNIGHT, "g1", "♞"}, 
            {DID_NOT_MOVE_YET,  WHITE, PAWN,   "a2", "♟"}, 
            {DID_NOT_MOVE_YET,  WHITE, PAWN,   "b2", "♟"}, 
            {DID_NOT_MOVE_YET, WHITE, PAWN,   "c2", "♟"}, 
            {DID_NOT_MOVE_YET, WHITE, PAWN,   "d2", "♟"}, 
            {DID_NOT_MOVE_YET, WHITE, PAWN,   "e2", "♟"}, 
            {DID_NOT_MOVE_YET, WHITE, PAWN,   "f2", "♟"}, 
            {DID_NOT_MOVE_YET, WHITE, PAWN,   "g2", "♟"}, 
            {DID_NOT_MOVE_YET, WHITE, PAWN,   "h2", "♟"}
        }
    };

    for (char pl=0; pl<2; ++pl){
        for (piece* p_pc=Players[pl].pieces; p_pc<=Players[pl].pieces+15; ++p_pc){
            set_board(p_pc->pos, p_pc);
        }
    }
    for (char pos[3]="a3";  pos[1]<='6'; ++pos[1]){
        for (pos[0]='a';  pos[0]<='h'; ++pos[0]){
            set_board(pos, p_EmptySquare);
        }
    }
}

void do_move(char* move){
    piece* p_pc1 = get_board(move+1);
    piece* p_pc2 = get_board(move+3);
    
    set_board(move+1, p_EmptySquare);
    set_board(move+3, p_pc1);

    p_pc1->pos[0] = move[3]; p_pc1->pos[1] = move[4];
    p_pc1->status = MOVED_ONCE_BEFORE;
    p_pc2->status = DEAD;

    if (*move == 'P' && (move[4] == '1' || move[4] == '8')){ // pawn in end of board
        for (char tp=2; tp<6; ++tp)
            printf("%d: %s, ", tp, PIECE_SYMBOLS[(Term&1)][tp]);
        printf("\b\b \n");

        char type=0;
        while (type<2 || 5<type){
            printf("which pice type do you prefer?");
            scanf("%d", &type);
        }

        piece* thePawn = get_board(move+3);
        thePawn->type = type;
        thePawn->symbol[0] = PIECE_SYMBOLS[(Term&1)][type][0];
        thePawn->symbol[1] = PIECE_SYMBOLS[(Term&1)][type][1];
        thePawn->symbol[2] = PIECE_SYMBOLS[(Term&1)][type][2];
    }
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
            printf(" %s ", get_board(pos)->symbol);
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

        printf("%i.%i> ", (Term+1)/2, !(Term&1));
        scanf("%s", move);

        if ('1' <= move[1] && move[1] <= '8'){
            for (char i=3; i >= 0; --i) move[i+1] = move[i];
            move[0] = MOVE_SYMBOLS[get_board(move+1)->type];
        }
        *move = toupper(*move);
        move[5] = '\0';

        char errorCode;
        if ((errorCode=not_possible_move(move))){
            printf("error code: %i\n", errorCode);
            continue;
        }


        do_move(move);
        Term++;


        if (any_possible_move()) continue;

        if (is_checked()){
            printf("checkmate\n");
        } else {
            printf("stalemate\n");
        }
        break;
    }

    return 0;
}


// naming conventions:

// https://stackoverflow.com/questions/1722112/what-are-the-most-common-naming-conventions-in-c

// Trivial Variables: i,n,c,etc...
// Local Variables: camelCase
// Global Variables: PascalCase
// Const Variables: ALL_CAPS
// Pointer Variables: add a p_ to the prefix. 
// Structs: one word? lower case: PascalCase
// Struct Member Variables: camelCase
// Enums: like Structs
// Enum Values: ALL_CAPS
// Functions: snake_case
// Macros: snake_case





// is a valid move
// is valid move
// not a valid move
// not valid move



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
