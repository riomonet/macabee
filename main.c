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

#define INPUT(id, xx, yy, ww)                   \
    X(id, VIS_INPUT, xx, yy, ww, "", 0, 0, 0)

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

/* void update_failed_login_screen (u8 *respbuf, size_t *respbufsz) { */
/* #define LOGIN_ERROR_UPDATE                      \ */
/*     STATE(6, "Sorry, try again.", 0, RED) */

/*     // Generate the state-only array (or just a single struct) */
/* #define X(id, txt, len, flg, col)                           \ */
/*     { .field_id = (id), .text = (txt), .text_len = (len),   \ */
/*             .fg_color = (col), .flags = (flg) } */

/*     struct field_state login_error_update[] = { LOGIN_ERROR_UPDATE }; */
/* #undef X */
    
/*     u8 *pos = respbuf; */
/*     struct update_header h = { .opcode_a = 0x02, */
/*                                .opcode_b = 0, */
/*                                .num_fields = 1, */
/*                                .reserved = 0, */
/*                                .state_bytes = sizeof(login_error_update) */
/*     }; */
/*     memcpy(pos, &h, sizeof(h)); */
/*     pos += sizeof(h); */
/*     memcpy(pos,login_error_update, sizeof(login_error_update)); */
/*     pos += sizeof(login_error_update); */
/*     *respbufsz = (size_t)(pos - respbuf); */
/* } */

struct screen {
    int id;
    u8 *buf;
    size_t len;
};

struct screen serialize_screen(struct field_state *fs, struct field_layout *fl, int num_fields, u8 opA, u8 opB) {
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
    
    struct screen scr = {
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

#define LOGIN_SCREEN_FIELDS                             \
    LABEL(0,  9,  8, 27, "USER . . . . . . . . . . . ") \
    LABEL(1,  9, 10, 27, "PASSWORD . . . . . . . . . ") \
    INPUT(2, 38,  8, 24) \
    INPUT(3, 38, 10, 24) \
    LABEL_F(4,  5,  5, 37, "Tab to change fields, Enter to submit", FAINT) \
    LABEL(5, 40,  1, 19, "Marina 59 | Sign On") \
    STATUS(6, 38, 14, 17, "Sorry, try again.", HIDDEN, RED)

#define X(id, t, xx, yy, w, txt, len, flg, col)                         \
    { .field_id = (id), .type = (t), .x = (xx), .y = (yy), .width = (w) },
    struct field_layout login_screen_layout[] = {
        LOGIN_SCREEN_FIELDS 
    };
#undef X

#define X(id, t, x, y, w, txt, len, flg, col)               \
    { .field_id = (id), .text = (txt), .text_len = (len),   \
            .fg_color = (col), .flags = (flg) },
    struct field_state login_screen_state[] = {
        LOGIN_SCREEN_FIELDS
    }; 
#undef X

struct sc {
    u8 op_A;
    u8 op_B;
    size_t nFields;
    struct field_layout *layout;
    struct field_state *state;
};

#define MAX_SLOTS(arr) (sizeof(arr)/sizeof(arr[0]))

#define MAKE_SCREEN_DEF(opA, opB, layout_arr, state_arr)    \
    {       .op_A = (opA),                                  \
            .op_B = (opB),                                  \
            .layout = (layout_arr),                         \
            .state = (state_arr),                           \
            .nFields = MAX_SLOTS(state_arr)              \
            }

struct sc screens[] = {
    [SCR_LOGIN] = MAKE_SCREEN_DEF(OP_A_NEW, OP_B_DEF, login_screen_layout, login_screen_state)
};



/* ---------------------------- state management ------------------------------------------------------------------- */

struct who_is_where {
    u64 conn_id;

    enum where_you_at wya;
    enum permissions p;
    UT_hash_handle hh;
};

struct who_is_where *users = NULL;

int verify_login_credentials(u8 *reqbuf, int reqbuflen, u8 *respbuf, size_t *respbufsz) {
    // extract user  
    // extract pw 
    // verify user and pw
    // retunr ok, bad_user, bad_pass. 

    /* send an array of state structs of only the fields you want to change add hide and unhide */
    
    return 1;
}

/* respbuf is created in mongoose portion, that we fill in to
 send back out over the wire.*/
struct screen big_fucking_business(u64 conn_id, u8 *reqbuf, int reqbuflen) {
    struct who_is_where *wiw = NULL;
    HASH_FIND_INT(users,&conn_id,wiw);
    if (!wiw) {
        struct who_is_where *wiw = malloc(sizeof(*wiw));
        wiw->conn_id = conn_id;
        wiw->wya = SCR_LOGIN;
        HASH_ADD_INT(users, conn_id, wiw);
        struct sc scr = screens[SCR_LOGIN];
        return serialize_screen(scr.state, scr.layout, scr.nFields, scr.op_A, scr.op_B);
    } else {
        switch(wiw->wya) {
        case SCR_LOGIN:
            printf("LOGIN");

            //if login verified
            // int res = verify_login_credentials(reqbuf, reqbuflen);
            wiw->wya = SCR_MAIN;
            break;

        case SCR_MAIN:
            printf("MAIN");
            break;
        }
        // render new screen based on state 

    }
}

/* ===========================================================================
       The MONGOOSE SHIT HERE, someone clean it up.
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
              struct screen scr = big_fucking_business(c->id, (u8*)wm->data.buf, wm->data.len);
              mg_ws_send(c, scr.buf,scr.len, WEBSOCKET_OP_BINARY);
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

update screen. 


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


