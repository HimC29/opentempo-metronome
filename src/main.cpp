#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <OneButton.h>
#include <ESP_I2S.h>

// holds data for the tick sound
#include "tick_data.h"

int bpm = 120;
int msPerBeat = 60000 / bpm;
unsigned long lastTick = 0;
bool isRunning = false;
unsigned long lastDrawTime = 0;

// MAX98357A
const byte bclkPin = 0;
const byte lrcPin = 1;
const byte dinPin = 3;

I2SClass i2s;

// OLED (0.96" SSD1306, 128x64, hardware I2C)
const byte sdaPin = 4;
const byte sclPin = 5;
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Rotary Encoder KY040
const byte clkPin = 6;
const byte dtPin = 7;
const byte swPin = 10;

OneButton button(swPin, true);

static volatile int rotaryDelta = 0;
static int lastState = 0;
static int8_t accumulator = 0;

static void IRAM_ATTR encoderISR() {
    static const int8_t enc_states[] = {0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0};
    
    int currentState = (digitalRead(clkPin) << 1) | digitalRead(dtPin);
    int8_t step = enc_states[(lastState << 2) | currentState];
    lastState = currentState;

    if (step != 0) {
        accumulator += step;
        if (accumulator >= 4) {
            accumulator = 0;
            rotaryDelta = rotaryDelta + 1;
        }
        else if (accumulator <= -4) {
            accumulator = 0;
            rotaryDelta = rotaryDelta - 1;
        }
    }
}

void initRotaryInterrupt() {
    pinMode(clkPin, INPUT_PULLUP);
    pinMode(dtPin, INPUT_PULLUP);
    
    // read initial state
    lastState = (digitalRead(clkPin) << 1) | digitalRead(dtPin);
    accumulator = 0;
    
    attachInterrupt(digitalPinToInterrupt(clkPin), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(dtPin), encoderISR, CHANGE);
}

// function to get current rotary encoder state
int readRotary() {
    if (rotaryDelta == 0) return 0;
    
    noInterrupts();
    int delta = rotaryDelta;
    rotaryDelta = 0;
    interrupts();
    
    return delta;
}
void playTick() {
    i2s.write(tickData, tickDataLen);
}

void setup() {
    Serial.begin(115200);
    while(!Serial) { delay(10); }
    Serial.println("setup");

    Wire.begin(sdaPin, sclPin);
    u8g2.begin();

    i2s.setPins(bclkPin, lrcPin, dinPin);
    i2s.begin(I2S_MODE_STD, 48000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);

    initRotaryInterrupt();

    pinMode(swPin, INPUT_PULLUP);

    button.attachClick([]() {
        isRunning = !isRunning;
    });
}

void loop() {
    button.tick();

    const int delta = readRotary();
    if (delta != 0) {
        bpm += delta;
        bpm = constrain(bpm, 40, 208);
        msPerBeat = 60000 / bpm;
    }

    if (millis() - lastDrawTime >= 50) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB10_tr);
            u8g2.setCursor(0, 20);
            u8g2.print(bpm);

            u8g2.setCursor(0, 40);
            u8g2.print(isRunning ? "ON" : "OFF");
        } while ( u8g2.nextPage() );
        lastDrawTime = millis();
    }

    if (isRunning) {
        if (millis() - lastTick >= msPerBeat) {
            lastTick = millis();
            playTick();
        }
    }
}

