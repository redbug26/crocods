#include "guest.h"

// #ifndef _WIN32
#include <sys/time.h>
// #endif

void refreshJoystickConfig(void);

uint16_t textureBytes[384 * 2 * 288];
u32 bufferWidth, bufferHeight;


int32_t mySDLflags;
u32 *incX, *incY;

extern core_crocods_t gb;

int16_t joystick_axies[4] = {0, 0, 0, 0};


u16 scanlineMask[] = {0b1110111101011101,
                      0b1110011100011100,
                      0b1100011000011000,
                      0b1000010000010000};

void guestInit(void)
{
// Set Video

    // printf("Guest Init\n");


} /* initSDL */

void screen_draw_resize(int w, int h);

/*
 * 0: PAD_LEFT;
 * 1: PAD_RIGHT;
 * 2: PAD_UP;
 * 3: PAD_DOWN;
 * 4: PAD_A;
 * 5: PAD_B;
 * 6: PAD_X;
 * 7: PAD_Y;
 * 8: PAD_L;
 * 9: PAD_R;
 * 10: PAD_START;
 * 11: PAD_SELECT;
 * 12: PAD_QUIT;
 * 13: PAD_L2;
 * 14: PAD_R2;
 */

int16_t button_state[18] =  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t button_time[18] =  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t button_virtual[18] =  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void guestStartAudio(void)
{
}


char guestIsFullscreen(core_crocods_t *core)
{

    return 0;

}

void guestFullscreen(core_crocods_t *core, char on)
{

}


void guestGetJoystick(core_crocods_t *core, char *string)
{

    return;
} /* guestGetJoystick */

void guestGetAllKeyPressed(core_crocods_t *core, char *string)
{
} /* guestGetAllKeyPressed */

void guestButtons(core_crocods_t *core,  frk_keyPress fct, frk_keyWasHandled handledfct, frk_handlePaste handlePaste)
{

} /* Buttons */

void guestExit(void)
{
    ddlog(&gb, 2, "Guest exit\n");
}


uint32_t guestGetMilliSeconds(void)
{
    struct timeval tval;    /* timing	*/

    gettimeofday(&tval, 0);
    return (uint32_t)(((tval.tv_sec * 1000000) + (tval.tv_usec)));
}


void guestSleep(u32 milisec)
{

}

void guest_queue_audio(void *data, int len)
{

}

void guestBlit(core_crocods_t *core, u16 *memBitmap, u32 width1, u32 height1, u32 left1, u32 top1, u32 bpl1, u16 *buffer_scr, u16 width2, u16 height2)
{

} /* guestBlit */

void guestScreenDraw(core_crocods_t *core)
{

} /* screen_draw */
