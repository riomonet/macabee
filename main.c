
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
#define VIS_INPUT 0
#define VIS_LABEL 1
#define MAX_FLD_SIZE 40


struct __attribute__((packed)) packet_header {
    u8 opcode_a;
    u8 opcode_b;
    u8 num_fields;
    u8 reserved;
    u16 layout_bytes;
    u16 state_bytes;
};

// Immutable
struct __attribute__((packed)) field_layout {
    u8 field_id;
    u8 type; // (label | input)
    u8 x;
    u8 y;
    u8 width;
    u8 r1;
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
    u8 r1;
    u8 r2;
    u8 r3;
    char text[MAX_FLD_SIZE];
};


void make_me_a_login_screen (u8 *respbuf, size_t *respbufsz) {

    struct field_layout login_screen_layout[] = {
        {.field_id = 0, .type = VIS_LABEL, .x = 9, .y = 8, .width = 27},
        {.field_id = 1, .type = VIS_LABEL, .x = 9, .y = 10, .width = 27},
        {.field_id = 2, .type = VIS_INPUT, .x = 38, .y = 8, .width = 24},
        {.field_id = 3, .type = VIS_INPUT, .x = 38, .y = 10, .width = 24},
        {.field_id = 4, .type = VIS_LABEL, .x = 5, .y = 5, .width = 37},
        {.field_id = 5, .type = VIS_LABEL, .x = 40, .y = 1, .width = 19},
        
    };

    struct field_state login_screen_state[] = {
        {.field_id = 0, .fg_color = 7, .flags = 0, .text = "USER . . . . . . . . . . . ",.text_len = 27},
        {.field_id = 1, .fg_color = 7, .flags = 0, .text = "PASSWORD . . . . . . . . . ",.text_len = 27},
        {.field_id = 2, .fg_color = 7, .flags = 0, .text = "", .text_len = 0},
        {.field_id = 3, .fg_color = 7, .flags = 0, .text = "", .text_len = 0},
        {.field_id = 5, .fg_color = 7, .flags = 0, .text = "Tab to change fields, Enter to submit", .text_len = 37},
        {.field_id = 4, .fg_color = 7, .flags = 0, .text = "Marina 59 | Sign On", .text_len = 19},
    };
    
    u8 *pos = respbuf;

    struct packet_header h = { .opcode_a = 0,
                               .opcode_b = 0,
                               .num_fields = 6,
                               .reserved = 0,
                               .layout_bytes = sizeof(login_screen_layout),
                               .state_bytes = sizeof(login_screen_state)
    };
    
    memcpy(pos, &h, sizeof(h));
    pos += sizeof(h);
    
    memcpy(pos, login_screen_layout, sizeof(login_screen_layout));
    pos += sizeof(login_screen_layout);
    
    memcpy(pos,login_screen_state, sizeof(login_screen_state));
    pos += sizeof(login_screen_state);
    
    *respbufsz = (size_t)(pos - respbuf);
}

/* ---------------------------- state management ------------------------------------------------------------------- */
void parse_incoming_bs_from_client_and_render_something(u8 *reqbuf, int reqbuflen, u8 *respbuf, size_t *respbufsz) {
    make_me_a_login_screen(respbuf, respbufsz);
}

/* respbuf is created in mongoose portion, that we fill in to
 send back out over the wire.*/
void big_fucking_business(u8 *reqbuf, int reqbuflen, u8 *respbuf, size_t *respbufsz) {
	parse_incoming_bs_from_client_and_render_something(reqbuf, reqbuflen, respbuf, respbufsz);
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
                size_t respbufsz = 0;
                u8 respbuf[4096] = {0};
		
                big_fucking_business((u8*)wm->data.buf, wm->data.len,
                                     respbuf, &respbufsz);
                mg_ws_send(c, respbuf, respbufsz, WEBSOCKET_OP_BINARY);
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
    
  | opcode A 8bits | opcode B 8bits | Number of headers 1byte| headers (number of headers * sz of header) | payload data 1 item per header variable len |

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
