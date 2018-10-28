#include <stdlib.h>
#include <math.h>
#include <cab202_timers.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <string.h>

// all of the code here has been referenced by the lecture notes

// set the screen width of cygwin to 105 width and 39 height
#define DELAY 10 /* milliseconds */
#define TITLE_WIDTH (90)
#define TITLE_HEIGHT (1)
#define PLAYER_WIDTH (3)
#define PLAYER_HEIGHT (3)
#define PLATFORM_WIDTH (7)
#define PLATFORM_HEIGHT (2)
#define DEADZONE_WIDTH (7)
#define DEADZONE_HEIGHT (2)
#define GAMEOVER_WIDTH (40)
#define GAMEOVER_HEIGHT (1)
#define MAX_DEADZONE 6
#define MAX_PLATFORM 50
#define LINE_WIDTH (105)
#define LINE_HEIGHT (1)
#define TREASURE_WIDTH (3)
#define TREASURE_HEIGHT (3)

// sprite_id's and timer_id's
sprite_id title;
sprite_id standby;
sprite_id top_line;
sprite_id bottom_line;
sprite_id player;
sprite_id treasure;
sprite_id platforms[MAX_PLATFORM];
sprite_id deadzones[MAX_DEADZONE];
timer_id timer;
// Globle variables
bool game_over;
bool paused = false;
bool fall;
bool update_screen = true;
bool run = true;
int lives = 10;
int score;
int seconds;
int minutes;
int treasure_image_animation = 0;
int num_platforms = 32;
int num_deadzone = 8;
int treasurespeed = 2;
double player_dx = 0;
double player_dy = 0;
char * platform_image = /**/    "======="
                        /**/    "=======";
char * deadzone_image = /**/    "XXXXXXX"
                        /**/    "XXXXXXX";

char * gameover_image = "Game Over: Press 'q' to quit"
" press 'r' to restart the game";
char * playerstanding_image =
/**/" o "
/**/"/|\\"
/**/"/ \\";

char * pa_image =
/**/"(*) "
/**/"/|\\"
/**/"/ \\";

char * playerrunningleft_image =
/**/" o "
/**/"\\|>"
/**/"< \\";

char * playerrunningright_image =
/**/" o "
/**/"<|/"
/**/"/ >";
char * playerjump_image =
/**/"\\o/"
/**/ " | "
/**/"/ \\";

char * playerfalling_image =
/**/"\\o/"
/**/ " | "
/**/ "( )";

char * treasure_image =
/**/"---"
/**/"|*|"
/**/"---";

char * treasureanimation_image =
/**/"***"
/**/"*$*"
/**/"***";

// setting up the platform in random position
sprite_id setup_a_platform()
{
    int pfx = 2 + rand() %  (screen_width() - 5 - 2) + 1; // the x location of the platforms
    int width[] = { 2, 14, 26, 38, 50, 62, 74, 86 }; // set the dimensions of the platforms width
    int x = rand() % 8; // the amount of columns of platforms which is 8 since that is the size of the array
    pfx = width[x]; // setting the pfx to the width array of x
    int pfy = 10 + rand() %  (screen_height() - PLATFORM_HEIGHT); // set the y location of the platforms
    int heights[] = { 17, 24, 31}; // set the dimensions of the platforms height
    int y = rand() % 3; // sets the amount of rows of platforms which is 3 since that is the size of the array
    pfy = heights[y]; // setting the pfy to the height array of the y
    sprite_id new_platform = sprite_create( pfx, pfy, PLATFORM_WIDTH, PLATFORM_HEIGHT, platform_image); // creates a new sprite platform with the locations pfx and pfy which would randomize the platforms
    platforms[0] = sprite_create(2, 11, PLATFORM_WIDTH, PLATFORM_HEIGHT, platform_image);
    platforms[1] = sprite_create(14, 11, PLATFORM_WIDTH, PLATFORM_HEIGHT, platform_image);
    platforms[2] = sprite_create(26, 11, PLATFORM_WIDTH, PLATFORM_HEIGHT, platform_image);
    platforms[3] = sprite_create(38, 11, PLATFORM_WIDTH, PLATFORM_HEIGHT, platform_image);
    platforms[4] = sprite_create(50, 11, PLATFORM_WIDTH, PLATFORM_HEIGHT, platform_image);
    platforms[5] = sprite_create(62, 11, PLATFORM_WIDTH, PLATFORM_HEIGHT, platform_image);
    platforms[6] = sprite_create(74, 11, PLATFORM_WIDTH, PLATFORM_HEIGHT, platform_image);
    platforms[7] = sprite_create(86, 11, PLATFORM_WIDTH, PLATFORM_HEIGHT, platform_image);
    return new_platform; // returns the new_platform sprite_id
}

// setting up the deadzone in random position
sprite_id setup_a_deadzone()
{
    int dfx = 2 + rand() %  (screen_width() - -5 - 2) + 1; // the x location of the deadzones
    int width[] = { 2, 14, 26, 38, 50, 62, 74}; // set the dimensions of the deadzones width
    int x = rand() % 7; // sets the amount of columns of deadzones which is 7 since that is the size of the array
    dfx = width[x]; // setting the dfx to the width array of x
    int dfy = 10 + rand() %  (screen_height() - DEADZONE_HEIGHT); // set the y location of the deadzones
    int heights[] = { 17, 24, 31 }; // set the dimensions of the deadzones height
    int y = rand() % 3; // sets the amount of rows of the deadzones which is 3 since that is the size of the array
    dfy = heights[y]; // setting the dfy to the height array of y
    sprite_id new_deadzone = sprite_create( dfx , dfy, 7, 2, deadzone_image); // creates a new sprite deadzone with the locations dfx and dfy which would randomize the deadzones
    return new_deadzone; // returns the new_deadzone sprite_id
}

// drawing the sprite arrays, such as platforms and deadzones
void draw_sprites( sprite_id sids[], int n )
{
    for ( int i = 0; i < n; i++)
    {
        sprite_draw(sids[i]);
    }
}

// game play time which checks if it is 60 seconds it goes to 1 minute and restarts the seconds
void process_timer()
{
  if(timer_expired(timer))
  {
    seconds++;
    if(seconds == 60)
    {
      minutes++;
      seconds = 0;
    }

    timer_reset(timer);
  }
}

// sets the timer to 1000 milliseconds
void setup_timer()
{
  timer = create_timer(1000);
}

// moves the treasure right and left
void move_treasure (void)
{
	int treasure_x = round(sprite_x(treasure));
	double treasure_dx = sprite_dx(treasure);
	double treasure_dy = sprite_dy(treasure);
	sprite_step(treasure);
	if ( treasure_x >= screen_width() - sprite_width(treasure) - abs(treasurespeed)) // when the players touches the edgoe of the screen he moves to the other direction
  {
    treasurespeed *= -1;
	}
	if (treasure_x < 0 + abs(treasurespeed)) // gets the speed of the treasure so the treasure doesn't bounce right and left in an invisible wall
  {
    treasurespeed = abs(treasurespeed);
	}
	if (treasure_dx != sprite_dx(treasure) || treasure_dy != sprite_dy(treasure)) // checks if the treasure needs to change directions
  {
	   sprite_back(treasure);
	   sprite_turn_to(treasure, treasure_dx, treasure_dy);
	}
}

// collision for the sprites
bool sprite_collision (sprite_id s1, sprite_id s2)
{
  // determines where s1 and s2 sprites are located
	int l1 = round(sprite_x(s1));
  int l2 = round(sprite_x(s2));
  int t1 = round(sprite_y(s1));
  int t2 = round(sprite_y(s2));
  int r1 = l1 + sprite_width(s1) - 1;
  int r2 = l2 + sprite_width(s2) - 1;
  int b1 = t1 + sprite_height(s1) + 0.5;
  int b2 = t2 + sprite_height(s2) - 1;

  // checks the platform collisions
  if(l1 > r2) return false;
  if(l2 > r1) return false;
  if(t1 > b2) return false;
  if(t2 > b1) return false;
  return true;
}

// draws all of the sprites
void draw_all(void)
{
  clear_screen();
  sprite_draw(title);
  sprite_draw(top_line);
  sprite_draw(bottom_line);
  sprite_draw(player);
  sprite_draw(standby);
  sprite_draw(treasure);
  sprite_move(treasure, treasurespeed, 0);
  move_treasure();
  draw_sprites(platforms, num_platforms);
  draw_sprites(deadzones, num_deadzone);
  draw_formatted(92, 2, "%d", score);
  draw_formatted(49, 2, "%d", lives);
  draw_formatted(70,2,"%02d:%02d",minutes, seconds);
}


// setting up the number of platforms on the screen
void setup_platforms()
{
  for(int i = 0; i < num_platforms; i++)
  {
    platforms[i] = setup_a_platform();
  }
}

// setting up the number of deadzones on the screen
void setup_deadzones()
{
  for(int i = 0; i < num_deadzone; i++)
  {
    deadzones[i] = setup_a_deadzone();
  }
}


// checks if a single sprite collides with a sprite_id  array
sprite_id sprites_colliding_with_any(sprite_id s, sprite_id sprites[], int n)
{
	sprite_id result = NULL;

	for(int i = 0; i < n; i++)
	{
		if(sprite_collision(s, sprites[i]))
		{
			result = sprites[i];
		}
	}

	return result;
}

// setting up the sprites
void setup(void)
{
  char * title_image = "Daniel Pretlove: N10193308               Lives:                Time:"
                      "                Score: ";
  char * line_image = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                      "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
  title = sprite_create(1,2,TITLE_WIDTH, TITLE_HEIGHT, title_image);
  top_line = sprite_create(0,0,LINE_WIDTH,LINE_HEIGHT,line_image);
  bottom_line = sprite_create(0,4,LINE_WIDTH,LINE_HEIGHT,line_image);
  player = sprite_create(97,8, PLAYER_WIDTH, PLAYER_HEIGHT, playerstanding_image);
  standby = sprite_create(97,11, PLATFORM_WIDTH, PLATFORM_HEIGHT, platform_image);
  treasure = sprite_create(1, 36, TREASURE_WIDTH, TREASURE_HEIGHT, treasure_image);
  setup_platforms();
  setup_deadzones();
}


// when game over this screen shows up
void end_game(void)
{
//	game_over = true;
	clear_screen();
	int w = screen_width();
	int h = screen_height();
	int gameover_height = 2;
  int gameover_width = strlen(gameover_image) / gameover_height;
  int gameover_left = (w - gameover_width) / 2;
  int gameover_top = (h - gameover_height) / 2;
  // prints your current score
	draw_formatted(3, 12, "Current Score: %d", score);
  // prints your game play
  draw_formatted(3,14,"Game Play: %02d:%02d", minutes, seconds);
	sprite_id gameover = sprite_create(gameover_left, gameover_top, gameover_width, gameover_height, gameover_image);
  // draws gamover message
  sprite_draw(gameover);
	show_screen();
}

void treasure_collision_detection(void)
{
  // checks if player and treasure is collided and if so then the player spawns back in the starting point
  // and gains 2 points
  if (sprite_collision(player, treasure))
  {
    lives += 2;
    clear_screen();
    setup();
    sprite_destroy(player);
    player = sprite_create(14,8, PLAYER_WIDTH, PLAYER_HEIGHT, playerstanding_image);
    sprite_draw(player);
    //changes the images of the player and treasure when they are being spawned
    sprite_set_image(player, pa_image);
    sprite_set_image(treasure, treasureanimation_image);
  }
}

void collision_detection(void)
{
  int py = round(sprite_y(player));
  // if the player the touches the bottom of the screen
  // then he dies and spawns back to the standby platform
  if(py == (screen_height() - sprite_height(player)))
  {
      lives--;
      clear_screen();
      setup();
      sprite_destroy(player);
      player = sprite_create(50,8, PLAYER_WIDTH, PLAYER_HEIGHT, playerstanding_image);
      sprite_draw(player);
  }

  // if the lives are zero then it goes to the end game screen
  if(lives == 0)
  {
    game_over = true;
    end_game();
  }
}

void platforms_collision_detection(void)
{
  // checks if the player isn't touching the standby platform or the platforms array then
  // the plays falls
  if(!sprites_colliding_with_any(player, platforms, num_platforms))
  {
    if((!sprite_collision(player, standby)))
    {
      sprite_move(player, 0, 1);
      sprite_set_image(player, playerfalling_image);
    }
    // if the player falls on the platforms then he adds a point
    if(sprites_colliding_with_any(player, platforms, num_platforms))
    {
        score = score + 1;
        fall = true;
    }
  }
}

void deadzones_collision_detection(void)
{
  // loses one life if a player touches the deadzone and if there are more lives
  // he spawn back at starting point
  if(sprites_colliding_with_any(player, deadzones, num_deadzone))
  {
    lives--;
    clear_screen();
    setup();
    sprite_destroy(player);
    player = sprite_create(74,8, PLAYER_WIDTH, PLAYER_HEIGHT, playerstanding_image);
    sprite_draw(player);
  }
}
void process(void)
{
    clear_screen();
    // calls the process_timer function
    process_timer();
    // changes the treasures image continously
    if (treasure_image_animation == 0)
    {
  		sprite_set_image( treasure, treasure_image);
  		treasure_image_animation = treasure_image_animation + 1;
  	}

    else if (treasure_image_animation == 1)
    {
  		sprite_set_image( treasure, treasureanimation_image);
  		treasure_image_animation = 0;
  	}
    treasure_collision_detection();
    platforms_collision_detection();
    deadzones_collision_detection();
    collision_detection();
		draw_all();
		sprite_set_image(treasure, treasure_image);
}

// moves the player left, right, up and down if a certain key is being preseed
void game_movement()
{
  // pauses the game if t is being pressed
  int key = paused ? wait_char() :get_char();
  int px = round(sprite_x(player));
  int py = round(sprite_y(player));

  // if a is pressed then player moves left
  if(key == 'a' && px > 1)
  {
    sprite_move(player, -1, 0);
    sprite_set_image(player, playerrunningleft_image);
  }
  // if d is pressed then player moves right
  if(key == 'd' && px < (screen_width() - sprite_width(player)))
  {
    sprite_move(player, 1, 0);
    sprite_set_image(player, playerrunningright_image);
  }

  // if the player is collided with the platforms and the standby platform
  if(sprites_colliding_with_any(player, platforms, num_platforms) || (sprite_collision(player, standby)))
  {
    // if key is pressed the player can jump once
    if(key == 'w' && py > 5)
    {
      sprite_move(player, 0, -4);
      timer_pause(10);
      sprite_set_image(player, playerjump_image);
    }
  }

  // if 't' is pressed then pause the game
  if (key == 't')
  {
    paused = !paused;
  }
}

// if 'r' is pressed then restart the game
void restart_quit(void)
{
  int key = get_char();

  if(key == 'r')
  {
    clear_screen();
    setup();
    lives = 10;
    seconds = 0;
    minutes = 0;
    score = 0;
    game_over = false;
  }
  // if 'q' is pressed then end the game
  if(key == 'q')
  {
    game_over = true;
    exit(0);
  }
}
// checks if the key isn't being pressed
void playerstandingposition(void)
{
  int key = get_char();
  if(key == -1)
  {
    timer_pause(DELAY);
    sprite_set_image(player, playerstanding_image);
  }
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
    setup_timer();
#if defined(AUTO_SAVE_SCREEN)
    auto_save_screen(true);
#endif

    setup();
    show_screen();

    while (run){
      show_screen();
      if ( !game_over )
      {
          process();
          process_timer();
          game_movement();
          playerstandingposition();
          if ( update_screen )
          {
              show_screen();
          }

          timer_pause(70);
      }

      else if(game_over)
      {
        end_game();
        restart_quit();
      }
    }
		clear_screen();
    cleanup();
    return 0;
}