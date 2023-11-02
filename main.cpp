#include <Arduino.h>
#include <HardwareSerial.h>
#include <Servo.h>

#define N_TARGETS 6
#define N_PEEKERS 3
#define N_MOVERS 0

#define GAME_DUR 90000UL // 1min 30secs
#define WAKEUP_DELAY 10000 // 10secs
#define PEEK_HALF_PERIOD 3000  // 3secs
#define MOVE_HALF_PERIOD 3000  // 3secs
#define TRG_COOLDOWN 500

#define NO_GUN

unsigned long target_wakeup_timer[N_TARGETS] = {0,};
const int target_pins[N_TARGETS] = {22, 23, 24, 25, 26, 27};
const int wakeup_servo_pins[N_TARGETS] = {28, 29, 30, 31, 32, 33};
Servo wakeup_servos[N_TARGETS];
int wakeup_srv_position[N_TARGETS] = {0, 0, 0}; // 0 for standing, 1 for lying
int wakeup_srv_up_value[N_TARGETS] = {70, 70, 0, 0, 0, 0};
int wakeup_srv_down_value[N_TARGETS] = {0, 0, 70, 70, 70, 70};

unsigned long peek_timer[N_PEEKERS] = {0, 0, 0};
int peek_servo_pins[N_PEEKERS] = {34, 35, 36};
Servo peek_servos[N_PEEKERS];
int peek_srv_hide_value[N_PEEKERS] = {0, 180, 180};
int peek_srv_show_value[N_PEEKERS] = {90, 0, 0};
int peek_srv_position[N_PEEKERS] = {0, 0, 0}; // 0 for hide, 1 for show

unsigned long move_timer[N_MOVERS] = {};
int move_servo_pins[N_MOVERS] = {};
Servo move_servos[N_MOVERS];
int move_srv_value_left[N_MOVERS] = {};
int move_srv_value_right[N_MOVERS] = {};
int move_srv_value_neutral[N_MOVERS] = {};
int move_srv_last_movement[N_MOVERS] = {}; // -1 for left, +1 for right, 0 for neutral

bool trigger = false;
bool trigger_prev = false;
int trigger_pin = 37;
unsigned long trigger_cooldown_timer;
int lazer_pin = 38;

unsigned long gameover_timer;

void setup() {
    for (int i=0;i<N_TARGETS;i++) {
        pinMode(target_pins[i], INPUT);
        wakeup_servos[i].attach(wakeup_servo_pins[i]);
        wakeup_servos[i].write(wakeup_srv_up_value[i]);
        wakeup_srv_position[i] = 0;
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

    pinMode(trigger_pin, INPUT_PULLUP);
    pinMode(lazer_pin, OUTPUT);

    Serial.begin(9600);
}

void loop() {
    // wait for input
    if (!Serial.available()) {
        return;
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

    Serial.println("b");

    int score = 0;

    for (;;) {
        unsigned int t = millis();

        if (t - gameover_timer >= GAME_DUR) {
            break;
        }

        trigger_prev = trigger;
        trigger = !digitalRead(trigger_pin);
        #ifdef NO_GUN
        bool lazer_active = true;
        #else
        bool lazer_active = trigger && !trigger_prev;
        #endif

        if (lazer_active && t - trigger_cooldown_timer >= TRG_COOLDOWN) {
            digitalWrite(lazer_pin, HIGH);
            trigger_cooldown_timer = t;
        } else {
            digitalWrite(lazer_pin, LOW);
        }
        
        delay(50);

        bool alldown = true;
        for (int i=0;i<N_TARGETS;i++) {
            if (digitalRead(target_pins[i]) && !wakeup_srv_position[i] && lazer_active) {
                wakeup_srv_position[i] = true;
                target_wakeup_timer[i] = t;
                wakeup_servos[i].write(wakeup_srv_down_value[i]);
                score += 5;
            } else {
                if (t - target_wakeup_timer[i] >= WAKEUP_DELAY && wakeup_srv_position[i]) {
                    wakeup_srv_position[i] = false;
                    wakeup_servos[i].write(wakeup_srv_up_value[i]);
                }
            }

            alldown = alldown && wakeup_srv_position[i];
        }

        if (alldown) {
            for (int i=0;i<N_TARGETS;i++)
            {
                wakeup_srv_position[i] = false;
                wakeup_servos[i].write(wakeup_srv_up_value[i]);
                score += 2;
            }
        }

        for (int i=0;i<N_PEEKERS;i++) {
            if (t - peek_timer[i] >= PEEK_HALF_PERIOD) {
                peek_timer[i] = t;
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
                    move_srv_last_movement[i] = 1;
                    move_servos[i].write(move_srv_value_right[i]);
                } else {
                    move_srv_last_movement[i] = -1;
                    move_servos[i].write(move_srv_value_left[i]);
                }
            }
        }

        Serial.println(score);
    }
    Serial.println("q");
}
