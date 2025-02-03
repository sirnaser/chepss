// includes
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include <sys/ioctl.h>
    #include <unistd.h>
// 

// style
    // more about ANSI escape sequences:
    // https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

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
        #define BLACK           0
        #define WHITE           1
        // #define NONE_OF_THEM    2
    // 

    // piece.status
        #define DEAD                0
        #define DID_NOT_MOVE_YET    1
        #define MOVED_JUST_NOW      2
        // #define NONE_OF_THEM        3
    // 

    // piece.type
        #define PAWN            0
        #define KNIGHT          1
        #define BISHOP          2
        #define ROOK            3
        #define QUEEN           4
        #define KING            5
        // #define NONE_OF_THEM    6
    // 

    // none of them (max)
        #define NONE_OF_THEM    6
    // 


    typedef struct{
        char owner;
        char status;
        char type;   
        char pos[3];    // ex: a1
        char* symbol; // ♔♕♖♗♘♙♚♛♜♝♞♟
    } piece;

    typedef struct{
        piece pieces[16];
        // {king, queen, rook*2, bishop*2, knight*2, pawn*8}
    } player;
// 

// variables
    char g_quit, g_exit, g_end;
    int Term, MaxTerm;
    char Movements[1000][7];
    player Players[2];
    piece* Board[8][8];
    piece* p_Unoccupied = &(piece){.owner=NONE_OF_THEM, .status=DEAD, .type=NONE_OF_THEM, .symbol=" "};

    #define TURN            (Term&1) // -> piece.owner
    #define BOARD(pos)      Board[(pos)[0]-'a'][(pos)[1]-'1']
// 

// constants
    static const char* PIECE_SYMBOLS[2][NONE_OF_THEM+1] = {
        [BLACK]={[PAWN]="♙", [KNIGHT]="♘", [BISHOP]="♗", [ROOK]="♖", [QUEEN]="♕", [KING]="♔", [NONE_OF_THEM]=" "},
        [WHITE]={[PAWN]="♟", [KNIGHT]="♞", [BISHOP]="♝", [ROOK]="♜", [QUEEN]="♛", [KING]="♚", [NONE_OF_THEM]=" "}
    };

    static const char MOVEMENT_SYMBOLS[NONE_OF_THEM+1] = {
        [PAWN]='P', [KNIGHT]='N', [BISHOP]='B', [ROOK]='R', [QUEEN]='Q', [KING]='K', [NONE_OF_THEM]=' '
    };
// 



#define DO_MOVE(move)                       \
        BOARD((move))->pos[0] = (move)[2];  \
        BOARD((move))->pos[1] = (move)[3];  \
        BOARD((move)+2)->status = DEAD;     \
        BOARD((move)+2) = BOARD((move));    \
        BOARD((move)) = p_Unoccupied;
// 

// validation
    #define abs(x)      ((x)>0? (x): -(x))
    #define sign(x)     ((x)>0? 1: (x)<0? -1: 0)
    #define dif_x(move) ((move)[2]-(move)[0])
    #define dif_y(move) ((move)[3]-(move)[1])
    #define dis_x(move) abs(dif_x((move)))
    #define dis_y(move) abs(dif_y((move)))
    #define dir_x(move) sign(dif_x((move)))
    #define dir_y(move) sign(dif_y((move)))


    #define MRC_ANY_INTERVENING_PIECE   6
    int any_intervening_piece(char* move){
        if (dif_x(move) && dif_y(move) && (dis_x(move) != dis_y(move))) return -1;
        int passed = 0, error;

        char pos[3]={move[0]+dir_x(move), move[1]+dir_y(move)};
        while (pos[0] != move[2] || pos[1] != move[3]){
            if ((error=BOARD(pos)->type != NONE_OF_THEM)) return passed+error;
            passed++;

            pos[0]+=dir_x(move), pos[1]+=dir_y(move);
        }
        return 0;
    }


    #define MRC_NOT_PAWN_MOVE           8
    int not_pawn_move(char* move){
        if (BOARD(move)->owner == WHITE && dif_y(move) <= 0) return 1;
        if (BOARD(move)->owner == BLACK && dif_y(move) >= 0) return 2;
        if (BOARD(move+2)->type == NONE_OF_THEM){
            // advance
            if (dif_x(move)) return 3;
            if (dis_y(move) > 2) return 4;
            if (dis_y(move) == 2){
                if (BOARD(move)->status != DID_NOT_MOVE_YET) return 5;
                if (any_intervening_piece(move)) return 6;
            }
        } else {
            // capture
            if (dis_x(move) != 1) return 7;
            if (dis_y(move) != 1) return 8;
        }
        return 0;
    }

    #define MRC_NOT_KNIGHT_MOVE         1
    int not_knight_move(char* move){
        if (!(dis_x(move) == 1 && dis_y(move) == 2) 
        &&  !(dis_x(move) == 2 && dis_y(move) == 1)) return 1;
        return 0;
    }

    #define MRC_NOT_BISHOP_MOVE         1   \
            + MRC_ANY_INTERVENING_PIECE
    int not_bishop_move(char* move){
        if (dis_x(move) != dis_y(move)) return 1;
        int error;
        if ((error=any_intervening_piece(move))) return 1+error;
        return 0;
    }

    #define MRC_NOT_ROOK_MOVE           1   \
            + MRC_ANY_INTERVENING_PIECE
    int not_rook_move(char* move){
        if (dif_x(move) && dif_y(move)) return 1;
        int error;
        if ((error=any_intervening_piece(move))) return 1+error;
        return 0;
    }

    #define MRC_NOT_QUEEN_MOVE          1
    int not_queen_move(char* move){
        if (not_bishop_move(move)
        &&  not_rook_move(move)) return 1;
        return 0;
    }

    #define MRC_NOT_KING_MOVE           2
    int not_king_move(char* move){
        if (dis_x(move) > 1) return 1;
        if (dis_y(move) > 1) return 2;
        return 0;
    }



    #define MRC_NOT_COMMON_MOVE          \
            + MRC_NOT_PAWN_MOVE         \
            + MRC_NOT_KNIGHT_MOVE       \
            + MRC_NOT_BISHOP_MOVE       \
            + MRC_NOT_ROOK_MOVE         \
            + MRC_NOT_QUEEN_MOVE        \
            + MRC_NOT_KING_MOVE
    int not_common_move(char* move){
        int passed = 0, error;
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
    int is_checked(){
        char move[5] = {
            [2]=Players[TURN].pieces[0].pos[0],
            [3]=Players[TURN].pieces[0].pos[1]
        }; // the king pos

        for (piece* p_pc=Players[!TURN].pieces; p_pc<=Players[!TURN].pieces+15; ++p_pc){
            if (p_pc->status == DEAD) continue;
            move[0] = p_pc->pos[0];
            move[1] = p_pc->pos[1];
            if (!not_common_move(move)){
                return (p_pc-Players[!TURN].pieces)+1;
            }
        }

        return 0;
    }

    #define MRC_NOT_SAFE_MOVE           \
            MRC_IS_CHECKED
    int not_safe_move(char* move){
        int error;

        piece* p_pc1 = BOARD(move);
        piece* p_pc2 = BOARD(move+2);
        piece pc1_cpy = *p_pc1;
        piece pc2_cpy = *p_pc2;
        DO_MOVE(move);
        error = is_checked();
        *p_pc1 = pc1_cpy;
        *p_pc2 = pc2_cpy;
        BOARD(move) = p_pc1;
        BOARD(move+2) = p_pc2;

        if (error) return error;
        return 0;
    }



    #define MRC_NOT_IN_BOARD            4
    int not_in_board(char* pos){
        if (pos[0] < 'a') return 1;
        if ('h' < pos[0]) return 2;
        if (pos[1] < '1') return 3;
        if ('8' < pos[1]) return 4;
        return 0;
    }

    #define MRC_NOT_POSSIBLE_MOVE       3   \
            + MRC_NOT_IN_BOARD*2
    int not_possible_move(char* move){
        int passed = 0, error;

        if ((error = not_in_board(move))) return passed+error;
        passed += MRC_NOT_IN_BOARD;
        if ((error = not_in_board(move+2))) return passed+error;
        passed += MRC_NOT_IN_BOARD;

        if ((error = BOARD(move)->status == DEAD)) return passed+error;
        passed++;
        if ((error = BOARD(move)->owner != TURN)) return passed+error;
        passed++;
        if ((error = BOARD(move+2)->owner == TURN)) return passed+error;
        passed++;

        return 0;
    }



    #define MRC_NOT_EN_PASSANT          9   \
            + MRC_NOT_SAFE_MOVE
    int not_en_passant(char* move){
        int passed = 0, error;

        if ((error=BOARD(move)->type != PAWN)) return passed+error;
        passed++;
        if ((error=BOARD(move+2)->type != NONE_OF_THEM)) return passed+error;
        passed++;
        if ((error=BOARD(move)->owner == WHITE && dif_y(move) <= 0)) return passed+error;
        passed++;
        if ((error=BOARD(move)->owner == BLACK && dif_y(move) >= 0)) return passed+error;
        passed++;
        if ((error=dis_y(move) != 1)) return passed+error;
        passed++;
        if ((error=dis_x(move) != 1)) return passed+error;
        passed++;

        char pos[3] = {move[0]+dir_x(move), move[1]};
        piece* pawn = BOARD(pos);
        if ((error=pawn->type != PAWN)) return passed+error;
        passed++;
        if ((error=pawn->status != MOVED_JUST_NOW)) return passed+error;
        passed++;
        if ((error=pos[1] != '4' && pos[1] != '5')) return passed+error;
        passed++;

        piece pawn_cpy = *pawn;
        pawn->status = DEAD;
        BOARD(pos) = p_Unoccupied;
        error = not_safe_move(move);
        BOARD(pos) = pawn;
        *pawn = pawn_cpy;

        if (error) return passed+error;
        return 0;
    }

    #define MRC_NOT_CASTLING            9   \
            + MRC_IS_CHECKED*3
    int not_castling(char* move){
        int passed = 0, error;
        piece* king = BOARD(move);

        if ((error=king->type != KING)) return passed+error;
        passed++;
        if ((error=king->status != DID_NOT_MOVE_YET)) return passed+error;
        passed++;
        if ((error=dif_y(move))) return passed+error;
        passed++;
        if ((error=dis_x(move) != 2)) return passed+error;
        passed++;

        char pos[3] = {dif_x(move)<0? 'a': 'h', move[1]};
        piece* rook = BOARD(pos);
        if ((error=rook->status != DID_NOT_MOVE_YET)) return passed+error;
        passed++;

        // MRC = 3
        pos[0] = king->pos[0]+dif_x(move);
        while (pos[0] != rook->pos[0]){
            if ((error=BOARD(pos)->type != NONE_OF_THEM)) return passed+error;
            passed++;
            pos[0] += dir_x(move);
        }

        char c=0;
        while (c <3){
            if ((error=is_checked())) break;
            passed += MRC_IS_CHECKED;

            if (++c >= 3) break;
            BOARD(king->pos) = p_Unoccupied;
            king->pos[0] += dir_x(move);
            BOARD(king->pos) = king;
        }
        BOARD(king->pos) = p_Unoccupied;
        king->pos[0] = move[0];
        BOARD(king->pos) = king;

        if (error) return passed+error;
        return 0;
    }



    #define MRC_NOT_LEGAL_MOVE          \
            MRC_NOT_POSSIBLE_MOVE       \
            + MRC_NOT_EN_PASSANT        \
            + MRC_NOT_CASTLING          \
            + MRC_NOT_COMMON_MOVE
    int not_legal_move(char* move){
        int passed = 0, error;
        
        if ((error=not_possible_move(move))) return passed+error;
        passed += MRC_NOT_POSSIBLE_MOVE;

        if (BOARD(move)->type == PAWN && BOARD(move+2)->type == NONE_OF_THEM && dis_x(move) == 1){
            // en passant
            if ((error=not_en_passant(move))) return passed+error;
            return 0;
        }
        passed += MRC_NOT_EN_PASSANT;

        if (BOARD(move)->type == KING && dis_x(move) == 2) {
            // castling
            
            if ((error=not_castling(move))) return passed+error;
            return 0;
        }
        passed += MRC_NOT_CASTLING;

        // common moves
        if ((error=not_common_move(move))) return passed+error;
        passed += MRC_NOT_COMMON_MOVE;

        if ((error=not_safe_move(move))) return passed+error;
        passed += MRC_NOT_COMMON_MOVE;

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
                     && Movements[Term-1][3] == pos[0]
                     && Movements[Term-1][4] == pos[1]? STYLE_LAST_ACTION: "",
                    BOARD(pos)->symbol, STYLE_DEFAULT);
                }
            }
        // 

        // border and labels
            row = 1; col = 5;
            printf("\e[%d;%dH%s", row, col+1, "╭────────────────────────╮");
            printf("\e[%d;%dH%s", row+9, col+1, "╰─a──b──c──d──e──f──g──h─╯");
            for (char y='8'; y>='1'; --y){
                printf("\e[%d;%dH%c", row+1+('8'-y), col, y);
                printf("\e[%d;%dH%s", row+1+('8'-y), col+1, "│");
                printf("\e[%d;%dH%s", row+1+('8'-y), col+26, "│");
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
            struct winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

            row = 1; col = 37;
            int width = 21, r;
            int move = MaxTerm-((w.ws_col-col+1)/width*8*2);
            if (!(move&1)) move++;
            if (Term-1 < move) move = Term-1;
            if (!(move&1)) move--;
            if (move <= 0) move = 1;

            while (col+width-1 <= w.ws_col){
                r = row;
                printf("\e[%d;%dH%s", r++, col, " No. white   black  ");
                printf("\e[%d;%dH%s", r++, col, "─────────────────────");
                for (; r<row+10; ++r){
                    if (move >= MaxTerm) break;
                    printf("\e[%d;%dH %-3d %s%-6s%s  %s%-6s%s ", 
                    r, col, (move+1)/2, 
                    (move+1==Term? STYLE_HIGHLIGHT: ""), Movements[move],STYLE_DEFAULT,
                    (move+2==Term? STYLE_HIGHLIGHT: ""), Movements[move+1], STYLE_DEFAULT);
                    move += 2;
                }
                col += width;
                if (col+width-1 > w.ws_col || move >= MaxTerm) break;

                r = row;
                printf("\e[%d;%dH%s", r++, col-1, "│");
                printf("\e[%d;%dH%s", r++, col-1, "┼");
                for (; r<row+10; ++r){
                    printf("\e[%d;%dH%s", r, col-1, "│");
                }
            }
        // 

        printf("\e[%d;%dH%s", 12, 6, STYLE_DEFAULT);
        return 0;
    }

    char reset_game(){
        Term = 1;

        Players[BLACK] = (player){
            {
                {BLACK,  DID_NOT_MOVE_YET, KING,   "e8", (char*)PIECE_SYMBOLS[BLACK][KING]  }, 
                {BLACK,  DID_NOT_MOVE_YET, QUEEN,  "d8", (char*)PIECE_SYMBOLS[BLACK][QUEEN] }, 
                {BLACK,  DID_NOT_MOVE_YET, ROOK,   "a8", (char*)PIECE_SYMBOLS[BLACK][ROOK]  }, 
                {BLACK,  DID_NOT_MOVE_YET, ROOK,   "h8", (char*)PIECE_SYMBOLS[BLACK][ROOK]  }, 
                {BLACK,  DID_NOT_MOVE_YET, BISHOP, "c8", (char*)PIECE_SYMBOLS[BLACK][BISHOP]}, 
                {BLACK,  DID_NOT_MOVE_YET, BISHOP, "f8", (char*)PIECE_SYMBOLS[BLACK][BISHOP]}, 
                {BLACK,  DID_NOT_MOVE_YET, KNIGHT, "b8", (char*)PIECE_SYMBOLS[BLACK][KNIGHT]}, 
                {BLACK,  DID_NOT_MOVE_YET, KNIGHT, "g8", (char*)PIECE_SYMBOLS[BLACK][KNIGHT]}, 
                {BLACK,  DID_NOT_MOVE_YET, PAWN,   "a7", (char*)PIECE_SYMBOLS[BLACK][PAWN]  }, 
                {BLACK,  DID_NOT_MOVE_YET, PAWN,   "b7", (char*)PIECE_SYMBOLS[BLACK][PAWN]  }, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "c7", (char*)PIECE_SYMBOLS[BLACK][PAWN]  }, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "d7", (char*)PIECE_SYMBOLS[BLACK][PAWN]  }, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "e7", (char*)PIECE_SYMBOLS[BLACK][PAWN]  }, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "f7", (char*)PIECE_SYMBOLS[BLACK][PAWN]  }, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "g7", (char*)PIECE_SYMBOLS[BLACK][PAWN]  }, 
                {BLACK, DID_NOT_MOVE_YET, PAWN,   "h7", (char*)PIECE_SYMBOLS[BLACK][PAWN]  }
            }
        };

        Players[WHITE] = (player){
            {
                {WHITE,  DID_NOT_MOVE_YET, KING,   "e1", (char*)PIECE_SYMBOLS[WHITE][KING]  }, 
                {WHITE,  DID_NOT_MOVE_YET, QUEEN,  "d1", (char*)PIECE_SYMBOLS[WHITE][QUEEN] }, 
                {WHITE,  DID_NOT_MOVE_YET, ROOK,   "a1", (char*)PIECE_SYMBOLS[WHITE][ROOK]  }, 
                {WHITE,  DID_NOT_MOVE_YET, ROOK,   "h1", (char*)PIECE_SYMBOLS[WHITE][ROOK]  }, 
                {WHITE,  DID_NOT_MOVE_YET, BISHOP, "c1", (char*)PIECE_SYMBOLS[WHITE][BISHOP]}, 
                {WHITE,  DID_NOT_MOVE_YET, BISHOP, "f1", (char*)PIECE_SYMBOLS[WHITE][BISHOP]}, 
                {WHITE,  DID_NOT_MOVE_YET, KNIGHT, "b1", (char*)PIECE_SYMBOLS[WHITE][KNIGHT]}, 
                {WHITE,  DID_NOT_MOVE_YET, KNIGHT, "g1", (char*)PIECE_SYMBOLS[WHITE][KNIGHT]}, 
                {WHITE,  DID_NOT_MOVE_YET, PAWN,   "a2", (char*)PIECE_SYMBOLS[WHITE][PAWN]  }, 
                {WHITE,  DID_NOT_MOVE_YET, PAWN,   "b2", (char*)PIECE_SYMBOLS[WHITE][PAWN]  }, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "c2", (char*)PIECE_SYMBOLS[WHITE][PAWN]  }, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "d2", (char*)PIECE_SYMBOLS[WHITE][PAWN]  }, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "e2", (char*)PIECE_SYMBOLS[WHITE][PAWN]  }, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "f2", (char*)PIECE_SYMBOLS[WHITE][PAWN]  }, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "g2", (char*)PIECE_SYMBOLS[WHITE][PAWN]  }, 
                {WHITE, DID_NOT_MOVE_YET, PAWN,   "h2", (char*)PIECE_SYMBOLS[WHITE][PAWN]  }
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



    char apply_action(char* movement){
        for (piece* p_pc=Players[TURN].pieces; p_pc<=Players[TURN].pieces+15; ++p_pc){
            if (p_pc->status == MOVED_JUST_NOW){
                p_pc->status = NONE_OF_THEM;
            }
        }

        char* move = movement+1;
        piece* p_pc1 = BOARD(move);
        piece* p_pc2 = BOARD(move+2);
        DO_MOVE(move);
        p_pc1->status = p_pc1->status == DID_NOT_MOVE_YET? MOVED_JUST_NOW: NONE_OF_THEM;

        if (p_pc1->type == PAWN
        && ((p_pc1->owner == WHITE && p_pc1->pos[1] == '8')
            || (p_pc1->owner == BLACK && p_pc1->pos[1] == '1'))){
            // promotion
            int type=0;
            if (movement[5]){
                for (type=1; type<4&&MOVEMENT_SYMBOLS[type]!=movement[5]; ++type);
            } else {
                printf("\e[%d;%dH%s", 14, 6, STYLE_RESPONSE);
                for (char tp=1; tp<5; ++tp)
                    printf("%d: %s, ", tp, PIECE_SYMBOLS[TURN][tp]);
                printf("\b\b \n");

                while (type<1 || 4<type){
                    printf("\e[%d;%dH%s%s", 15, 6, STYLE_PROMPT, "which type of piece do you promote to? ");
                    scanf("%d", &type);
                    while (getchar() != '\n');
                }
                printf(STYLE_DEFAULT);
            }
            movement[5] = MOVEMENT_SYMBOLS[type];
            movement[6] = '\0';

            p_pc1->type = type;
            p_pc1->symbol = (char*)PIECE_SYMBOLS[TURN][type];
        }
        if (p_pc1->type == PAWN && p_pc2->type == NONE_OF_THEM && dis_x(move) == 1){
            // en passant
            char pos[3] = {move[0]+dir_x(move), move[1]};
            BOARD(pos)->status = DEAD;
            BOARD(pos) = p_Unoccupied;
        }
        if (p_pc1->type == KING && dis_x(move) == 2) {
            // castling
            char rookMove[5] = {dif_x(move)<0? 'a': 'h', move[1], dif_x(move)<0? 'd': 'f', move[1]};
            DO_MOVE(rookMove);
        }

        sprintf(Movements[Term++], "%s", movement);
        if (Term>MaxTerm) MaxTerm=Term;
        return 0;
    }

    char any_legal_move(char* move){
        move[4] = '\0';
        for (piece* p_pc=Players[TURN].pieces; p_pc<=Players[TURN].pieces+15; ++p_pc){
            move[0] = p_pc->pos[0];
            move[1] = p_pc->pos[1];
            for (move[2]='a';  move[2]<='h'; ++move[2]){
                for (move[3]='1';  move[3]<='8'; ++move[3]){
                    if (!not_legal_move(move)){
                        return 1;
                    }
                }
            }
        }

        return 0;
    }

    char end_of_game(){
        char move[5];
        if (!any_legal_move(move)){
            if (is_checked()){
                printf(STYLE_RESPONSE"the game ended in checkmate. "STYLE_DEFAULT);
            } else {
                printf(STYLE_RESPONSE"the game ended in stalemate. "STYLE_DEFAULT);
            }
            while (getchar() != '\n');
            g_end = 1;
            return 1;
        }


        int alive = 0;
        char knightOrBishop = 0;
        for (char pl=0; pl<2; ++pl){
            for (piece* p_pc=Players[pl].pieces; p_pc<=Players[pl].pieces+15; ++p_pc){
                alive += p_pc->status != DEAD;
                knightOrBishop |= p_pc->status != DEAD 
                        && (p_pc->type == KNIGHT || p_pc->type == BISHOP);
            }
        }

        if (alive == 2 || (alive == 3 && knightOrBishop)){
            // dead position
            printf(STYLE_RESPONSE"the game ended in dead position. "STYLE_DEFAULT);
            while (getchar() != '\n');
            g_end = 1;
            return 1;
        }


        int repetition = 0, noCaptureOrPawn = 0, term = Term;
        player players[2] = {Players[0], Players[1]};

        reset_game();
        while (Term<term){
            noCaptureOrPawn++;
            if (BOARD(Movements[Term]+3)->type != NONE_OF_THEM
            ||  BOARD(Movements[Term]+1)->type == PAWN){
                noCaptureOrPawn = 0;
            }

            apply_action(Movements[Term]);
            if (!memcmp(&players, &Players, sizeof(player)*2)){
                repetition++;
            }
        }

        if (repetition >= 5){
            // fivefold repetition
            printf(STYLE_RESPONSE"the game ended in fivefold repetition. "STYLE_DEFAULT);
            while (getchar() != '\n');
            g_end = 1;
            return 1;
        } else if (repetition >= 3){
            // threefold repetition
            printf(STYLE_RESPONSE"the game may end in threefold repetition. "STYLE_DEFAULT);
            while (getchar() != '\n');
            return 1;
        }


        if (noCaptureOrPawn >= 75){
            // seventy-five-move
            printf(STYLE_RESPONSE"the game ended in seventy-five-move rule. "STYLE_DEFAULT);
            while (getchar() != '\n');
            g_end = 1;
            return 1;
        } else if (noCaptureOrPawn >= 50){
            // fifty-move
            printf(STYLE_RESPONSE"the game may end in fifty-move rule. "STYLE_DEFAULT);
            while (getchar() != '\n');
            return 1;
        }


        if (is_checked()){
            printf(STYLE_RESPONSE"check. "STYLE_DEFAULT);
            while (getchar() != '\n');
        }
        return 0;
    }



    void help(){
        printf(
            "\e[%d;%dH\n"
           "chepss is a simple chess game.                                                                              \n"
           "which is clear to understand, easy to modify, strong to be extended                                         \n"
           " and general to be used as a template for other board games.                                                \n"
           "                                                                                                            \n"
           "    commands:                                                                                               \n"
           "        [piece_type]<source><dest>  move the piece in the source square to the destination square           \n"
           "        goto [<round>[.<turn>]]     navigate in game and continue, 0 for the last action, nothing to next   \n"
           "        save <file>                 save the game as a plain text file, you can change the file manually    \n"
           "        exit                        exit current game then you can restore any saved one                    \n"
           "        quit                        quit the game                                                           \n"
           "        help                        display this help                                                       \n"
           "                                                                                                            \n"
           "    examples:                                                                                               \n"
           "        Pe2e4  or  e2e4             moves (pawn) from e2 to e4                                              \n"
           "        goto 1 or 1.0               navigate to round 1, turn 0 (white's turn)                              \n"
           "        goto 2.1                    navigate to round 2, turn 1 (black's turn)                              \n"
           "        goto 0                      navigate to last round, last turn                                       \n"
           "        goto                        navigate to the next action if done before                              \n"
           "        save session.txt            saves the game in 'session.txt' file                                    \n"
           "        exit                        exit the game then you can restore any saved one like 'session.txt'     \n"
           "\n"
           "%shttps://github.com/sirnaser/chepss.git%s ",
            13, 1, STYLE_LINK, STYLE_DEFAULT);
        while (getchar() != '\n');
    }

    void save_game(char* file){
        FILE* game = fopen(file, "w");
        if (game != NULL){
            int term = 1;
            while (term<MaxTerm){
                fprintf(game, "%-6s  ", Movements[term++]);
                if (term<MaxTerm){
                    fprintf(game, "%-6s\n", Movements[term++]);
                }
            }
            fclose(game);
            printf("\e[%d;%dH%s%s%s", 14, 6, STYLE_RESPONSE, "successful save. ", STYLE_DEFAULT);
            while (getchar() != '\n');
        } else {
            printf("\e[%d;%dH%s%s%s", 14, 6, STYLE_ERROR, "no such file or directory! ", STYLE_DEFAULT);
            while (getchar() != '\n');
        }
    }

    void goto_term(int term){
        g_end = 0;
        reset_game();

        if (term > MaxTerm) term = MaxTerm;
        while (Term<term){
            apply_action(Movements[Term]);
        }
        end_of_game();
    }

    char get_action(char* action){
        printf("\e[%d;%dH%i.%i> ", 12, 6, (Term+1)/2, !TURN);
        scanf("%[^\n]s", action);
        while (getchar() != '\n');

        if (action[0] == 'q'
        &&  action[1] == 'u'
        &&  action[2] == 'i'
        &&  action[3] == 't'){
            g_quit = 1;
            return 1;
        }

        if (action[0] == 'e'
        &&  action[1] == 'x'
        &&  action[2] == 'i'
        &&  action[3] == 't'){
            g_exit = 1;
            return 1;
        }

        if (action[0] == 'h'
        &&  action[1] == 'e'
        &&  action[2] == 'l'
        &&  action[3] == 'p'){
            help();
            return 1;
        }

        if (action[0] == 's'
        &&  action[1] == 'a'
        &&  action[2] == 'v'
        &&  action[3] == 'e'){
            save_game(action+5);
            return 1;
        }

        if (action[0] == 'g'
        &&  action[1] == 'o'
        &&  action[2] == 't'
        &&  action[3] == 'o'){
            int round=-1, turn=0;
            sscanf(action+4, "%d.%d", &round, &turn);
            if (round == -1){
                goto_term(Term+1);
            } else if (round == 0){
                goto_term(MaxTerm);
            } else {
                goto_term(round*2-1+!(!turn));
            }
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
            while (fscanf(game, "%s", Movements[Term++]) == 1);
            MaxTerm = --Term;
            goto_term(Term);
            fclose(game);
        }

        return 0;
    }

    void error_message(char error){
        printf("\e[%d;%dH%s", 14, 6, STYLE_ERROR);

        // if (1 <= error && error <= MRC_NOT_LEGAL_MOVE){
        //     printf("not a legal move> ");
        //     if (1 <= error && error <= MRC_NOT_IN_BOARD){
        //         printf("source position> ");
        //         if (error == 1) printf("x so low");
        //         if (error == 2) printf("x so high");
        //         if (error == 3) printf("y so low");
        //         if (error == 4) printf("y so hight");
        //     } error -= MRC_NOT_IN_BOARD;

        //     if (1 <= error && error <= MRC_NOT_IN_BOARD){
        //         printf("dest position> ");
        //         if (error == 1) printf("x so low");
        //         if (error == 2) printf("x so high");
        //         if (error == 3) printf("y so low");
        //         if (error == 4) printf("y so hight");
        //     } error -= MRC_NOT_IN_BOARD;

        //     if (error == 1) printf("piece is dead");
        //     if (error == 2) printf("piece is not yours");
        //     if (error == 3) printf("dest is your piece");
        // } error -= MRC_NOT_LEGAL_MOVE;

        // if (1 <= error && error <= MRC_NOT_VALID_MOVE){
        //     printf("not a valid move> ");
        //     if (1 <= error && error <= MRC_NOT_PAWN_MOVE){
        //         printf("pawn> ");
        //         if (error == 1) printf("can not move horizontally");
        //         if (error == 2) printf("white pawn first move should go upper");
        //         if (error == 3) printf("black pawn first move should go lower");
        //         if (error == 4) printf("can not move more than 2 distances in first move");
        //         if (error == 5) printf("there is an intervening piece");
        //         if (error == 6) printf("can not capture in first move");
        //         if (error == 7) printf("can not move more than 1 distance (except first move)");
        //     } error -= MRC_NOT_PAWN_MOVE;

        //     if (1 <= error && error <= MRC_NOT_KNIGHT_MOVE){
        //         printf("knight> ");
        //         if (error == 1) printf("can not move like that");
        //     } error -= MRC_NOT_KNIGHT_MOVE;

        //     if (1 <= error && error <= MRC_NOT_BISHOP_MOVE){
        //         printf("bishop> ");
        //         if (error == 1) printf("not a diagonal move");
        //         if (error == 2) printf("can not go left in the left half");
        //         if (error == 3) printf("can not go right in the right half");
        //         if (error == 4) printf("can not go lower in the lower half");
        //         if (error == 5) printf("can not go upper in the upper half");
        //         if (error >= 6) printf("there is an intervening piece in step %d", error-5);
        //     } error -= MRC_NOT_BISHOP_MOVE;

        //     if (1 <= error && error <= MRC_NOT_ROOK_MOVE){
        //         printf("rook> ");
        //         if (error == 1) printf("not a vertical or horizontal move");
        //         if (error >= 2) printf("there is an intervening piece in step %d", error-1);
        //     } error -= MRC_NOT_ROOK_MOVE;

        //     if (1 <= error && error <= MRC_NOT_QUEEN_MOVE){
        //         printf("queen> ");
        //         if (error == 1) printf("is not any type of queen moves");
        //     } error -= MRC_NOT_QUEEN_MOVE;

        //     if (1 <= error && error <= MRC_NOT_KING_MOVE){
        //         printf("king> ");
        //         if (error == 1) printf("can not move more than 1 distance horizontally");
        //         if (error == 2) printf("can not move more than 1 distance vertically");
        //     } error -= MRC_NOT_KING_MOVE;
        // } error -= MRC_NOT_VALID_MOVE;

        // if (1 <= error && error <= MRC_NOT_SAFE_MOVE){
        //     printf("not a safe move> you will be checked by %s piece", Players[!TURN].pieces[error-1].pos);
        // } error -= MRC_NOT_SAFE_MOVE;
        printf(STYLE_DEFAULT);

        char legalMove[5];
        any_legal_move(legalMove);
        printf(" (error code: %i) \n%stry a legal move like %s%s ", error, STYLE_PROMPT, legalMove, STYLE_DEFAULT);
        while (getchar() != '\n');
    }

    char validate_action(char* movement){
        char error;
        if ((error=not_legal_move(movement+1))){
            error_message(error);
            return 1;
        }
        return 0;
    }
// 


int main(){
    char action[50];

    while (!g_quit) {
        start_game();

        while (!g_quit && !g_exit){
            if (display()) continue;

            if (get_action(action) || g_end) continue;

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

// using define macro instead of enums:
    // i used some define macros instead of enums, 
    // because 4 bytes is too big for less than ten different status.
//  

// max return code protocol (MRC):
    // this is an innovative protocol for validation, 
    // that guarantees to distinguish every single invalidity.

    // if everything is fine, the validation functions should return 0, 
    // otherwise they should return a specific positive number as error code.

    // to achieve this, for each validation function, we define an MRC variable 
    // that represents the maximum error code returned by the function.

    // then for larger validation functions that need to check smaller functions,
    // MRC should be the sum of smaller functions MRS's.

    // and the return code for errors that occur in validation n should be 
    // the sum of the MRSs of the validations before n, 
    // plus the error code returned by validation n.
// 

// printf formatting protocol:
    // if no identifier used, pass the style values in format string directly
    // otherwise use %s identifier to pass style values as arguments
// 

// TODO:
    // error messages
// 

// thank you for your patience, to get something perfect. ♥️
// https://github.com/sirnaser/chepss.git
