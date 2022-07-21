#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "renderer.h"

static SDL_Window  * window;
static SDL_Renderer* renderer;
static TTF_Font    * font;


const SDL_Rect drag_area = {0, 0, 700, 25};

SDL_HitTestResult hit(SDL_Window* window, const SDL_Point* area, void* data) {
  if(SDL_PointInRect(area, &drag_area)) {
    return SDL_HITTEST_DRAGGABLE;
  }  
  return SDL_HITTEST_NORMAL;
}

void r_init(int width, int height) {
  /* init SDL window */ 
  window = SDL_CreateWindow("SkyRat", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS);
  renderer = SDL_CreateRenderer(window, -1, 0);
  font = TTF_OpenFont("font.ttf", 12);
    
  SDL_SetWindowHitTest(window, hit, NULL);
}

void r_clean(void) {
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  TTF_CloseFont(font);
}

void r_draw_rect(mu_Rect rect, mu_Color color) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_Rect sdl_rect = {rect.x, rect.y, rect.w, rect.h};
  SDL_RenderFillRect(renderer, &sdl_rect);
}

void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
  SDL_Color clr = {color.r, color.g, color.b, color.a};
  SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, clr);
  SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
  SDL_FreeSurface(text_surface);
    
  int text_size[2] = {r_get_text_size(text)[0], r_get_text_size(text)[1]};
  SDL_Rect sdl_dst = {pos.x, pos.y, text_size[0], text_size[1]};
  SDL_RenderCopy(renderer, text_texture, NULL, &sdl_dst);
    
  SDL_DestroyTexture(text_texture);
}

void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
  if (id != 2) // checkbox
    return;
    
  //SDL_Surface* img_surface;
  //SDL_Texture* img_texture;
   
  SDL_Surface* img_surface = IMG_Load("new.bmp");
  SDL_Texture* img_texture = SDL_CreateTextureFromSurface(renderer, img_surface);
  SDL_FreeSurface(img_surface);
    
  SDL_Rect img_rect = {rect.x, rect.y, rect.w, rect.h};
    
  SDL_RenderCopy(renderer, img_texture, NULL, &img_rect);
    
  SDL_DestroyTexture(img_texture);
}

int* r_get_text_size(const char *text) {
  static int text_size[2];
  int width, height;
  TTF_SizeText(font, text, &width, &height);
  text_size[0] = width;
  text_size[1] = height;
  return text_size;
}

void r_clear(mu_Color clr) {
  SDL_SetRenderDrawColor(renderer, clr.r,clr.g,clr.b,clr.a);
  SDL_RenderClear(renderer);
}

void r_present(void) {
  SDL_RenderPresent(renderer);
}
