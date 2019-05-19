#include <stdio.h>
#include <stdlib.h>
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

#define BUTTONS_XNUM		6
#define BUTTONS_YNUM		4
#define LABEL_MAXCHARS		5
#define DISPLAY_MAXCHARS	16
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
	bool positive;
} display = {.string = "1234", .positive = true};

TTF_Font *font = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *rlabels[BUTTONS_YNUM][BUTTONS_XNUM];
SDL_Surface *minus_sign = NULL;

void draw_ui(void);
void append_character(char *string, int c);

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
								if (0 == indpos.y)
								{
								}
								else if (1 == indpos.y)
								{
								}
								else if (2 == indpos.y)
								{
								}
								else if (3 == indpos.y)
								{
								}
							}
							else if (1 == indpos.x)
							{
								if (0 == indpos.y)
								{
									append_character(display.string, '7');
								}
								else if (1 == indpos.y)
								{
									append_character(display.string, '4');
								}
								else if (2 == indpos.y)
								{
									append_character(display.string, '1');
								}
								else if (3 == indpos.y)
								{
									append_character(display.string, '0');
								}
							}
							else if (2 == indpos.x)
							{
								if (0 == indpos.y)
								{
									append_character(display.string, '8');
								}
								else if (1 == indpos.y)
								{
									append_character(display.string, '5');
								}
								else if (2 == indpos.y)
								{
									append_character(display.string, '2');
								}
								else if (3 == indpos.y)
								{
									append_character(display.string, '.');
								}
							}
							else if (3 == indpos.x)
							{
								if (0 == indpos.y)
								{
									append_character(display.string, '9');
								}
								else if (1 == indpos.y)
								{
									append_character(display.string, '6');
								}
								else if (2 == indpos.y)
								{
									append_character(display.string, '3');
								}
								else if (3 == indpos.y)
								{
									display.positive = !display.positive;
								}
							}
							else if (4 == indpos.x)
							{
								if (0 == indpos.y)
								{
								}
								else if (1 == indpos.y)
								{
								}
								else if (2 == indpos.y)
								{
								}
								else if (3 == indpos.y)
								{
								}
							}
							else if (5 == indpos.x)
							{
								if (0 == indpos.y)
								{
								}
								else if (1 == indpos.y)
								{
								}
								else if (2 == indpos.y)
								{
									display.string[0] = '0';
									display.string[1] = '\0';
								}
								else if (3 == indpos.y)
								{
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
	
	if (!display.positive)
	{
		rect.x = 16 + DISPLAY_BORDER + 8;
		SDL_BlitSurface(minus_sign, NULL, screen, &rect);
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

void append_character(char *string, int c)
{
	bool dot = false;
	if (strlen(string) >= DISPLAY_MAXCHARS)
		return;
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
