#include <hardware/watchdog.h>
#include <stdio.h>
#include <string.h>
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pwm.h"
#include "servo.h"
#include "stepper.h"

const int UP_POS = 78;
const int DOWN_POS = 47;
const int WAIT_POS = 138;
const int STP_POS_1 = 240;

Servo servo(21);
Gpio stp(8, OUTPUT);
Gpio slp(28, OUTPUT);
Gpio dir(14, OUTPUT);

Gpio sw0(22, INPUT_PD);
Gpio sw1(23, INPUT_PD);
int current, next;
bool flag;

void up() {
    for (int i = DOWN_POS; i < UP_POS; i++) {
        servo.write(i);
        sleep_ms(25);
    }
}

void down() {
    for (int i = UP_POS; i > DOWN_POS; i--) {
        servo.write(i);
        sleep_ms(25);
    }
    servo.write_ms(0);
}

void yurayura() {
    for (int i = DOWN_POS; i < DOWN_POS + 15; i++) {
        servo.write(i);
        sleep_ms(20);
    }
    sleep_ms(50);
    for (int i = DOWN_POS + 15; i > DOWN_POS; i--) {
        servo.write(i);
        sleep_ms(20);
    }
}

void step(int num, int delay = 1800) {
    for (int i = 0; i < num; i++) {
        stp.write(1);
        sleep_ms(1);
        stp.write(0);
        sleep_us(delay);
    }
}

void guriguri(bool _dir) {
    dir.write(_dir);
    step(int(50 * 200.0 / (21 * 3.1415)), 3500);
    sleep_ms(200);
    dir.write(!_dir);
    step(int(50 * 200.0 / (21 * 3.1415)), 3500);
    sleep_ms(200);
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

    while (1) {
        static char buf[255];
        int cnt = 0;
        while (1) {
            char c = getchar_timeout_us(100000000000);
            buf[cnt] = c;
            if (c == '\n') {
                printf("%s", buf);
                cnt = 0;
                // sscanf(buf, "%c\n", &c);
                // memset(buf, '\0', 255);
                break;
            }
            cnt++;
        }
        if (buf[0] == 's') {
            break;
        }
    }
    up();
    for (int i = UP_POS; i < WAIT_POS; i++) {
        servo.write(i);
        sleep_ms(25);
    }

    while (1) {
        printf("%d\n", sw0.read());
        step(1);
        if (!sw0.read()) {
            current = 3;
            break;
        }
    }

    multicore_launch_core1(serial_read);
    next = 3;
    while (1) {
        while (1) {
            if (current != next) {
                break;
            }
            sleep_ms(10);
        }
        dir.write((current - next) < 0);
        slp.write(1);
        switch (next) {
            case 0:
                if (!flag) {
                    for (int i = WAIT_POS; i > UP_POS; i--) {
                        servo.write(i);
                        sleep_ms(25);
                    }
                    flag = true;
                } else {
                    up();
                }
                printf("next -> 0\n");
                while (1) {
                    step(1);
                    if (!sw1.read()) {
                        current = 0;
                        break;
                    }
                }
                down();
                break;
            case 1:
                if (!flag) {
                    for (int i = WAIT_POS; i > UP_POS; i--) {
                        servo.write(i);
                        sleep_ms(25);
                    }
                    flag = true;
                } else {
                    up();
                }
                printf("next -> 1\n");
                step(int(STP_POS_1 * 200.0 / (21 * 3.1415)));
                current = 1;
                down();
                break;

            case 2:
                if (!flag) {
                    for (int i = WAIT_POS; i > UP_POS; i--) {
                        servo.write(i);
                        sleep_ms(25);
                    }
                    flag = true;
                } else {
                    up();
                }
                printf("next -> 2\n");
                while (1) {
                    step(1);
                    if (!sw0.read()) {
                        current = 2;
                        break;
                    }
                }
                down();
                break;

            case 3:
                if (!flag) {
                    for (int i = WAIT_POS; i > UP_POS; i--) {
                        servo.write(i);
                        sleep_ms(25);
                    }
                    flag = true;
                } else {
                    up();
                }
                printf("next -> 3\n");
                while (1) {
                    step(1);
                    if (!sw0.read()) {
                        current = 3;
                        break;
                    }
                }
                for (int i = UP_POS; i < WAIT_POS; i++) {
                    servo.write(i);
                    sleep_ms(25);
                }
                while (1) {
                    sleep_ms(10);
                }
                break;

            case 4:
                printf("yurayura\n");
                while (1) {
                    yurayura();
                    if (next == 7)
                        break;
                }
                next = current;
                break;

            case 5:
                printf("guriguri\n");
                for (int i = DOWN_POS; i < DOWN_POS + 15; i++) {
                    servo.write(i);
                    sleep_ms(20);
                }
                while (1) {
                    switch (current) {
                        case 0:
                            guriguri(true);
                            break;
                        case 1:
                            guriguri(true);
                            guriguri(false);
                            break;
                        case 2:
                            guriguri(false);
                            break;
                    }
                    if (next == 6)
                        break;
                }
                for (int i = DOWN_POS + 15; i > DOWN_POS; i--) {
                    servo.write(i);
                    sleep_ms(20);
                }
                next = current;
                break;
            default:
                break;
        }
        slp.write(0);
        // servo.write_ms(0);
        printf("current: %d\n", current);
    }
}
