#include "md.h"

typedef struct {
    volatile unsigned char reserved0;
    volatile unsigned char FIFODATA;
    volatile unsigned short FIFOSTAT;
} EDIO;

#define EDIO_REG ((EDIO *)0xA130D0)
#define FIFO_CPU_RXF 0x8000
#define FIFO_RXF_MSK 0x07FF

#define SCREEN_X 4
#define SCREEN_Y 6
#define SCREEN_W 40
#define SCREEN_H 28

static int cursorX = SCREEN_X;
static int cursorY = SCREEN_Y;

static void clear_terminal(void)
{
    char spaces[SCREEN_W + 1];
    int i;

    for (i = 0; i < SCREEN_W; i++)
        spaces[i] = ' ';

    spaces[SCREEN_W] = '\0';

    for (i = 0; i < SCREEN_H; i++)
        vdp_puts(VDP_PLAN_A, spaces, 0, i);

    cursorX = SCREEN_X;
    cursorY = SCREEN_Y;
}

static void print_char(char c)
{
    char str[2];

    str[0] = c;
    str[1] = '\0';

    if (c == '\r')
        return;

    if (c == '\n') {
        cursorX = SCREEN_X;
        cursorY++;
    } else {
        vdp_puts(VDP_PLAN_A, str, cursorX, cursorY);
        cursorX++;
    }

    if (cursorX >= SCREEN_W) {
        cursorX = SCREEN_X;
        cursorY++;
    }

    if (cursorY >= SCREEN_H) {
        clear_terminal();
    }
}

static unsigned char receive_byte(void)
{
    while (EDIO_REG->FIFOSTAT & FIFO_CPU_RXF) {
        vdp_vsync();
    }

    while ((EDIO_REG->FIFOSTAT & FIFO_RXF_MSK) == 0) {
        vdp_vsync();
    }

    return EDIO_REG->FIFODATA;
}

int main(void)
{
    vdp_init();
    enable_ints;

    vdp_color(0, 0x080);

    vdp_puts(VDP_PLAN_A, "EVERDRIVE USB TERMINAL", 4, 3);
    vdp_puts(VDP_PLAN_A, "Waiting for Linux...", 4, 4);

    while (1) {
        unsigned char c = receive_byte();
        print_char(c);
    }

    return 0;
}
