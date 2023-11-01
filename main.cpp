#include <Arduino.h>
#include <HardwareSerial.h>
#include <Servo.h>

#define N_TARGETS 6
#define N_PEEKERS 2
#define N_MOVERS 1

#define GAME_DUR 100000UL // 1min 40secs
#define WAKEUP_DELAY 5000 // 5secs
#define PEEK_HALF_PERIOD 3000  // 3secs
#define MOVE_HALF_PERIOD 3000  // 3secs

#define TARGET_UP_SRV_VALUE 0
#define TARGET_DOWN_SRV_VALUE 90

unsigned long target_wakeup_timer[N_TARGETS] = {0,};
const int target_pins[N_TARGETS] = {22, 23, 24, 25, 26, 27};
Servo wakeup_servos[N_TARGETS];
int target_srv_position[N_TARGETS] = {0,};

unsigned long peek_timer[N_PEEKERS] = {0,};
int peek_servo_pins[N_PEEKERS] = {28, 29};
Servo peek_servos[N_PEEKERS];
int peek_srv_hide_value[N_PEEKERS] = {0, 0};
int peek_srv_show_value[N_PEEKERS] = {90, 90};
int peek_srv_position[N_PEEKERS] = {0}; // 0 for hide, 1 for show

unsigned long move_timer[N_MOVERS] = {0,};
int move_servo_pins[N_MOVERS] = {30};
Servo move_servos[N_MOVERS];
int move_srv_value_left[N_MOVERS] = {80};
int move_srv_value_right[N_MOVERS] = {100};
int move_srv_value_neutral[N_MOVERS] = {90};
int move_srv_last_movement[N_MOVERS] = {0}; // -1 for left, +1 for right, 0 for neutral

unsigned long gameover_timer;

void setup() {
    for (int i=0;i<N_TARGETS;i++) {
        pinMode(target_pins[i], INPUT);
    }
    for (int i=0;i<N_PEEKERS;i++) {
        peek_servos[i].attach(peek_servo_pins[i]);
        peek_servos[i].write(peek_srv_hide_value[i]);
        peek_srv_position[i] = 0;
    }
    for (int i=0;i<N_MOVERS;i++) {
        move_servos[i].attach(move_servo_pins[i]);
        move_servos[i].write(move_srv_value_neutral[i]);
        move_srv_last_movement[i] = 0;
    }

    Serial.begin(9600);
}

void loop() {
    // wait for input
    if (!Serial.available()) {
        // return;
    }

    while (Serial.available()) {
        Serial.read();
    }

    unsigned int t = millis();
    gameover_timer = t;
    for (int i=0;i<N_TARGETS;i++) {
        target_wakeup_timer[i] = t;
    }
    for (int i=0;i<N_PEEKERS;i++) {
        peek_timer[i] = t;
    }
    for (int i=0;i<N_MOVERS;i++) {
        move_timer[i] = t;
        move_srv_last_movement[i] = -1;
        move_servos[i].write(move_srv_value_left[i]);
    }

    for (;;) {
        unsigned int t = millis();

        if (t - gameover_timer >= GAME_DUR) {
            break;
        }

        for (int i=0;i<N_TARGETS;i++) {
            if (digitalRead(target_pins[i])) {
                target_srv_position[i] = true;
                target_wakeup_timer[i] = t;
                wakeup_servos[i].write(TARGET_DOWN_SRV_VALUE);
            } else {
                if (t - target_wakeup_timer[i] >= WAKEUP_DELAY) {
                    target_srv_position[i] = false;
                    wakeup_servos[i].write(TARGET_UP_SRV_VALUE);
                }
            }
        }

        for (int i=0;i<N_PEEKERS;i++) {
            if (t - peek_timer[i] >= PEEK_HALF_PERIOD) {
                peek_timer[i] = t;
                Serial.println("asdf");
                if (peek_srv_position[i] == 1) {
                    peek_srv_position[i] = 0;
                    peek_servos[i].write(peek_srv_hide_value[i]);
                } else {
                    peek_srv_position[i] = 1;
                    peek_servos[i].write(peek_srv_show_value[i]);
                }
            }
        }

        for (int i=0;i<N_MOVERS;i++) {
            if (t - move_timer[i] >= MOVE_HALF_PERIOD) {
                move_timer[i] = t;
                if (move_srv_last_movement[i] == -1) {
                    move_srv_last_movement[i] == 1;
                    move_servos[i].write(move_srv_value_right[i]);
                } else {
                    move_srv_last_movement[i] == -1;
                    move_servos[i].write(move_srv_value_left[i]);
                }
            }
        }
    }
}
