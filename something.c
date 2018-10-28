#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>

// ADD INCLUDES HERE

// Configuration
#define DELAY (10) /* Millisecond delay between game updates */

#define HERO_WIDTH (5)
#define HERO_HEIGHT (5)
#define ZOMBIE_WIDTH (5)
#define ZOMBIE_HEIGHT (5)

// Game state.
bool game_over = false; /* Set this to true when game is over */
bool update_screen = true; /* Set to false to prevent screen update. */
bool paused = false;

char * hero_image =
/**/	"H   H"
/**/	"H   H"
/**/	"HHHHH"
/**/	"H   H"
/**/	"H   H";

char * zombie_image =
/**/	"ZZZZZ"
/**/	"   Z "
/**/	"  Z  "
/**/	" Z   "
/**/	"ZZZZZ";

char * msg_image =
/**/	"Goodbye and thank-you for playing ZombieDash Jr."
/**/	"            Press any key to exit...            ";

// (c) Declare a sprite_id called hero.
sprite_id hero;

// (m) Declare a sprite_id called zombie.
sprite_id zombie;


void draw_border() {
    int w = screen_width(), h = screen_height(), ch = '*';
    draw_line(0, 0, 0, h - 1, ch);
    draw_line(0, 0, w - 1, 0, ch);
    draw_line(0, h - 1, w - 1, h - 1, ch);
    draw_line(w - 1, 0, w - 1, h - 1, ch);
}

void draw_all() {
    clear_screen();
    draw_border();
    sprite_draw(hero);
    sprite_draw(zombie);
}

//  Initialises the hero at the centre of the terminal window.
//
//  Returns:
//      A sprite_id which is connected to a Sprite displaying the hero image
//      at the centre of the screen,

sprite_id setup_hero()
{
    int w, h;
    get_screen_size(&w, &h);

    int hw = HERO_WIDTH;
    int hh = HERO_HEIGHT;
    int x = (w - hw) / 2;
    int y = (h - hh) / 2;
    return sprite_create(x, y, hw, hh, hero_image);
}

//  Creates a sprite at a random location on the terminal window,
//  And prepare it to move in a random direction with speed 0.1
//  screen units per step.
//
//  Returns:
//      A sprite_id connected to a Sprite which displays the zombie image.

sprite_id setup_zombie() {
    int w, h;
    get_screen_size(&w, &h);

    // (n)  Set up the zombie at a random location on the screen.
    int zx = 1 + rand() % (w - ZOMBIE_WIDTH - 2);
    int zy = 1 + rand() % (h - ZOMBIE_HEIGHT - 2);
    sprite_id sp = sprite_create(zx, zy, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, zombie_image);

    // (p) Set the zombie in motion.
    int angle = rand() % 360;
    sprite_turn_to(sp, 0.1, 0);
    sprite_turn(sp, angle);

    return sp;
}

//  Setup game.
//
//  Side effects:
//      Global variables hero and zombie are initialised and the
//      display has been rendered.

void setup(void)
{
    hero = setup_hero();
    srand(9139532); // Usually: srand(get_current_time());
    zombie = setup_zombie();
    draw_all();
}

// Detect collision between arbitrary sprites.
//
// Parameters:
//  s1: the sprite_id of a sprite.
//  s2: the sprite_id of another sprite.
//
// Returns: true if and only if the bounding boxes of the sprites intersect.

bool sprites_collide(sprite_id s1, sprite_id s2)
{
    int top1 = round(sprite_y(s1));
    int bottom1 = top1 + sprite_height(s1) - 1;
    int left1 = round(sprite_x(s1));
    int right1 = left1 + sprite_width(s1) - 1;

    // TODO: In Topic 4 show how to eliminate this duplicated code.
    int top2 = round(sprite_y(s2));
    int bottom2 = top2 + sprite_height(s2) - 1;
    int left2 = round(sprite_x(s2));
    int right2 = left2 + sprite_height(s2) - 1;

    if (top1 > bottom2)
    {
        return false;
    }
    else if (top2 > bottom1)
    {
        return false;
    }
    else if (right1 < left2)
    {
        return false;
    }
    else if (right2 < left1)
    {
        return false;
    }
    else {
        return true;
    }
    // Alternatively:
    // return !( (top1 > bottom2) || (top2 > bottom1) || (right1 < left2) || (right2 < left1));
}

void do_game_over()
{
    game_over = true;
    clear_screen();

    // Get dimensions of screen and message text.
    int w = screen_width();
    int h = screen_height();
    int msg_height = 2;
    int msg_width = strlen(msg_image) / msg_height;
    int msg_left = (w - msg_width) / 2;
    int msg_top = (h - msg_height) / 2;

    // Create a sprite to display the message in the middle of the screen
    sprite_id msg = sprite_create(
        msg_left, msg_top, msg_width, msg_height, msg_image
    );

    // Display the message
    sprite_draw(msg);
    show_screen();

    // Purge input buffer then wait for a fresh key-press.
    timer_pause(1000);

    while (get_char() >= 0)
    {
        // Do nothing, but a key-press has been consumed by get_char.
    }

    wait_char();
}

//  Reads a character from the terminal. Intercepts the 'p' key
//  to enable or disable paused mode. In this program, pausing
//  switches to single step mode to enable convenient testing.
//
//  Side effects:
//      Attempts to fetch a key code from standard input. If
//      paused flag is set, then waits for a key; otherwise returns
//      immediately.If the key code is 'p': toggles the paused flag.
//
//  Returns:
//      If a key event is available: returns the key code.
//      Otherwise: returns -1.

int read_char()
{
    int key = paused ? wait_char() : get_char();

    if (key == 'p')
    {
        paused = !paused;
    }

    return key;
}


//  Given a key code and the dimensions of the terminal window,
//  tests to see if the hero sprite should move.
//
//  Parameters:
//      key_code: the key code.
//      term_width: the width of the terminal window.
//      term_height: the height of the terminal window.
//
//  Side effects:
//      If key is a, d, s, or w and the hero is free to move in the
//      designated direction, then hero sprite moves one unit in that
//      direction.

void move_hero(int key_code, int term_width, int term_height )
{
    int hx = round(sprite_x(hero));
    int hy = round(sprite_y(hero));

    // (i) Move hero left according to specification.
    if (key_code == 'a' && hx > 1) sprite_move(hero, -1, 0);

    // (i) Move hero right according to specification.
    if (key_code == 'd' && hx < term_width - sprite_width(hero) - 1) sprite_move(hero, +1, 0);

    // (j) Move hero up according to specification.
    if (key_code == 'w' && hy > 1) sprite_move(hero, 0, -1);

    // (k) Move hero down according to specification.
    if (key_code == 's' && hy < term_height - sprite_height(hero) - 1) sprite_move(hero, 0, +1);
}

//  Automatically moves a sprite (identified by parameter sid) when the
//  supplied key code is negative or equal to 'z'. The sprite moves as
//  dictated by its dx and dy values. It bounces off the border when it
//  collides with it.
//
//  Parameters:
//      sid: A sprite_id connected to a Sprite that will be moved.
//      key_code: A numeric key code.
//      term_width: the width of the terminal window.
//      h: the height of the terminal window.
//
//  Side effects:
//      The internal state of the Sprite is updated to reflect the new position
//      and (if it bounced off the wall) step sizes after motion.

void move_zombie(sprite_id sid, int key_code, int term_width, int h)
{
    if (key_code == 'z' || key_code < 0)
    {
        // (r) Zombie takes one step.
        sprite_step(sid);

        // (s) Get screen location of zombie.
        int zx = round(sprite_x(sid));
        int zy = round(sprite_y(sid));

        // (t) Get the displacement vector of the zombie.
        double zdx = sprite_dx(sid);
        double zdy = sprite_dy(sid);

        // (u) Test to see if the zombie hit the left or right border.
        if (zx <= 0)
        {
            zdx = fabs(zdx);
        }
        else if (zx >= term_width - sprite_width(sid))
        {
            zdx = -fabs(zdx);
        }

        // (v) Test to see if the zombie hit the top or bottom border.
        if (zy <= 0)
        {
            zdy = fabs(zdy);
        }
        else if (zy >= h - sprite_height(sid))
        {
            zdy = -fabs(zdy);
        }

        // (w) Test to see if the zombie needs to step back and change direction.
        if (zdx != sprite_dx(sid) || zdy != sprite_dy(sid))
        {
            sprite_back(sid);
            sprite_turn_to(sid, zdx, zdy);
        }

        // (q.b) End else-if test to move the zombie if key is 'z' or negative.
    }
}

// Perform one iteration of the event loop.
void process(void)
{
    int w = screen_width();
    int h = screen_height();
    int key = read_char();

    if (key == 'q')
    {
        do_game_over();
        return;
    }

    move_hero(key, w, h);
    move_zombie(zombie, key, w, h);

    if (sprites_collide(zombie,hero))
    {
        do_game_over();
        return;
    }

    draw_all();
}

// Clean up game
void cleanup(void)
{
    // STATEMENTS
}

// Program entry point.
int main(void)
{
    setup_screen();
    setup();
    show_screen();

    while (!game_over)
    {
        process();

        if (update_screen)
        {
            show_screen();
        }

        timer_pause(DELAY);
    }

    cleanup();

    return 0;
}
