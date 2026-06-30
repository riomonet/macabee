#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "libsodium-stable/build/include/sodium.h"
#include <assert.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "uthash.h"
#include "mongoose.h"
#include "sqlite3.h"
#include "cJSON.h"
#include "mac_types.h"

#define DEV_MODE 1
#define DB_RESET 0

#define UID_NF UINT16_MAX // UID NOT FOUND. [SENTINEL]

sqlite3 *db;

/* Role values must be explictly defined or db values will no align. */
enum ROLES {
    ROLE_NONE = 0,
    ROLE_ALPHA = 1,
    ROLE_OFFICE = 2,
    ROLE_YARD = 3,
    ROLE_ACCOUNTING = 4,
  };

/* ============================================================================
              SCREEN MACROS SYSTEM (DSL)
=============================================================================== */

/* ---------------------------- screen definitions ------------------------------------------------------------------- */


#define LABEL(id, yy, xx, ww, txt)                                  \
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, 0, 0, 0, 0, 0)

#define INPUT(id, yy, xx, ww)                           \
    X(id, VIS_INPUT, xx, yy, ww, "", 0, 0, 0, 0,0,0)

#define INPUT_F(id, yy, xx, ww,flg)                     \
    X(id, VIS_INPUT, xx, yy, ww, "", 0, flg, 0, 0,0,0)

#define HL(id, yy, xx, ww)                          \
    X(id, VIS_LINE, xx, yy, ww, "", 0, 0, 0, 0,0,0)

#define LABEL_F(id, yy, xx, ww, txt, flg)                           \
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, flg, 0, 0,0,0)

#define LABEL_C(id, yy, xx, ww, txt, col)                           \
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, 0, col, 0,0,0)

#define LABEL_FC(id, yy, xx, ww, txt, flg, col)                         \
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, flg, col, 0,0,0)

#define LABEL_FCF(id, yy, xx, ww, txt, flg, col)                        \
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, flg, col, CLICKABLE ,0,0)

#define STATUS(id, yy, xx, ww, txt, flg)        \
    LABEL_F(id, yy, xx, ww, txt, flg)

#define STATE(id, txt, flg, col)                \
    X(id, txt, sizeof(txt)-1, flg, col)

#define STATE_LEN(id, txt, len, flg, col)       \
    X(id, txt, len, flg, col)

#define MAX_SLOTS(arr) (sizeof(arr)/sizeof(arr[0]))
#define STR2(X) #X
#define STR(X) STR2(X)

#define MAKE_SCREEN_DEF(opA, opB, layout_arr, state_arr, cursor_pos, cnt)   \
    {       .op_A = (opA),                                  \
            .op_B = (opB),                                  \
            .layout = (layout_arr),                         \
            .state = (state_arr),                           \
    .nFields = cnt,                                         \
            .ic = (cursor_pos)                              \
    }


/* -------------------- SCREEN DEFINTIONS START--------------------- */

typedef int (*screen_handler)(struct player *, u8 *);
typedef void(*screen_renderer)(struct player *);

#define SCREENS_LIST                                \
    YMB(LOGIN_SCREEN, login_screen, LOGIN_IUSER)   \
    YMB(MAIN_SCREEN, main_screen, MAIN_ISELECT)    \
    YMB(M_ACT_SCREEN, m_act_screen, M_ACT_CODE)    \
    YMB(MENU_TEMPLATE, menu_template,IC_NONE)   

#define SCREEN_TEMPLATES                        \
    YMB(MENU_TEMPLATE, menu_template,IC_NONE)   


/* ENUM FOR SCRID, SCRID_NO_SCREEN is screen 0 */
#define YMB(SCR,scr,IC) SCRID_##SCR,
enum SCRID {SCRID_NO_SCREEN, SCREENS_LIST};
#undef YMB

/* Function prototypes for handlers  */
#define YMB(scrid, name, select)                \
    int handler_##name(struct player *p,u8 *reqbuf);

int handler_no_screen(struct player *p,u8 *reqbuf);
SCREENS_LIST
#undef YMB

/* Function prototypes for renderers */
#define YMB(scrid, name, select)                \
    void renderer_##name(struct player *p);

void renderer_no_screen(struct player *p);
SCREENS_LIST
#undef YMB

/* Array of handlers */
#define YMB(scrid, name, select)                \
    [SCRID_##scrid] = handler_##name,

screen_handler screen_handlers[] = {
    [SCRID_NO_SCREEN] = handler_no_screen,
    SCREENS_LIST
};
#undef YMB

/* array of renderers */
#define YMB(scrid, name, select)                \
    [SCRID_##scrid] = NULL,

screen_renderer screen_renderers[] ={
    [SCRID_NO_SCREEN] = NULL,
    SCREENS_LIST
};
#undef YMB



/* HANDLERS AND RENDERERS */

#define H_NO_ACTION 0xFFFFFFFFu

enum H_NO_SCREEN {
    H_NO_SCR_WEB_LOGIN,
    H_NO_SCR_MOBILE_ACTIVATE
};

int handler_no_screen(struct player *p, u8 *reqbuf) {

    (void) p;
    int Op_A = reqbuf[0];
    /* web user */
    if(Op_A == 0x88) {
        return H_NO_SCR_WEB_LOGIN;
    }
    
    /* mobile user */
    else if (Op_A == 0x89) {
        return H_NO_SCR_MOBILE_ACTIVATE;
    }
    return H_NO_ACTION;
}

/*______________________________MAIN SCREEN__________________________ */

int handler_menu_template(struct player *p , u8 *reqbuf) {
    (void)    p;
    (void)    reqbuf;
    return 0;
}


void renderer_menu_template(struct player *p) {
    char *r[] = {
        "ari", "safari", "zoo"
    };
}

enum H_MAIN_SCREEN {
    H_MAIN_LOGOUT
};


int handler_main_screen(struct player *p , u8 *reqbuf) {

    p->scrat = 1;
    //    (void) p;
    (void) reqbuf;
    #if 0
    int k = get_aid_key(reqbuf);
    switch(k) {
    case 0xf1: break;
    case 0xf2: break;
    case 0xf3: break;
    case 0xFF: break;
    }
    #endif
    return H_NO_ACTION;
}

void renderer_main_screen(struct player *p) {
    (void)p;
        
}
    

/*______________________________LOGIN SCREEN__________________________ */
enum H_LOGIN_SCREEN {
    H_LOGIN_SCR_SUCCESS,
};
int try_login(struct player *, u8 *);
int handler_login_screen(struct player *p, u8 *reqbuf) {
    if (try_login(p,reqbuf)) {
        return H_LOGIN_SCR_SUCCESS;
    }
    return H_NO_ACTION;
}

/*______________________________END LOGIN SCREEN__________________________ */


enum H_M_ACT_SCREEN {
    RC_HANDLER_M_ACT_LOGOUT
};

/* MOBILE HANDLERS AND RENDERERS */
int handler_m_act_screen(struct player *p, u8 *reqbuf) {
    (void) p;
    (void) reqbuf;
     return 1;
}

/* SCREEN STUBS */
#define SCREEN_STUB_HEADER(SCR,title)                                   \
    LABEL(SCR##_FLD_USER,1,7,32, "")                                    \
    LABEL(SCR##_FLD_DATE,1,67,32, "")                                   \
    LABEL(SCR##_FLD_TIME,2,67,12, "")                                   \
    LABEL(SCR##_FLD_TITLE,1,29,21, title)                               \
    LABEL_FC(SCR##_L6,6,6,28, "Select one of the following:", FAINT, CYAN)  

#define SCREEN_STUB_FOOTER(SCR)                             \
    LABEL(SCR##FLD_SELECTION,23,1,9,"Selection")            \
    LABEL(SCR##FLD_ARROW,24,1,4, "-->")                     \
    HL(SCR##_FLD_HL1,26,1,100)                              \
    LABEL_FC(SCR##_FLD_F1,28,6,9,"F2=Logout", FAINT, CYAN)  \
    LABEL(SCR##_FLD_F2,28,19,9, "")                         \
    LABEL(SCR##_FLD_F3,28,31,16,"")                         \
    HL(SCR##_FLD_HL2,29,0,100)                              \
    HL(SCR##_FLD_HL3,24,7,90)                               \
    INPUT(SCR##_ISELECT,24,6,1)


#define NO_SCREEN NULL
#define IC_NONE -1

/* LOGIN SCREEN row, col */
#define LOGIN_SCREEN                                                    \
    LABEL(LOGIN_L1       , 8,   9, 27, "USER . . . . . . . . . . . ")   \
    LABEL(LOGIN_L2       , 10,  9, 27, "PASSWORD . . . . . . . . . ")   \
    INPUT(LOGIN_IUSER    , 8,  38, 24)                                  \
    INPUT_F(LOGIN_IPW    , 10, 38, 24,  PASSWORD)                       \
    LABEL_FC(LOGIN_L3    , 5,   5, 37, "Tab to change fields, Enter to submit", FAINT, CYAN) \
    LABEL_C(LOGIN_L4     , 1,  40, 19, "Marina 59 | Sign On", WHITE)    \
    STATUS(LOGIN_WARNING , 12, 38, 42, "", HIDDEN)                      

/*   id, col, row, width */
/* #define MAIN_SCREEN                                             \ */

#define MAIN_SCREEN                                             \
    SCREEN_STUB_HEADER(MAIN_SCREEN, "Marina 59 | Main Menu" )   \
    LABEL(MAIN_L7,8,10,21,   "1. Create New Contact")           \
    SCREEN_STUB_FOOTER(MAIN)


#define MENU_TEMPLATE                           \
    LABEL_F(MENU_1 , 8, 9, 27, "",HIDDEN)       \
    LABEL_F(MENU_2 ,10, 9, 27, "",HIDDEN)       \
    LABEL_F(MENU_3 ,12, 9, 27, "",HIDDEN)       \
    LABEL_F(MENU_4 ,14, 9, 27, "",HIDDEN)       \
    LABEL_F(MENU_5 ,16, 9, 27, "",HIDDEN)       \
    LABEL_F(MENU_6 ,18, 9, 27, "",HIDDEN)       \
    LABEL_F(MENU_7 ,20, 9, 27, "",HIDDEN)       \
    LABEL_F(MENU_8 ,22, 9, 27, "",HIDDEN)       \
    LABEL_F(MENU_9 ,24, 9, 27, "",HIDDEN)       \
    LABEL_F(MENU_10,26, 9, 27, "",HIDDEN)       \
    LABEL_F(MENU_11,28, 9, 27, "",HIDDEN)       \
    LABEL_F(MENU_12,30, 9, 27, "",HIDDEN) 
        

#define MAIN_SCREEN_ALPHA                               \
    SCREEN_STUB_HEADER(MAIN, Marina 59 | MAIN MENU )    \
    LABEL(MAIN_L7,8,10,15,   "1. Send SMS invite")      \
    LABEL(MAIN_L8,9,10,15,   "2. Send email invite")    \
    LABEL(MAIN_L9,10,10,15,   "3. Add new contact")     \
    SCREEN_STUB_FOOTER(MAIN)

#define ADD_NEW_USER                                                    \
    LABEL(ADD_USER_LAB_FIRST , 8 ,  9, 27, "FIRST NAME . . . . . . . . . . . ") \
    LABEL(ADD_USER_LAB_LAST  , 10, 9, 27, "LAST NAME  . . . . . . . . . . . ") \
    LABEL(ADD_USER_LAB_EMAIL , 12, 9, 27, "EMAIL  . . . . . . . . . . . . . ") \
    LABEL(ADD_USER_LAB_PHONE , 14, 9, 27, "PHONE  . . . . . . . . . . . . . ") \
    INPUT(ADD_FIRST    ,8,  38, 24)                                     \
    INPUT(ADD_LAST     ,10, 38, 24)                                     \
    INPUT(ADD_EMAIL    ,12, 38, 24)                                     \
    INPUT(ADD_PHONE    ,14, 38, 24)                                     \
    LABEL_FC(LOGIN_L3    , 5,   5, 37, "Tab to change fields, Enter to submit", FAINT, CYAN) \
    LABEL_C(LOGIN_L4     , 1,  40, 19, "Marina 59 | Add New Macabee user", WHITE) \
    STATUS(LOGIN_WARNING , 32, 12, 42, "", HIDDEN)
 

/* MOBILE SCREENS (1,1) -> (59,45) */
#define M_ACT_SCREEN                                                    \
    LABEL_F(M_ACT_INST_TIT,    5, 6 ,13 , "Instructions:", BOLD)             \
    LABEL_FC(M_ACT_INST_L1,     7, 6 ,33 , "We sent you an activation code in",FAINT, CYAN) \
    LABEL_FC(M_ACT_INST_L2,     8, 6 ,31 , "a text message. Please enter it", FAINT, CYAN) \
    LABEL_FC(M_ACT_INST_L3,     9, 6 ,37 , "after the prompt then press activate.",FAINT,CYAN) \
    LABEL_F(M_ACT_TITLE,     13, 6 ,9 , "Marina 59", BOLD)                   \
    LABEL(M_ACT_MSG,       15, 6, 22, "Enter Activation Code:")         \
    INPUT(M_ACT_CODE,      15, 30, 9 )                                  \
    LABEL_FCF(M_ACT_ENTER,  19, 6, 12, " [ACTIVATE] ", BOLD | INVERSE, GREEN)

//(1,5) - (45,59)
// 45 wide y range is 5-59 x range is 1-45


/* -------------------------------------------- END SCREEN DEFIvNTIONS --------------------------------------------- */
#define X(id, t, x, y, w, txt, len, flg, col,r1,r2,r3) id,
#define YMB(SCR,scr, IC) enum SCR##_IDX {SCR SCR##_FIELD_COUNT};
SCREENS_LIST
#undef X
#undef YMB


#define YMB(SCR,scr,IC) struct field_layout scr##_layout[] = {SCR};   
#define X(id, t, xx, yy, w, txt, len, flg, col, ht, rr2, rr3)                 \
    { .field_id = (id), .type = (t), .x = (xx), .y = (yy), .width = (w), .height = (ht), .r2= (rr2), .r3 = (rr3) },
SCREENS_LIST
#undef X
#undef YMB

#define YMB(SCR,scr,IC) struct field_state scr##_state[] = {SCR};
#define X(id, t, x, y, w, txt, len, flg, col, flg_h, rr2, rr3)        \
    { .field_id = (id), .text = (txt), .text_len = (len),   \
    .fg_color = (col), .flags = (flg), .flags_h = (flg_h), .r2 = (rr2), .r3 = (rr3) },
SCREENS_LIST
#undef X
#undef YMB

struct net_payload_screen {
    int id;
    u8 *buf;
    size_t len;
};

struct net_payload_screen serialize_screen(struct field_state *fs, struct field_layout *fl, int num_fields, u8 opA, u8 opB, u8 ic, u8 *buf) {
    int is_new = 0;
    if (opA == OP_A_NEW)  {
        is_new = 1;
    }

    size_t layout_bytes = (is_new * num_fields) * sizeof(struct field_layout);
    size_t state_bytes =  num_fields * sizeof(struct field_state);
    size_t total_bytes = layout_bytes  + state_bytes + sizeof(struct packet_header);

    struct packet_header h = { .opcode_a = opA,
                               .opcode_b = opB,
                               .num_fields = num_fields,
                               .reserved = ic,
                               .layout_bytes = layout_bytes, /* u16, Num bytes in each array. */
                               .state_bytes =  state_bytes
    };
    
    struct net_payload_screen netscr = {
        .id = 1,
        .buf = buf,
        .len = total_bytes
    };

    u8 *pos = netscr.buf;

    memcpy(pos, &h, sizeof(h));
    pos += sizeof(h);

    if (is_new) {
    memcpy(pos, fl, h.layout_bytes);
    pos += layout_bytes;
    }

    memcpy(pos,fs, state_bytes);
    pos += state_bytes;

    return netscr;
}

/* |--------------------------------------- GLOBAL SCREEN TEMPLATES -------------------------------- */

/* Global Screen templates:  */

#define YMB(SCR,scr,IC)	[SCRID_##SCR] = MAKE_SCREEN_DEF(OP_A_NEW, OP_B_DEF, scr##_layout, scr##_state, IC, SCR##_FIELD_COUNT),
struct screen screens[] = {SCREENS_LIST};
#undef YMB


/* ---------------------------- World state management ------------------------------------ */

/* 'players' is a pointer to a global hash table.  It is 
 * a container of 'player' who have connected. They are either 
 * in an authorized or unauthorized state. */
struct player *players = NULL;

/* Add a new player to the world. The player will start in
 * an unauthenticated state on 'SCR_LOGIN'. */
struct player *onboard_new_player(struct mg_connection *c) {
    struct player *player = (struct player*) malloc(sizeof(*player));
    player->c = c;
    player->scrid = SCRID_NO_SCREEN;
    player->scrat = 0;
    player->auth.logintim = 0;
    player->auth.attempts = 0;
    player->auth.role = 0;
    memset(player->auth.uname, 0, sizeof(player->auth.uname)); 
    player->auth.id = UID_NF;           
    player->auth.locked_until = 0;
    HASH_ADD_PTR(players, c, player);
    return player;
}

/* Wrapper around mg_ws_send */
void mb_send (struct player *player) {
    u8 buffer[4096];

    struct live_screen *scr = &player->scr;
    struct net_payload_screen payload = serialize_screen(scr->state,
                                                         scr->layout,
                                                         scr->nFields,
                                                         scr->op_A,
                                                         scr->op_B,
                                                         scr->ic,
                                                         buffer
                                                         );

    mg_ws_send(player->c, payload.buf, payload.len, WEBSOCKET_OP_BINARY);
}

/* ----------------------------- Render functions ------------------------------------------------------------------- */

void set_field_text(struct screen *scr, int field, char *value) {
    strcpy(scr->state[field].text, value);
}
 
/* TODO: Make this more general, it should take a column color and text */
void render_login_warning(struct player *player, char *txt) {

    struct field_state buf[1];

    player->scr.op_A = OP_A_UPDATE;
    player->scr.op_B = OP_B_DEF;
    player->scr.ic = LOGIN_IUSER;
    player->scr.nFields = MAX_SLOTS(buf);
        
    struct field_state f = login_screen_state[LOGIN_WARNING];
    f.fg_color = BROWN;
    //f.bg_color = GREEN;
    f.flags &= ~HIDDEN;
    strcpy(f.text, txt);
    f.text_len = strlen(f.text);
    
    buf[0] = f;
    memcpy(player->scr.state,buf, sizeof(*buf)) ;
    mb_send(player);
}

/* ============================================================================
              DATABASE TABLE AND CRUD MACROS GENERATION SYSTEM (TCGS)
=============================================================================== */

#define SQLITE_JOURNAL_MODE wal
#define SQLITE_SYNC normal
#define SQLITE_FOREIGN_KEYS on
#define SQLITE_SET_PRAGMA(mode,val) sqlite3_exec(db,"PRAGMA " #mode "=" STR(val), NULL, NULL, NULL)

/* --------------------------------- SCHEMA --------------------------------------- */

#define x20 " "
#define COMMA ","

/* column constraints */
#define PK x20 "PRIMARY KEY"
#define NN x20 "NOT NULL"
#define UQ x20 "UNIQUE"
#define DF(dval) x20 "DEFAULT" x20 #dval


#define GENERIC32_T 32
#define NAME_T 25
#define EMAIL_T 128
#define PHONE_T 17 //phone
#define BLOB_T 512
#define PW_HASH_T crypto_pwhash_STRBYTES

typedef char generic32_T[GENERIC32_T];
typedef char name_t  [NAME_T]; 
typedef char email_t [EMAIL_T];
typedef char phone_t  [PHONE_T];
typedef char pw_t[PW_HASH_T];

#define BIND_TEXT(stmt, col_num, field)sqlite3_bind_text(stmt, col_num, field, -1, SQLITE_STATIC)
#define BIND_INT(stmt, col_num, field) sqlite3_bind_int(stmt,  col_num, field)

#define BIND_name_t(stmt, col_num, field) BIND_TEXT(stmt,col_num, field)
#define BIND_email_t(stmt, col_num, field) BIND_TEXT(stmt,col_num, field)
#define BIND_phone_t(stmt, col_num, field) BIND_TEXT(stmt,col_num, field)
#define BIND_pw_t(stmt, col_num, field) BIND_TEXT(stmt,col_num, field)
#define BIND_generic32_t(stmt, col_num, field) BIND_TEXT(stmt,col_num, field)

#define BIND_u8(stmt, col_num, field) BIND_INT(stmt, col_num, field)
#define BIND_u16(stmt, col_num, field) BIND_INT(stmt, col_num, field)
#define BIND_u32(stmt, col_num, field) BIND_INT(stmt, col_num, field)
#define BIND_u64(stmt, col_num, field) BIND_INT(stmt, col_num, field)


#define ACCESS_INT(stmt, col_num,field) table->rec[i].field = sqlite3_column_int(stmt,col_num);
#define ACCESS_TEXT(stmt,col_num,field) strcpy(table->rec[i].field, (const char *)sqlite3_column_text(stmt,col_num));

#define ACCESS_name_t(stmt, col_num, field) ACCESS_TEXT(stmt,col_num,field)
#define ACCESS_email_t(stmt, col_num, field) ACCESS_TEXT(stmt,col_num, field)
#define ACCESS_phone_t(stmt, col_num, field) ACCESS_TEXT(stmt,col_num, field)
#define ACCESS_pw_t(stmt, col_num, field) ACCESS_TEXT(stmt,col_num, field)
#define ACCESS_generic32_t(stmt, col_num, field) ACCESS_TEXT(stmt,col_num, field)

#define ACCESS_u8(stmt, col_num, field) ACCESS_INT(stmt, col_num, field)
#define ACCESS_u16(stmt, col_num, field) ACCESS_INT(stmt, col_num, field)
#define ACCESS_u32(stmt, col_num, field) ACCESS_INT(stmt, col_num, field)
#define ACCESS_u64(stmt, col_num, field) ACCESS_INT(stmt, col_num, field)


/* TABLE INDEX */
#define DB_TABLES                              \
    X(usr, USR_SCHEMA)                         \
    X(pw,  PW_SCHEMA)                          \
    X(dev, DEV_SCHEMA)

/* TABLES */

/* mk1 */
/* pw.id == usr.id */
#define PW_SCHEMA                                   \
    TCV(PW_C,  id,  u16,  INTEGER, NN UQ)           \
    TCVL(PW_C, hash, pw_t, TEXT,    NN   )	

#define USR_SCHEMA                              \
    TCV(USR_C,  id,   u16, INTEGER, NN UQ)		\
    TCV(USR_C,  uname, name_t, TEXT, NN UQ)     \
    TCV(USR_C,  role,  u8, INTEGER, NN)         \
    TCV(USR_C,  email, email_t, TEXT)           \
    TCV(USR_C,  phone, phone_t, TEXT)           \
    TCV(USR_C,  first, name_t, TEXT)            \
    TCVL(USR_C, last, name_t, TEXT)			

/* ID = usr_id */
#define INVITATION_SCHEMA                                               \
    TCV(INVITE_C, id, u16, INTEGER, NN)                                 \
    TCV(INVITE_C, token, generic32_t, TEXT, NN )                        \
    TCV(INVITE_C, expiry, u64, INTEGER, NN)                             \
    TCVL(INVITE_C, type, u8, INTEGER, DF(0)) /* 0 email invitation, 1 for sms invitation */

#define DEV_SCHEMA                                                      \
    TCV(DEV_C,  id, u16, INTEGER, PK)                                   \
    TCV(DEV_C,  active, u8, INTEGER, DF(0) NN)                          \
    TCVL(DEV_C, type, u8, INTEGER, DF(0))  //0 for mobile phone 1 for farpointe clicker
/* NOTE:(ari) Add phone as text in e.164 format  */


/* Schema derived db tables. */
#define TCV(tbl,  name, ctype, type, ...) #name x20 #type  __VA_ARGS__ COMMA
#define TCVL(tbl, name, ctype, type, ...) #name x20 #type  __VA_ARGS__ 
#define SQL_CREAT(name, tbl) "CREATE TABLE IF NOT EXISTS"  x20 #name "(" tbl ");"
#define SQL_DROP(name) "DROP TABLE IF EXISTS" x20 #name ";"

struct db_table {
    const char *creat;
    const char *drop;
};

#define X(tbl, SCHEMA) {SQL_CREAT(tbl,SCHEMA),SQL_DROP(tbl)},

struct db_table db_schema[32] = {
    DB_TABLES
};

#undef X
#undef TCV
#undef TCVL

/* Schema derived enums. */
#define TCV(tbl, name, ctype, type, ...)  tbl##_##name,
#define TCVL(tbl, name, ctype,type,  ...) tbl##_##name,
#define TCOLS(name, tbl) enum name##enum { tbl name##_CNT };
#define X(name, tbl) enum name##_cols { tbl name##_cols_CNT };

DB_TABLES

#undef TCV
#undef TCVL
#undef X

/* Schema derived structs. */
#define TCV(tbl, name, ctype, type, ...) ctype name;
#define TCVL(tbl, name, ctype,type,  ...) ctype name;
#define STRUCT_REC(name, tbl) struct name##_rec {tbl};
#define X(name, tbl) struct name##_rec {tbl};

DB_TABLES

#undef TCV
#undef TCVL
#undef X
/* Schema derived strings. */
#define TCV(tbl, name, ctype, type, ...) #name,
#define TCVL(tbl, name, ctype,type,  ...) #name
#define X(name, SCHEMA) const char *name##_table_strings[] = {SCHEMA};

DB_TABLES

#undef X
#undef TCV
#undef TCVL

#define TCV(tbl, name, ctype, type, ...) "?,"
#define TCVL(tbl, name, ctype,type,  ...) "?"
#define X(name, SCHEMA) \
    const char name##_qmarks[] =  SCHEMA;
DB_TABLES
#undef X
#undef TCV
#undef TCVL

#define TCV(tbl, name, ctype, type, ...) #name ","
#define TCVL(tbl, name, ctype,type,  ...) #name
#define X(name, SCHEMA) \
    const char name##_insert_fields[] = SCHEMA;
DB_TABLES
#undef X
#undef TCV
#undef TCVL



#define TCV(tbl, name, ctype, type, ...) #name"=?, "
#define TCVL(tbl, name, ctype,type,  ...) #name"=?"
#define X(name, SCHEMA) \
    const char name##_update_fields[] = SCHEMA;
DB_TABLES
#undef X
#undef TCV
#undef TCVL


#define X(name, SCHEMA) \
    sqlite3_stmt *create_##name##_stmt = NULL;

DB_TABLES

#undef X

/* This prototye was needed here */
sqlite3_stmt *create_statement(sqlite3 *db, char *q);

#define TCV(tbl, name, ctype, type, ...) BIND_##ctype(stmt,tbl##_##name + 1,table->name);
#define TCVL(tbl,name, ctype,type,  ...) BIND_##ctype(stmt,tbl##_##name + 1,table->name);
#define X(name, SCHEMA)                                         \
    int create_##name(sqlite3 *db, struct name##_rec *table) {   \
                                                                \
    if(!create_##name##_stmt) {                                 \
                                                                \
    char buf[512];                                              \
                                                                \
    snprintf(buf,512,"INSERT INTO " #name "(%s) VALUES"         \
    "(%s)", name##_insert_fields , name##_qmarks );             \
                                                                \
    create_##name##_stmt = create_statement(db,buf);            \
    }                                                           \
    sqlite3_stmt *stmt = create_##name##_stmt;                  \
                                                                \
    SCHEMA                                                      \
    int rc = sqlite3_step(stmt);                                \
    if(rc != SQLITE_DONE) {                                     \
    fprintf(stderr, "create_ %s\n", sqlite3_errmsg(db));        \
    }                                                           \
    sqlite3_reset(stmt);                                        \
    sqlite3_clear_bindings(stmt);                               \
    return rc == SQLITE_DONE;                                   \
    }

DB_TABLES

#undef X
#undef TCV
#undef TCVL


#define X(name, SCHEMA)                                                \
    struct all_##name {                                                \
    int len;                                                           \
    struct name##_rec rec[4096];                                       \
    };

DB_TABLES
#undef X


#define X(name, SCHEMA) \
    sqlite3_stmt *getall_##name##_stmt = NULL;
DB_TABLES
#undef X



#define TCV(tbl, name, ctype, type, ...) ACCESS_##ctype(stmt,tbl##_##name, name )
#define TCVL(tbl,name, ctype,type,  ...) ACCESS_##ctype(stmt,tbl##_##name, name)
#define X(name, SCHEMA)                                             \
    int read_##name##_all(sqlite3 *db, struct all_##name *table) {  \
                                                                    \
    if(!getall_##name##_stmt) {                                     \
                                                                    \
    char buf[512];                                                  \
                                                                    \
    snprintf(buf,512,"SELECT * FROM " #name);                       \
                                                                    \
    getall_##name##_stmt = create_statement(db,buf);                \
    }                                                               \
    sqlite3_stmt *stmt = getall_##name##_stmt;                      \
    int rc;                                                         \
    int i = 0;                                                      \
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {                \
    SCHEMA                                                          \
    i++;                                                            \
    }                                                               \
    table->len = i;                                                 \
    sqlite3_reset(stmt);                                            \
    sqlite3_clear_bindings(stmt);                                   \
    return rc == SQLITE_DONE;                                       \
    }

DB_TABLES

#undef X
#undef TCV
#undef TCVL


#define X(name, SCHEMA) \
    sqlite3_stmt *update_##name##_stmt = NULL;
DB_TABLES
#undef X


#define TCV(tbl, name, ctype, type, ...) BIND_##ctype(stmt,tbl##_##name + 1,rec->name);
#define TCVL(tbl,name, ctype,type,  ...) BIND_##ctype(stmt,tbl##_##name + 1,rec->name);
#define X(name, SCHEMA)                                         \
    int update_##name(sqlite3 *db, struct name##_rec *rec) {    \
                                                                \
    if(!update_##name##_stmt) {                                 \
                                                                \
    char buf[512];                                              \
                                                                \
    snprintf(buf,512,"UPDATE " #name " SET %s "                 \
    "WHERE id = ?", name##_update_fields);                      \
                                                                \
    update_##name##_stmt = create_statement(db,buf);            \
    }                                                           \
    sqlite3_stmt *stmt = update_##name##_stmt;                  \
                                                                \
    SCHEMA                                                      \
    sqlite3_bind_int(stmt, name##_cols_CNT +1, rec->id);        \
    int rc = sqlite3_step(stmt);                                \
    if(rc != SQLITE_DONE) {                                     \
    fprintf(stderr, "update_ %s\n", sqlite3_errmsg(db));        \
    }                                                           \
    sqlite3_reset(stmt);                                        \
    sqlite3_clear_bindings(stmt);                               \
    return rc == SQLITE_DONE;                                   \
    }
 
DB_TABLES

#undef X
#undef TCV
#undef TCVL

#define X(name, SCHEMA) \
    sqlite3_stmt *delete_##name##_stmt = NULL;
DB_TABLES
#undef X

#define X(name, SCHEMA)                                         \
    int delete_##name(sqlite3 *db, struct name##_rec *rec) {    \
                                                                \
    if(!delete_##name##_stmt) {                                 \
                                                                \
    char buf[512];                                              \
                                                                \
    snprintf(buf,512,"DELETE FROM " #name                       \
    "WHERE id = ?");                                            \
                                                                \
    delete_##name##_stmt = create_statement(db,buf);            \
    }                                                           \
    sqlite3_stmt *stmt = delete_##name##_stmt;                  \
    sqlite3_bind_int(stmt, 1 , rec->id);                        \
    int rc = sqlite3_step(stmt);                                \
    if(rc != SQLITE_DONE) {                                     \
    fprintf(stderr, "delete_ %s\n", sqlite3_errmsg(db));        \
    }                                                           \
    sqlite3_reset(stmt);                                        \
    sqlite3_clear_bindings(stmt);                               \
    return rc == SQLITE_DONE;                                   \
    }
 
DB_TABLES

#undef X
#undef TCV
#undef TCVL


#undef x20
#undef COMMA
#undef PK
#undef NN
#undef UQ
#undef DF
#undef TBC_FK
#undef TBCL_FK


/* ---------------------------------  CRUD QUERIES    ------------------------------------------------------------ */

#define SQLITE_READ_TO_NUL -1

sqlite3_stmt *create_statement(sqlite3 *db, char *q) { 
    sqlite3_stmt *stmt;
    int rc =  sqlite3_prepare_v2(db, q, SQLITE_READ_TO_NUL, &stmt,NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "CREATE_STATMENT ERROR: %s %s\n", sqlite3_errmsg(db), q);
        exit(1);
    }
    return stmt;
}

/* ID RANGES */
sqlite3_stmt *next_uid_admin_stmt; /* Admin range is 10 < uid <= 100 */
sqlite3_stmt *next_uid_staff_stmt; /* Staff range is 100 < uid  <= 200  */
sqlite3_stmt *next_uid_customer_stmt;  /* customre range is 1000 < uid < U16 max */
sqlite3_stmt *read_pw_stmt = NULL;   //by id

enum UID_T {
    ADMIN_UID_T, STAFF_UID_T, CUSTOMER_UID_T
};

/* Get the MAX + 1 uid for ranges by UID_T enum 
 *        admin    9 < uid  20
 *        staff   99 < uid < 120
 *    customers  999 < uid < 30000    */
int next_uid(sqlite3 *db,int typ) {
    if(!next_uid_admin_stmt)
        next_uid_admin_stmt = create_statement(db,"select MAX(id) "
                                               " from usr "
                                               "where id > 9 and id < 20"
                                               );

    if(!next_uid_staff_stmt)
        next_uid_staff_stmt = create_statement(db,"select MAX(id) "
                                               " from usr "
                                               "where id > 99 and id < 110"
                                               );
    if(!next_uid_customer_stmt)
        next_uid_customer_stmt = create_statement(db,"select MAX(id) "
                                                  " from usr "
                                                  "where id > 999 and id < 30000"
                                                  );
    sqlite3_stmt *stmt = NULL;
    int id = UID_NF;
    switch (typ) {
    case  ADMIN_UID_T:
        stmt = next_uid_admin_stmt;
        id = 10;
            break;
    case STAFF_UID_T:
        stmt = next_uid_staff_stmt;
        id = 100;
            break;
    case CUSTOMER_UID_T:
        stmt = next_uid_customer_stmt;
        id = 1000;
        break;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            id = (sqlite3_column_int(stmt, 0) + 1);
        }
    } else {
        fprintf(stderr,"sqlite3_step failed: %s", sqlite3_errmsg(db));
    }
    
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    return id;
}

/* Search for uid and return the related hashed 'pw'. */
int read_pw (sqlite3 *db, struct pw_rec *pw) {
    int found = 0;
    if(!read_pw_stmt) {
        read_pw_stmt = create_statement(db, "select hash from pw where id = ?");
    }
    
    sqlite3_stmt *stmt = read_pw_stmt;
    sqlite3_bind_int(stmt,  1, pw->id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *hash = (const char *)sqlite3_column_text(stmt, 0);
        strcpy(pw->hash, hash);
        found = 1;
    }
    
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    return found;
}


/* -------------------------------------INITIALIZATION----------------------------------------------------------   */


void create_tables(sqlite3 *db) {
    for(size_t i = 0; i < MAX_SLOTS(db_schema); i++ ) {
        if(db_schema[i].creat){
            if(DB_RESET) {
                sqlite3_exec(db, db_schema[i].drop  ,NULL, NULL, NULL);
                sqlite3_exec(db, db_schema[i].creat ,NULL, NULL, NULL);
            } else {
                sqlite3_exec(db, db_schema[i].creat ,NULL, NULL, NULL);
            }
        }
    }
}

sqlite3 *init_db() {
    sqlite3 *db;
    int rc = sqlite3_open("mb_data.db",&db);
    (void) rc;
    SQLITE_SET_PRAGMA(journal_mode, SQLITE_JOURNAL_MODE);
    SQLITE_SET_PRAGMA(synchronous, SQLITE_SYNC);
    SQLITE_SET_PRAGMA(foreign_keys,SQLITE_FOREIGN_KEYS);
    return db;
}

void pw_encrypt_and_add_to_db(sqlite3 *db, int uid, char *clr_pw) {
    struct pw_rec pw;
    pw.id = uid;
    if (crypto_pwhash_str ((char*)pw.hash,
                           clr_pw, strlen(clr_pw),
                           crypto_pwhash_OPSLIMIT_INTERACTIVE,
                           crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        perror("FAILED TO HASH PASSWORD");
        exit(1); //T0DO: How should I handle this case???
    }
    create_pw(db, &pw);
}


/* Retrieve pw_hash from the database searching by uid.
 * Verify hashed clr_pw matches what is in the db. Return 1
 * on success and 0 on failure */
int pw_check_from_db(sqlite3 *db, int uid, char *clr_pw) {
    struct pw_rec pw;
    pw.id = uid;
    int res = read_pw(db, &pw);
    (void) res;
    int verified = crypto_pwhash_str_verify(pw.hash, clr_pw, strlen(clr_pw));
    return (verified >= 0);
}

void view_users_test(sqlite3 *db) {
    struct all_usr all;
    all.len = 0;
    read_usr_all(db, &all);
    for (int i = 0; i < all.len; i++) {
	printf("%d %s %s %s %s\n",
	       all.rec[i].id,
	       all.rec[i].email,
	       all.rec[i].phone,
	       all.rec[i].first,
	       all.rec[i].last);
    }
}

/* ============================     END DB          ============================================= */

/* ============================== SANITIZE,NORMALIZE, VALIDATE ================================= */

int snv_email(char *email, char *cln) {

    int j;
    int e = 0;
    
    struct tmp_str {
        unsigned char str[128];
        int len;
    };

    struct tmp_str local = {0};
    struct tmp_str domain = {0};

    for (j = 0; email[j] != '@' && j < EMAIL_T - 1; j++) {
        char c = email[j];
        if(isalnum((unsigned char) c) || (c == '.') || (c == '_') ||
           (c == '-') ||
           (c == '+')) {
            local.str[local.len++] = c;
            cln[e++] = tolower((unsigned char)c);
        }   
    }
    int dot_cnt = 0;
    if (email[j] == '@') {
        cln[e++] = '@';
        j++;
        for(; j < EMAIL_T - 1; j++) {
            char c = email[j];
            if (isalnum((unsigned char)c) || (c == '.') || (c == '-')) {
                domain.str[domain.len++] = c;
                cln[e++] = tolower((unsigned char)c);
                if (c == '.') {
                    dot_cnt++;
                }
            }
        }
    } else {
        return 0; //poorly formed email
    }
    
    if ((domain.len == 0) ||
        (local.len == 0) ||
        (local.str[0] == '.') ||
        (local.str[local.len - 1] == '.') ||
        (domain.str[0] == '.') ||
        (domain.str[domain.len - 1] == '.') ||
        (dot_cnt < 1)) {
        return 0; //poorly formed email
        
    }

    /* Check for consecutive '.' in the email domain portion */
    for (int i = 0; i < domain.len - 1; i++) {
        if(domain.str[i] == '.' && domain.str[i + 1] == '.'){
            return 0; //poorly formed email
        }
    }

    /* Check for consecutive '.' in the email local portion */
    for (int i = 0; i < local.len - 1; i++) {
        if(local.str[i] == '.' && local.str[i + 1] == '.') {
            return 0; //poorly formed email
        }
    }
    return 1;
}

/* NOTE:(ari) Deal with country code in better way.  */
int snv_phone(char *phone, char *cln) {
    int p = 1;
    cln[0] = '+';

    for (int i = 0; i < PHONE_T - 1; i++) {
        if(isdigit((unsigned char)phone[i])) {
            cln[p++]  = phone[i];
        } 
    }
    if ((strlen(cln) < 12) ||
        (strlen(cln) > 16)) {
        return 0;     
    }
    return 1;
}

int snv_name(char *name, char *cln) {
    int f = 0;
    for (int i = 0; i < NAME_T - 1; i++) {
        if (isalpha((unsigned char) name[i])) {
            cln[f++] = tolower((unsigned char)name[i]);
        }
    }
    if(!strlen(cln)) return 0;
    return 1;
}

/* client req header item */
struct __attribute__((packed)) cfh {
    u8 opcode;
    u8 AID;
    u8 nFields;
};

/* client req field block field item */
struct __attribute__((packed)) cfb {
    u8 id; //field_id
    u8 len;
    char val[24]; 
};

struct login_attempt {
    struct cfh head;
    struct cfb username;
    struct cfb password;
};

void print_login_attempt(struct login_attempt *login) {
    printf("**********************************************************\n"); 
    printf("feild-id: %d\t",login->username.id);
    printf("field_len: %d\t", login->username.len);
    printf("text: %.*s\n",24,login->username.val);
    printf("feild-id: %d\t",login->password.id);
    printf("field_len: %d\t", login->password.len);
    printf("text: %.*s\n",24,login->password.val);
    printf("**********************************************************\n");
}

struct usr_rec new_usr_rec() {
    struct usr_rec usr = {
        .id = UID_NF,
        .role = ROLE_NONE
    };
    return usr;
}

void set_live_screen(struct player *player, enum SCRID scrid) {

    struct screen tmpl = screens[scrid];
    struct live_screen *scr = &player->scr;
    player->scrid = scrid;
    
    scr->op_A = tmpl.op_A;
    scr->op_B = tmpl.op_B;
    scr->ic   = tmpl.ic;
    scr->nFields = tmpl.nFields;

    memcpy(scr->layout, tmpl.layout, tmpl.nFields * sizeof(struct field_layout));
    memcpy(scr->state, tmpl.state, tmpl.nFields * sizeof(struct field_state));
}

void today(char *buf, int len) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    strftime(buf, len, "%a %b %d, %Y", tm);
}

void time_now(char *buf, int len) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    strftime(buf, len, "%H:%M:%S", tm);
}
    
void set_screen_text(struct player *player, int col, char *txt) {
    player->scr.state[col].text_len = strlen(txt);
    strcpy(player->scr.state[col].text, txt);
}

#define TITLE_BAR(SCR)					\
    char user[32];					\
    snprintf(user, 32, "user: %s", player->auth.uname);	\
    set_screen_text(player,SCR##_FLD_USER, user);	\
    char date[32];					\
    today(date, sizeof(date));				\
    set_screen_text(player,SCR##_FLD_DATE, date);

void goto_main_screen(struct player *player) {
    set_live_screen(player, SCRID_MAIN_SCREEN);
    TITLE_BAR(MAIN_SCREEN)
    mb_send(player);    
}


/* On  update just send the correct opcdoe and thee  */
void mb_update(struct player *player, int nFields, int IC, struct field_state *update) {

    player->scr.op_A = OP_A_UPDATE;
    player->scr.op_B = OP_B_DEF;
    player->scr.ic = IC;
    player->scr.nFields = nFields;
    memset(player->scr.state,0,100*sizeof(struct field_state));
    memcpy(player->scr.state,update,nFields*sizeof(struct field_state));
    mb_send(player);
}


void tick(char *time) {
    struct player *p = players;
    for(p = players; p != NULL; p = p->hh.next) {
        if(p->scrid == SCRID_MAIN_SCREEN) {
            struct field_state f[] = {main_screen_state[MAIN_SCREEN_FLD_TIME]};
            f[0].text_len = strlen(time);
    strcpy(f[0].text, time);
            mb_update(p,1,MAIN_ISELECT,f);
        }
    }
}

void goto_main_screen_alpha(struct player *player) {
    set_live_screen(player, SCRID_MAIN_SCREEN);
    // set user name
    // set date
    // set time with seconds.....
    mb_send(player);    
}

int try_activate(struct player *player,u8 *reqbuf) {
    (void) player;
    struct cfh *header = (struct cfh *)reqbuf;
    if(header->AID == M_ACT_ENTER) {
        printf("have recieved an enter");
    }
    return 1;
}

int try_login(struct player *player, u8 *reqbuf) {

    /* Prevents login. 3 seconds after 3 missed attempts */

    if (time(NULL) < player->auth.locked_until) {
        char buf[50];
        snprintf(buf, 50, "Login lockour for %zu seconds.",
                             player->auth.locked_until - time(NULL));
        render_login_warning(player,buf);
        return 0;
    }
        
    
    struct login_attempt *attempt = (struct login_attempt*) reqbuf;

    struct all_usr users = {0};
    struct usr_rec usr = new_usr_rec();
    struct usr_rec cur = new_usr_rec();

    char username[NAME_T] = {0};
    char password[NAME_T] = {0};

    memcpy(username, attempt->username.val, attempt->username.len);
    memcpy(password, attempt->password.val, attempt->password.len);

    snv_name(username, usr.uname);

    /* search db for user */
    read_usr_all(db, &users);
    for (int i = 0; i < users.len; i++ ) {
        cur = users.rec[i];
        if(strcmp(usr.uname, cur.uname ) == 0) {
            usr = cur;
            break;
        }
    }
    
    if (pw_check_from_db(db, usr.id, password)) {
        player->auth.logintim = time(NULL);
        player->auth.attempts = 0;
        player->auth.role = usr.role;
        player->auth.id = usr.id;
        strcpy(player->auth.uname, usr.uname);
        player->auth.locked_until = 0;
        return 1;
    } else {
        player->auth.attempts++;

        if (player->auth.attempts == 3) {
            player->auth.locked_until = (time(NULL) + 5);
            render_login_warning(player, "Login lockout for 5 seconds.");
            player->auth.attempts = 0;
            return 0;
        }
        
        if (attempt->head.nFields < 2) {
            render_login_warning(player, "All fields required.");
        } else {
            render_login_warning(player, "Invalid Credentials. plase try again");
        }
        
        return 0;
        }
}


/* variable outcomes */
const u8 screen_router[][32] = {
    [SCRID_NO_SCREEN] = {
        [H_NO_SCR_WEB_LOGIN] = SCRID_LOGIN_SCREEN,
        [H_NO_SCR_MOBILE_ACTIVATE] = SCRID_M_ACT_SCREEN
    },
    [SCRID_LOGIN_SCREEN] = {
        [H_LOGIN_SCR_SUCCESS]  = SCRID_MAIN_SCREEN,
        // NOTE: failed login return H_NO_ACTION from handler
    }
    
    /* [SCRID_MAIN_SCREEN] = { */
    /*     [OUTCOME_GOTO_ADD_USER] = SCR_ADD_USER_FORM, */
    /*     [OUTCOME_GOTO_VESSELS]  = SCR_VESSEL_ENTRY, */
    /*     [OUTCOME_GOTO_REPORTS]  = SCR_REPORTS_MENU */
    /* }, */
};


/* Dispatch Busines Logic */
void dispatch_business_logic(struct mg_connection *c, u8 *reqbuf, int reqbuflen) {
    (void) reqbuflen;

    
    struct player *player = NULL;
    u32 nxt_screen, handler_res;

    
    HASH_FIND_PTR(players, &c, player);

    if(!player) {
        player = onboard_new_player(c);
    };

    /* array of handler functions */
    handler_res = screen_handlers[player->scrid](player, reqbuf);
    
    if (handler_res != H_NO_ACTION) {
        nxt_screen = screen_router[player->scrid][handler_res];
        set_live_screen(player, nxt_screen);

        /* Some screens have customizations to the templates,
         * if they do call the customizer function here. */
        if (screen_renderers[player->scrid]) {
            screen_renderers[player->scrid](player);
        }
        
        mb_send(player);
    }
    
}

/* ===========================================================================
       The MONGOOSE HERE
   =========================================================================== */

void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    switch (ev) {
    case MG_EV_HTTP_MSG:
        {  
            struct mg_http_message *hm = (struct mg_http_message *) ev_data;
            if ( mg_match (hm->uri, mg_str("/"), NULL)) {
                struct mg_http_serve_opts opts = {.root_dir = ".", .fs = &mg_fs_posix};
                mg_http_serve_dir(c, hm, &opts);
            } else {
                mg_ws_upgrade(c, hm, NULL);
            }
        } break;
    case MG_EV_WS_MSG:
        {
            struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
            if ((wm->flags & 0x0f) == WEBSOCKET_OP_BINARY) {
                dispatch_business_logic(c, (u8*)wm->data.buf, wm->data.len); 
            }
        }
    }
}

void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}


void stdin_read_password(char *buf, size_t size)
{
    struct termios old, new;
    tcgetattr(STDIN_FILENO, &old);
    new = old;

    new.c_lflag &= ~(ECHO);   /* turn off echo */
    tcsetattr(STDIN_FILENO, TCSANOW, &new);

    fgets(buf, size, stdin);
    buf[strcspn(buf, "\n")] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &old);  /* restore */
    printf("\n");
}

void flush_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int read_stdin_interactive(char *buf,char *prompt ,int len) {

    printf("%s",prompt);
    for (int i = 0; i < len; i++ ) {
        buf[i] = 0;
    }
    fgets(buf, len, stdin);
    if (strchr(buf, '\n') == NULL) {
        printf("Input buffer exceeded length\n");
        flush_stdin();
        return 0;
    }
    buf[strcspn(buf, "\n")] = '\0';
    return 1;
}

void console_add_admin(sqlite3 *db) {
    
    clear_screen();
    char *prompt;

    struct usr_rec usr = {0};
    char email[EMAIL_T] = {0};
    char phone[PHONE_T] = {0};
    char first[NAME_T]  = {0};
    char  last[NAME_T]  = {0};

    usr.id = next_uid(db, ADMIN_UID_T);                

 EMAIL:
    prompt = "Email: ";

    if(!read_stdin_interactive(email, prompt, EMAIL_T)) {
        goto EMAIL;
    }
                
    if (!snv_email(email, usr.email)) {
        printf("Malformed email: try again\n");
        goto EMAIL;
    } 
                
 PHONE:
    prompt = "Phone (Country Code prefix required, 1 for USA): ";

    if(!read_stdin_interactive(phone,prompt, PHONE_T))
        goto PHONE;
                
    if (!snv_phone(phone, usr.phone)) {
        printf("Illegal phone format: try again\n");
        goto PHONE;
    }
                
 FIRST:
    prompt = "First: ";
    if (!read_stdin_interactive(first,prompt, NAME_T)) {
        goto FIRST;
    }
                
    if (!snv_name(first, usr.first)) {
        printf("Error: First name required, try again\n");
        goto FIRST;
    }
                
 LAST:
    prompt = "Last: ";
                
    if(!read_stdin_interactive(last,prompt, NAME_T)) {
        goto LAST;
    }
                
    if (!snv_name(last, usr.last)) {
        printf("Error: last name required, try again\n");
        goto LAST;
    }

 PASSWORD:
    char pass[32], confirm[32];

    printf("Password:");
    stdin_read_password(pass, 32);

    printf("Confirm password:");
    stdin_read_password(confirm, 32);

    if (strcmp(pass,confirm) != 0) {
        printf("try again\n");
        goto PASSWORD;
    }
    
    clear_screen();
    char add[8] = {0};

    printf("%s %s %s %s uid:%d\n Add admin? (y/n): ",usr.first, usr.last,usr.phone,usr.email,usr.id);
    read_stdin_interactive(add,"", sizeof(add));

    if(add[0] == 'y') {
        pw_encrypt_and_add_to_db(db, usr.id, pass);
        strcpy(usr.uname, "alpha"); 
        usr.role = ROLE_ALPHA;


        /* TODO: Need to handle this in the application interactively with user feedback. */        
        int res = create_usr(db, &usr);
        if(!res) {
            fprintf(stderr,"Unable to create usr at line %d in file %s \n", __LINE__, __FILE__);
            exit (1);
        }
    }
}

/* uid 0, is root. It will never exist in the usr table
 * uid 10, is master admin account. It will exist in the
 * usr table and the pw table. Both accounts/pw are created
 * during first boot from the cmd line. */
void root_menu(sqlite3 *db) {
    
    char *prompt;
    clear_screen();
    prompt =
        "Macabee Root Console Menu\n\n"
        "1) Add/Edit/Update Master Admin Account\n"
        "2) Continue to boot\n"
        "\n> ";

    char response[16];
    read_stdin_interactive(response,
                           prompt,
                           sizeof(response));
            
    u8 choice = (u8)strtoul(response,NULL,10);
            
    if(choice == 1) {
        console_add_admin(db);
    }
}

/* Root password required for startup */
void require_root(sqlite3 *db) {
    
    clear_screen();
    struct pw_rec pw = {.id = 0, .hash = ""};
    int found = read_pw(db, &pw);
    
    if (!found) {               /* root password NOT found get new root password.*/

        char pass1[32];
        char pass2[32];
        printf("Welcome to MACABEE,"
               "initial boot.\n"
               "Create root password.\n");

    MATCH_PASSWORD:
        
        printf("\nPassword:");
        stdin_read_password(pass1, 32);

        printf("Confirm password:");
        stdin_read_password(pass2, 32);
        
        if (strcmp(pass1,pass2) != 0) {
            printf("try again\n");
            goto MATCH_PASSWORD;
        }
        
        pw_encrypt_and_add_to_db(db, 0, pass1);
        
    } else {                    /* root password found */
        name_t pw_entered;
        int tries = 0;
        printf("(MACABEE BOOT: credentials required)\n\n");

    BOOT:
        printf("Password: ");
        stdin_read_password(pw_entered, NAME_T);
        
        if (!pw_check_from_db(db, 0, pw_entered)) {
            tries++;
            printf("Failed attempt %d\n", tries);
            if(tries == 3) {
                fprintf(stderr, "%s\n", "[Process Exit] failure limit exceeded.");
                exit(1);
            }
            goto BOOT;
        }
    }
    /* Root menu is server maintanance menu */
    root_menu(db);
    clear_screen();
    printf("Macabee engaged, server started: \n\n");
    return;
}

int main(void) {

    db = init_db();
    create_tables(db);
    
#if DEV_MODE == 0
    printf("DEV_MODE NOT");
    require_root(db);
#endif
            
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8001", ev_handler, NULL);

    //    time_t last_tick = time(NULL);
    
    for(;;) {
    	mg_mgr_poll(&mgr,1000);

        /* time_t now = time(NULL); */
        
        /* if (now != last_tick) { */
        /*     last_tick = now; */
        /*     //tick(now); */
        /*     char time[32]; */
        /*     time_now(time, 32); */
        /*     tick(time); */
        /* } */
    }
    return 0;
    sqlite3_close(db);
}

//visual spec
//state patch

/* struct state relations, member attributes */
/* meaning */
/* lifetime */
/* ownership */
/* mutability !!!!!!!!!!!!!!!*/
/* update frequency */
/* responsibility */
/* runtime behavior */
/* representation needs */

/* shared identity */
/* shared lifetime */
/* shared invariants */
/* shared ownership */
/* shared runtime behavior */
/* shared subsystem responsibility */
/* shared access/update patterns */

/*-----------------------------------------------------------  Server => Client Message Package ---------------------------------
    | opcode A 8bits | opcode B 8bits | Number of headers 1byte| headers (number of headers * sz of header)| payload data  item per header variable len |

  ------------------------------------------------------------------------------------------------------------------------------ */
/*

  struct __attribute__((packed))
  Opcode 1
  type of data, labales and inputs, images, multimedia
  <input> labels/inputs/actions
  <video> audio/video media
  <img> image container
  <canvas> container

 */
/* ------------------------------------big fucking business is the barrier between netowrking and business, dispatch */
// need to start tracking client state by token or fd now and here.
// buf is the input buf. its an an aid key
     /* 
      *      ┌───────────────────────────────────────────────────────────────────────────┐
      *      │                       buf                                                 │
      *      ├──────────────┬────────────────────────────────────────────────────────────┤
      *      │ Header       │ [ fieldCount (1 byte) ]                                    │
      *      ├──────────────┼────────────────────────────────────────────────────────────┤
      *      │ Field 1 Block│ [ slotIndex (1 byte) ] [ stringLength (1 byte) ] [ ASCII ] │
      *      ├──────────────┼────────────────────────────────────────────────────────────┤
      *      │ Field 2 Block│ [ slotIndex (1 byte) ] [ stringLength (1 byte) ] [ ASCII ] │
      *      └──────────────┴────────────────────────────────────────────────────────────┘ */

/* 

update screen 

|opcodeA|opcodeB|numfield|stateBytes|

 */
/* #define MAIN_MENU_FIELDS \ */
/*     LABEL(0, 10, 5, 30, "MAIN MENU") \ */
/*     LABEL(1, 10, 8, 30, "1. Accounts") \ */
/*     // ... etc. */

/* #define X(id, t, x, y, w, txt, len, flg, col) { .field_id = id, .type = t, .x = x, .y = y, .width = w }, */
/* struct field_layout main_menu_layout[] = { MAIN_MENU_FIELDS }; */
/* #undef X */

/* #define X(id, t, x, y, w, txt, len, flg, col) { .field_id = id, .text = txt, .text_len = len, .fg_color = col, .flags = flg }, */
/* struct field_state main_menu_state[] = { MAIN_MENU_FIELDS }; */
/* #undef X */

/* Account locked. Try again in 3 seconds. */

/* sqlite common settings
   ---------------------
   PRAGMA journal_mode;
   PRAGMA synchronous;
   PRAGMA foreign_keys;
   PRAGMA cache_size;
   PRAGMA page_size;
   PRAGMA temp_store;
   PRAGMA mmap_size;
*/

/* MESSAGE FORMATS */

/* ------------------ CLIENT to SERVER MESSAGE FORMAT   ---------------------------
         Header:           |   opcode u8  |  aidkey u8   | nFields u8    |
         Field blocks:     | field_id  u8 |  fldlen u8   | field_val 24 |
---------------------------------------------------------------------------------- */

/* array of handler functions to handle interactions with screens indexed by scrid */
/* array of render functions to render screens indexed by scrid */
/* 2d navigation array 2 lookup and assign next screen based on return value of handler func */
