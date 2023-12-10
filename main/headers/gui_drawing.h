
#include "shared.h"
#define COLOR_BG SDL_MapRGB(actualScreen->format, 5, 3, 2)
#define COLOR_OK SDL_MapRGB(actualScreen->format, 0, 0, 255)
#define COLOR_KO SDL_MapRGB(actualScreen->format, 255, 0, 0)
#define COLOR_INFO SDL_MapRGB(actualScreen->format, 0, 255, 0)
#define COLOR_LIGHT SDL_MapRGB(actualScreen->format, 255, 255, 0)
#define COLOR_ACTIVE_ITEM SDL_MapRGB(actualScreen->format, 255, 255, 255)
#define COLOR_INACTIVE_ITEM SDL_MapRGB(actualScreen->format, 255, 255, 255)

/* Shows menu items and pointing arrow	*/
#define SPRX (16)
#define OFF_X 0

/* Re-adujusting Menu Y position */
#define OFF_Y (-6)

void screen_showchar(SDL_Surface *s, int32_t x, int32_t y, uint8_t a, const int32_t fg_color, const int32_t bg_color);

void print_string(const char *s, const uint16_t fg_color, const uint16_t bg_color, int32_t x, int32_t y);
void print_string_video(int16_t x, const int16_t y, const char *s);

void clear_screen_menu(void);

void draw_bluerect_menu(uint8_t i);
void draw_bluerect_file(uint8_t i);
