#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>

#define DELAY (10) //Configures the millisecond delay between game updates.

#define PLAYER_WIDTH (5) //Configures the width of the player sprite.
#define PLAYER_HEIGHT (4) //Configures the height of the player sprite.
#define PLATFORM_WIDTH (7) //Configures the width of both good and bad platforms as they share the same width.
#define PLATFORM_HEIGHT (2) //Configures the height of both good and bad platforms as they share the same height.

#define MAX_GOOD_PLATFORM (50)
#define MAX_BAD_PLATFORM (8)

bool game_over = false;	//Set this to true when the game is over.
bool update_screen = true; //Set this to false to prevent the screen from updating.
bool paused = false; //Set this to true to pause the game.

//Declare a sprite_id known as 'good_platform'.
sprite_id good_platform[MAX_GOOD_PLATFORM];
int num_good_platforms = 32;

//Declare a sprite_id known as 'bad_platform'.
sprite_id bad_platform[MAX_BAD_PLATFORM];
int num_bad_platforms = 6;

char *good_platform_image =
	/**/ "======="
		 /**/ "=======";

char *bad_platform_image =
	/**/ "XXXXXXX"
		 /**/ "XXXXXXX";


sprite_id setup_a_goodplatform() {
	int pfx = 2 + rand() % (screen_width() - 5 - 2) + 1;
	int pfy = 2 + rand() % (screen_height() - 5 - 2) + 1;

	int platform_width_array[] = {4, 20, 40, 60, 80};
	int platform_height_array[] = {2, 10, 30, 50, 70};

	int x = rand() % 5;
	int y = rand() % 5;

	pfx = platform_width_array[x];
	pfy = platform_height_array[y];

	sprite_id good_platform = sprite_create(pfx, pfy, 7, 2, good_platform_image);

	return good_platform;
}

sprite_id setup_a_badplatform() {
	int bfx = 2 + rand() % (screen_width() - 5 - 2) + 1;
	int bfy = 2 + rand() % (screen_height() - 5 - 2) + 1;

	int bad_width_array[] = {4, 20, 40, 60, 80};
	int bad_height_array[] = {2, 10, 30, 50, 70};

	int x = rand() % 5;
	int y = rand() % 5;

	bfx = bad_width_array[x];
	bfy = bad_height_array[y];

	sprite_id bad_platform = sprite_create(bfx, bfy, 7, 2, bad_platform_image);

	return bad_platform;
}

void draw_sprites (sprite_id sids[], int n) {
	for (int i = 0; i < n; i++) {
		sprite_draw(sids[i]);
	}
}


void setup_goodplatforms () {
	for (int i = 0; i < num_good_platforms; i++) {
		good_platform[i] = setup_a_goodplatform();
	}
}

void setup_badplatforms () {
	for (int i = 0; i < num_bad_platforms; i++) {
		bad_platform[i] = setup_a_badplatform();
	}
}
void draw_all() {
	draw_sprites(good_platform, num_good_platforms);
	draw_sprites(bad_platform, num_bad_platforms);
}

// Setup the game.
void setup(void) {
	setup_goodplatforms();
	setup_badplatforms();
	draw_all();
}

// Play one turn of game.
void process(void)
{

}

void cleanup(void)
{
	// STATEMENTS
}


//Beginning of 'main' function.
int main(void) {
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
