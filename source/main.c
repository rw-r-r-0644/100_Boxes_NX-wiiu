#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#if !defined(USE_SDL_INPUT)
#include <switch.h>
#else
typedef struct
{
	uint32_t px, py;
} touchPosition;
enum inputKeys
{
	KEY_A    = (1 << 0), KEY_B     = (1 << 1),
	KEY_UP   = (1 << 2), KEY_DOWN  = (1 << 3),
	KEY_LEFT = (1 << 4), KEY_RIGHT = (1 << 5),
	KEY_PLUS = (1 << 6), KEY_TOUCH = (1 << 7),
};
#endif

#if defined(__SWITCH__)
#define BASE_PATH		"romfs:/"
#define appRunning()	appletMainLoop()
#elif defined(__WIIU__)
#include <whb/proc.h>
#define BASE_PATH		"fs:/vol/external01/wiiu/apps/100_Boxes/"
#define	appRunning()	WHBProcIsRunning()
#else
#error Unknown platform
#endif

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define TILE_SIZEX 60
#define TILE_SIZEY 60
#define MAX_TILEX 10
#define MAX_TILEY 10


SDL_Window * 	window;
SDL_Renderer * 	renderer;
SDL_Surface *	surface;

touchPosition Stylus;
uint32_t kDown;

typedef struct 
{
	SDL_Texture * texture;
	SDL_Rect SrcR;
	SDL_Rect DestR;
} 
images;
images background, sprite[3];

uint16_t level_courant[MAX_TILEX*MAX_TILEY];
uint8_t colonnes, lignes;
uint8_t compteur;

uint8_t CASE_X, CASE_Y;
uint8_t TILE_X, TILE_Y;


#if defined(USE_SDL_INPUT)
void SDL_InitInput()
{
	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_JoystickOpen(i) == NULL)
		{
			printf("SDL_JoystickOpen: %s\n", SDL_GetError());
			SDL_Quit();
			return;
		}
	}
}
void SDL_ScanInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		kDown &= ~KEY_A;
		kDown &= ~KEY_B;
		kDown &= ~KEY_UP;
		kDown &= ~KEY_DOWN;
		kDown &= ~KEY_LEFT;
		kDown &= ~KEY_RIGHT;
		kDown &= ~KEY_PLUS;
		if (event.type == SDL_JOYBUTTONDOWN)
		{
			kDown |= ((event.jbutton.button == 0)? KEY_A : 0);
			kDown |= ((event.jbutton.button == 1)? KEY_B : 0);
			kDown |= ((event.jbutton.button == 13)? KEY_UP : 0);
			kDown |= ((event.jbutton.button == 15)? KEY_DOWN : 0);
			kDown |= ((event.jbutton.button == 12)? KEY_LEFT : 0);
			kDown |= ((event.jbutton.button == 14)? KEY_RIGHT : 0);
			kDown |= ((event.jbutton.button == 10)? KEY_PLUS : 0);
		}
		if (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERMOTION)
		{
			kDown |= KEY_TOUCH;
			Stylus.py = event.tfinger.y * 720;
			Stylus.px = event.tfinger.x * 1280;
		}
		if (event.type == SDL_FINGERUP)
		{
			kDown &= ~KEY_TOUCH;
		}
	}
}
#endif


//STYLUS
bool DowntouchInBox(touchPosition touch, int x1, int y1, int x2, int y2)
{
	int tx=touch.px;
	int ty=touch.py;

	if (kDown & KEY_TOUCH && tx > x1 && tx < x2 && ty > y1 && ty < y2)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int Srcx, int Srcy, int Destx, int Desty, int w, int h)
{
	SDL_Rect srce;
	srce.x = Srcx;
	srce.y = Srcy;
	srce.w = w;
	srce.h = h;
	
	SDL_Rect dest;
	dest.x = Destx;
	dest.y = Desty;
	dest.w = w;
	dest.h = h;

	SDL_RenderCopy(ren, tex, &srce, &dest);
}


void Affiche_trois_chiffres(uint16_t valeur, uint16_t posx, uint16_t posy)
{
	uint8_t unite = 0;
	uint8_t dizaine = 0;
	uint16_t centaine = 0;

	if (valeur < 10)
	{
		unite = valeur;
		dizaine = 0;
		centaine = 0;
	}
	else if (valeur < 100)
	{
		centaine = 0;
		dizaine = (valeur / 10);
		unite = valeur - (dizaine * 10);
	}
	else
	{
		centaine = (valeur / 100);
		dizaine = (valeur - (centaine * 100)) / 10;
		unite = valeur - (centaine * 100) - (dizaine * 10);
	}

	renderTexture(sprite[2].texture, renderer, centaine*39, 0, posx, posy, 39, 69);
	renderTexture(sprite[2].texture, renderer, dizaine*39, 0, posx+39, posy, 39, 69);
	renderTexture(sprite[2].texture, renderer, unite*39, 0, posx+78, posy, 39, 69);
}


void printGame()
{
	//CLEAR
	SDL_RenderClear(renderer);

	//BG
	SDL_RenderCopy(renderer, background.texture, NULL, NULL);

	//TILES
	for(lignes = 0; lignes < MAX_TILEX; lignes++)
	{
		for(colonnes = 0; colonnes < MAX_TILEY; colonnes++)
		{
			//Les tiles à afficher			
			renderTexture(sprite[1].texture, renderer, level_courant[lignes*MAX_TILEY + colonnes] * TILE_SIZEX, 0, 595+lignes*TILE_SIZEX, 98+colonnes*TILE_SIZEY, TILE_SIZEX, TILE_SIZEY);
		}
	}

	//CURSOR
	renderTexture(sprite[0].texture, renderer, 0, 0, 595-12+TILE_X*TILE_SIZEX, 98-12+TILE_Y*TILE_SIZEY, 84, 84);

	//TXT
	Affiche_trois_chiffres(compteur, 836, 13);

	//REFRESH
    	SDL_RenderPresent(renderer);
}

void debloqueChoix()
{
	//On efface les anciens choix
	for(lignes = 0; lignes < MAX_TILEX; lignes++)
		for(colonnes = 0; colonnes < MAX_TILEY; colonnes++)
			if (level_courant[lignes*MAX_TILEY + colonnes] == 2) level_courant[lignes*MAX_TILEY + colonnes] = 0;

	//Les nouveaux choix possible
	if ((TILE_X-3 >= 0) && (level_courant[(TILE_X-3)*MAX_TILEY + TILE_Y]!=1)) level_courant[(TILE_X-3)*MAX_TILEY + TILE_Y] = 2;
	if ((TILE_X+3 <= MAX_TILEX-1) && (level_courant[(TILE_X+3)*MAX_TILEY + TILE_Y]!=1)) level_courant[(TILE_X+3)*MAX_TILEY + TILE_Y] = 2;

	if (((TILE_X-2 >= 0) && (TILE_Y+2 <= MAX_TILEY-1)) && (level_courant[(TILE_X-2)*MAX_TILEY + TILE_Y+2]!=1)) level_courant[(TILE_X-2)*MAX_TILEY + TILE_Y+2] = 2;
	if (((TILE_X-2 >= 0) && (TILE_Y-2 >= 0)) && (level_courant[(TILE_X-2)*MAX_TILEY + TILE_Y-2]!=1)) level_courant[(TILE_X-2)*MAX_TILEY + TILE_Y-2] = 2;

	if (((TILE_X+2 <= MAX_TILEX-1) && (TILE_Y+2 <= MAX_TILEY-1)) && (level_courant[(TILE_X+2)*MAX_TILEY + TILE_Y+2]!=1)) level_courant[(TILE_X+2)*MAX_TILEY + TILE_Y+2] = 2;
	if (((TILE_X+2 <= MAX_TILEX-1) && (TILE_Y-2 >= 0)) && (level_courant[(TILE_X+2)*MAX_TILEY + TILE_Y-2]!=1)) level_courant[(TILE_X+2)*MAX_TILEY + TILE_Y-2] = 2;

	if ((TILE_Y-3 >= 0) && (level_courant[TILE_X*MAX_TILEY + TILE_Y-3]!=1)) level_courant[TILE_X*MAX_TILEY + TILE_Y-3] = 2;
	if ((TILE_Y+3 <= MAX_TILEY-1) && (level_courant[TILE_X*MAX_TILEY + TILE_Y+3]!=1)) level_courant[TILE_X*MAX_TILEY + TILE_Y+3] = 2;
}

void manageInput()
{
	if (DowntouchInBox(Stylus, 595, 98, 595 + 600, 98 + 600))
	{
		TILE_X = (Stylus.px-595)/TILE_SIZEX;
		TILE_Y = (Stylus.py-98)/TILE_SIZEY;

		if ((compteur == 0) || (level_courant[TILE_X*MAX_TILEY + TILE_Y] == 2))
		{
			//1er Touché
			level_courant[TILE_X*MAX_TILEY + TILE_Y] = 1;
			compteur++;
			
			//On affiche les 8 possibilités si existante.
			debloqueChoix();
		}
	}

	if ( ((kDown & KEY_A) && (compteur == 0)) || ((kDown & KEY_A) && (level_courant[TILE_X*MAX_TILEY + TILE_Y] == 2)))
	{
		level_courant[TILE_X*MAX_TILEY + TILE_Y] = 1;
		compteur++;

		//On affiche les 8 possibilités si existante.
		debloqueChoix();
	}
	else if (kDown & KEY_B)
	{
		//Reset, new game
		compteur = 0;
		for(lignes = 0; lignes < MAX_TILEX; lignes++)
			for(colonnes = 0; colonnes < MAX_TILEY; colonnes++)
				level_courant[lignes*MAX_TILEY + colonnes] = 0;
	}

	if ((kDown & KEY_UP) && (TILE_Y > 0))
	{
		TILE_Y--;
	}
	else if ((kDown & KEY_DOWN) && (TILE_Y < MAX_TILEY-1))
	{
		TILE_Y++;
	}
	else if ((kDown & KEY_RIGHT) && (TILE_X < MAX_TILEX-1))
	{
		TILE_X++;
	}
	else if ((kDown & KEY_LEFT) && (TILE_X > 0))
	{
		TILE_X--;
	}
}

#include <coreinit/debug.h>

int main(int argc, char **argv)
{
	// Initialize
	SDL_Init(SDL_INIT_EVERYTHING);
#if defined(USE_SDL_INPUT)
	SDL_InitInput();
#endif
	IMG_Init(IMG_INIT_PNG);
#if defined(__SWITCH__)
	romfsInit();
#endif

    	// Create an SDL window & renderer
	window = SDL_CreateWindow("Main-Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
    	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
 
	// Create bg texture:
	surface = IMG_Load(BASE_PATH "resources/main.png");
	background.texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	//Le curseur
	surface = IMG_Load(BASE_PATH "resources/CURSOR.png");
	sprite[0].texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	//Les tiles
	surface = IMG_Load(BASE_PATH "resources/TILES.png");
	sprite[1].texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	//Les nombres
	surface = IMG_Load(BASE_PATH "resources/NUMBERS.png");
	sprite[2].texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	// Game loop:
	while (appRunning())
	{
		// Get inputs
#if defined(USE_SDL_INPUT)
		SDL_ScanInput();
#else
		hidScanInput();
		kDown = hidKeysDown(CONTROLLER_P1_AUTO);
		hidTouchRead(&Stylus, 0);
#endif

		manageInput();

		printGame();

		// On controller 1 input...PLUS
		if (kDown & KEY_PLUS)
			break; 	// Break out of main applet loop to exit
	}
	
	SDL_Quit();				// SDL cleanup
	return EXIT_SUCCESS; 	// Clean exit to HBMenu
}
