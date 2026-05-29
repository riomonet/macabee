#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mac_types.h"
#include "mongoose.h"
#include "cJSON.h"
#include "uthash.h"

/*============================================================================
       BIG FUCKING BUSINESS HERE
===============================================================================*/

/* ---------------------------- screen definitions ------------------------------------------------------------------- */

#define LABEL(id, xx, yy, ww, txt) \
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, 0, 0)

#define INPUT(id, xx, yy, ww)                  \
    X(id, VIS_INPUT, xx, yy, ww, "", 0, 0, 0)

#define HL(id, xx, yy, ww)                      \
    X(id, VIS_LINE, xx, yy, ww, "", 0, 0, 0)

#define LABEL_F(id, xx, yy, ww, txt, flg)                       \
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, flg, 0)

#define LABEL_C(id, xx, yy, ww, txt, col) \
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, 0, col)

#define LABEL_CF(id, xx, yy, ww, txt, flg, col) \
    X(id, VIS_LABEL, xx, yy, ww, txt, sizeof(txt)-1, flg, col)

#define STATUS(id, xx, yy, ww, txt, flg, col) \
    LABEL_CF(id, xx, yy, ww, txt, flg, col)

#define STATE(id, txt, flg, col) \
    X(id, txt, sizeof(txt)-1, flg, col)

#define STATE_LEN(id, txt, len, flg, col) \
    X(id, txt, len, flg, col)

#define MAX_SLOTS(arr) (sizeof(arr)/sizeof(arr[0]))

#define MAKE_SCREEN_DEF(opA, opB, layout_arr, state_arr)   \
    {       .op_A = (opA),                                  \
            .op_B = (opB),                                  \
            .layout = (layout_arr),                         \
            .state = (state_arr),                           \
            .nFields = MAX_SLOTS(state_arr)                 \
}

/* -------------------- LOGIN SCREEN --------------------- */

#define LOGIN_SCREEN_FIELDS                                             \
    LABEL(LOGIN_L1,  9,  8, 27, "USER . . . . . . . . . . . ")  \
        LABEL(LOGIN_L2,  9, 10, 27, "PASSWORD . . . . . . . . . ") \
        INPUT(LOGIN_IUSER, 38,  8, 24)                             \
        INPUT(LOGIN_IPW,38, 10, 24)                                \
        LABEL_F(LOGIN_L3,5,  5, 37, "Tab to change fields, Enter to submit", FAINT) \
        LABEL(LOGIN_L4, 40,  1, 19, "Marina 59 | Sign On")           \
        STATUS(LOGIN_L5,38, 14, 17, "Sorry, try again.", HIDDEN, RED)


#define X(id, t, x, y, w, txt, len, flg, col) id,
enum LOGIN_SCR_IDX {
    LOGIN_SCREEN_FIELDS
    LOGIN_FIELD_COUNT
};
#undef X


#define X(id, t, xx, yy, w, txt, len, flg, col)                         \
    [id] = { .field_id = (id), .type = (t), .x = (xx), .y = (yy), .width = (w) },
    struct field_layout login_screen_layout[] = {
        LOGIN_SCREEN_FIELDS 
    };
#undef X

#define X(id, t, x, y, w, txt, len, flg, col)               \
   [id] = { .field_id = (id), .text = (txt), .text_len = (len),   \
            .fg_color = (col), .flags = (flg) },
    struct field_state login_screen_state[] = {
        LOGIN_SCREEN_FIELDS
    }; 
#undef X


/* --------------------------------------------END LOGIN SCREENS ---------------------------------------------- */
/* -------------------------------------------- MAIN SCREEN START --------------------------------------------- */
/*   id, col, row, width */
#define MAIN_SCREEN_FIELDS                                      \
    LABEL(MAIN_L1,7,1,8, "DSP_USER")                            \
        LABEL(MAIN_L2,67,1,8, "DSP_DATE")                       \
        LABEL(MAIN_L3,67,2,8, "DSP_TIME")                       \
        INPUT(MAIN_ISELECT,6,24,1)                              \
        LABEL(MAIN_L4,29,1,21, "Marina Access Control")         \
        LABEL(MAIN_L5,35,2,9, "MAIN MENU")                      \
        LABEL(MAIN_L6,6,6,28, "Select one of the following:")   \
        LABEL(MAIN_L7,10,8,15,   "1. Add customer")             \
        LABEL(MAIN_L8,10,9,17,   "2. View customers")           \
    LABEL(MAIN_L9,10,10,17,  "3. Access history")               \
    LABEL(MAIN_L10,10,11,15,  "4. Live montior")                \
    LABEL(MAIN_L11,1,23,9,"Selection")                          \
    LABEL(MAIN_L12,1,24,4, "===>")                              \
    HL(MAIN_L13,1,26,100)                                       \
    LABEL(MAIN_L14,6,28,9,"F6=Logout")                          \
    LABEL(MAIN_L15,19,28,9, "F7=Search")                        \
    LABEL(MAIN_L16,31,28,16,"F8=Redraw screen")                 \
    HL(MAIN_L17,0,29,100)

#define X(id, t, x, y, w, txt, len, flg, col) id,
enum MAIN_SCR_IDX {
    MAIN_SCREEN_FIELDS
    MAIN_FIELD_COUNT
};
#undef X

#define X(id, t, xx, yy, w, txt, len, flg, col)                         \
    { .field_id = (id), .type = (t), .x = (xx), .y = (yy), .width = (w) },
    struct field_layout main_screen_layout[] = {
        MAIN_SCREEN_FIELDS 
    };
#undef X

#define X(id, t, x, y, w, txt, len, flg, col)               \
    { .field_id = (id), .text = (txt), .text_len = (len),   \
            .fg_color = (col), .flags = (flg) },
    struct field_state main_screen_state[] = {
        MAIN_SCREEN_FIELDS
    };
#undef X

/* -------------------------------------------- MAIN SCREEN END --------------------------------------------- */


struct net_payload_screen {
    int id;
    u8 *buf;
    size_t len;
};

struct net_payload_screen serialize_screen(struct field_state *fs, struct field_layout *fl, int num_fields, u8 opA, u8 opB) {
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
                               .reserved = 0,
                               .layout_bytes = layout_bytes,
                               .state_bytes =  state_bytes
    };
    
    struct net_payload_screen scr = {
        .id = 1,
        .buf = malloc(total_bytes),
        .len = total_bytes
    };

    u8 *pos = scr.buf;

    memcpy(pos, &h, sizeof(h));
    pos += sizeof(h);

    if (is_new) {
    memcpy(pos, fl, h.layout_bytes);
    pos += layout_bytes;
    }

    memcpy(pos,fs, state_bytes);
    pos += state_bytes;

    return scr;
}

struct screen {
    u8 op_A;
    u8 op_B;
    u8 ic;
    size_t nFields;
    struct field_layout *layout;
    struct field_state *state;
};

/* |--------------------------------------- ALL SCREEN DEFINTIONNS HERE -------------------------------- */
struct screen screens[] = {
    [SCR_LOGIN] = MAKE_SCREEN_DEF(OP_A_NEW, OP_B_DEF, login_screen_layout, login_screen_state),
    [SCR_MAIN] = MAKE_SCREEN_DEF(OP_A_NEW, OP_B_DEF, main_screen_layout, main_screen_state)
};


/* ---------------------------- state management ------------------------------------------------------------------- */

struct world_state {
    u64 conn_id;
    u8 cur_scr;
    u8 p;
    UT_hash_handle hh;
};

struct world_state *players = NULL;

/* ----------------------------- Business Rules ------------------------------------------------------------------- */
int verify_login_credentials(u8 *reqbuf, int reqbuflen, u8 *respbuf, size_t *respbufsz) {
    // extract user  
    // extract pw 
    // verify user and pw
    // retunr ok, bad_user, bad_pass. 

    return 1;
}


struct screen dispatch_business_logic(u64 conn_id, u8 *reqbuf, int reqbuflen) {

    struct world_state *player = NULL;
    HASH_FIND_INT(players,&conn_id, player);

    if (!player) {
        struct world_state *player = malloc(sizeof(*player));
        player->conn_id = conn_id;
        player->cur_scr = SCR_LOGIN;
        HASH_ADD_INT(players, conn_id, player);
        return screens[SCR_LOGIN];

    } else {
        switch(player->cur_scr) {
        case SCR_LOGIN:
            //player submitting credentials here.
            //if credential are bad return login
            return screens[SCR_MAIN];
            // else update player screenn and return main
            player->cur_scr = SCR_MAIN;
            break;
        case SCR_MAIN:
            break;
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
              struct screen nxt_scr = dispatch_business_logic(c->id, (u8*)wm->data.buf, wm->data.len);
              struct net_payload_screen payload = serialize_screen(nxt_scr.state, 
                                                                  nxt_scr.layout, 
                                                                  nxt_scr.nFields, 
                                                                  nxt_scr.op_A, 
                                                                   nxt_scr.op_B
                                                                   );
              
              mg_ws_send(c, payload.buf,payload.len, WEBSOCKET_OP_BINARY);
            }
        }
    }
}
    
int main(void) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8001", ev_handler, NULL);

    for(;;) {
	mg_mgr_poll(&mgr,1000);
     }
    return 0;
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


