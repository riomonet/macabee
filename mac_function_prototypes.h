void mb_send (struct player *);
void logout_player(struct player *);


/* used to create updates */
void field_set_flags(struct field_state *, u8 flags);
void field_unhide(struct field_state * ); 
void field_hide(struct field_state * );
void field_set_color(struct field_state *, enum colors );
void field_set_text(struct field_state *, char *);


/* used in renderers */
void set_screen_text(struct player *player,int col, char *buffer);


void today(char *, int);
void time_now(char *, int);



struct field_state field_copy(struct field_state old);


int try_login(struct player *, u8 *);
void mb_send_update(struct player *player, int nFields, struct field_state *buf, int ic);


int snv_name(char *name, char *cln);
int snv_phone(char *phone, char *cln);
int snv_email(char *email, char *cln);



struct usr_rec new_usr_rec();


