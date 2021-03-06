/****************************************************************************
 *     _____        _____ _   _ _______   _____  _    _ _   _
 *    |  __ \ /\   |_   _| \ | |__   __| |  __ \| |  | | \ | |
 *    | |__) /  \    | | |  \| |  | |    | |__) | |  | |  \| |
 *    |  ___/ /\ \   | | | . ` |  | |    |  _  /| |  | | . ` |
 *    | |  / ____ \ _| |_| |\  |  | |    | | \ \| |__| | |\  |
 *    |_| /_/    \_\_____|_| \_|  |_|    |_|  \_\\____/|_| \_|
 *
 * Copyright (C) 2015 Franklin Wei
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include <assert.h>
#include <math.h>
#include <time.h>

#include "sdl.h"

#include "../../src/dash.h"
#include "../../src/compat.h"

static SDL_Surface *screen = NULL;
static uint32_t fgcol, bgcol;
static TTF_Font *gameover_font = NULL, *font = NULL;

void plat_clear(void)
{
    SDL_FillRect(screen, NULL, bgcol);
}

void plat_set_background(unsigned col)
{
    bgcol = col;
}

void plat_set_foreground(unsigned col)
{
    fgcol = col;
}

void plat_hline(int x1, int x2, int y)
{
    if(y<0)
        return;
    uint8_t *left = screen->pixels + screen->pitch * y;
    uint8_t *right = left + x2 * 4;
    left += x1 * 4;
    while(left < right)
    {
        *(uint32_t*) left = fgcol;
        left += 4;
    }
}

void plat_vline(int x, int y1, int y2)
{
    if(y1>y2)
    {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }
    uint8_t *i = screen->pixels + screen->pitch * y1 + x * 4;
    uint8_t *stop = screen->pixels + screen->pitch * y2 + x * 4;
    while(i < stop)
    {
        *(uint32_t*)i = fgcol;
        i += screen->pitch;
    }
}

void plat_drawpixel(int x, int y)
{
    uint32_t *pix = screen->pixels;
    pix += screen->w * y;
    pix += x;
    *pix = fgcol;
}

void plat_drawrect(int x, int y, int w, int h)
{
    plat_hline(x, x+w, y);
    plat_hline(x, x+w, y+h);
    plat_vline(y, y+h, x);
    plat_vline(y, y+h, x+w);
}

void plat_fillrect(int x, int y, int w, int h)
{
    SDL_Rect rect = {x, y, w, h};
    SDL_FillRect(screen, &rect, fgcol);
}

void plat_filltri(int x1, int y1, int x2, int y2, int x3, int y3)
{
    /* TODO */
}

void plat_log(const char *str)
{
    printf("%s\n", str);
}

void plat_logf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[128];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    fprintf(stderr, "%s\n", buf);
    va_end(ap);
}

void plat_update(void)
{
    SDL_Flip(screen);
}

#define PI 3.141592653589793238462643383279502884197169399375105820974944592317816406286208998628

fixed_t plat_sin(fixed_t ang)
{
    double d = (double) ang / (1<<FRACBITS);
    d *= PI/180;
    return sin(d) * (1<<FRACBITS);
}

fixed_t plat_cos(fixed_t ang)
{
    double d = (double) ang / (1<<FRACBITS);
    d *= PI/180;
    return cos(d) * (1<<FRACBITS);
}

fixed_t plat_sqrt(fixed_t n)
{
    double d = (double) n / (1<<FRACBITS);
    return sqrt(d) * (1<<FRACBITS);
}

unsigned int plat_rand(void)
{
    return rand();
}

void plat_srand(void)
{
    srand(time(NULL));
}

long plat_time(void)
{
    return SDL_GetTicks();
}

void plat_sleep(long ms)
{
    SDL_Delay(ms);
}

void plat_fillcircle(int cx, int cy, int r)
{
    plat_logf("fillcircle %d %d %d", cx, cy, r);
    int d = 3 - (r * 2);
    int x = 0;
    int y = r;
    while(x <= y)
    {
        plat_hline(cx - x, cx + x, cy + y);
        plat_hline(cx - x, cx + x, cy - y);
        plat_hline(cx - y, cx + y, cy + x);
        plat_hline(cx - y, cx + y, cy - x);
        if(d < 0)
        {
            d += (x * 4) + 6;
        }
        else
        {
            d += ((x - y) * 4) + 10;
            --y;
        }
        ++x;
    }
}

unsigned int sdlplat_rgbpack(uint8_t r, uint8_t g, uint8_t b)
{
    return SDL_MapRGB(screen->format, r,g,b);
}

static enum keyaction_t action;

void plat_yield(void)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_QUIT:
            exit(0);
            break;
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_SPACE)
            {
                action = ACTION_JUMP;
            }
            else if(event.key.keysym.sym == SDLK_ESCAPE)
            {
                action = ACTION_PAUSE;
            }
            else
            {
                action = ACTION_OTHER;
            }
            break;
        }
    }
}

enum keyaction_t plat_pollaction(void)
{
    enum keyaction_t ret = action;
    action = NONE;
    return ret;
}

enum menuaction_t plat_domenu(void)
{
    return MENU_DOGAME;
}

static void fade_out(void)
{
#define FADE_FRAMES 100
    for(int i = 0; i < FADE_FRAMES; ++i)
    {
        for(int j = 0; j < screen->h * screen->pitch; ++j)
            *(uint8_t*)(screen->pixels + j) *= .95;
        plat_update();
    }
}

void plat_gameover(struct game_ctx_t *ctx)
{
    SDL_Surface *text = TTF_RenderText_Blended(gameover_font, "Game Over", (SDL_Color){255,255,255,0});
    if(!text)
    {
        plat_logf("WARNING: font render failed");
        return;
    }

    SDL_Rect dest = { screen->w / 2 - text->w / 2,
                      screen->h / 2 - 3 * text->h,
                      text->w, text->h };

    SDL_BlitSurface(text, NULL, screen, &dest);
    SDL_FreeSurface(text);

    text = TTF_RenderText_Blended(font, "Press any key to continue...", (SDL_Color){255,255,255,0});
    if(!text)
    {
        plat_logf("WARNING: font render failed");
        return;
    }

    SDL_Rect dest2 = { screen->w / 2 - text->w / 2,
                       screen->h / 2 - 3 * text->h,
                       text->w, text->h };

    SDL_BlitSurface(text, NULL, screen, &dest2);
    SDL_FreeSurface(text);

    plat_update();
    while(plat_pollaction() == NONE)
    {
        plat_yield();
    }
    fade_out();
}

void plat_paused(struct game_ctx_t *ctx)
{

}

void plat_drawscore(fixed_t score)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%ld m", score >> FRACBITS);
    SDL_Surface *text = TTF_RenderText_Blended(gameover_font, buf, (SDL_Color){255,255,255,0});

    SDL_Rect dest = { screen->w / 2 - text->w / 2,
                      screen->h / 2 - 4 * text->h,
                      text->w, text->h };

    SDL_BlitSurface(text, NULL, screen, &dest);
    SDL_FreeSurface(text);
}

fixed_t plat_cos8(uint8_t phase)
{
    static const fixed_t cos_table[256] = {
        65536, 65516, 65457, 65358, 65220, 65043, 64826, 64571,
        64276, 63943, 63571, 63162, 62714, 62228, 61705, 61144,
        60547, 59913, 59243, 58538, 57797, 57022, 56212, 55368,
        54491, 53581, 52639, 51665, 50660, 49624, 48558, 47464,
        46340, 45189, 44011, 42806, 41575, 40319, 39039, 37736,
        36409, 35061, 33692, 32302, 30893, 29465, 28020, 26557,
        25079, 23586, 22078, 20557, 19024, 17479, 15923, 14359,
        12785, 11204,  9616,  8022,  6423,  4821,  3215,  1608,
        0, -1608, -3215, -4821, -6423, -8022, -9616, -11204,
        -12785, -14359, -15923, -17479, -19024, -20557, -22078, -23586,
        -25079, -26557, -28020, -29465, -30893, -32302, -33692, -35061,
        -36409, -37736, -39039, -40319, -41575, -42806, -44011, -45189,
        -46340, -47464, -48558, -49624, -50660, -51665, -52639, -53581,
        -54491, -55368, -56212, -57022, -57797, -58538, -59243, -59913,
        -60547, -61144, -61705, -62228, -62714, -63162, -63571, -63943,
        -64276, -64571, -64826, -65043, -65220, -65358, -65457, -65516,
        -65536, -65516, -65457, -65358, -65220, -65043, -64826, -64571,
        -64276, -63943, -63571, -63162, -62714, -62228, -61705, -61144,
        -60547, -59913, -59243, -58538, -57797, -57022, -56212, -55368,
        -54491, -53581, -52639, -51665, -50660, -49624, -48558, -47464,
        -46340, -45189, -44011, -42806, -41575, -40319, -39039, -37736,
        -36409, -35061, -33692, -32302, -30893, -29465, -28020, -26557,
        -25079, -23586, -22078, -20557, -19024, -17479, -15923, -14359,
        -12785, -11204, -9616, -8022, -6423, -4821, -3215, -1608,
        0,  1608,  3215,  4821,  6423,  8022,  9616, 11204,
        12785, 14359, 15923, 17479, 19024, 20557, 22078, 23586,
        25079, 26557, 28020, 29465, 30893, 32302, 33692, 35061,
        36409, 37736, 39039, 40319, 41575, 42806, 44011, 45189,
        46340, 47464, 48558, 49624, 50660, 51665, 52639, 53581,
        54491, 55368, 56212, 57022, 57797, 58538, 59243, 59913,
        60547, 61144, 61705, 62228, 62714, 63162, 63571, 63943,
        64276, 64571, 64826, 65043, 65220, 65358, 65457, 65516 };
    return cos_table[phase];
}

int main(int argc, char* argv[])
{
    srand(7);
    plat_logf("SDL init");
    SDL_Init(SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(LCD_WIDTH, LCD_HEIGHT, 32, SDL_HWSURFACE | SDL_HWACCEL);

    SDL_EnableKeyRepeat(1, SDL_DEFAULT_REPEAT_INTERVAL);

    assert(screen != NULL);
    fgcol = LCD_RGBPACK(255,255,255);
    bgcol = LCD_RGBPACK(0,0,0);

    action = NONE;

    SDL_WM_SetCaption(GAME_TITLE, GAME_TITLE);
    atexit(SDL_Quit);

    plat_logf("TTF init");
    if(TTF_Init() < 0)
    {
        plat_logf("TTF init fail!\n");
    }

    gameover_font = TTF_OpenFont("/usr/share/fonts/TTF/LiberationSans-Regular.ttf", LCD_HEIGHT / 12);

    font = TTF_OpenFont("/usr/share/fonts/TTF/LiberationSans-Regular.ttf", LCD_HEIGHT / 24);

    if(!gameover_font)
    {
        plat_logf("WARNING: font loading failed");
    }

    atexit(TTF_Quit);

    dash_main();
    return 0;
}
