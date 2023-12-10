#include <string.h>

#include "shared.h"

#include "game_input.h"
#include "guest.h"

#ifdef JOYSTICK
extern int16_t joystick_axies[4];
#define JOYSTICK_UP    (joystick_axies[1] < -2048 ? 1 : 0)
#define JOYSTICK_RIGHT (joystick_axies[0] > 2048 ? 1 : 0)
#define JOYSTICK_LEFT  (joystick_axies[0] < -2048 ? 1 : 0)
#define JOYSTICK_DOWN  (joystick_axies[1] > 2048 ? 1 : 0)
#endif

int32_t WsInputGetState(core_crocods_t *core)
{
    // int32_t mode = 0;
    // static int16_t old_button_state[18];

    /* enum     KEYPAD_BITS {
     * KEY_A = BIT(0), KEY_B = BIT(1), KEY_SELECT = BIT(2), KEY_START = BIT(3),
     * KEY_RIGHT = BIT(4), KEY_LEFT = BIT(5), KEY_UP = BIT(6), KEY_DOWN = BIT(7),
     * KEY_R = BIT(8), KEY_L = BIT(9), KEY_X = BIT(10), KEY_Y = BIT(11),
     * KEY_TOUCH = BIT(12), KEY_LID = BIT(13), KEY_R2 = BIT(14), KEY_L2 = BIT(15),
     * }; */

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

    // int8_t szFile[256];
    int32_t button = 0;

    guestButtons(core, NULL, NULL, NULL);
    // button = Fire_buttons();

    /* Button A	*/
    button |= button_state[4] ? (1 << 0) : 0;
    /* Button B	*/
    button |= button_state[5] ? (1 << 1) : 0;

    /* SELECT BUTTON */
    button |= button_state[11] ? (1 << 2) : 0;

    /* START BUTTON */
    button |= button_state[10] ? (1 << 3) : 0;

    /* RIGHT -> X1	*/
    button |= button_state[1] ? (1 << 4) : 0;
    /* LEFT -> X1	*/
    button |= button_state[0] ? (1 << 5) : 0;
    /* UP -> X1		*/
    button |= button_state[2] ? (1 << 6) : 0;
    /* DOWN -> X1	*/
    button |= button_state[3] ? (1 << 7) : 0;

    /* R1	*/
    button |= button_state[9] ? (1 << 8) : 0;
    /* L1	*/
    button |= button_state[8] ? (1 << 9) : 0;

    /* X    */
    button |= button_state[6] ? (1 << 10) : 0;
    /* Y    */
    button |= button_state[7] ? (1 << 11) : 0;

    /* R2    */
    button |= button_state[14] ? (1 << 14) : 0;
    /* L2    */
    button |= button_state[13] ? (1 << 15) : 0;


    if (button_state[11]) {
        printf("WsInputGetState: select\n");
    }

    return button;
} /* WsInputGetState */

int32_t Fire_buttons(void)
{
    static uint8_t x_button = 0, y_button = 0;
    int32_t button = 0;

    switch (GameConf.input_layout) {
        case 0:
        case 1:
            if (button_state[6] > 0)
                x_button++;         /* (Rapid Fire A) */
            else
                x_button = 0;

            if (button_state[7] > 0)
                y_button++;         /* (Rapid Fire B) */
            else
                y_button = 0;
            break;
        case 2:
            if (button_state[14] > 0)
                x_button++;         /* (Rapid Fire A) */
            else
                x_button = 0;

            if (button_state[16] > 0)
                y_button++;         /* (Rapid Fire B) */
            else
                y_button = 0;
            break;
        case 3:
        case 4:
#ifdef JOYSTICK
            if (JOYSTICK_LEFT)
                x_button++;         /* (Rapid Fire A) */
            else
                x_button = 0;

            if (JOYSTICK_UP)
                y_button++;         /* (Rapid Fire B) */
            else
                y_button = 0;
#endif
            break;
    } /* switch */

    if (x_button == 1) {
        button |= (1 << 11);
    } else if (x_button > 1) {
        button |= (0 << 11);
        x_button = 0;
    }

    if (y_button == 1) {
        button |= (1 << 10);
    } else if (y_button > 1) {
        button |= (0 << 10);
        y_button = 0;
    }

    return button;
} /* Fire_buttons */
