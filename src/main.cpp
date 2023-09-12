#include <hardware/watchdog.h>
#include <stdio.h>
#include <string.h>
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pwm.h"
#include "servo.h"
#include "stepper.h"

const int UP_POS = 115;
const int DOWN_POS = 142;
const int STP_POS_1 = 240;

Servo servo(21);
Gpio stp(8, OUTPUT);
Gpio slp(28, OUTPUT);
Gpio dir(14, OUTPUT);

Gpio sw0(22, INPUT_PD);
Gpio sw1(23, INPUT_PD);
int current, next;

void up() {
    for (int i = DOWN_POS; i > UP_POS; i--) {
        servo.write(i);
        sleep_ms(25);
    }
}

void down() {
    for (int i = UP_POS; i < DOWN_POS; i++) {
        servo.write(i);
        sleep_ms(25);
    }
    servo.write_ms(0);
}

void step(int num) {
    for (int i = 0; i < num; i++) {
        stp.write(1);
        sleep_ms(1);
        stp.write(0);
        sleep_us(1600);
    }
}

void reset() {
    watchdog_enable(1, 1);
    while (1)
        ;
}

void serial_read() {
    static char buf[255];
    while (1) {
        int cnt = 0;
        while (1) {
            char c = getchar_timeout_us(100000000000);
            buf[cnt] = c;
            if (c == '\n') {
                cnt = 0;
                sscanf(buf, "%d\n", &next);
                memset(buf, '\0', 255);
                break;
            }
            if (c == 'r') {
                // reset microcontroller
                reset();
            }
            cnt++;
        }
    }
}

int main(void) {
    stdio_init_all();
    servo.init();
    stp.init();
    slp.init();
    slp.write(1);
    dir.init();
    dir.write(1);
    sw0.init();
    servo.write(UP_POS);

    while (1) {
        printf("%d\n", sw0.read());
        step(1);
        if (!sw0.read()) {
            current = 2;
            break;
        }
    }

    //  while (1) {
    //     int cnt = 0;
    //     static char buf[255];
    //     while (1) {
    //         char c = getchar_timeout_us(100000000000);
    //         buf[cnt] = c;
    //         if (c == '\n') {
    //             cnt = 0;
    //             break;
    //         }
    //         if (c == 'r') {
    //             // reset microcontroller
    //             reset();
    //         }
    //         cnt++;
    //     }
    // }


    multicore_launch_core1(serial_read);
    next = 2;
    while (1) {
        dir.write((current - next) < 0);
        switch (next) {
            case 0:
                printf("next -> 0\n");
                while (1) {
                    step(1);
                    if (!sw1.read()) {
                        current = 0;
                        break;
                    }
                }
                break;
            case 1:
                printf("next -> 1\n");
                step(int(STP_POS_1 * 200.0 / (21 * 3.1415)));
                current = 1;
                break;

            case 2:
                printf("next -> 2\n");
                while (1) {
                    step(1);
                    if (!sw0.read()) {
                        current = 2;
                        break;
                    }
                }
                break;
            default:
                break;
        }
        slp.write(0);
        down();
        servo.write_ms(0);
        printf("current: %d\n", current);
        while (1) {
            if (current != next) {
                break;
            }
            sleep_ms(10);
        }        
        up();
        slp.write(1);
    }
}
