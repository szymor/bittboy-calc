#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <linux/fb.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#define XSTR(S)				#S
#define STR(S)				XSTR(S)

#define BUTTONS_XNUM		6
#define BUTTONS_YNUM		4
#define LABEL_MAXCHARS		5
#define DISPLAY_MAXCHARS	32
#define DISPLAY_PRECISION	22
#define DISPLAY_BORDER		5

const char labels[BUTTONS_YNUM][BUTTONS_XNUM][LABEL_MAXCHARS] =
	{
		{"MC", "7", "8", "9",   "/", "sqrt"},
		{"MR", "4", "5", "6",   "*", "%"},
		{"M+", "1", "2", "3",   "-", "C"},
		{"M-", "0", ".", "+/-", "+", "="}
	};

struct
{
	int x;
	int y;
} indpos = {.x = BUTTONS_XNUM - 1, .y = BUTTONS_YNUM - 1};

struct
{
	char string[DISPLAY_MAXCHARS + 1];
	bool negative;
} display = {.string = "0", .negative = false};

enum
{
	OP_NULL,
	OP_PLUS,
	OP_MINUS,
	OP_MULTIPLY,
	OP_DIVIDE
} operation = OP_NULL;

double left_operand = 0.0;
double right_operand = 0.0;
double memory_result = 0.0;
bool new_entry = false;

TTF_Font *font = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *rlabels[BUTTONS_YNUM][BUTTONS_XNUM];
SDL_Surface *minus_sign = NULL;
SDL_Surface *m_symbol = NULL;

void draw_ui(void);
void append_character(int c);
double signum(bool neg);
void show_number(double n);
double display_operand(void);
void do_operation(void);
void clear_display(void);

int main(int argc, char* args[])
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("%s, failed to SDL_Init\n", __func__);
		return -1;
	}
	TTF_Init();
	
	font = TTF_OpenFont("LiberationMono-Regular.ttf", 12);
	if (NULL == font)
	{
		printf("%s, failed to TTF_OpenFont: %s\n", __func__, TTF_GetError());
		return -1;
	}

	screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if (screen == NULL)
	{
		printf("%s, failed to SDL_SetVideoMode\n", __func__);
		return -1;
	}
	
	SDL_Color fgc = {0, 255, 0};
	for (int y = 0; y < BUTTONS_YNUM; ++y)
		for (int x = 0; x < BUTTONS_XNUM; ++x)
		{
			rlabels[y][x] = TTF_RenderText_Blended(font, labels[y][x], fgc);
		}
		
	fgc.r = fgc.g = fgc.b = 0;
	minus_sign = TTF_RenderText_Blended(font, "-", fgc);
	m_symbol = TTF_RenderText_Blended(font, "M", fgc);
	
	SDL_ShowCursor(0);
	draw_ui();

	SDL_Event event;
	bool quit = false;
	while (!quit)
	{
		if (SDL_WaitEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_UP:
							--indpos.y;
							if (indpos.y < 0)
								indpos.y = BUTTONS_YNUM - 1;
							break;
						case SDLK_DOWN:
							++indpos.y;
							indpos.y %= BUTTONS_YNUM;
							break;
						case SDLK_LEFT:
							--indpos.x;
							if (indpos.x < 0)
								indpos.x = BUTTONS_XNUM - 1;
							break;
						case SDLK_RIGHT:
							++indpos.x;
							indpos.x %= BUTTONS_XNUM;
							break;
						case SDLK_SPACE:	// B
							break;
						case SDLK_LCTRL:	// A
							if (0 == indpos.x)
							{
								if (0 == indpos.y)		// MC
								{
									memory_result = 0.0;
								}
								else if (1 == indpos.y)		// MR
								{
									show_number(memory_result);
									new_entry = false;
								}
								else if (2 == indpos.y)		// M+
								{
									memory_result += display_operand();
								}
								else if (3 == indpos.y)		// M-
								{
									memory_result -= display_operand();
								}
							}
							else if (1 == indpos.x)
							{
								if (0 == indpos.y)		// 7
								{
									append_character('7');
								}
								else if (1 == indpos.y)		// 4
								{
									append_character('4');
								}
								else if (2 == indpos.y)		// 1
								{
									append_character('1');
								}
								else if (3 == indpos.y)		// 0
								{
									append_character('0');
								}
							}
							else if (2 == indpos.x)
							{
								if (0 == indpos.y)		// 8
								{
									append_character('8');
								}
								else if (1 == indpos.y)		// 5
								{
									append_character('5');
								}
								else if (2 == indpos.y)		// 2
								{
									append_character('2');
								}
								else if (3 == indpos.y)		// .
								{
									append_character('.');
								}
							}
							else if (3 == indpos.x)
							{
								if (0 == indpos.y)		// 9
								{
									append_character('9');
								}
								else if (1 == indpos.y)		// 6
								{
									append_character('6');
								}
								else if (2 == indpos.y)		// 3
								{
									append_character('3');
								}
								else if (3 == indpos.y)		// +/-
								{
									display.negative = !display.negative;
								}
							}
							else if (4 == indpos.x)
							{
								if (0 == indpos.y)		// /
								{
									if (!new_entry)
									{
										right_operand = display_operand();
										do_operation();
									}
									left_operand = display_operand();
									operation = OP_DIVIDE;
									new_entry = true;
								}
								else if (1 == indpos.y)		// *
								{
									if (!new_entry)
									{
										right_operand = display_operand();
										do_operation();
									}
									left_operand = display_operand();
									operation = OP_MULTIPLY;
									new_entry = true;
								}
								else if (2 == indpos.y)		// -
								{
									if (!new_entry)
									{
										right_operand = display_operand();
										do_operation();
									}
									left_operand = display_operand();
									operation = OP_MINUS;
									new_entry = true;
								}
								else if (3 == indpos.y)		// +
								{
									if (!new_entry)
									{
										right_operand = display_operand();
										do_operation();
									}
									left_operand = display_operand();
									operation = OP_PLUS;
									new_entry = true;
								}
							}
							else if (5 == indpos.x)
							{
								if (0 == indpos.y)		// sqrt
								{
									show_number(sqrt(display_operand()));
									new_entry = true;
								}
								else if (1 == indpos.y)		// %
								{
									if (!new_entry)
									{
										right_operand = display_operand();
										switch (operation)
										{
											case OP_NULL:
												break;
											case OP_PLUS:
											case OP_MINUS:
												right_operand *= left_operand / 100.;
												break;
											case OP_MULTIPLY:
											case OP_DIVIDE:
												right_operand /= 100.;
												break;
										}
									}
									do_operation();
									left_operand = display_operand();
									new_entry = true;
								}
								else if (2 == indpos.y)		// C
								{
									clear_display();
									operation = OP_NULL;
								}
								else if (3 == indpos.y)		// =
								{
									if (!new_entry)
										right_operand = display_operand();
									do_operation();
									left_operand = display_operand();
									new_entry = true;
								}
							}
							break;
						case SDLK_LSHIFT:	// TB
							break;
						case SDLK_LALT:		// TA
							break;
						case SDLK_ESCAPE:	// Select
						{
							SDL_Event ev;
							ev.type = SDL_QUIT;
							SDL_PushEvent(&ev);
						} break;
						case SDLK_RETURN:	// Start
							break;
						case SDLK_RCTRL:	// Reset
						{
							SDL_Event ev;
							ev.type = SDL_QUIT;
							SDL_PushEvent(&ev);
						} break;
					}
					draw_ui();
					break;
				case SDL_QUIT:
					quit = true;
					break;
			}
		}
	}
	
	SDL_FreeSurface(minus_sign);
	SDL_FreeSurface(m_symbol);
	for (int y = 0; y < BUTTONS_YNUM; ++y)
		for (int x = 0; x < BUTTONS_XNUM; ++x)
		{
			SDL_FreeSurface(rlabels[y][x]);
		}
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}

void draw_ui(void)
{
	SDL_Color fgc = {0, 0, 0};
	SDL_Surface *disp = TTF_RenderText_Blended(font, display.string, fgc);
	
	SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0x40, 0x40, 0x40));

	SDL_Rect rect;
	rect.x = 16;
	rect.y = 16;
	rect.w = 320 - 32;
	rect.h = 48;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
	
	rect.x = 16 + DISPLAY_BORDER;
	rect.y = 16 + DISPLAY_BORDER;
	rect.w = 320 - 32 - DISPLAY_BORDER * 2;
	rect.h = 48 - DISPLAY_BORDER * 2;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));
	
	rect.x += rect.w - disp->w - 8;
	rect.y += (rect.h - disp->h) / 2;
	SDL_BlitSurface(disp, NULL, screen, &rect);
	
	if (display.negative)
	{
		rect.x = 16 + DISPLAY_BORDER + 8;
		SDL_BlitSurface(minus_sign, NULL, screen, &rect);
	}
	
	if (memory_result != 0.0)
	{
		rect.x = 16 + DISPLAY_BORDER + 8;
		rect.y -= 3 * minus_sign->h / 4;
		SDL_BlitSurface(m_symbol, NULL, screen, &rect);
	}

	for (int x = 0; x < BUTTONS_XNUM; ++x)
		for (int y = 0; y < BUTTONS_YNUM; ++y)
		{
			rect.x = 16 + x * 51;
			rect.y = 16 + 48 + 12 + y * 40;
			rect.w = 32;
			rect.h = 32;
			Uint32 col;
			if (indpos.x == x && indpos.y == y)
				col = SDL_MapRGB(screen->format, 0x80, 0x00, 0x00);
			else
				col = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
			SDL_FillRect(screen, &rect, col);
			rect.x += (rect.w - rlabels[y][x]->w) / 2;
			rect.y += (rect.h - rlabels[y][x]->h) / 2;
			SDL_BlitSurface(rlabels[y][x], NULL, screen, &rect);
		}

	SDL_Flip(screen);
	SDL_FreeSurface(disp);
}

void append_character(int c)
{
	if (new_entry)
	{
		clear_display();
	}
	
	if (strlen(display.string) >= DISPLAY_MAXCHARS)
		return;
	
	bool dot = false;
	char *string = display.string;
	if ('0' != string[0])	// leading zero check
	{
		while (*string)
		{
			if ('.' == *string)
				dot = true;
			++string;
		}
	}
	if ('.' == c && dot)
		return;
	*string = c;
	*(string + 1) = '\0';
}

double signum(bool neg)
{
	return neg ? -1. : 1.;
}

void show_number(double n)
{
	sprintf(display.string, "%." STR(DISPLAY_PRECISION) "g", n);
	if ('-' == display.string[0])
	{
		display.string[0] = ' ';
		display.negative = true;
	}
	else
		display.negative = false;
}

double display_operand(void)
{
	return signum(display.negative) * atof(display.string);
}

void do_operation(void)
{
	switch (operation)
	{
		case OP_NULL:
			break;
		case OP_PLUS:
			show_number(left_operand + right_operand);
			break;
		case OP_MINUS:
			show_number(left_operand - right_operand);
			break;
		case OP_DIVIDE:
			show_number(left_operand / right_operand);
			break;
		case OP_MULTIPLY:
			show_number(left_operand * right_operand);
			break;
		default:
			strcpy(display.string, "Error.");
	}
}

void clear_display(void)
{
	display.string[0] = '0';
	display.string[1] = '\0';
	new_entry = false;
	display.negative = false;
}
