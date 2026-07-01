
/* =============================================================================== 
DATABASE TABLE AND CRUD MACROS GENERATION SYSTEM (TCGS)
* =============================================================================== */

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
