#ifndef MAC_TYPES_INCLUDED
#define MAC_TYPES_INCLUDED
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


#define OP_A_NEW  1
#define OP_A_UPDATE 2
#define OP_B_DEF 1

enum VIS_TYP {
    VIS_NONE =  0,
    VIS_INPUT = 1,
    VIS_LABEL = 2,

    VIS_HLINE = 30,
    VIS_HLINE_HVY = 31,
    VIS_VLINE = 40,
    VIS_VLINE_HVY = 41,
};
    

#define MAX_FLD_SIZE 120


#define DEV_MODE 1
#define DB_RESET 0

#define UID_NF UINT16_MAX // UID NOT FOUND. [SENTINEL]

#define MAX_SLOTS(arr) (sizeof(arr)/sizeof(arr[0]))
#define STR2(X) #X
#define STR(X) STR2(X)

sqlite3 *db;

struct __attribute__((packed)) update_header {
    u8 opcode_a;
    u8 opcode_b;
    u8 num_fields;
    u8 reserved;
    u16 state_bytes;
};                              

/* struct __attribute__((packed)) packet_header { */
/*     u8 opcode_a; */
/*     u8 opcode_b; */
/*     u8 num_fields; */
/*     u8 reserved; */
/*     u16 layout_bytes;           /\* Num bytes in array *\/ */
/*     u16 state_bytes;            /\* Num bytes in array *\/ */
/* }; */

struct __attribute__((packed)) packet_header {
    u8 opcode_a;
    u8 opcode_b;
    u8 num_fields;
    u8 reserved;                /* IC right now.  */
    u16 layout_bytes;           /* Num bytes in array */
    u16 state_bytes;            /* Num bytes in array */
    u8 grid_type;
    u8 def_fg_color;
    u8 def_bg_color;            /* Body Background Color */
    u8 screen_flags;
    u8 reserved_slots[4];
};

// Immutable 1 for each field
struct __attribute__((packed)) field_layout {
    u8 field_id;
    u8 type;                    /* Widget type */
    u8 x;
    u8 y;
    u8 width;
    u8 height;
    u8 r2;
    u8 r3;
};

// Mutatable
struct __attribute__((packed)) field_state {
    u8 field_id;
    u8 flags;
    u8 fg_color;
    u8 bg_color;
    u8 text_len;
    u8 flags_h; 
    u8 r2;
    u8 r3;
    char text[MAX_FLD_SIZE];
};


enum permissions {
    ADMIN, CUSTOMER
};


enum colors {
    DEFAULT,
    RED,
    GREEN,
    BROWN,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    AMBER,
    BLACK,
    YELLOW
};



enum DSP {
    BOLD      = 1 << 0,
    FAINT     = 1 << 1,
    INVERSE   = 1 << 2,
    BLINK     = 1 << 3,
    UNDERLINE = 1 << 4,
    HIDDEN    = 1 << 5,
    H_LINE    = 1 << 6,
    PASSWORD  = 1 << 7
};

enum FLAGS_H {
    CLICKABLE = 1 << 0,
    NUMERIC =  1 << 1,
};


enum AID_KEYS {
    F2 = 0xF2,
    F6 = 0xF6,
    F7 = 0xF7,
    F8 = 0xF8,
    F9 = 0xF9,
    ENTER = 0xFF
};


/* def_screen */
struct screen {
    u8 op_A;
    u8 op_B;
    u8 ic;
    size_t nFields;
    struct field_layout *layout;
    struct field_state *state;
};


/* live screen is the actual transmit screen */
struct live_screen {
    u8 op_A;
    u8 op_B;
    u8 ic;
    size_t nFields;
    struct field_layout layout[100];
    struct field_state state[100];
};


/* Role values must be explictly defined or db values will no align. */
enum ROLES {
    ROLE_NONE = 0,
    ROLE_ALPHA = 1,
    ROLE_OFFICE = 2,
    ROLE_YARD = 3,
    ROLE_ACCOUNTING = 4,
  };

/* def_player */
struct player {
    struct mg_connection *c;
    UT_hash_handle hh;
    u8 scrid;
    u8 scrat;
    struct live_screen scr;
    struct auth {
        time_t logintim;
        u8 attempts;
        u8 role;
        u16 id;
        char uname[25];
        time_t locked_until;
    } auth;
    u32 last_seen;
};



/* client req header item */
struct __attribute__((packed)) cfh {
    u8 opcode;
    u8 AID;
    u8 nFields;
};


#define MAX_CLIENT_FIELD 128
/* client req field block field item */
struct __attribute__((packed)) cfb {
    u8 id; //field_id
    u8 len;
    char val[MAX_CLIENT_FIELD]; 
};

struct login_attempt {
    struct cfh head;
    struct cfb username;
    struct cfb password;
};

struct anc_form {
    struct cfh head;
    struct cfb first;
    struct cfb last;
    struct cfb email;
    struct cfb phone;
};




#endif
