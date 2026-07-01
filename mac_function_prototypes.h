void mb_send (struct player *);
void logout_player(struct player *);

void set_screen_flags(struct player *, int, u8);
void set_screen_unhide(struct player *, int ); 
void set_screen_hide(struct player *, int );
void set_screen_color(struct player *, int , enum colors );
void set_screen_text(struct player *, int, char *);

void today(char *, int);
void time_now(char *, int);
