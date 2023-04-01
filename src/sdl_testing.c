#include <stdlib.h>
#include <stdio.h>

#include "SDL.h"

#include "cvsdtp.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

enum KeyPressSurfaces
{
    KEY_PRESS_SURFACE_DEFAULT,
    KEY_PRESS_SURFACE_UP,
    KEY_PRESS_SURFACE_DOWN,
    KEY_PRESS_SURFACE_LEFT,
    KEY_PRESS_SURFACE_RIGHT,
    KEY_PRESS_SURFACE_TOTAL
};

SDL_Surface* getSurfaceOnKeyPress(int key, SDL_Surface* keyPressSurfaces[]) {
    switch (key) {
        case SDLK_UP:
            printf("Up key held\n");
            return keyPressSurfaces[KEY_PRESS_SURFACE_UP];
        case SDLK_DOWN:
            printf("Down key held\n");
            return keyPressSurfaces[KEY_PRESS_SURFACE_DOWN];
        case SDLK_LEFT:
            printf("Left key held\n");
            return keyPressSurfaces[KEY_PRESS_SURFACE_LEFT];
        case SDLK_RIGHT:
            printf("Right key held\n");
            return keyPressSurfaces[KEY_PRESS_SURFACE_RIGHT];
        default:
            return keyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];
    }
}

SDL_Surface* loadSurface(const char* fullPath, SDL_Surface* screenSurface) {
    SDL_Surface* surface = SDL_LoadBMP(fullPath);
    SDL_Surface* optimizedSurface = NULL;
    if (surface == NULL)
        printf("Couldn't load image \"%s\"; Error: %s", fullPath, SDL_GetError());
    else {
        /* Convert bitmap from 24-bit to 32-bit so the conversion
        doesn't have to be done every time the surface is blitted */
        optimizedSurface = SDL_ConvertSurface(surface, screenSurface->format, 0);
        if (optimizedSurface == NULL)
            printf("Couldn't optimize image \"%s\", Error: %s", fullPath, SDL_GetError());
    }
    SDL_FreeSurface(surface);
    return optimizedSurface;
}

void loadBMPs(SDL_Surface* keyPressSurfaces[], SDL_Surface* screenSurface) {
    keyPressSurfaces[KEY_PRESS_SURFACE_UP] = loadSurface("D:\\Code Stuff\\C & C++ Things\\psych_proj\\bin\\up.bmp", screenSurface);
    keyPressSurfaces[KEY_PRESS_SURFACE_DOWN] = loadSurface("D:\\Code Stuff\\C & C++ Things\\psych_proj\\bin\\down.bmp", screenSurface);
    keyPressSurfaces[KEY_PRESS_SURFACE_LEFT] = loadSurface("D:\\Code Stuff\\C & C++ Things\\psych_proj\\bin\\left.bmp", screenSurface);
    keyPressSurfaces[KEY_PRESS_SURFACE_RIGHT] = loadSurface("D:\\Code Stuff\\C & C++ Things\\psych_proj\\bin\\right.bmp", screenSurface);
    keyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT] = loadSurface("D:\\Code Stuff\\C & C++ Things\\psych_proj\\bin\\hello.bmp", screenSurface);
    for (int i = 0; i < KEY_PRESS_SURFACE_TOTAL; i++) {
        if (keyPressSurfaces[i] == NULL)
            printf("yeah you're gonna wanna look in loadBMPs...; Error: %s", SDL_GetError());
    }
}

void freeBMPs(SDL_Surface* keyPressSurfaces[]) {
    // bad to not check for NULL on these but it's just an example...
    SDL_FreeSurface(keyPressSurfaces[KEY_PRESS_SURFACE_UP]);
    SDL_FreeSurface(keyPressSurfaces[KEY_PRESS_SURFACE_DOWN]);
    SDL_FreeSurface(keyPressSurfaces[KEY_PRESS_SURFACE_LEFT]);
    SDL_FreeSurface(keyPressSurfaces[KEY_PRESS_SURFACE_RIGHT]);
    SDL_FreeSurface(keyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT]);
}

void fillWindow(SDL_Window* window, SDL_Surface* screenSurface) {
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);
}

// Initialize SDL subsystems
SDL_Window* initWindow() {
    // Video subsystem
    SDL_Window* window = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error! %s\n", SDL_GetError());
        return NULL;
    }
    window = SDL_CreateWindow("ok", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    return window;
}

//int main(int argc, char* argv[]) {
//    SDL_Window* window = initWindow();
//    // This is the image inside the window
//    SDL_Surface* screenSurface = NULL;
//    // Get the window's surface so we can draw on it
//    screenSurface = SDL_GetWindowSurface(window);
//
//    SDL_Surface* keyPressSurfaces[KEY_PRESS_SURFACE_TOTAL];
//    loadBMPs(keyPressSurfaces, screenSurface);
//
//    if (window == NULL)
//        printf("your window doesn't work");
//    else {
//        SDL_Event event;
//        bool quit = false;
//        SDL_Surface* curSurface = keyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];
//        while (!quit) {
//            while (SDL_PollEvent(&event)) {
//                if (event.type == SDL_QUIT)
//                    quit = true;
//                else if (event.type == SDL_KEYDOWN)
//                    curSurface = getSurfaceOnKeyPress(event.key.keysym.sym, keyPressSurfaces);
//                else
//                    curSurface = keyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];
//                SDL_BlitSurface(curSurface, NULL, screenSurface, NULL);
//                SDL_UpdateWindowSurface(window);
//            }
//        }
//    }
//    freeBMPs(keyPressSurfaces);
//    SDL_DestroyWindow(window);
//    SDL_Quit();
//    return 0;
//}