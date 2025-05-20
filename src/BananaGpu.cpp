#include "BananaGpu.h"
#include "BananaConsole.h"
#include "BananaMemory.h"
#include <cstdio>
#include <iostream>

BananaGpu::BananaGpu(BananaConsole *console)
    : console(console), lastRenderTime(0) {
  if (!initSDL()) {
    std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
    exit(1);
  }
}

BananaGpu::~BananaGpu() { cleanup(); }

bool BananaGpu::initSDL() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    return false;
  }

  window =
      SDL_CreateWindow("Banana Console", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH * WINDOW_SCALE,
                       DISPLAY_HEIGHT * WINDOW_SCALE, SDL_WINDOW_SHOWN);

  if (!window) {
    return false;
  }

  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    return false;
  }

  // Create texture with the optimal format for your platform
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                              SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH,
                              DISPLAY_HEIGHT);

  if (!texture) {
    return false;
  }

  // Set scaling quality
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

  return true;
}

void BananaGpu::cleanup() {
  if (texture) {
    SDL_DestroyTexture(texture);
  }
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  SDL_Quit();
}

uint16_t BananaGpu::getPixelAddress(int width, int height) const {
  return VRAM_START + width + (height * DISPLAY_WIDTH);
}

void BananaGpu::decodePixel(uint8_t pixel, uint8_t &r, uint8_t &g,
                            uint8_t &b) const {
  // Extract RGB values and convert to 8-bit color in one step
  r = ((pixel >> 4) & 0x03) * 85; // 85 is approximately 255/3
  g = ((pixel >> 2) & 0x03) * 85;
  b = (pixel & 0x03) * 85;
}

bool BananaGpu::shouldRender() const {
  uint32_t currentTime = SDL_GetTicks();
  return (currentTime - lastRenderTime) >= FRAME_INTERVAL;
}

void BananaGpu::render() {
  // Only render if enough time has passed
  if (!shouldRender()) {
    return;
  }

  // Update last render time
  lastRenderTime = SDL_GetTicks();

  // Handle SDL events
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      exit(0);
    }
    if (event.type == SDL_KEYDOWN) {
      SDL_Keycode key = event.key.keysym.sym;
      if (key == SDLK_a) {
        console->input |= CONTROLLER_LEFT_MASK;
      } if (key == SDLK_w) {
        console->input |= CONTROLLER_UP_MASK;
      } if (key == SDLK_s) {
        console->input |= CONTROLLER_DOWN_MASK;
      } if (key == SDLK_d) {
        console->input |= CONTROLLER_RIGHT_MASK;
      } if (key == SDLK_g) {
        console->input |= CONTROLLER_SELECT_MASK;
      } if (key == SDLK_h ) {
        console->input |= CONTROLLER_START_MASK;
      } if (key == SDLK_k) {
        console->input |= CONTROLLER_A_MASK;
      } if (key == SDLK_l ) {
        console->input |= CONTROLLER_B_MASK;
      }
    }
    if (event.type == SDL_KEYUP){
      SDL_Keycode key = event.key.keysym.sym;
      if (key == SDLK_a) {
        console->input &=  ~CONTROLLER_LEFT_MASK;
      } if (key == SDLK_w) {
        console->input &= ~CONTROLLER_UP_MASK;
      } if (key == SDLK_s) {
        console->input &=  ~CONTROLLER_DOWN_MASK;
      } if (key == SDLK_d) {
        console->input &=  ~CONTROLLER_RIGHT_MASK;
      } if (key == SDLK_g) {
        console->input &=  ~CONTROLLER_SELECT_MASK;
      } if (key == SDLK_h ) {
        console->input &=  ~CONTROLLER_START_MASK;
      } if (key == SDLK_k) {
        console->input &=  ~CONTROLLER_A_MASK;
      } if (key == SDLK_l ) {
        console->input &=  ~CONTROLLER_B_MASK;
      }
    }
    
    // else if (event.type == SDL_KEYUP){

    // }
  }
  
//   #define CONTROLLER_A_MASK ((uint8_t)0x80)
// #define CONTROLLER_B_MASK ((uint8_t)0x40)
// #define CONTROLLER_SELECT_MASK ((uint8_t)0x20)
// #define CONTROLLER_START_MASK ((uint8_t)0x10)
// #define CONTROLLER_UP_MASK ((uint8_t)0x08)
// #define CONTROLLER_DOWN_MASK ((uint8_t)0x04)
// #define CONTROLLER_LEFT_MASK ((uint8_t)0x02)
// #define CONTROLLER_RIGHT_MASK ((uint8_t)0x01)

  // Lock texture for direct access
  void *pixels;
  int pitch;
  SDL_LockTexture(texture, NULL, &pixels, &pitch);

  // Update all pixels at once
  uint8_t *pixelData = static_cast<uint8_t *>(pixels);
  for (int y = 0; y < DISPLAY_HEIGHT; y++) {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
      uint16_t addr = getPixelAddress(x, y);
      uint8_t pixel = console->memory.l8_u(addr);

      int pos = (y * pitch) + (x * 3);
      decodePixel(pixel, pixelData[pos], pixelData[pos + 1],
                  pixelData[pos + 2]);
    }
  }

  // Unlock texture
  SDL_UnlockTexture(texture);

  // Render the texture
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}
