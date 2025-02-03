// includes
    #include <stdio.h>
    #include <string.h>
    #include <windows.h>
    // #include <sys/ioctl.h>
    // #include <unistd.h>
// 

// style
    // 1: bold 
    // 2: shadow
    // 3: italic
    // 4: underlined
    // 5: blinking 
    // 6: blinking
    // 7: color reveresed
    // 8: hide 
    // 9: lined 
    // 30-37: foreground color
    // 40-47: background color
    // 90-97:   light foreground color
    // 100-107: light background color
    // 0: black, 1: red, 2: green, 3: yellow, 4: blue, 5: magenta, 6: cyan, 7: white

    #define STYLE_DEFAULT       "\e[0m"
    #define STYLE_RESPONSE      "\e[1m"
    #define STYLE_ERROR         "\e[2m"
    #define STYLE_PROMPT        "\e[3m"
    #define STYLE_LINK          "\e[4m"
    #define STYLE_LAST_ACTION   "\e[5m"
    #define STYLE_HIGHLIGHT     "\e[100m"
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
    #define BOARD(pos)     Board[(pos)[0]-'a'][(pos)[1]-'1']

    char g_quit, g_exit;
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


// validation
    #define abs(x)      (x>0? x: -x)
    #define sign(x)     (x>0? 1: x<0? -1: 0)
    #define dif_x(move) (move[2]-move[0])
    #define dif_y(move) (move[3]-move[1])
    #define dis_x(move) abs(dif_x(move))
    #define dis_y(move) abs(dif_y(move))


    #define MRC_ANY_INTERVENING_PIECE   6
    char any_intervening_piece(char* move){
        char dir[3] = {sign(dif_x(move)), sign(dif_y(move))};
        char pos[3] = {move[0]+dir[0], move[1]+dir[1]};
        char step = 1;
        for (; pos[0]!=move[2]||pos[1]!=move[3]; ){
            if (BOARD(pos)->type != NONE){
                return step;
            }
            pos[0]+=dir[0], pos[1]+=dir[1];
            step++;
        }
        return 0;
    }

    #define MRC_NOT_PAWN_MOVE           7
    char not_pawn_move(char* move){
        // TODO : En passant
        if (dif_x(move)) return 1;
        if (BOARD(move)->status == DID_NOT_MOVE_YET){
            if (BOARD(move)->owner == WHITE && dif_y(move) <= 0) return 2;
            if (BOARD(move)->owner == BLACK && dif_y(move) >= 0) return 3;
            if (dis_y(move) > 2) return 4;
            if (dis_y(move) == 2){
                if (any_intervening_piece(move)) return 5;
                if (BOARD(move+2)->type != NONE) return 6;
            }
        }
        if (BOARD(move)->status == MOVED_ONCE_BEFORE 
        && dis_y(move) > 1) return 7;
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
            if (dif_x(move) <= 0) return 2;
        } else {
            if (dif_x(move) >= 0) return 3;
        }
        // if (move[1] <= '4'){
        //     if (dif_y(move) <= 0) return 4;
        // } else {
        //     if (dif_y(move) >= 0) return 5;
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

        piece* p_pc1 = BOARD(move);
        piece* p_pc2 = BOARD(move+2);

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
        char type = BOARD(move)->type;

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
        char move[5] = "a1a1";
        move[2] = Players[(Term&1)].pieces[0].pos[0];
        move[3] = Players[(Term&1)].pieces[0].pos[1];

        for (piece* p_pc=Players[!(Term&1)].pieces; p_pc<=Players[!(Term&1)].pieces+15; ++p_pc){
            if (p_pc->status == DEAD) continue;
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
        
        piece* p_pc1 = BOARD(move);
        piece* p_pc2 = BOARD(move+2);
        piece pc1 = *p_pc1;
        piece pc2 = *p_pc2;

        // TODO : castling
        BOARD(move) = p_Unoccupied;
        BOARD(move+2) = p_pc1;
        p_pc1->pos[0] = move[2]; p_pc1->pos[1] = move[3];
        p_pc1->status = MOVED_ONCE_BEFORE;
        p_pc2->status = DEAD;
        error = is_checked();
        *p_pc1 = pc1;
        *p_pc2 = pc2;
        BOARD(move) = p_pc1;
        BOARD(move+2) = p_pc2;

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
// 


// game control
    char display(){
        printf(STYLE_DEFAULT"\e[1;1H\e[2J");
        int row, col;

        // board
            row = 2; col = 7;
            for (char pos[3]="a1"; pos[0]<='h'; ++pos[0]){
                for (pos[1]='1'; pos[1]<='8'; ++pos[1]){
                    printf("\e[%d;%dH%s%s %s %s", row+('8'-pos[1]), col+(pos[0]-'a')*3, 
                    (pos[0]&1)^(pos[1]&1)? STYLE_HIGHLIGHT: "",
                    Term-1 >= 1 
                     && Moves[Term-1][3] == pos[0]
                     && Moves[Term-1][4] == pos[1]? STYLE_LAST_ACTION: "",
                    BOARD(pos)->symbol, STYLE_DEFAULT);
                }
            }
        // 

        // border and labels
            row = 1; col = 5;
            printf("\e[%d;%dH%s", row, col+1, "╭────────────────────────╮");
            printf("\e[%d;%dH%s", row+9, col+1, "╰─a──b──c──d──e──f──g──h─╯");
            for (char y[2]="8"; y[0]>='1'; --y[0]){
                printf("\e[%d;%dH%s", row+1+('8'-y[0]), col, y);
                printf("\e[%d;%dH%s", row+1+('8'-y[0]), col+1, "│");
                printf("\e[%d;%dH%s", row+1+('8'-y[0]), col+26, "│");
            }
        // 

        // dead pieces
            char death = 0;
            row = 9; col = 2;
            for (piece* p_pc=Players[WHITE].pieces; p_pc<=Players[WHITE].pieces+15; ++p_pc){
                if (p_pc->status == DEAD){
                    printf("\e[%d;%dH%s", row-(death/2) ,col+!(death&1), p_pc->symbol);
                    death++;
                }
            }

            death = 0;
            row = 2; col = 33;
            for (piece* p_pc=Players[BLACK].pieces; p_pc<=Players[BLACK].pieces+15; ++p_pc){
                if (p_pc->status == DEAD){
                    printf("\e[%d;%dH%s", row+(death/2) ,col+(death&1), p_pc->symbol);
                    death++;
                }
            }
        // 

        // table of moves
            printf(STYLE_DEFAULT);
            // struct winsize w;
            // ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            // 57 = 57

            row = 1; col = 37;
            int width = 19, r;
            int move = MaxTerm-((57-col+1)/width*8*2);
            if (!(move&1)) move++;
            if (Term-1 < move) move = Term-1;
            if (!(move&1)) move--;
            if (move <= 0) move = 1;

            while (col+width-1 <= 57){
                r = row;
                printf("\e[%d;%dH%s", r++, col, " No. white  black ");
                printf("\e[%d;%dH%s", r++, col, "──────────────────");
                for (; r<=10; ++r){
                    if (move >= MaxTerm) break;
                    printf("\e[%d;%dH %-3d %s%s%s  %s%s%s ", 
                    r, col, (move+1)/2, 
                    (move+1==Term? STYLE_HIGHLIGHT: ""), Moves[move],STYLE_DEFAULT,
                    (move+2==Term? STYLE_HIGHLIGHT: ""), Moves[move+1], STYLE_DEFAULT);
                    move += 2;
                }
                col += width;
                if (col+width-1 > 57 || move >= MaxTerm) break;

                r = row;
                printf("\e[%d;%dH%s", r++, col-1, "│");
                printf("\e[%d;%dH%s", r++, col-1, "┼");
                for (; r<=10; ++r){
                    printf("\e[%d;%dH%s", r, col-1, "│");
                }
            }
        // 

        printf("\e[%d;%dH%s", 12, 6, STYLE_DEFAULT);
        return 0;
    }

    char apply_action(char* movement){
        char* move = movement+1;
        piece* p_pc1 = BOARD(move);
        piece* p_pc2 = BOARD(move+2);

        BOARD(move) = p_Unoccupied;
        BOARD(move+2) = p_pc1;

        p_pc1->pos[0] = move[2]; p_pc1->pos[1] = move[3];
        p_pc1->status = MOVED_ONCE_BEFORE;
        p_pc2->status = DEAD;

        // promotion
        if (p_pc1->type == PAWN
         && ((p_pc1->owner == WHITE && p_pc1->pos[1] == '8')
             || (p_pc1->owner == BLACK && p_pc1->pos[1] == '1'))){
            printf("\e[%d;%dH%s", 14, 1, STYLE_RESPONSE);
            for (char tp=1; tp<5; ++tp)
                printf("%d: %s, ", tp, PIECE_SYMBOLS[(Term&1)][tp]);
            printf("\b\b \n");

            char type=0;
            while (type<1 || 4<type){
                printf("\e[%d;%dH%s%s", 15, 6, STYLE_PROMPT, "which type of piece do you promote to? ");
                scanf("%d", &type);
                while (getchar() != '\n');
            }
            printf(STYLE_DEFAULT);

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

    char reset_game(){
        Term = 1;

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
                BOARD(p_pc->pos) = p_pc;
            }
        }
        for (char pos[3]="a3";  pos[1]<='6'; ++pos[1]){
            for (pos[0]='a';  pos[0]<='h'; ++pos[0]){
                BOARD(pos) = p_Unoccupied;
            }
        }

        return 0;
    }

    char start_game(){
        g_exit = 0;
        MaxTerm = 1;
        reset_game();

        char file[50];
        printf(STYLE_PROMPT"any game to restore?"STYLE_DEFAULT" ");
        scanf("%[^\n]s", file);
        while (getchar() != '\n');

        FILE* game = fopen(file, "r");
        if (game != NULL){
            while (fscanf(game, "%s", Moves[Term]) == 1){
                apply_action(Moves[Term]);
            }
            MaxTerm = Term;
            fclose(game);
        }

        return 0;
    }

    void goto_term(int term){
        reset_game();

        if (term > MaxTerm) term = MaxTerm;
        while (Term<term){
            apply_action(Moves[Term]);
        }
    }

    void save_game(char* file){
        FILE* game = fopen(file, "w");
        if (game != NULL){
            int term = 1;
            while (term<MaxTerm){
                fprintf(game, "%s  ", Moves[term++]);
                if (term<MaxTerm){
                    fprintf(game, "%s\n", Moves[term++]);
                }
            }
            fclose(game);
            printf("\e[%d;%dH%s%s%s", 14, 6, STYLE_RESPONSE, "successful save.", STYLE_DEFAULT);
            while (getchar() != '\n');
        } else {
            printf("\e[%d;%dH%s%s%s", 14, 6, STYLE_ERROR, "no such file to save!", STYLE_DEFAULT);
            while (getchar() != '\n');
        }
    }

    void help(){
        printf(
            "\e[%d;%dH\n"
           "chepss is a simple game between two players.                                                                \n"
           "which is clear to understand, easy to modify, strong to be extended                                         \n"
           " and general to be used as a template for other board games.                                                \n"
           "                                                                                                            \n"
           "    commands:   [piece_type]<source><dist>                                                                  \n"
           "        help                        display this help                                                       \n"
           "        save <file>                 save the game as a plain text file. you can change the file manually    \n"
           "        goto [<round>[.<turn>]]     navigate in game and continue, 0 or nothing for the last action taken   \n"
           "        exit                        exit current game and ask to restore any saved one                      \n"
           "        quit                        quit the game                                                           \n"
           "\n%s"
           "https://github.com/sirnaser/chepss.git\n"
           "%s\n", 
            13, 1, STYLE_LINK, STYLE_DEFAULT);
        while (getchar() != '\n');
    }

    char get_action(char* action){
        printf("\e[%d;%dH%i.%i> ", 12, 6, (Term+1)/2, !(Term&1));
        scanf("%[^\n]s", action);
        while (getchar() != '\n');

        if (action[0] == 'q'
         && action[1] == 'u'
         && action[2] == 'i'
         && action[3] == 't'){
            g_quit = 1;
            return 1;
        }

        if (action[0] == 'e'
         && action[1] == 'x'
         && action[2] == 'i'
         && action[3] == 't'){
            g_exit = 1;
            return 1;
        }

        if (action[0] == 's'
         && action[1] == 'a'
         && action[2] == 'v'
         && action[3] == 'e'){
            save_game(action+5);
            return 1;
        }

        if (action[0] == 'g'
         && action[1] == 'o'
         && action[2] == 't'
         && action[3] == 'o'){
            if (action[4] && '0' <= action[5] && action[5] <= '9'){
                int round, turn=0;
                sscanf(action+5, "%d.%d", &round, &turn);
                if (round){
                    goto_term(round*2-1+!(!turn));
                    return 1;
                }
            }
            goto_term(MaxTerm);
            return 1;
        }

        if (action[0] == 'h'
         && action[1] == 'e'
         && action[2] == 'l'
         && action[3] == 'p'){
            help();
            return 1;
        }


        if ('1' <= action[1] && action[1] <= '8'){
            for (int i=3; i >= 0; --i) action[i+1] = action[i];
        }
        if (not_in_board(action+1)) return 1;
        action[0] = MOVEMENT_SYMBOLS[BOARD(action+1)->type];
        action[5] = '\0';

        return 0;
    }

    void error_message(char error){
        printf("\e[%d;%dH%s", 14, 6, STYLE_ERROR);

        if (1 <= error && error <= MRC_NOT_LEGAL_MOVE){
            printf("not a legal move: ");
            if (1 <= error && error <= MRC_NOT_IN_BOARD){
                printf("source position ");
                if (error == 1) printf("x so low");
                if (error == 2) printf("x so high");
                if (error == 3) printf("y so low");
                if (error == 4) printf("y so hight");
            } error -= MRC_NOT_IN_BOARD;

            if (1 <= error && error <= MRC_NOT_IN_BOARD){
                printf("dist position ");
                if (error == 1) printf("x so low");
                if (error == 2) printf("x so high");
                if (error == 3) printf("y so low");
                if (error == 4) printf("y so hight");
            } error -= MRC_NOT_IN_BOARD;

            if (error == 1) printf("piece is dead");
            if (error == 2) printf("piece is not yours");
            if (error == 3) printf("dist is your piece");
        } error -= MRC_NOT_LEGAL_MOVE;

        if (1 <= error && error <= MRC_NOT_VALID_MOVE){
            printf("not a valid move: ");
            if (1 <= error && error <= MRC_NOT_PAWN_MOVE){
                printf("pawn: ");
                if (error == 1) printf("can not move horizontally");
                if (error == 2) printf("white pawn first move should go upper");
                if (error == 3) printf("black pawn first move should go lower");
                if (error == 4) printf("can not move more than 2 distances in first move");
                if (error == 5) printf("there is an intervening piece");
                if (error == 6) printf("can not capture in first move");
                if (error == 7) printf("can not move more than 1 distance (except first move)");
            } error -= MRC_NOT_PAWN_MOVE;

            if (1 <= error && error <= MRC_NOT_KNIGHT_MOVE){
                printf("knight: ");
                if (error == 1) printf("can not move like that");
            } error -= MRC_NOT_KNIGHT_MOVE;

            if (1 <= error && error <= MRC_NOT_BISHOP_MOVE){
                printf("bishop: ");
                if (error == 1) printf("not a diagonal move");
                if (error == 2) printf("can not go left in the left half");
                if (error == 3) printf("can not go right in the right half");
                if (error == 4) printf("can not go lower in the lower half");
                if (error == 5) printf("can not go upper in the upper half");
                if (error >= 6) printf("there is an intervening piece in step %d", error-5);
            } error -= MRC_NOT_BISHOP_MOVE;

            if (1 <= error && error <= MRC_NOT_ROOK_MOVE){
                printf("rook: ");
                if (error == 1) printf("not a vertical or horizontal move");
                if (error >= 2) printf("there is an intervening piece in step %d", error-1);
            } error -= MRC_NOT_ROOK_MOVE;

            if (1 <= error && error <= MRC_NOT_QUEEN_MOVE){
                printf("queen: ");
                if (error == 1) printf("is not any type of queen moves");
            } error -= MRC_NOT_QUEEN_MOVE;

            if (1 <= error && error <= MRC_NOT_KING_MOVE){
                printf("king: ");
                if (error == 1) printf("can not move more than 1 distance horizontally");
                if (error == 2) printf("can not move more than 1 distance vertically");
            } error -= MRC_NOT_KING_MOVE;
        } error -= MRC_NOT_VALID_MOVE;

        if (1 <= error && error <= MRC_NOT_SAFE_MOVE){
            printf("not a safe move: you will be checked by %s piece", Players[!(Term&1)].pieces[error-1].pos);
        } error -= MRC_NOT_SAFE_MOVE;

        printf(STYLE_DEFAULT);
    }

    char validate_action(char* movement){
        char error = not_possible_move(movement+1);
        if (error){
            error_message(error);
            printf(" (error code: %i) ", error);
            while (getchar() != '\n');
            return 1;
        }
        return 0;
    }

    char any_possible_move(){
        char move[5] = "a1a1";

        for (piece* p_pc=Players[(Term&1)].pieces; p_pc<=Players[(Term&1)].pieces+15; ++p_pc){
            move[0] = p_pc->pos[0];
            move[1] = p_pc->pos[1];
            for (move[3]='1';  move[3]<='8'; ++move[3]){
                for (move[2]='a';  move[2]<='h'; ++move[2]){
                    if (!not_possible_move(move)){
                        return 1;
                    }
                }
            }
        }

        return 0;
    }

    char end_of_game(){
        if (!any_possible_move()){
            if (is_checked()){
                printf(STYLE_RESPONSE"the game ended in checkmate.\n"STYLE_DEFAULT);
            } else {
                printf(STYLE_RESPONSE"the game ended in stalemate.\n"STYLE_DEFAULT);
            }
            while (getchar() != '\n');
            return 1;
        }

        int repetition = 0, term = Term;
        player players[2] = {Players[0], Players[1]};

        reset_game();
        while (Term<term){
            apply_action(Moves[Term]);
            if (!memcmp(&players, &Players, sizeof(player)*2)){
                repetition++;
            }
        }

        // five/threefold repetition
        if (repetition >= 5){
            printf(STYLE_RESPONSE"the game ended in fivefold repetition.\n"STYLE_DEFAULT);
            while (getchar() != '\n');
            return 1;
        }
        if (repetition >= 3){
            printf(STYLE_RESPONSE"the game may end in threefold repetition.\n"STYLE_DEFAULT);
            while (getchar() != '\n');
            return 1;
        }

        return 0;
    }
// 


int main(){
    SetConsoleOutputCP(65001);
    char action[50];

    while (!g_quit) {
        start_game();

        while (!g_quit && !g_exit){
            if (display()) continue;

            if (get_action(action)) continue;

            if (validate_action(action)) continue;

            if (apply_action(action)) continue;

            end_of_game();
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

    // regards:
    // https://stackoverflow.com/questions/1722112/what-are-the-most-common-naming-conventions-in-c
// 

// validation function naming:
    // it would be a multi value proposition
    // terminology:
        // is a valid move -> is valid move
        // not a valid move -> not valid move
// 

// printf formatting protocol:
    // if no identifier used, pass the style values in format string directly
    // otherwise use %s identifier to pass style values as arguments
// 

// TODO:
    // implementing real chess validation
        // castling
        // En passant
// 

// be patient, if it's not as good as possible ♥️
// https://github.com/sirnaser/chepss.git
