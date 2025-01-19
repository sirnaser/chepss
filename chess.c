#include <stdio.h>
#include <string.h>
#include <wchar.h>

// typedef enum{
//     white,
//     black
// } color;

// typedef enum{
//     checked,
//     stable
// } playerStatus;

// typedef enum{
//     dead,
//     didntMoveYet
// } pieceStatus;

// typedef enum{
//     none,
//     pawn,
//     knight,
//     bishop,
//     rook,
//     queen,
//     king
// } pieceType;


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
    // char status;
        // 0: checked
    piece pieces[16];
    // {king, queen, rook*2, bishop*2, knight*2, pawn*8}
} player;


int numOfMoves, turn;
char moves[6][1000];
player players[2];
piece* board[8][8];
piece* emptySquare = &(piece){.status=0, .type=0, .symbol=" "};



void set_piece(char* pos, piece* p){
    board[pos[0]-'a'][pos[1]-'1'] = p;
}

piece* get_piece(char* pos){
    return board[pos[0]-'a'][pos[1]-'1'];
}

void reset_game(){
    players[0] = (player){
        {
            {
                1,
                0,
                6,
                "e8",
                "♔"
            }, 
            {
                1,
                0,
                5,
                "d8",
                "♕"
            }, 
            {
                1,
                0,
                4,
                "a8",
                "♖"
            }, 
            {
                1,
                0,
                4,
                "h8",
                "♖"
            }, 
            {
                1,
                0,
                3,
                "c8",
                "♙"
            }, 
            {
                1,
                0,
                3,
                "f8",
                "♙"
            }, 
            {
                1,
                0,
                2,
                "b8",
                "♘"
            }, 
            {
                1,
                0,
                2,
                "g8",
                "♘"
            }, 
            {
                1,
                0,
                1,
                "a7",
                "♙"
            }, 
            {
                1,
                0,
                1,
                "b7",
                "♙"
            }, 
            {
                1,
                0,
                1,
                "c7",
                "♙"
            }, 
            {
                1,
                0,
                1,
                "d7",
                "♙"
            }, 
            {
                1,
                0,
                1,
                "e7",
                "♙"
            }, 
            {
                1,
                0,
                1,
                "f7",
                "♙"
            }, 
            {
                1,
                0,
                1,
                "g7",
                "♙"
            }, 
            {
                1,
                0,
                1,
                "h7",
                "♙"
            }
        }
    };

    players[1] = (player){
        {
            {
                1,
                1,
                6,
                "e1",
                "♚"
            }, 
            {
                1,
                1,
                5,
                "d1",
                "♛"
            }, 
            {
                1,
                1,
                4,
                "a1",
                "♜"
            }, 
            {
                1,
                1,
                4,
                "h1",
                "♜"
            }, 
            {
                1,
                1,
                3,
                "c1",
                "♝"
            }, 
            {
                1,
                1,
                3,
                "f1",
                "♝"
            }, 
            {
                1,
                1,
                2,
                "b1",
                "♞"
            }, 
            {
                1,
                1,
                2,
                "g1",
                "♞"
            }, 
            {
                1,
                1,
                1,
                "a2",
                "♟"
            }, 
            {
                1,
                1,
                1,
                "b2",
                "♟"
            }, 
            {
                1,
                1,
                1,
                "c2",
                "♟"
            }, 
            {
                1,
                1,
                1,
                "d2",
                "♟"
            }, 
            {
                1,
                1,
                1,
                "e2",
                "♟"
            }, 
            {
                1,
                1,
                1,
                "f2",
                "♟"
            }, 
            {
                1,
                1,
                1,
                "g2",
                "♟"
            }, 
            {
                1,
                1,
                1,
                "h2",
                "♟"
            }
        }
    };

    turn = 1;
    numOfMoves = 0;

    piece* somePiece;
    for (char pc=0; pc<16; ++pc){
        for (char pl=0; pl<2; ++pl){
            somePiece = players[pl].pieces+pc;
            set_piece(somePiece->pos, somePiece);
        }
    }
    for (char pos[3]="a3";  pos[1]<='6'; ++pos[1]){
        for (pos[0]='a';  pos[0]<='h'; ++pos[0]){
            set_piece(pos, emptySquare);
        }
    }
}

void display(){
    // reset color and clear the terminal
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

// validation
// char is_king_move(char* move){
char not_king_move(char* move){
    return 0;
}

char not_queen_move(char* move){
    return 0;
}

char not_rook_move(char* move){
    return 0;
}

char not_bishop_move(char* move){
    return 0;
}

char not_knight_move(char* move){
    return 0;
}

char not_pawn_move(char* move){
    return 0;
}

char not_in_board(char* pos){
    if (pos[0] < 'a' || 'h' < pos[0]){
        return 1;
    }
    if (pos[1] < '1' || '8' < pos[1]){
        return 2;
    }
    return 0;
}

char is_checked(){
    
}

void do_move(char* move){
    piece* p1 = get_piece(move+1);
    piece* p2 = get_piece(move+3);
    
    set_piece(move+3, p1);
    strcpy(p1->pos, move+3);

    p1->status = 2; // moved once before
    p2->status = 0; // dead
}

char not_valid_move(char* move){
    if (not_in_board(move+1)) return not_in_board(move+1);
    if (not_in_board(move+3)) return 2+not_in_board(move+3);
    if (get_piece(move+1)->status == 0) return 5;
    if (get_piece(move+1)->owner != turn) return 6;
    if (get_piece(move+3)->owner == turn) return 7;
    
    switch (*move) {
        case 'K':
            if (not_king_move(move+1)) return 8;
            break;
        case 'Q':
            if (not_queen_move(move+1)) return 9;
            break;
        case 'R':
            if (not_rook_move(move+1)) return 10;
            break;
        case 'B':
            if (not_bishop_move(move+1)) return 11;
            break;
        case 'N':
            if (not_knight_move(move+1)) return 12;
            break;
        case 'P':
            if (not_pawn_move(move+1)) return 13;
            break;
        default:
            return 14;
    }

    piece* p1 = get_piece(move+1);
    piece* p2 = get_piece(move+3);

    // checkmate detection
    if (p2->type == 6) return 15; // king

    piece lastP1 = *p1, lastP2 = *p2;
    do_move(move);
    char isChecked = is_checked();
    *p1 = lastP1; *p2 = lastP2;
    set_piece(move+1, p1);
    set_piece(move+2, p2);

    if (isChecked) return 15+isChecked;
        
    return 0;
}

int main()
{
    // ♔♕♗♘♙♖♚♛♝♞♟♜
    int a;
    players[0] = (player) {a};
    
    printf("\x1b[37;40m♔♕♗♘♙♖♚♛♝♞♟♜\n");
    printf("\x1b[37;40m♔♕♗♘♙♖♚♛♝♞♟♜\n");
    printf("\x1b[100m♔♕♗♘♙♖♚♛♝♞♟♜\n");
    printf("\x1b[37;40m♔♕♗♘♙♖♚♛♝♞♟♜\n");
    printf("\x1b[37;40m♔♕♗♘♙♖♚♛♝♞♟♜\n");

    reset_game();
    display();

    printf("\e[37;40mhello\n");
    printf("%s\n", players[1].pieces[0].pos);
    printf("%d\n", players[1].pieces[0].type);
    printf("%d\n", get_piece("e1")->type);

    // char move[20];
    // while (1){
    //     scanf("%s", move);
    //     if (isValidMove(move)){
    //         do_move(move);
    //     }
    //     turn = !turn;
    //     if (end_of_game()){
    //         break;
    //     }
    // }

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
