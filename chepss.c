// includes
    #include <stdio.h>
// 


// data structures
    // piece.owner
        #define BLACK   0
        #define WHITE   1
        #define NEITHER 2
    // 

    // piece.status
        #define DID_NOT_MOVE_YET    0
        #define MOVED_ONCE_BEFORE   1
        #define DEAD                2
    // 

    // piece.type
        #define PAWN    0
        #define KNIGHT  1
        #define BISHOP  2
        #define ROOK    3
        #define QUEEN   4
        #define KING    5
        #define NONE    6
    // 


    typedef struct{
        char owner;
        char status;
        char type;   
        char pos[3];    // ex: a1
        char symbol[5]; // ♔♕♖♗♘♙♚♛♜♝♞♟
    } piece;

    typedef struct{
        piece pieces[16];
        // {king, queen, rook*2, bishop*2, knight*2, pawn*8}
    } player;
// 

// variables
    char g_quit;
    int Term, MaxTerm;
    char Moves[1000][6];
    player Players[2];
    piece* Board[8][8];
    piece* p_Unoccupied = &(piece){.owner=NEITHER, .status=DEAD, .type=NONE, .symbol=" "};
// 

// constants
    static const char PIECE_SYMBOLS[2][7][5] = {
        [BLACK]={[PAWN]="♙", [KNIGHT]="♘", [BISHOP]="♗", [ROOK]="♖", [QUEEN]="♕", [KING]="♔", [NONE]=" "},
        [WHITE]={[PAWN]="♟", [KNIGHT]="♞", [BISHOP]="♝", [ROOK]="♜", [QUEEN]="♛", [KING]="♚", [NONE]=" "}
    };

    static const char MOVEMENT_SYMBOLS[7] = {
        [PAWN]='P', [KNIGHT]='N', [BISHOP]='B', [ROOK]='R', [QUEEN]='Q', [KING]='K', [NONE]=' '
    };
// 


// utilies
    void set_board(char* pos, piece* pc){
        Board[pos[0]-'a'][pos[1]-'1'] = pc;
    }

    piece* get_board(char* pos){
        return Board[pos[0]-'a'][pos[1]-'1'];
    }

    void print_pos(int x, int y, char* str){
        printf("e[1;1H");
    }
// 

// validation
    #define abs(x)      (x>0? x: -x)
    #define sign(x)     (x>0? 1: x<0? -1: 0)
    #define dif_x(move) (move[2]-move[0])
    #define dif_y(move) (move[3]-move[1])
    #define dis_x(move) abs(dif_x(move))
    #define dis_y(move) abs(dif_y(move))
    #define dir_x(move) sign(dif_x(move))
    #define dir_y(move) sign(dif_y(move))


    #define MRC_ANY_INTERVENING_PIECE   6
    char any_intervening_piece(char* move){
        char dir[3] = {dir_x(move), dir_y(move)};
        char pos[3] = {move[0]+dir[0], move[1]+dir[1]};
        char step = 1;
        for (; pos[0]!=move[2]||pos[1]!=move[3]; ){
            if (get_board(pos)->type != NONE){
                return step;
            }
            pos[0]+=dir[0], pos[1]+=dir[1];
            step++;
        }
        return 0;
    }

    #define MRC_NOT_PAWN_MOVE           6+1
    char not_pawn_move(char* move){
        // TODO : En passant
        if (dif_x(move)) return 1;
        if (get_board(move)->status == DID_NOT_MOVE_YET){
            if (get_board(move)->owner == WHITE && dir_y(move) <= 0) return 2;
            if (get_board(move)->owner == BLACK && dir_y(move) >= 0) return 3;
            if (dis_y(move) > 2) return 4;
            if (dis_y(move) == 2
                && (any_intervening_piece(move)
                    || get_board(move+2)->type != NONE)) return 5;
        }
        if (get_board(move)->status == MOVED_ONCE_BEFORE 
        && dis_y(move) > 1) return 6;
        return 0;
    }

    #define MRC_NOT_KNIGHT_MOVE         1
    char not_knight_move(char* move){
        if (!(dis_x(move) == 2 && dis_y(move) == 3) 
        && !(dis_x(move) == 3 && dis_y(move) == 2)) return 1;
        return 0;
    }

    #define MRC_NOT_BISHOP_MOVE         5   \
            + MRC_ANY_INTERVENING_PIECE
    char not_bishop_move(char* move){
        if (dis_x(move) != dis_y(move)) return 1;
        if (move[0] <= 'd'){
            if (dir_x(move) <= 0) return 2;
        } else {
            if (dir_x(move) >= 0) return 3;
        }
        // if (move[1] <= '4'){
        //     if (dir_y(move) <= 0) return 4;
        // } else {
        //     if (dir_y(move) >= 0) return 5;
        // }
        char step;
        if ((step=any_intervening_piece(move))) return 5+step;
        return 0;
    }

    #define MRC_NOT_ROOK_MOVE           1   \
            + MRC_ANY_INTERVENING_PIECE
    char not_rook_move(char* move){
        if (dif_x(move) && dif_y(move)) return 1;
        char step;
        if ((step=any_intervening_piece(move))) return 1+step;
        return 0;
    }

    #define MRC_NOT_QUEEN_MOVE          1
    char not_queen_move(char* move){
        if (!(dis_x(move) == 1 && dis_y(move) == 2) 
        && !(dis_x(move) == 2 && dis_y(move) == 1)
        && not_rook_move(move)
        && not_bishop_move(move)) return 1;
        return 0;
    }

    #define MRC_NOT_KING_MOVE           2
    char not_king_move(char* move){
        // TODO : castling
        if (dis_x(move) > 1) return 1;
        if (dis_y(move) > 1) return 2;
        return 0;
    }

    #define MRC_NOT_IN_BOARD            4
    char not_in_board(char* pos){
        if (pos[0] < 'a') return 1;
        if ('h' < pos[0]) return 2;
        if (pos[1] < '1') return 3;
        if ('8' < pos[1]) return 4;
        return 0;
    }

    #define MRC_NOT_LEGAL_MOVE          \
            MRC_NOT_IN_BOARD*2          \
            + 3
    char not_legal_move(char* move){
        char passed = 0, error;

        if ((error = not_in_board(move))) return passed+error;
        passed += MRC_NOT_IN_BOARD;
        if ((error = not_in_board(move+2))) return passed+error;
        passed += MRC_NOT_IN_BOARD;

        piece* p_pc1 = get_board(move);
        piece* p_pc2 = get_board(move+2);

        if ((error = p_pc1->status == DEAD)) return passed+error;
        passed++;
        if ((error = p_pc1->owner != (Term&1))) return passed+error;
        passed++;
        if ((error = p_pc2->owner == (Term&1))) return passed+error;
        passed++;

        return 0;
    }

    #define MRC_NOT_VALID_MOVE          \
            + MRC_NOT_PAWN_MOVE         \
            + MRC_NOT_KNIGHT_MOVE       \
            + MRC_NOT_BISHOP_MOVE       \
            + MRC_NOT_ROOK_MOVE         \
            + MRC_NOT_QUEEN_MOVE        \
            + MRC_NOT_KING_MOVE
    char not_valid_move(char* move){
        char passed = 0, error;
        char type = get_board(move)->type;

        if (type == PAWN && (error = not_pawn_move(move))) return passed+error;
        passed += MRC_NOT_PAWN_MOVE;
        if (type == KNIGHT && (error = not_knight_move(move))) return passed+error;
        passed += MRC_NOT_KNIGHT_MOVE;
        if (type == BISHOP && (error = not_bishop_move(move))) return passed+error;
        passed += MRC_NOT_BISHOP_MOVE;
        if (type == ROOK && (error = not_rook_move(move))) return passed+error;
        passed += MRC_NOT_ROOK_MOVE;
        if (type == QUEEN && (error = not_queen_move(move))) return passed+error;
        passed += MRC_NOT_QUEEN_MOVE;
        if (type == KING && (error = not_king_move(move))) return passed+error;
        passed += MRC_NOT_KING_MOVE;

        return 0;
    }

    #define MRC_IS_CHECKED              16
    char is_checked(){
        char move[5] = "";
        move[2] = Players[(Term&1)].pieces[0].pos[0];
        move[3] = Players[(Term&1)].pieces[0].pos[1];

        for (piece* p_pc=Players[!(Term&1)].pieces; p_pc<=Players[!(Term&1)].pieces+15; ++p_pc){
            move[0] = p_pc->pos[0];
            move[1] = p_pc->pos[1];
            if (!not_valid_move(move)){
                return (p_pc-Players[!(Term&1)].pieces)+1;
            }
        }

        return 0;
    }

    #define MRC_NOT_SAFE_MOVE           \
            MRC_IS_CHECKED
    char not_safe_move(char* move){
        char error;
        
        piece* p_pc1 = get_board(move);
        piece* p_pc2 = get_board(move+2);
        piece pc1 = *p_pc1;
        piece pc2 = *p_pc2;

        // TODO : castling
        set_board(move, p_Unoccupied);
        set_board(move+2, p_pc1);
        p_pc1->pos[0] = move[2]; p_pc1->pos[1] = move[3];
        p_pc1->status = MOVED_ONCE_BEFORE;
        p_pc2->status = DEAD;
        error = is_checked();
        *p_pc1 = pc1;
        *p_pc2 = pc2;
        set_board(move, p_pc1);
        set_board(move+2, p_pc2);

        if (error) return error;
        return 0;
    }

    #define MRC_NOT_POSSIBLE_MOVE       \
            MRC_NOT_LEGAL_MOVE          \
            + MRC_NOT_VALID_MOVE        \
            + MRC_NOT_SAFE_MOVE
    char not_possible_move(char* move){
        char passed = 0, error;

        if ((error=not_legal_move(move))) return passed+error;
        passed += MRC_NOT_LEGAL_MOVE;
        if ((error=not_valid_move(move))) return passed+error;
        passed += MRC_NOT_VALID_MOVE;
        if ((error=not_safe_move(move))) return passed+error;
        passed += MRC_NOT_SAFE_MOVE;

        return 0;
    }

    #define MRC_ANY_POSSIBLE_MOVE       1
    char any_possible_move(){
        char move[5] = "";

        for (piece* p_pc=Players[(Term&1)].pieces; p_pc<=Players[(Term&1)].pieces+15; ++p_pc){
            move[0] = p_pc->pos[0];
            move[1] = p_pc->pos[1];
            for (move[3]='1';  move[3]<='8'; ++move[3]){
                for (move[2]='a';  move[2]<='h'; ++move[2]){
                    if (!not_possible_move(move)){
                        // check point
                        printf("1 %s\n", move);
                        getchar();
                        return 1;
                    }
                }
            }
        }

        return 0;
    }
// 


// game control functions
    char reset_game(){
        Term = 1;
        MaxTerm = 1;

        Players[BLACK] = (player){
            {
                {BLACK,  DID_NOT_MOVE_YET, KING,   "e8", "♔"}, 
                {BLACK,  DID_NOT_MOVE_YET, QUEEN,  "d8", "♕"}, 
                {BLACK,  DID_NOT_MOVE_YET, ROOK,   "a8", "♖"}, 
                {BLACK,  DID_NOT_MOVE_YET, ROOK,   "h8", "♖"}, 
                {BLACK,  DID_NOT_MOVE_YET, BISHOP, "c8", "♙"}, 
                {BLACK,  DID_NOT_MOVE_YET, BISHOP, "f8", "♙"}, 
                {BLACK,  DID_NOT_MOVE_YET, KNIGHT, "b8", "♘"}, 
                {BLACK,  DID_NOT_MOVE_YET, KNIGHT, "g8", "♘"}, 
                {BLACK,  DID_NOT_MOVE_YET, PAWN,   "a7", "♙"}, 
                {BLACK,  DID_NOT_MOVE_YET, PAWN,   "b7", "♙"}, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "c7", "♙"}, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "d7", "♙"}, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "e7", "♙"}, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "f7", "♙"}, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "g7", "♙"}, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "h7", "♙"}
            }
        };

        Players[WHITE] = (player){
            {
                {WHITE,  DID_NOT_MOVE_YET, KING,   "e1", "♚"}, 
                {WHITE,  DID_NOT_MOVE_YET, QUEEN,  "d1", "♛"}, 
                {WHITE,  DID_NOT_MOVE_YET, ROOK,   "a1", "♜"}, 
                {WHITE,  DID_NOT_MOVE_YET, ROOK,   "h1", "♜"}, 
                {WHITE,  DID_NOT_MOVE_YET, BISHOP, "c1", "♝"}, 
                {WHITE,  DID_NOT_MOVE_YET, BISHOP, "f1", "♝"}, 
                {WHITE,  DID_NOT_MOVE_YET, KNIGHT, "b1", "♞"}, 
                {WHITE,  DID_NOT_MOVE_YET, KNIGHT, "g1", "♞"}, 
                {WHITE,  DID_NOT_MOVE_YET, PAWN,   "a2", "♟"}, 
                {WHITE,  DID_NOT_MOVE_YET, PAWN,   "b2", "♟"}, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "c2", "♟"}, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "d2", "♟"}, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "e2", "♟"}, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "f2", "♟"}, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "g2", "♟"}, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "h2", "♟"}
            }
        };

        for (char pl=0; pl<2; ++pl){
            for (piece* p_pc=Players[pl].pieces; p_pc<=Players[pl].pieces+15; ++p_pc){
                set_board(p_pc->pos, p_pc);
            }
        }
        for (char pos[3]="a3";  pos[1]<='6'; ++pos[1]){
            for (pos[0]='a';  pos[0]<='h'; ++pos[0]){
                set_board(pos, p_Unoccupied);
            }
        }

        return 0;
    }

    char display(){
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
        return 0;
    }

    void goto_term(int term){

    }

    char get_action(char* movement){
        printf("%i.%i> ", (Term+1)/2, !(Term&1));
        scanf("%s", movement);
        getchar();

        if ('0' <= movement[0] && movement[0] <= '9'){
            // goto term
            return 1;
        }

        if ('1' <= movement[1] && movement[1] <= '8'){
            for (char i=3; i >= 0; --i) movement[i+1] = movement[i];
        }
        if (not_in_board(movement+1)) return 1;
        movement[0] = MOVEMENT_SYMBOLS[get_board(movement+1)->type];
        movement[5] = '\0';

        return 0;
    }

    char validate_action(char* movement){
        char error = not_possible_move(movement+1);
        if (error){
            printf("error code: %i\n", error);
            // printf("%s\n", ERROR_MESSAGES[error]);
            while (getchar() != '\n');
            return 1;
        }
        return 0;
    }

    char apply_action(char* movement){
        piece* p_pc1 = get_board(movement+1);
        piece* p_pc2 = get_board(movement+3);
        
        set_board(movement+1, p_Unoccupied);
        set_board(movement+3, p_pc1);

        p_pc1->pos[0] = movement[3]; p_pc1->pos[1] = movement[4];
        p_pc1->status = MOVED_ONCE_BEFORE;
        p_pc2->status = DEAD;

        // promotion
        if (p_pc1->type == PAWN && (p_pc1->pos[1] == '1' || p_pc1->pos[1] == '8')){
            for (char tp=1; tp<5; ++tp)
                printf("%d: %s, ", tp, PIECE_SYMBOLS[(Term&1)][tp]);
            printf("\b\b \n");

            char type=0;
            while (type<1 || 4<type){
                printf("which type of piece do you promote to?");
                scanf("%d", &type);
            }

            piece* p_pc1 = get_board(movement+3);
            p_pc1->type = type;
            p_pc1->symbol[0] = PIECE_SYMBOLS[(Term&1)][type][0];
            p_pc1->symbol[1] = PIECE_SYMBOLS[(Term&1)][type][1];
            p_pc1->symbol[2] = PIECE_SYMBOLS[(Term&1)][type][2];
        }

        // TODO : castling

        sprintf(Moves[Term++], "%s", movement);
        if (Term>MaxTerm) MaxTerm=Term;
        return 0;
    }

    char end_of_game(){
        if (any_possible_move()) return 0;

        // Threefold repetition
        if (is_checked()){
            printf("checkmate\n");
        } else {
            printf("stalemate\n");
        }
        while (getchar() != '\n');

        return 1;
    }
// 


int main(){
    // ♔♕♗♘♙♖♚♛♝♞♟♜
    char action[20];

    while (!g_quit) {
        if (reset_game()) break;

        while (!g_quit){
            if (display()) continue;

            if (get_action(action)) continue;

            if (validate_action(action)) continue;

            if (apply_action(action)) continue;

            if (end_of_game()) break;
        }

    }

    return 0;
}


// naming conventions:
    // Trivial Variables: i,n,c,etc...
    // Local Variables: camelCase
    // Global Variables: is trivial? add a g_ to the prefix: PascalCase
    // Const Variables: ALL_CAPS
    // Pointer Variables: add a p_ to the prefix. 
    // Structs: single word? flatcase: PascalCase
    // Struct Member Variables: camelCase
    // Enums: single word? flatcase: PascalCase
    // Enum Values: ALL_CAPS
    // Functions: snake_case
    // Macros: snake_case

    // https://stackoverflow.com/questions/1722112/what-are-the-most-common-naming-conventions-in-c
// 


// is a valid move
// is valid move
// not a valid move
// not valid move



// TODO:
    // check end of game
    // display borderd board
    // display last move

    // castling
    // En passant

    // display dead pieces
    // show all valid moves
    // invalid move handling
    // shah ghalae
    // simple move format
    // display moves
    // save and restore moves
    // iterate in moves sequence
// 