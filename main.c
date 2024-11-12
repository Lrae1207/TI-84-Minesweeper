// Based on code at https://github.com/andymwat/TI-84-CE-DooM
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphx.h>
#include <debug.h>
#include <assert.h>
#include <compression.h>
#include "macros.h"
#include "algorithms.h"
#include "gfx/gfx_group.h"
#include "gfx/my_palette.h"
#include "lookup_table.h"
#include "helpers.h"

typedef struct {
	int x;
	int y;
}Vector2i;

typedef struct {
	float x;
	float y;
}Vector2f;

typedef struct {
	Vector2f origin;
	Vector2f direction;
}Ray;

typedef struct {
	//Vector2f position;
	gfx_sprite_t* sprite;
	bool isMine;
	bool isHidden;
	int mineNeighbors;
}Tile;

void initTile(Tile* tile, bool asMine) {
	tile->sprite = gfx_MallocSprite(TILE_SIZE_PX, TILE_SIZE_PX);
	zx7_Decompress(tile->sprite, enemy1_compressed);

	tile->isMine = asMine;
	tile->isHidden = true;

	tile->mineNeighbors = 0;
}

typedef struct {
	int height;
	int width;
	Tile* tileList;
}TileMap;

// Info that would need to be global but rather is passed around in this struct
typedef struct {
	kb_key_t key;
	TileMap* tileList;
	RandState* random;
	bool partial_redraw;
}GameInfo;

void initData(GameInfo* info, int scale) {
	info->key = 0;
	info->tileList = NULL;
	info->partial_redraw = false;
}

bool handleInput(GameInfo* data) {
	kb_Scan();
	data->key = kb_Data[7];

	// Exit program
	if (kb_Data[1] == EXIT_KEY) {
		return false;
	}

	return true;
}

// returns number of mines
int generateMap(GameInfo* data, TileMap* map) {
	map->height = DEFAULT_HEIGHT_TILES;
	map->width = DEFAULT_WIDTH_TILES;

	int mapSize = map->height * map->width;

	map->tileList = malloc(sizeof(Tile) * mapSize);
	for (int i = 0; i < mapSize; ++i) {
		bool isMine = testChance(data->random, MINE_CHANCE);
		initTile(&map->tileList[i], isMine);
	}

	for (int i = 0; i < mapSize; ++i) {
		map->tileList[i].mineNeighbors = getNeighborCount(map, i);
	}
}

//gfx_UninitedSprite(akSprite, doomak2_width, doomak2_height);
void main() {
	dbg_sprintf(dbgout, "----MINESWEEPER----\n");

	GameInfo data;
	

	// sets up not yet implemented error message
	char* nyiError = "Not Yet Implemented";
	for (int i = 0; i < 20; i++) {
		os_AppErr1[i] = nyiError[i];
	}

	start(&data);
	gfx_Begin();
	gfx_SetDrawBuffer(); // Draw to the buffer to avoid rendering artifats



	bool running = true;
	// until exit key is pressed
	while (running) {
		if (data.partial_redraw) {// Only want to redraw part of the previous frame?
			gfx_BlitScreen(); // Copy previous frame as a base for this frame
		}
		running = handleInput(&data);
		render(&data); // As little non-rendering logic as possible
		gfx_SwapDraw(); // Queue the buffered frame to be displayed
	}

	gfx_End();
	end();


	os_ClrHome();


	while (!os_GetCSC());
}

// Runs once upon starting
void start(GameInfo* data) {
	kb_Scan();
	data->key = kb_Data[7];

	gfx_SetTransparentColor(gfx_group_transparent_color_index);
}

void end() {

}

// render
void render(GameInfo* data) {
	int row = LCD_WIDTH / 2;
	char textBuffer[10];

	//clear the screen
	gfx_FillScreen(255);

	//loop for each vertical line

	gfx_FillRectangle_NoClip(row, LCD_HEIGHT / 2 - temp, fillSpacing, 2 * temp);

	// draw
	// cant find documentation so basically this is it:
	// gcvt(float value, float precision?, char* buffer);

	gcvt(playerPosition[0], 3, &textBuffer);
	gfx_SetTextFGColor(245);
	gfx_SetTextXY(0, 0);
	gfx_PrintString(&textBuffer);

	//draw objects
	drawObjects();

	//draw gun sprite
	gfx_SetPalette(gfx_group_pal, sizeof_gfx_group_pal, 0);
	gfx_ScaledTransparentSprite_NoClip(akSprite, (LCD_WIDTH / 2) - 68, LCD_HEIGHT - doomak2_height * 2, 2, 2);
}


// gfx stuff
/*
void drawMap() {
	float lookview[2];
	int face = 0;
	float scale = 1.25;
	uint24_t x1, y1;
	lookview[0] = lookDirection[0];
	lookview[1] = lookDirection[1];
	//draw map box
	gfx_SetColor(245);
	gfx_Line_NoClip(LCD_WIDTH - 5, 5, LCD_WIDTH - 5, 50);
	gfx_Line_NoClip(LCD_WIDTH - 5, 5, LCD_WIDTH - 50, 5);
	gfx_Line_NoClip(LCD_WIDTH - 50, 5, LCD_WIDTH - 50, 50);
	gfx_Line_NoClip(LCD_WIDTH - 50, 50, LCD_WIDTH - 5, 50);

	//draw faces
	for (face = 0; face < faceArrayCurrentSize; face++)
	{
		gfx_SetColor(faceArray[face].color);
		gfx_Line_NoClip(faceArray[face].point1[0] * scale + LCD_WIDTH - 25, faceArray[face].point1[1] * -scale + 25, faceArray[face].point2[0] * scale + LCD_WIDTH - 25, faceArray[face].point2[1] * -scale + 25);

	}




	x1 = playerPosition[0] * scale + LCD_WIDTH - 25;
	y1 = playerPosition[1] * -scale + 25;

	dbg_sprintf(dbgout, "X:%d\n", x1);
	dbg_sprintf(dbgout, "Y:%d\n", y1);
	//exit if the player is outside the boundaries
	if (x1 > LCD_WIDTH || x1 < 0)
	{
		return;
	}
	if (y1 > LCD_HEIGHT || y1 < 0)
	{
		return;
	}
	//draw player
	gfx_Line_NoClip(x1 - 1, y1, x1 + 1, y1);
	gfx_Line_NoClip(x1, y1 - 1, x1, y1 + 1);

	//draw view
	gfx_SetColor(245);
	rotateVector(&lookview, fov / 2);
	gfx_Line_NoClip(x1, y1, x1 + lookview[0] * scale * 4, y1 + lookview[1] * -scale * 4);
	rotateVector(&lookview, -fov);
	gfx_Line_NoClip(x1, y1, x1 + lookview[0] * scale * 4, y1 + lookview[1] * -scale * 4);
}
*/

//draws the sprites on the screen (not the gun sprite)
void drawObjects()
{
	int num = 0;
	float dot;
	float dist;
	float distToFace;
	bool hit;
	int scale;
	float angleFromCenter = 0;
	Ray testRay;
	float toObject[2];

	gfx_SetPalette(gfx_group_pal, sizeof_gfx_group_pal, 0);
	gfx_ScaledTransparentSprite_NoClip(objectArray[num].sprite, (LCD_WIDTH / 2) + (angleFromCenter * 100 * fov) - (enemy1_width * dist) / 2, LCD_HEIGHT / 2 - (enemy1_height * dist) / 2, dist, dist);
}