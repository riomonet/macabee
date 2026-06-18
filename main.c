#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sodium.h>
#include <assert.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "mac_types.h"
#include "mongoose.h"
#include "sqlite3.h"
#include "cJSON.h"
#include "uthash.h"


#define UID_NF UINT16_MAX // UID NOT FOUND. [SENTINEL]

sqlite3 *db;

/* Role values must be explictly defined or db values will no align. */
enum ROLES {
    ROLE_NONE = 0,
    ROLE_ALPHA = 1,
    ROLE_OFFICE = 2,
    ROLE_YARD = 3,
    ROLE_ACCOUNTING = 4
};

/* ============================================================================
       BIG FUCKING BUSINESS HERE
=============================================================================== */

/* ---------------------------- screen definitions ------------------------------------------------------------------- */

#define LABEL(id, xx, yy, ww, txt)				\
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, 0, 0)

#define INPUT(id, xx, yy, ww)			\
    X(id, VIS_INPUT, xx, yy, ww, "", 0, 0, 0)

#define INPUT_F(id, xx, yy, ww,flg)		\
    X(id, VIS_INPUT, xx, yy, ww, "", 0, flg, 0)

#define HL(id, xx, yy, ww)                      \
    X(id, VIS_LINE, xx, yy, ww, "", 0, 0, 0)

#define LABEL_F(id, xx, yy, ww, txt, flg)                       \
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, flg, 0)

#define LABEL_C(id, xx, yy, ww, txt, col)			\
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, 0, col)

#define LABEL_CF(id, xx, yy, ww, txt, flg, col)			\
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, flg, col)

#define STATUS(id, xx, yy, ww, txt, flg)	\
    LABEL_F(id, xx, yy, ww, txt, flg)

#define STATE(id, txt, flg, col)		\
    X(id, txt, sizeof(txt)-1, flg, col)

#define STATE_LEN(id, txt, len, flg, col)	\
    X(id, txt, len, flg, col)

#define MAX_SLOTS(arr) (sizeof(arr)/sizeof(arr[0]))
#define STR2(X) #X
#define STR(X) STR2(X)

#define MAKE_SCREEN_DEF(opA, opB, layout_arr, state_arr, cursor_pos, cnt)   \
    {       .op_A = (opA),                                  \
            .op_B = (opB),                                  \
            .layout = (layout_arr),                         \
            .state = (state_arr),                           \
	    .nFields = cnt,				    \
            .ic = (cursor_pos)                              \
    }


/* -------------------- SCREEN DEFINTIONS START--------------------- */

#define SCREENS_LIST						\
    YMB (LOGIN_SCREEN, login_screen, LOGIN_IUSER)		\
    YMB (MAIN_SCREEN, main_screen, MAIN_ISELECT)		\


/* SCREEN STUBS */
#define SCREEN_STUB_HEADER(SCR,title)				\
    LABEL(SCR##_USER,7,1,8, "DSP_USER")				\
    LABEL(SCR##_DATE,67,1,8, "DSP_DATE")			\
    LABEL(SCR##_TIME,67,2,8, "DSP_TIME")			\
    LABEL(SCR##_TITLE,29,1,21, STR(title))			\
    LABEL(SCR##_MENU_NAME,35,2,9, STR(SCR) " MENU")		\
    LABEL_CF(MAIN_L6,6,6,28, "Select one of the following:", FAINT, CYAN)  

#define SCREEN_STUB_FOOTER(SCR)				\
    LABEL(SCR##FOOTER_SELECTION,1,23,9,"Selection")	\
    LABEL(SCR##FOOTER_ARROW,1,24,4, "-->")		\
    HL(SCR##_FOOTER_HL1,1,26,100)			\
    LABEL(MAIN_L14,6,28,9,"F6=Logout")			\
    LABEL(MAIN_L15,19,28,9, "F7=Search")		\
    LABEL(MAIN_L16,31,28,16,"F8=Redraw screen")		\
    HL(SCR##_FOOTER_HL2,0,29,100)			\
    HL(SCR##_FOOTER_HL3,7,24,90)			\
    INPUT(SCR##_ISELECT,6,24,1)                              

/* LOGIN SCREEN col, row */
#define LOGIN_SCREEN                                                    \
    LABEL(LOGIN_L1       , 9,   8, 27,     "USER . . . . . . . . . . . ") \
    LABEL(LOGIN_L2       , 9,  10, 27, "PASSWORD . . . . . . . . . ")     \
    INPUT(LOGIN_IUSER    , 38,  8, 24)                                    \
    INPUT_F(LOGIN_IPW    , 38, 10, 24,  PASSWORD)                         \
    LABEL_CF(LOGIN_L3    , 5,   5, 37, "Tab to change fields, Enter to submit", FAINT,CYAN) \
    LABEL_C(LOGIN_L4     , 40,  1, 19, "Marina 59 | Sign On", WHITE)    \
    STATUS(LOGIN_WARNING , 38, 12, 42, "", HIDDEN)                      

/*   id, col, row, width */
/* #define MAIN_SCREEN                                             \ */



#define MAIN_SCREEN \
    SCREEN_STUB_HEADER(MAIN, Macabee Main Menu )	    \
        LABEL(MAIN_L7,10,8,15,   "1. Contacts")             \
        LABEL(MAIN_L8,10,9,17,   "2. Contracts")           \
    LABEL(MAIN_L9,10,10,17,  "3. Access Control")               \
    LABEL(MAIN_L10,10,11,15,  "4. Live montior")                \
    SCREEN_STUB_FOOTER(MAIN)

#define MAIN_SCREEN_ALPHA			\
    SCREEN_STUB_HEADER(MAIN, Macabee Main Menu )	    \
        LABEL(MAIN_L7,10,8,15,   "1. Contacts")             \
        LABEL(MAIN_L8,10,9,17,   "2. Contracts")           \
    LABEL(MAIN_L9,10,10,17,  "3. Access Control")               \
    LABEL(MAIN_L10,10,11,15,  "4. Live montior")                \
    SCREEN_STUB_FOOTER(MAIN)


/* -------------------------------------------- END SCREEN DEFINTIONS --------------------------------------------- */

#define X(id, t, x, y, w, txt, len, flg, col) id,
#define YMB(SCR,scr, IC) enum SCR##_IDX {SCR SCR##_FIELD_COUNT};
SCREENS_LIST
#undef X
#undef YMB


#define YMB(SCR,scr,IC) struct field_layout scr##_layout[] = {SCR};   
#define X(id, t, xx, yy, w, txt, len, flg, col)                         \
    { .field_id = (id), .type = (t), .x = (xx), .y = (yy), .width = (w) },
SCREENS_LIST
#undef X
#undef YMB

#define YMB(SCR,scr,IC) struct field_state scr##_state[] = {SCR};
#define X(id, t, x, y, w, txt, len, flg, col)               \
    { .field_id = (id), .text = (txt), .text_len = (len),   \
            .fg_color = (col), .flags = (flg) },
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

/* def_screen */
struct screen {
    u8 op_A;
    u8 op_B;
    u8 ic;
    size_t nFields;
    struct field_layout *layout;
    struct field_state *state;

};

struct live_screen {
    u8 op_A;
    u8 op_B;
    u8 ic;
    size_t nFields;
    struct field_layout layout[100];
    struct field_state state[100];
};


/* |--------------------------------------- GLOBAL SCREEN TEMPLATES -------------------------------- */

/* Global Screen templates:  */

#define YMB(SCR,scr,IC) SCR##_ID,
enum SCRID {SCREENS_LIST};
#undef YMB

#define YMB(SCR,scr,IC)		[SCR##_ID] = MAKE_SCREEN_DEF(OP_A_NEW, OP_B_DEF, scr##_layout, scr##_state, IC, SCR##_FIELD_COUNT),
struct screen screens[] = {SCREENS_LIST};
#undef YMB


/* ---------------------------- World state management ------------------------------------ */

/* def_player */
struct player {
    struct mg_connection *c;
    UT_hash_handle hh;
    u8 scrid;
    struct live_screen scr;
    struct auth {
        time_t logintim;
        u8 attempts;
        u8 role;
        u16 uid;
        char uname[25];
        time_t locked_until;
    } auth;

};

/* 'players' is a pointer to a global hash table.  It is 
 * a container of 'player' who have connected. They are either 
 * in an authorized or unauthorized state. */
struct player *players = NULL;

/* Add a new player to the world. The player will start in
 * an unauthenticated state on 'SCR_LOGIN'. */
struct player *onboard_new_player(struct mg_connection *c) {
    struct player *player = (struct player*) malloc(sizeof(*player));
    player->c = c;
    player->scrid = LOGIN_SCREEN_ID;
    player->auth.logintim = 0;
    player->auth.attempts = 0;
    player->auth.role = 0;
    memset(player->auth.uname, 0, sizeof(player->auth.uname)); 
    player->auth.uid = UID_NF;           
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
    /* player->scr.state = buf;	 */
    mb_send(player);
}

/* =============================== DATABASE DSL ====================================== */


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


#define NAME_T 25
#define EMAIL_T 128
#define PHONE_T 17 //phone
#define PW_HASH_T crypto_pwhash_STRBYTES

typedef char name_t  [NAME_T]; 
typedef char email_t [EMAIL_T];
typedef char phone_t  [PHONE_T];
typedef char pw_t[PW_HASH_T];


/* TABLE INDEX */
#define DB_TABLES                               \
    X(usr,   USR_SCHEMA)                        \
    X(sys_state,  SYS_STATE_SCHEMA)		\
    X(pw,  PW_SCHEMA)		


/* COLUMN DEFINTIONS FOR ALL SCHEMA */
#define SYS_STATE_SCHEMA                                    \
    TCV(sys_state_col,  admin_uid, u16, INTEGER, DF(1))     \
    TCV(sys_state_col,  boat_id,   u16, INTEGER, DF(100))	\
    TCVL(sys_state_col, cust_uid,  u16, INTEGER, DF(100))	

#define PW_SCHEMA                               \
    TCV(PW_C,  uid,  u16,  INTEGER, NN UQ)      \
    TCVL(PW_C, hash, pw_t, TEXT,    NN   )	

#define USR_SCHEMA                              \
    TCV(USR_C,  uid,   u16, INTEGER, NN UQ)		\
    TCV(USR_C,  uname, name_t, TEXT, NN UQ)     \
    TCV(USR_C,  role,  u8, INTEGER, NN)         \
    TCV(USR_C,  email, email_t, TEXT)           \
    TCV(USR_C,  phone, phone_t, TEXT)           \
    TCV(USR_C,  first, name_t, TEXT)            \
    TCVL(USR_C, last, name_t, TEXT)			


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
#undef x20
#undef COMMA
#undef PK
#undef NN
#undef UQ
#undef DF
#undef TBC_FK
#undef TBCL_FK


/* ---------------------------------  CRUD QUERIES    ------------------------------------------------------------ */
// create statment
// bind
// fillout struct and step, or, step and fill out struct.

#define SQLITE_READ_TO_NUL -1

sqlite3_stmt *create_statement(sqlite3 *db, char *q) { 
    sqlite3_stmt *stmt;
    int rc =  sqlite3_prepare_v2(db, q, SQLITE_READ_TO_NUL, &stmt,NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "%s\n", sqlite3_errmsg(db));
        exit(1);
    }
    return stmt;
}

/* PW crud */
sqlite3_stmt *create_pw_stmt = NULL;
sqlite3_stmt *read_pw_stmt = NULL;   //by uid

int create_pw (sqlite3 *db, struct pw_rec *pw) {
    if(!create_pw_stmt) {
	create_pw_stmt = create_statement(db,
						"INSERT INTO pw"
						"(uid, hash)"
						"VALUES (?, ?)"
						);
    }
    sqlite3_stmt *stmt = create_pw_stmt;

	
    sqlite3_bind_int(stmt,  1, pw->uid);
    sqlite3_bind_text(stmt, 2, pw->hash, -1, SQLITE_STATIC);

    int INSERT_OK = sqlite3_step(stmt);
    if (INSERT_OK != SQLITE_DONE) {
	fprintf(stderr, "%s\n", sqlite3_errmsg(db));
	return 0;
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    return 1;    
}


/* ID RANGES */
sqlite3_stmt *next_uid_admin_stmt; /* Admin range is 10 < uid <= 100 */
sqlite3_stmt *next_uid_staff_stmt; /* Staff range is 100 < uid  <= 200  */
sqlite3_stmt *next_uid_customer_stmt;  /* customre range is 1000 < uid < U16 max */

enum UID_T {
    ADMIN_UID_T, STAFF_UID_T, CUSTOMER_UID_T
};

/* Get the MAX + 1 uid for ranges by UID_T enum 
 *        admin    9 < uid  20
 *        staff   99 < uid < 120
 *    customers  999 < uid < 30000    */

int next_uid(sqlite3 *db,int typ) {
    if(!next_uid_admin_stmt)
        next_uid_admin_stmt = create_statement(db,"select MAX(uid) "
                                               " from usr "
                                               "where uid > 9 and uid < 20"
                                               );

    if(!next_uid_staff_stmt)
        next_uid_staff_stmt = create_statement(db,"select MAX(uid) "
                                               " from usr "
                                               "where uid > 99 and uid < 110"
                                               );
    if(!next_uid_customer_stmt)
        next_uid_customer_stmt = create_statement(db,"select MAX(uid) "
                                                  " from usr "
                                                  "where uid > 999 and uid < 30000"
                                                  );
    sqlite3_stmt *stmt;
    int uid;
    switch (typ) {
    case  ADMIN_UID_T:
        stmt = next_uid_admin_stmt;
        uid = 10;
            break;
    case STAFF_UID_T:
        stmt = next_uid_staff_stmt;
        uid = 100;
            break;
    case CUSTOMER_UID_T:
        stmt = next_uid_customer_stmt;
        uid = 1000;
        break;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            uid = (sqlite3_column_int(stmt, 0) + 1);
        }
    } else {
        fprintf(stderr,"sqlite3_step failed: %s", sqlite3_errmsg(db));
    }
    
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    return uid;
}

/* Search for uid and return the related hashed 'pw'. */
int read_pw (sqlite3 *db, struct pw_rec *pw) {
    int found = 0;
    if(!read_pw_stmt) {
        read_pw_stmt = create_statement(db, "select hash from pw where uid = ?");
    }
    
    sqlite3_stmt *stmt = read_pw_stmt;
    sqlite3_bind_int(stmt,  1, pw->uid);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *hash = (const char *)sqlite3_column_text(stmt, 0);
        strcpy(pw->hash, hash);
        found = 1;
    }
    
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    return found;
}

/* USR crud */
sqlite3_stmt *insert_usr_stmt = NULL;
sqlite3_stmt *delete_usr_stmt = NULL;
sqlite3_stmt *update_usr_stmt = NULL;
sqlite3_stmt *getall_usr_stmt = NULL;

int create_usr(sqlite3 *db, struct usr_rec *usr) {
    if(!insert_usr_stmt) {
        insert_usr_stmt = create_statement(db,
                                           "INSERT INTO usr"
                                           "(uid, role, uname, email, phone, first, last)"
                                           "VALUES (?, ?, ?, ?, ?, ?, ?)"
                                           );
    }
    
    sqlite3_stmt *stmt = insert_usr_stmt;
    sqlite3_bind_int(stmt,  1, usr->uid);
    sqlite3_bind_int(stmt,  2, usr->role);
    sqlite3_bind_text(stmt, 3, usr->uname, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, usr->email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, usr->phone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, usr->first, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, usr->last,  -1, SQLITE_STATIC);

    int INSERT_OK = sqlite3_step(stmt);
    if (INSERT_OK != SQLITE_DONE) {
        fprintf(stderr, "[create_usr] %s\n", sqlite3_errmsg(db));
        return 0;
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    return 1;    
}

struct all_users {
    int len;
    struct usr_rec rec[4096];
};


int read_usr_all(sqlite3 *db,struct all_users *usr ) {

    if(!getall_usr_stmt) {
	getall_usr_stmt = create_statement(db, "select * from usr");
    }
    
    sqlite3_stmt *stmt = getall_usr_stmt;

    int i = 0;
    while(sqlite3_step(stmt) == SQLITE_ROW) {
	usr->rec[i].uid = sqlite3_column_int(stmt,USR_C_uid);
    usr->rec[i].role = sqlite3_column_int(stmt,USR_C_role);
    strcpy(usr->rec[i].uname, (const char*)sqlite3_column_text(stmt, USR_C_uname));
	strcpy(usr->rec[i].email, (const char*)sqlite3_column_text(stmt, USR_C_email));
	strcpy(usr->rec[i].phone, (const char*)sqlite3_column_text(stmt, USR_C_phone));
	strcpy(usr->rec[i].first, (const char*)sqlite3_column_text(stmt, USR_C_first));
	strcpy(usr->rec[i].last,  (const char*)sqlite3_column_text(stmt, USR_C_last));
	i++;
    }
    usr->len = i;
    
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    return 1;
}

int update_usr(sqlite3 *db, struct usr_rec *usr) {
    if(!update_usr_stmt) {
	update_usr_stmt = create_statement(db,
					   "UPDATE usr"
					   "(uid, email, phone, first, last)"
					   "VALUES (?, ?, ?, ?, ?)"
					   );
    }
        sqlite3_stmt *stmt = insert_usr_stmt;
	
    sqlite3_bind_int(stmt,  1, usr->uid);
    sqlite3_bind_text(stmt, 2, usr->email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, usr->phone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, usr->first, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, usr->last,  -1, SQLITE_STATIC);
    
    return 0;
}

/* UPDATE users */
/* SET email = ?, phone = ? */
/* WHERE uid = ? */

/* DELETE FROM users */
/* WHERE uid = ? */


/* -------------------------------------INITIALIZATION----------------------------------------------------------   */

#define DEV_MODE 1
#define DB_RESET 0
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
    pw.uid = uid;
    if (crypto_pwhash_str ((char*)pw.hash,
                           clr_pw, strlen(clr_pw),
                           crypto_pwhash_OPSLIMIT_INTERACTIVE,
                           crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        perror("FAILED TO HASH PASSWORD");
        exit(1); //TODO: How should I handle this case???
    }
    create_pw(db, &pw);
}

/* Retrieve pw_hash from the database searching by uid.
 * Verify hashed clr_pw matches what is in the db. Return 1
 * on success and 0 on failure */
int pw_check_from_db(sqlite3 *db, int uid, char *clr_pw) {
    struct pw_rec pw;
    pw.uid = uid;
    int res = read_pw(db, &pw);
    (void) res;
    int verified = crypto_pwhash_str_verify(pw.hash, clr_pw, strlen(clr_pw));
    return (verified >= 0);
}

void view_users_test(sqlite3 *db) {
    struct all_users all;
    all.len = 0;
    read_usr_all(db, &all);
    for (int i = 0; i < all.len; i++) {
	printf("%d %s %s %s %s\n",
	       all.rec[i].uid,
	       all.rec[i].email,
	       all.rec[i].phone,
	       all.rec[i].first,
	       all.rec[i].last);
    }
}

/* ============================     END DB          ============================================= */

/* -----------------------------  Business Rules    --------------------------------------------- */


/* SANITIZE,NORMALIZE, VALIDATE*/
/* NAME_T, EMAIL_T, PHONE_T */

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

/* TODO:(ari) Deal with country code in better way.  */
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
        .uid = UID_NF,
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


void try_login(struct player *player, u8 *reqbuf) {

    /* Prevents login. 3 seconds after 3 missed attempts */
    if (time(NULL) < player->auth.locked_until) {
	render_login_warning(player, "Login lockout for 3 seconds.");
	return;
    }
        
    struct login_attempt *attempt = (struct login_attempt*) reqbuf;
    struct all_users users = {0};
    struct usr_rec usr = new_usr_rec();
    struct usr_rec cur = new_usr_rec();

    char username[NAME_T] = {0};
    char password[NAME_T] = {0};

    memcpy(username, attempt->username.val, attempt->username.len);
    memcpy(password, attempt->password.val, attempt->password.len);

    snv_name(username, usr.uname);
    read_usr_all(db, &users);

    /* fill in the inforamtion */
    for (int i = 0; i < users.len; i++ ) {
        cur = users.rec[i];
        if(strcmp(usr.uname, cur.uname ) == 0) {
            usr = cur;
            break;
        }
    }
    
    if (pw_check_from_db(db, usr.uid, password)) {
	set_live_screen(player, MAIN_SCREEN_ID);
        player->auth.logintim = time(NULL);
        player->auth.attempts = 0;
        player->auth.role = usr.role;
        player->auth.uid = usr.uid;
        strcpy(player->auth.uname, usr.uname);
        player->auth.locked_until = 0;
    } else {
        player->auth.attempts++;
        printf("failed\n");
    }

    /* Verify both input fields were submitted.. */
    if (attempt->head.nFields < 2) {
        player->auth.attempts++;
        render_login_warning(player, "All fields required.");
        return;
    }
}

/* Dispatch Business Logic */
void dispatch_business_logic(struct mg_connection *c, u8 *reqbuf, int reqbuflen) {

    (void) reqbuflen;

    struct player *player = NULL;
    HASH_FIND_PTR(players,&c, player);

    if (!player) {              
        /* Player is not yet added to the world. 
         * add player to the world and send initial login screen. */
        player = onboard_new_player(c);
	set_live_screen(player, LOGIN_SCREEN_ID);
        mb_send(player);
    } else {
        /* Player is in the world. Handler functions
         * aways set next scrid. */
        switch(player->scrid) { 
        case LOGIN_SCREEN_ID:         
            {
                try_login(player,reqbuf);
		mb_send(player);
            } break;
	case MAIN_SCREEN_ID:
	    mb_send(player);

        }

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

    usr.uid = next_uid(db, ADMIN_UID_T);                

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

    printf("%s %s %s %s uid:%d\n Add admin? (y/n): ",usr.first, usr.last,usr.phone,usr.email,usr.uid);
    read_stdin_interactive(add,"", sizeof(add));

    if(add[0] == 'y') {
        pw_encrypt_and_add_to_db(db, usr.uid, pass);
        strcpy(usr.uname, "alpha");
        usr.role = ROLE_ALPHA;
        create_usr(db, &usr);
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
    struct pw_rec pw = {.uid = 0, .hash = ""};
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

    #ifndef DEV_MODE
    require_root(db);
    #endif
            
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8001", ev_handler, NULL);

    for(;;) {
    	mg_mgr_poll(&mgr,1000);
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
         Header:           | opcode u8  |  aidkey u8   | nFields u8    |
         Field blocks:     | field_id  u8 |  fldlen u8 | field_val u24 |
---------------------------------------------------------------------------------- */
