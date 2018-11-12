#include <ascii_font.h>
#include <avr/io.h>
#include "cpu_speed.h"
#include "sprite.h"
#include "lcd.h"
#include "lcd_model.h"
#include "graphics.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <macros.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <usb_serial.h>
#include "cab202_adc.h"
#include "Assignment2.h"
#include "cab202_adc.h"
// global variables 
char buffer[30];
int seconds, minutes, lives = 10, score = 0, food_count = 5, zombie_count = 5;
volatile int counter_overflow = 0;
volatile uint8_t bit_counter = 0;
volatile uint8_t is_pressed;
double player_dx = 0, player_dy = 0;
double zombie_dx = 0, zombie_dy = 0;
double food_dx = 0, food_dy = 0;  
bool paused = false;
bool game_over = false; 
bool falling; 
bool ground; 
bool is_safe[MAX_PLATFORM];
int num_platforms = 10; 
int num_zombies = 5; 

// sprites 
Sprite player;
Sprite starting_block; 
Sprite treasure;
Sprite food; 
Sprite zombie[MAX_ZOMBIE];  
Sprite platforms[MAX_PLATFORM];

// drawing the sprites 
unsigned char player_bitmap [3] =
{
	0b00111000,
	0b00101000,
	0b00111000,
};

unsigned char platforms_bitmap[4] =
{
	0b11111111, 0b11000000,
	0b11111111, 0b11000000,
};

unsigned char deadzone_bitmap[4] =
{
	0b10101010, 0b10000000,
	0b11111111, 0b11000000,
};

unsigned char treasure_bitmap[3]=
{
	0b00111000,
	0b00111000,
	0b00111000,
};

unsigned char zombie_bitmap[4] = 
{
	0b00101000, 
	0b00111000,
	0b00101000,
	0b00111000,
};

unsigned char food_bitmap[3] = 
{
	0b1111111,
	0b1110000,
	0b1111111,
};
// get the column of the platform 
int get_col(sprite_id s)
{
	return (int)round(s->x) / COL_WIDTH; 
}

// gets the row of the platform 
int get_row(sprite_id s)
{
	return ((int)round(s->y) + PLAYER_HEIGHT - 8) / ROW_HEIGHT - 1; 
}


// setting up the timer for the game time 
void setup_timer(void)
{
	TCCR3A = 0;
	TCCR3B = 3;
	TIMSK3 = 1;
	sei();
}

// counters the timer overflow 
ISR(TIMER3_OVF_vect)
{
	counter_overflow++;
}


// processes the time of the seconds and minutes
void process_timer()
{
	seconds = (counter_overflow * 65536.0 + TCNT3) * 64.0 / 8000000.0;
	if(seconds == 60)
	{
		minutes++;
		seconds = 0;
		counter_overflow = 0;
	}
}

// setting up the treasure 
void setup_treasure()
{
	sprite_init(&treasure, 45, LCD_Y / 2 + 8, 7, 3, treasure_bitmap);
	treasure.dx = 0.3; 
	treasure.dy = 0; 
}

// setting up the zombie 
void setup_zombie()
{
	for(int i = 0; i < num_zombies; i++)
	{
		int x = 5 + rand() % (70); 
		int y = 10;
		sprite_init(&zombie[i], x, y, 7, 4, zombie_bitmap);	
	}
}


// draws the score, lives, minutes and seconds on the top of the screen in the game mode 
void status(void)
{
	sprintf(buffer, "S:%d L:%d T:%02d:%02d", score, lives, minutes, seconds);
	draw_string(0,0, buffer, FG_COLOUR);
}

// sets up the platforms in rows and columns 
void setup_platforms(void)
{

	int cols = LCD_X / COL_WIDTH; 
	int rows = (LCD_Y - 8) / ROW_HEIGHT; 

	int needed = rows * cols * AVAILABILITY; 
	int out_of = rows * cols; 

	num_platforms = 0;

	int safe_per_column[MAX_PLATFORM] = {0}; 

	for(int row = 0; row < rows; row++)
	{
		for(int col = 0; col < cols; col++)
		{
			// checks if the num_platform is more than or equal to the MAX_PLATFORM and if that is the case it will break
			if(num_platforms >= MAX_PLATFORM)
			{
				break; 
			}
			// if true then it will radomize the x and y of the platforms
			// it will check if the platform is safe 
			// it will increment the num_platforms 
			if(true)
			{
				needed++;
				sprite_id plat = &platforms[num_platforms];   
				int x = col * 17 + rand() % (17 - PLATFORM_WIDTH); 
				int y = 10 + (row + 1) * ROW_HEIGHT - PLATFORM_HEIGHT; 
				sprite_init(plat, x,  y, PLATFORM_WIDTH, PLATFORM_HEIGHT, platforms_bitmap);
				is_safe[num_platforms] = true; 
				safe_per_column[col]++; 
				num_platforms++;
			}

			// increments out_of 
			out_of++;
		}
	}

	int num_deadzones = num_platforms * DEADZONE_PERCENT / 100; 

	if(num_deadzones < 2)
	{
		num_deadzones = 2; 
	}
	for(int i = 0; i < num_deadzones; i++)
	{
		#define MAX_TRAILS (50)
		// will make at least 3 platforms to dead_platofmrs 
		for(int trial = 0; trial < MAX_TRAILS; trial++)
		{
	 		int deadzone_index = 1 + rand() % (num_platforms - 1); 
	 		sprite_id plat = &platforms[deadzone_index]; 
	 		int col = get_col(plat);

	 		if(safe_per_column[col] > 1)
	 		{
	 			is_safe[deadzone_index] = false; 
	 			safe_per_column[col]--; 
	 			plat->bitmap = deadzone_bitmap; 
	 			break; 
	 		}
	 	 }
	 }
}

// will setup the treasure and platforms if in game_mode
void game_mode()
{
	clear_screen();
  	setup_platforms();
  	setup_treasure();
  	usb_serial_send("Game mode");
  	usb_serial_send("\r\n");
  	usb_serial_send("Player_x is: ");
  	usb_serial_send_int((int)player.x);
    usb_serial_send("\r\n");
  	usb_serial_send("Player_y is: ");
  	usb_serial_send_int((int)player.y);
  	show_screen();
}

void intro()
{
	// setting up the teensy
    lcd_init(LCD_LOW_CONTRAST);
    setup_usb_serial();
    CLEAR_BIT(DDRF, 6); // SW2
    CLEAR_BIT(DDRF, 5); // SW3
    CLEAR_BIT(DDRB, 0); // centre
    CLEAR_BIT(DDRD, 0); // right
    CLEAR_BIT(DDRB, 1); // left
    CLEAR_BIT(DDRD, 1); // up
    CLEAR_BIT(DDRB, 7); // down
    // enabling led0 and led1
    SET_BIT(DDRB, 2);
    SET_BIT(DDRB, 3);
    SET_BIT(DDRC, 7); 
    SET_BIT(PORTC, 7); 
    // draws my name and id when i'm in the intro screen 
    draw_string(6, 15, "Daniel Pretlove", FG_COLOUR);
    draw_string(20, 24, "n10193308", FG_COLOUR);
    show_screen();
    
    if ( usb_serial_available() ) 
	{
		int c = usb_serial_getchar();
		if(c == 's') game_mode();
	}

	// when PINF, 6 is pressed then it will bring me to game_mode screen 
    while(!BIT_IS_SET(PINF, 6));
  
    game_mode();
}

// draws an int on the screen 
void draw_int(uint8_t x, uint8_t y, int value, colour_t colour)
{
	snprintf(buffer, sizeof(buffer), "%d", value);
	draw_string(x, y, buffer, colour);
}

void game_pause(void)
{
	// when paused is true and if center joystick is pressed it will print the current score, lives, food, zombies and how long you've played the game so far 
	if(!paused)
	{
		if(BIT_IS_SET(PINB, 0))
		{
			paused = true;
			clear_screen();
			draw_string(5, 0, "Score:", FG_COLOUR);
			draw_int(40, 0, score, FG_COLOUR);
			draw_string(5, 10, "Lives:", FG_COLOUR);
			draw_int(40, 10, lives, FG_COLOUR);
			draw_string(5, 20, "Food:", FG_COLOUR);
			draw_int(40, 20, food_count, FG_COLOUR);
			draw_string(5, 30, "Zombies:", FG_COLOUR);
			draw_int(50, 30, zombie_count, FG_COLOUR);
			sprintf(buffer, "Time: %02d:%02d", minutes, seconds);
			draw_string(5, 40, buffer, FG_COLOUR);
			if(is_pressed == 1)
			{
				draw_string( 52, 20, "closed", FG_COLOUR);
			}

			else 
			{
				draw_string( 52, 20, "opened", FG_COLOUR);
			}
			show_screen();
		}

		if(usb_serial_available())
		{
			int c = usb_serial_getchar();
			if(c == 'p')
			{
				paused = true;
				clear_screen();
				draw_string(5, 0, "Score:", FG_COLOUR);
				draw_int(40, 0, score, FG_COLOUR);
				draw_string(5, 10, "Lives:", FG_COLOUR);
				draw_int(40, 10, lives, FG_COLOUR);
				draw_string(5, 20, "Food:", FG_COLOUR);
				draw_int(40, 20, food_count, FG_COLOUR);
				draw_string(5, 30, "Zombies:", FG_COLOUR);
				draw_int(50, 30, zombie_count, FG_COLOUR);
				sprintf(buffer, "Time: %02d:%02d", minutes, seconds);
				draw_string(5, 40, buffer, FG_COLOUR);
				show_screen();
			}
		}
		
	}
	// when paused is false you will continue playing the game 
	else if(paused)
	{
		if(BIT_IS_SET(PINB, 0))
		{
			paused = false;
			clear_screen();
  			if(is_pressed == 1)
			{
				draw_string( 50, 15, "closed", FG_COLOUR);
			}

			else 
			{
				draw_string( 50, 15, "opened", FG_COLOUR);
			}
			show_screen();
		}
	}
}

// draws all of the sprites on the screen 
void draw_all(void)
{
	clear_screen(); 
	sprite_draw(&player);
	sprite_draw(&treasure);
	sprite_draw(&starting_block);
	sprite_draw(&food);
	for(int i = 0; i < num_zombies; i++)
	{
		sprite_draw(&zombie[i]);
	}
	
	for(int i = 0; i < num_platforms; i++)
	{
		sprite_draw(&platforms[i]);
	}

	status();
	show_screen();
}

// get the current sprites on the screen 
int get_current_platform(sprite_id s)
{
	int sl = (int)round(s->x); 
	int sr = sl + s->width - 1; 
	int sy = (int)round(s->y);

	for(int plat = 0; plat < num_platforms; plat++)
	{
		sprite_id p = &platforms[plat]; 
		int pl = (int)round(p->x);
		int pr = pl + p->width - 1; 
		int py = (int)round(p->y);

		if(sr >= sl && sl <= pr && sy == py - s->height)
		{
			return plat;
		}
	}

	return -1; 
}

void end_game()
{
	// game_over is true it will print your total score and game play 
	if(game_over)
	{
		game_over = true; 
		clear_screen();
		paused = true;
		draw_string(0, 5, "Total Score:", FG_COLOUR);
		draw_int(60, 5, score, FG_COLOUR);
		sprintf(buffer, "Game Play:%02d:%02d", minutes, seconds);
		draw_string(0, 15, buffer, FG_COLOUR);
		draw_string(0, 28, "End:sw3 reset", FG_COLOUR);
		draw_string(0, 40, "sw2:id display", FG_COLOUR);
		// if sw3 is pressed, your game will restart 
		if(BIT_IS_SET(PINF, 5))
		{
			clear_screen();
			game_over = false; 
			paused = false; 
			game_mode();
			draw_all();
			lives = 10; 
			score = 0; 
			minutes = 0;
			seconds = 0; 
			counter_overflow = 0;
			usb_serial_send("Player respawns");
			usb_serial_send("\r\n"); 
			usb_serial_send("Player_x: ");
			usb_serial_send_int((int)player.x);
			usb_serial_send("\r\n");
			usb_serial_send("Player_y: ");
			usb_serial_send_int((int)player.y);
			if(is_pressed == 1)
			{
				draw_string( 50, 15, "closed", FG_COLOUR);
			}

			else 
			{
				draw_string( 50, 15, "opened", FG_COLOUR);
			}
		}

		if(usb_serial_available())
		{
			int c = usb_serial_getchar();
			if(c == 'r')
			{
				clear_screen();
				game_over = false; 
				paused = false; 
				game_mode();
				draw_all();
				lives = 10; 
				score = 0; 
				minutes = 0;
				seconds = 0; 
				counter_overflow = 0; 	
			}

			if(c == 'q')
			{
				clear_screen();
				intro();
			}
		}
		// when sw2 is pressed it will print the intro screen 
		if(BIT_IS_SET(PINF, 6))
		{
			clear_screen();
			game_over = false; 
		    draw_string(6, 15, "Daniel Pretlove", FG_COLOUR);
		    draw_string(20, 24, "n10193308", FG_COLOUR);
		}

		show_screen();
	}	
}

// setting up the sprite collision where one sprites collides with the other sprite 
bool sprite_collision (Sprite* s1, Sprite* s2)
{
    int l1 = round(s1->x);
  	int l2 = round(s2->x);
 	int t1 = round(s1->y);
 	int t2 = round(s2->y);
  	int r1 = l1 + s1->width - 4;
  	int r2 = l2 + s2->width - 4;
  	int b1 = t1 + s1->height + 0.5;
 	int b2 = t2 + s2->height - 1;
	
	if(l1 > r2) return false;
  	if(l2 > r1) return false;
  	if(t1 > b2) return false;
  	if(t2 > b1) return false;
    return true;
}

// moves the player when usb_serial 'a' and 'd' is pressed
void player_movement(void)
{
	
	if ( usb_serial_available() ) 
	{
		int c = usb_serial_getchar();
		if ( c == 'a' ) player.x -= 0.03;
		if ( c == 'd' ) player.x += 0.03;
	}

	if(player_dx || player_dy)
	{
		player.x += player_dx;
		player.y += player_dy;
	}
}

// setting up the usb serial so you can press button in the keyboard to move the player, pause the game, reset the game, quit the game etc. 
void setup_usb_serial(void) 
{
	// Set up LCD and display message
	lcd_init(LCD_LOW_CONTRAST);
	draw_string(10, 10, "Connect USB...", FG_COLOUR);
	show_screen();

	usb_init();

	while ( !usb_configured() ) 
	{

	}

	clear_screen();
	draw_string(10, 2, "USB connected", FG_COLOUR);
	show_screen();

	usb_serial_send("Please press W A S D to control the sprite!\r\n");
}

// will send a message within the putty
void usb_serial_send(char * message) 
{
	usb_serial_write((uint8_t *) message, strlen(message));
}

// will send the int value within the putty
void usb_serial_send_int(int value) 
{
	static char buffer[8];
	snprintf(buffer, sizeof(buffer), "%d", value);
	usb_serial_send( buffer );
}
// collision function 
void collision(void)
{
	// when falling is true then it will drop the player down and the zombie down 
	if(falling == true)
	{
		player_dy += 0.3;
		for(int j = 0; j < num_zombies; j++)
		{
			zombie[j].dy += 0.3;  
		}	
	}
	// when falling falling it will stop the player, food and zombie from falling 
	else if(falling == false)
	{
		player_dy = 0; 
		food.dy = 0;
		for(int j = 0; j < num_zombies; j++) 
		{
			zombie[j].dy -= 0.3;
		} 
	}
	int pl = (int)round(player.x);
	int pr = pl + player.width - 1; 
	int pt = (int)round(player.y);
	int pb = pt + player.height - 1; 
	int py = round(player.y);
	// when player collides with the LCD_X edges he will die and spawn back at the starting point 
	if(pl < 0 || pr >= LCD_X)
	{
		player_dy = 1;
		player_dx = 1; 
		draw_all();
		usb_serial_send("Player dies - "); 
		usb_serial_send("\r\n");
		usb_serial_send("How player dies: ");
		usb_serial_send("\r\n");
		usb_serial_send("Player dies from touching the LCD_X of the screen");
		usb_serial_send("\r\n");
		usb_serial_send("Lives after death: ");
		usb_serial_send_int((int)lives);
		usb_serial_send("\r\n");
		usb_serial_send("Score: ");
		usb_serial_send_int((int)score);
		usb_serial_send("\r\n");
		usb_serial_send("Game Time");
		usb_serial_send_int((int)minutes);
		usb_serial_send_int((int)seconds);
		lives--; 
	}
	// when the player collides with the LCD_Y edges he will die and spawn back at the starting point 
	if(pt < 8 || pb >= LCD_Y)
	{
		player_dy = 1; 
		player_dx = 1; 
		draw_all();
		usb_serial_send("Player dies - "); 
		usb_serial_send("\r\n");;
		usb_serial_send("How player dies: ");
		usb_serial_send("\r\n");
		usb_serial_send("Player dies from touching the LCD_Y of the screen");
		usb_serial_send("\r\n");
		usb_serial_send("Lives after death: ");
		usb_serial_send_int((int)lives);
		usb_serial_send("\r\n");
		usb_serial_send("Score: ");
		usb_serial_send_int((int)score);
		usb_serial_send("\r\n");
		usb_serial_send("Game Time");
		usb_serial_send_int((int)minutes);
		usb_serial_send_int((int)seconds);
		lives--; 
	}

	for(int i = 0; i < num_platforms; i++)
	{
		for(int j = 0; j < num_zombies; j++)
		{
			// if the zombies are not collided with the platforms they will fall 
			if(!sprite_collision(&platforms[i], &zombie[j]))
			{
				zombie[j].y += 0.03;
				SET_BIT(DDRB, 2);
				SET_BIT(DDRB, 3);
				SET_BIT(PORTB, 2);
				SET_BIT(PORTB, 3);
				CLEAR_BIT(PORTB, 2);
				CLEAR_BIT(PORTB, 3);
			}
			// if the zombies collides with the platforms they will stop falling and continous to move right and left 
			else if(sprite_collision(&platforms[i], &zombie[j]))
			{
				zombie[j].y -= 0.8;
				zombie[j].x += 0.4;
				int zl = (int)round(zombie[j].x); 
				int zr = zl + zombie[j].width - 1;

				if(zr >= LCD_X)
				{
					zombie[j].x -= 0.01;
				}
				
			}
		}
		// if the player isn't collided with the platforms or the starting point he will fall 
		if(!sprite_collision(&player, &platforms[i]))
		{
			falling = true; 

			// if left joystick is pressed playing will move left 
			if(BIT_IS_SET(PINB, 1))
			{
				player_dx -= 0.03;
				if(is_pressed == 1)
				{
					draw_string( 50, 15, "closed", FG_COLOUR);
				}

				else 
				{
					draw_string( 50, 15, "opened", FG_COLOUR);
				}
			}
			// if right joystick is pressed the player will move right
			if(BIT_IS_SET(PIND, 0))
			{
				player_dx += 0.03;
				if(is_pressed == 1)
				{
					draw_string( 50, 15, "closed", FG_COLOUR);
				}

				else 
				{
					draw_string( 50, 15, "opened", FG_COLOUR);
				}
			}
			if(!sprite_collision(&player, &starting_block))
			{
				falling = true;  
				if(BIT_IS_SET(PINB, 1))
				{
					player_dx -= 0.03;
				}

				if(BIT_IS_SET(PIND, 0))
				{
					player_dx += 0.03;
				}
			}

			// if the player collides with the starting point he will stop falling, and he will be able to press the down joystick to take his food out 
			else 
			{
				falling = false;
				if(BIT_IS_SET(PINB, 7))
				{
					sprite_init(&food, player.x + 2, player.y + 4, 3, 3, food_bitmap);
					food_count = food_count - 1;
					if(food_count < 0)
					{
						food_count = 0;
					}

					else
					{
						food.is_visible = false; 
						food_count = 5; 
					}
				}

				if(usb_serial_available())
				{
					int e = usb_serial_getchar();

					if(e == 's')
					{
						sprite_init(&food, player.x, player.y, 3, 3, food_bitmap);
						food_count = food_count - 1;
						if(food_count < 0)
						{
							food_count = 0;
						}
					}
				}
			}
		}
		// the player stops moving and he will be able to take his food out 
		else 
		{
			player_dy -= 0.3;
			if(BIT_IS_SET(PINB, 7))
			{
				sprite_init(&food, player.x, player.y, 3, 3, food_bitmap);
				food_count = food_count - 1;
				if(food_count < 0)
				{
					food_count = 0;
				}
			}

			if(usb_serial_available())
			{
				int d = usb_serial_getchar();

				if(d == 's')
				{
					sprite_init(&food, player.x, player.y, 3, 3, food_bitmap);
					food_count = food_count - 1;
					if(food_count < 0)
					{
						food_count = 0;
					}

					else 
					{
						food.is_visible = false; 

					}
				}
			}
			if(BIT_IS_SET(PIND, 1) && py > 5)
			{
				player_dy -= 3; 
				if(is_pressed == 1)
				{
					draw_string( 50, 15, "closed", FG_COLOUR);
				}

				else 
				{
					draw_string( 50, 15, "opened", FG_COLOUR);
				}
			}
			
			if(usb_serial_available())
			{
				int a = usb_serial_getchar();
				if(a == 'w') 
				{
					player.y -= 3; 
				}
			}
		}
		// if the food doesn't collide with the platforms, the food will drop down 
		if(!sprite_collision(&food, &platforms[i]))
		{
			food.y += 0.3; 
		}
		// if the food collides with the platform, the food will stop moving 
		else if(sprite_collision(&food, &platforms[i]))
		{
			food.y -= 3.9; 
			if(BIT_IS_SET(PIND, 1) && py > 5)
			{ 
				player_dy -= 3; 
				if(is_pressed == 1)
				{
					draw_string( 50, 15, "closed", FG_COLOUR);
				}

				else 
				{
					draw_string( 50, 15, "opened", FG_COLOUR);
				}
			}
			
			if(usb_serial_available())
			{
				int b = usb_serial_available();
				if(b == 'w')
				{
					player_dy -= 3; 
				} 
			}
		}

	}

	// when the player collides with treasure, he will gain 2 lives, spawn back at the starting point and the treasure will disappear 
	if(sprite_collision(&treasure, &player))
	{
		falling = false; 
		sprite_init(&treasure, LCD_X, LCD_Y, 8, 3, treasure_bitmap);
		treasure.is_visible = false; 
		player_dy = 1;
		player_dx = 1; 
		draw_all();
		lives += 2; 
		usb_serial_send("Player collides with the treasure: ");
		usb_serial_send("\r\n");
		usb_serial_send("Player's current score: ");
		usb_serial_send("\n");usb_serial_send("\r\n");
		usb_serial_send_int((int)score);
		usb_serial_send("\r\n");
		usb_serial_send("Player's current lives: "); 
		usb_serial_send("\r\n");
		usb_serial_send_int((int)lives);
		usb_serial_send("\r\n");
		usb_serial_send_int((int)minutes);
		usb_serial_send_int((int)seconds);
		usb_serial_send("\r\n");
		usb_serial_send("Player's x position after returning to the standing block");
		usb_serial_send("\r\n");
		usb_serial_send_int((int)player.x);
		usb_serial_send("Player's y position after returning to the standing block");
		usb_serial_send("\r\n");
		usb_serial_send_int((int)player.y);
	}
	// if the food, collides with any of the zombie, the zombie will disappear
	for(int i = 0; i < num_zombies; i++)
	{
		if(sprite_collision(&food, &zombie[0]))
		{
			zombie[0].is_visible = false; 
			sprite_init(&zombie[0], LCD_X, LCD_Y, 8, 6, zombie_bitmap);		
			food.is_visible = false; 
			sprite_init(&food, LCD_X, LCD_Y, 8, 6, food_bitmap);
			food_count = food_count + 1; 
			score += 10; 
			usb_serial_send("Zombie [0] collides with food: ");
			usb_serial_send("\r\n");
			usb_serial_send("Number of zombies on screen after collision: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)zombie_count);
			usb_serial_send("Number of food in inventory after collision: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)food_count);
			usb_serial_send("Game time: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)minutes);
			usb_serial_send_int((int)seconds);
		}

		if(sprite_collision(&food, &zombie[1]))
		{
			zombie[1].is_visible = false; 
			score += 10;
			sprite_init(&zombie[1], LCD_X, LCD_Y, 8, 6, zombie_bitmap);
			food.is_visible = false; 
			sprite_init(&food, LCD_X, LCD_Y, 8, 6, food_bitmap);
			food_count = food_count + 1; 
			usb_serial_send("Zombie [1] collides with food: ");
			usb_serial_send("\r\n");
			usb_serial_send("Number of zombies on screen after collision: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)zombie_count);
			usb_serial_send("Number of food in inventory after collision: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)food_count);
			usb_serial_send("Game time: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)minutes);
			usb_serial_send_int((int)seconds);
		}

		if(sprite_collision(&food, &zombie[2]))
		{
			zombie[2].is_visible = false; 
			score += 10;
			sprite_init(&zombie[2], LCD_X, LCD_Y, 8, 6, zombie_bitmap);
			food.is_visible = false; 
			sprite_init(&food, LCD_X, LCD_Y, 8, 6, food_bitmap);
			food_count = food_count + 1; 
			usb_serial_send("Zombie [2] collides with food: ");
			usb_serial_send("\r\n");
			usb_serial_send("Number of zombies on screen after collision: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)zombie_count);
			usb_serial_send("Number of food in inventory after collision: ");
			usb_serial_send("\r\n");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)food_count);
			usb_serial_send("Game time: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)minutes);
			usb_serial_send_int((int)seconds);
		}

		if(sprite_collision(&food, &zombie[3]))
		{
			zombie[3].is_visible = false; 
			score += 10;
			sprite_init(&zombie[3], LCD_X, LCD_Y, 8, 6, zombie_bitmap);
			food.is_visible = false; 
			sprite_init(&food, LCD_X, LCD_Y, 8, 6, food_bitmap);
			food_count = food_count + 1; 
			usb_serial_send("Zombie [3] collides with food: ");
			usb_serial_send("\r\n");
			usb_serial_send("Number of zombies on screen after collision: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)zombie_count);
			usb_serial_send("Number of food in inventory after collision: ");
			usb_serial_send("\r\n");;
			usb_serial_send_int((int)food_count);
			usb_serial_send("Game time: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)minutes);
			usb_serial_send_int((int)seconds);
		}

		if(sprite_collision(&food, &zombie[4]))
		{
			zombie[4].is_visible = false; 
			score += 10;
			sprite_init(&zombie[4], LCD_X, LCD_Y, 8, 6, zombie_bitmap);
			food.is_visible = false; 
			sprite_init(&food, LCD_X, LCD_Y, 8, 6, food_bitmap);
			food_count = food_count + 1; 
			usb_serial_send("Zombie [4] collides with food: ");
			usb_serial_send("\r\n");
			usb_serial_send("Number of zombies on screen after collision: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)zombie_count);
			usb_serial_send("Number of food in inventory after collision: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)food_count);
			usb_serial_send("Game time: ");
			usb_serial_send("\r\n");
			usb_serial_send_int((int)minutes);
			usb_serial_send_int((int)seconds);
		}
		// if the player collides with any of the zombies the playeer will loose a life 
		if(sprite_collision(&player, &zombie[i]))
		{
			player_dy = 1; 
			player_dx = 1; 
			draw_all();
			usb_serial_send("Player dies - "); 
			usb_serial_send("\r\n");
			usb_serial_send("How player dies: ");
			usb_serial_send("\r\n");
			usb_serial_send("Player dies from the zombie");
			usb_serial_send("\r\n");
			usb_serial_send("Lives after death: ");
			usb_serial_send_int((int)lives);
			usb_serial_send("\r\n");
			usb_serial_send("Score: ");
			usb_serial_send_int((int)score);
			usb_serial_send("\r\n");
			usb_serial_send("Game Time");
			usb_serial_send_int((int)minutes);
			usb_serial_send_int((int)seconds);
			lives--; 
		}

		int zl0 = (int)round(zombie[0].x);
		int zr0 = zl0 + zombie[0].width - 1; 
		int zt0 = (int)round(zombie[0].y);
		int zb0 = zt0 + zombie[0].height - 1; 
		
		int zl1 = (int)round(zombie[1].x);
		int zr1 = zl1 + zombie[1].width - 1; 
		int zt1 = (int)round(zombie[1].y);
		int zb1 = zt1 + zombie[1].height - 1; 
		
		int zl2 = (int)round(zombie[2].x);
		int zr2 = zl2 + zombie[2].width - 1; 
		int zt2 = (int)round(zombie[2].y);
		int zb2 = zt2 + zombie[2].height - 1; 
		
		int zl3 = (int)round(zombie[3].x);
		int zr3 = zl3 + zombie[3].width - 1; 
		int zt3 = (int)round(zombie[3].y);
		int zb3 = zt3 + zombie[3].height - 1; 
		
		int zl4 = (int)round(zombie[4].x);
		int zr4 = zl4 + zombie[4].width - 1; 
		int zt4 = (int)round(zombie[4].y);
		int zb4 = zt4 + zombie[4].height - 1; 

		// when the zombies collides with the LCD_Y edges they will respawn 
		if(zr0 >= LCD_X || zb0 >= LCD_Y)
		{ 
			sprite_init(&zombie[0], 20, 5, 7, 4, zombie_bitmap);
		}

		else if(zr1 >= LCD_X || zb1 >= LCD_Y)
		{
			sprite_init(&zombie[1], 30, 5, 7, 4, zombie_bitmap);
		}

		else if(zr2 >= LCD_X || zb2 >= LCD_Y)
		{
			sprite_init(&zombie[2], 40, 5, 7, 4, zombie_bitmap);
		}

		else if(zr3 >= LCD_X || zb3 >= LCD_Y)
		{
			sprite_init(&zombie[3], 50, 5, 7, 4, zombie_bitmap);
		}

		else if(zr4 >= LCD_X || zb4 >= LCD_Y)
		{
			sprite_init(&zombie[4], 60, 5, 7, 4, zombie_bitmap);
		}

	}
	// when the zombie_count is less then 0 the zombie_count will stay as zero rather than a negative number 
	if(zombie_count <= 0)
	{
		zombie_count = 0; 
	}
	// when the score is less then zero the score will stay as zero rather than a negative number 
	if(score <= 0)
	{
		score = 0;
	}

	//when lives is less then or equal to 0, the game over will be true and then the game will bring you to the end_game screen 
	while(lives <= 0)
	{
		game_over = true; 
		end_game();
	}

	show_screen();
}

// sets up the intro, treasure, platforms, zombies and the timer by calling these functions 
void setup(void)
{
	intro();
	setup_treasure();
	setup_platforms();
	setup_zombie();
  	setup_timer();
}


void process(void)
{
	// processes the game_pause and the timer 
	game_pause();
	process_timer();

	// when paused is false
	if(paused == false)
	{
		// it will draw the starting_block and the player 
		sprite_init(&starting_block, 5, 18, 10, 2, platforms_bitmap);
		sprite_init(&player, 5, 15, 7, 3, player_bitmap);
		// it will also call the player_movement, draw_all, and the collision functions 
		player_movement();
		draw_all();
		collision();
		// plat is equal to the get_current_platform function while using the player as the parameter 
		int plat = get_current_platform(&player); 
		// if the platforms is more than or equal to zero 
		if(plat >= 0)
		{
			// if the plat is safe 
			if(is_safe[plat])
			{
				// and if falling is false 
				if(falling)
				{
					// the player will gain a point 
					score = score + 1;
				}
			}
			// if the platform is not safe
			else if(!is_safe[plat])
			{ 
				// the player will loose a life and going back to the starting point 
				player_dx = 1; 
				player_dy = 1; 
				usb_serial_send("Player dies - "); 
				usb_serial_send("\r\n");
				usb_serial_send("How player dies: ");
				usb_serial_send("\r\n");
				usb_serial_send("Player dies from hitting a bad block: ");
				usb_serial_send("\r\n");
				usb_serial_send("Lives after death: ");
				usb_serial_send_int((int)lives);
				usb_serial_send("\r\n");
				usb_serial_send("Score: ");
				usb_serial_send_int((int)score);
				usb_serial_send("\r\n");
				usb_serial_send("Game Time");
				usb_serial_send_int((int)minutes);
				usb_serial_send_int((int)seconds);
			}
		}

		static bool treasure_paused = false; 
		// if sw3 is pressed the treasure will pause 
		if(BIT_IS_SET(PINF, 5))
		{
			treasure_paused = !treasure_paused; 
			if(is_pressed == 1)
			{
				draw_string( 50, 15, "closed", FG_COLOUR);
			}

			else 
			{
				draw_string( 50, 15, "opened", FG_COLOUR);
			}
		}

		if(usb_serial_available())
		{
			int c = usb_serial_getchar();
			if(c == 't') treasure_paused = !treasure_paused; 
		}

		if(!treasure_paused)
		{
			treasure.x += treasure.dx; 
			int tl = (int)round(treasure.x); 
			int tr = tl + treasure.width - 1; 

			if(tl < 0 || tr >= LCD_X)
			{
				treasure.x -= treasure.dx;
				treasure.dx = -treasure.dx;
			}
		}
		// if player hits bad platforms he dies and spawn back in starting point 
		if(sprite_collision(&player, &platforms[1]))
		{
			lives--; 
			player_dx = 1; 
			player_dy = 1;
		}

		if(sprite_collision(&player, &platforms[2]))
		{
			lives--; 
			player_dx = 1; 
			player_dy = 1;
		}

		if(sprite_collision(&player, &platforms[10]))
		{
			lives--; 
			player_dx = 1; 
			player_dy = 1;
		}
		// platform movement 
		platforms[0].x += 0.1; 
		platforms[1].x += 0.1; 
		platforms[2].x += 0.1; 
		platforms[3].x += 0.1; 
		platforms[4].x += 0.1; 
		platforms[6].x += 0.1; 
		platforms[7].x -= 0.1; 
		platforms[8].x -= 0.1; 
		platforms[9].x -= 0.1; 
		platforms[10].x -= 0.1; 
		platforms[11].x -= 0.1; 
		int pl0 = (int)round(platforms[0].x); 
		int pr0 = pl0 + platforms[0].width - 1;

		int pl1 = (int)round(platforms[1].x); 
		int pr1 = pl1 + platforms[1].width - 1; 

		int pl2 = (int)round(platforms[2].x); 
		int pr2 = pl2 + platforms[2].width - 1; 

		int pl3 = (int)round(platforms[3].x); 
		int pr3 = pl3 + platforms[3].width - 1; 

		int pl4 = (int)round(platforms[4].x); 
		int pr4 = pl4 + platforms[4].width - 1; 

		int pl6 = (int)round(platforms[6].x); 
	

		int pl7 = (int)round(platforms[7].x); 
		

		int pl8 = (int)round(platforms[8].x); 
	

		int pl9 = (int)round(platforms[9].x); 
	

		int pl10 = (int)round(platforms[10].x); 
	

		int pl11 = (int)round(platforms[11].x); 
	

		// when the platforms in the top hit the end of the screen they will be sent back to the start 
		// when the bottom row hit 0 x then the platform will be sent back to the end of the screen
		if(pr0 >= LCD_X)
		{
			platforms[0].x = 0;
		}

		if(pr1 >= LCD_X)
		{
			platforms[1].x = 0;
		}
		if(pr2 >= LCD_X)
		{
			platforms[2].x = 0;
		}
		if(pr3 >= LCD_X)
		{
			platforms[3].x = 0;
		}
		if(pr4 >= LCD_X)
		{
			platforms[4].x = 0;
		}

		if(pl6 < 0)
		{
			platforms[6].x = 74;
		}

		if(pl7 < 0)
		{
			platforms[7].x = 74;
		}

		if(pl8 < 0)
		{
			platforms[8].x = 74;
		}

		if(pl9 < 0)
		{
			platforms[9].x = 74;
		}

		if(pl10 < 0)
		{
			platforms[10].x = 74;
		}

		if(pl11 < 0)
		{
			platforms[11].x = 74;
		}

		// when the game_over is true then it will send you to the end_game screen 
		if(game_over == true)
		{
			clear_screen();
			end_game();
			show_screen();
		}
		show_screen();
	}
}

int main(void)
{
	setup();
	for ( ;; )
  	{
   	    process();
		_delay_ms(10);
	}

	return 0;
}