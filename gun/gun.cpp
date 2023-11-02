#include <Arduino.h>

int btn_pin = 2;
int lazer_pin = 13;

void fire() {
    noInterrupts();
    digitalWrite(lazer_pin, HIGH);
    delay(50);
    digitalWrite(lazer_pin, LOW);
    delay(500);
    interrupts();
}

void setup() {
    pinMode(btn_pin, INPUT_PULLUP);
    pinMode(lazer_pin, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(btn_pin), fire, RISING);
}

void loop() {
    
}
