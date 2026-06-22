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

#define VIS_INPUT 1 << 0
#define VIS_LABEL 1 << 1
#define VIS_LINE  1 << 2
#define MAX_FLD_SIZE 120

struct __attribute__((packed)) update_header {
    u8 opcode_a;
    u8 opcode_b;
    u8 num_fields;
    u8 reserved;
    u16 state_bytes;
};                              

struct __attribute__((packed)) packet_header {
    u8 opcode_a;
    u8 opcode_b;
    u8 num_fields;
    u8 reserved;
    u16 layout_bytes;           /* Num bytes in array */
    u16 state_bytes;            /* Num bytes in array */
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
    BLACK
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
};


#endif
