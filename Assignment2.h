#define PLATFORM_WIDTH (10)
#define PLATFORM_HEIGHT (2)
#define DEADZONE_WIDTH (10)
#define DEADZONE_HEIGHT (2)
#define PLAYER_WIDTH (7)
#define PLAYER_HEIGHT (9)
#define COL_WIDTH (PLATFORM_WIDTH + 2)
#define ROW_HEIGHT (PLAYER_HEIGHT * 2)
#define MAX_PLATFORM 25
#define MAX_ZOMBIE 5 
#define MAX_FOOD 5 
#define AVAILABILITY (20)
#define DEADZONE_PERCENT (25)

int get_col(sprite_id s);
int get_row(sprite_id s);
void setup_timer(void);
void setup_zombie_spawn_timer(void);
void process_timer();
void process_zombie_spawn_timer();
void setup_treasure();
void setup_zombie();
void status(void);
void setup_platforms(void);
void game_mode();
void intro();
void draw_int(uint8_t x, uint8_t y, int value, colour_t colour);
void game_pause(void);
void draw_all(void);
int get_current_platform(sprite_id s);
void end_game(void);
bool sprite_collision (Sprite* s1, Sprite* s2);
void setup_usb_serial(void);
void usb_serial_send(char * message);
void usb_serial_send_int(int value);
void player_movement(void);
void collision(void);
void setup(void);
void process(void);