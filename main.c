
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mac_types.h"
#include "mongoose.h"
#include "cJSON.h"
#include "uthash.h"



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

/*============================================================================
       BIG FUCKING BUSINESS HERE
===============================================================================*/

/* ---------------------------- screen rendering ------------------------------------------------------------------- */
#define VIS_INPUT 0
#define VIS_LABEL 1
#define MAX_FLD_SIZE 32

// Immutable
struct field_layout {
    u8 field_id;
    u8 type; // (label | input)
    u8 x;
    u8 y;
    u8 width;
};

// Mutatable
struct field_state {
    u16 flags;
    u8 field_id;
    u8 fg_color;
    u8 bg_color;
    u8 text_len;
    char text[MAX_FLD_SIZE];
};

struct field_layout login_screen_layout[] = {
    {.field_id = 0, .type = VIS_LABEL, .x = 10, .y = 10, .width = 30},
    {.field_id = 1, .type = VIS_LABEL, .x = 10, .y = 11, .width = 30},
    {.field_id = 2, .type = VIS_INPUT, .x = 40, .y = 10, .width = 24},
    {.field_id = 3, .type = VIS_INPUT, .x = 40, .y = 11, .width = 24}
};

// Initialize each field (you can put this in a helper function)
struct field_state *init_login_state(void) {
    struct field_state *login_screen_state = malloc(4 * sizeof(struct field_state));
    
    // field 0 - username label
    login_screen_state[0].field_id = 0;
    login_screen_state[0].fg_color = 7;      // whatever your normal color is
    login_screen_state[0].flags = 0;
    strcpy(login_screen_state[0].text, "username...................");
    login_screen_state[0].text_len = 24;

    // field 1 - password label
    login_screen_state[1].field_id = 1;
    login_screen_state[1].fg_color = 7;
    login_screen_state[1].flags = 0;
    strcpy(login_screen_state[1].text, "password...................");
    login_screen_state[1].text_len = 24;

    // field 2 - username input (starts empty)
    login_screen_state[2].field_id = 2;
    login_screen_state[2].fg_color = 7;
    login_screen_state[2].flags = 0;
    login_screen_state[2].text[0] = '\0';
    login_screen_state[2].text_len = 0;

    // field 3 - password input (starts empty)
    login_screen_state[3].field_id = 3;
    login_screen_state[3].fg_color = 7;
    login_screen_state[3].flags = 0;
    login_screen_state[3].text[0] = '\0';
    login_screen_state[3].text_len = 0;

    return login_screen_state;
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


/* ---------------------------- state management ------------------------------------------------------------------- */
void parse_incoming_bs_from_client_and_render_something(u8 *buf, int len, u8 *sndbuf, int *sbufsz) {}

//void render_login_screen(u8 *buf, int *len) {}


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

void big_fucking_business(u8 *buf, int len, u8 *sndbuf, int *sbufsz) {
    if( buf[0] == 0xff) {
	//	struct visual_screen *s = define_login_screen();
	//	render_login_screen(sndbuf, sbufsz, s);
    } else {
	// is client trying to login 
	// if client is logged in
	parse_incoming_bs_from_client_and_render_something(buf,len, sndbuf, sbufsz);
    }
}


/* ===========================================================================
       MONGOOSE SHIT HERE 
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
		int sndbufsz = 0;
		u8 sndbuf[4096] = {0};
		
		big_fucking_business((u8*)wm->data.buf, wm->data.len,
				     sndbuf, &sndbufsz);
		mg_ws_send(c, sndbuf, sndbufsz, WEBSOCKET_OP_BINARY);
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




