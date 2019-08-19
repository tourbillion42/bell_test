#include <util/delay.h>
#include <SPI.h>
#include <PN532_SPI.h>
#include "PN532.h"
#include <Adafruit_NeoPixel.h>

#define LED 5
#define BUTTON 6
#define NFC 10
#define BUZZER A1
int debounceDelay = 50;
unsigned long lastDebounceTime = 0;
int buttonState = HIGH;
int lastButtonState = HIGH;

uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidLength; // 4(Mifare Classic) or 7(Mifare Ultralight) bytes, ISO14443A

PN532_SPI pn532spi(SPI, NFC);
PN532 nfc(pn532spi);

Adafruit_NeoPixel pixels(1, LED, NEO_GRB + NEO_KHZ800);
const int brightness = 255;
const uint32_t color_off = pixels.Color(0, 0, 0);
const uint32_t color_red = pixels.Color(255, 0, 0);
const uint32_t color_green = pixels.Color(0, 150, 0);
const uint32_t color_blue = pixels.Color(0, 0, 150);
const uint32_t color_white = pixels.Color(255, 255, 255);

void setup(void) {
  Serial.begin(115200);

  pinMode(BUTTON, INPUT_PULLUP);
//  pinMode(BUZZER, OUTPUT);

  pixels.begin();
  pixels.setBrightness(brightness);
  
  fade();
  
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    blink(5, color_red);
    while (1);
  }

  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
    
  pixels.clear();
  pixels.show();
  _delay_ms(1000);

  beep(1318);
  blink(color_white, 2);
}

void loop(void) {  
  // button
  int reading = digitalRead(BUTTON);

  if (reading != lastButtonState) lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay) {  //check bounce
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        ledOn(color_blue);

        byte str[] = {0xF7, 0xA1, 0x01, 0x41, 0x01, 0x01, 0x12, 0xF2};
        for (int i=0; i < sizeof(str); i++) {
           Serial.write(str[i]);
        }
        ring();
        
        ledOff();
      }
    }
  }
  
  lastButtonState = reading;

  // nfc  
  boolean success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, debounceDelay);
  if (success) {
    beep(3520);
    
    byte str1[] = {0xF7, 0xA1, 0x01, 0x41, 0x05, 0x04, 0x17, 0xFA};
    for (int i=0; i < sizeof(str1); i++) {
      if(str1[i] == str1[6]) {
       // nfc Serial_Number
       for (uint8_t j = 0; j < uidLength; j++) {
        if (uid[j] <= 0xF);
        Serial.write(uid[j]);
        }
      }
      Serial.write(str1[i]);  
    }
    
    blink(color_green, 2);
    
    _delay_ms(900);
  }
}

void fade() {
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();

  for (int i = 0; i <= 255; i++) {
    pixels.setPixelColor(0, pixels.Color(i, i, i));
    pixels.show();

    _delay_ms(20);
  }

  for (int i = 255; i >= 0; i--) {
    pixels.setPixelColor(0, pixels.Color(i, i, i));
    pixels.show();

    _delay_ms(3);
  }

  pixels.clear();
  pixels.show();
}

void blink(uint32_t color, int count) {
  pixels.clear();
  pixels.show();
  
  for (int i = 0; i < count; i++) {
    pixels.setPixelColor(0, color);
    pixels.show();

    _delay_ms(50);

    pixels.clear();
    pixels.show();

    _delay_ms(50);
  }
}

void flash(uint32_t color, int t) {
  pixels.clear();
  pixels.show();
  
  pixels.setPixelColor(0, color);
  pixels.show();

  _delay_ms(t);

  pixels.clear();
  pixels.show();
}

void ledOn(uint32_t color) {
  pixels.clear();
  pixels.show();
  
  pixels.setPixelColor(0, color);
  pixels.show();
}

void ledOff(void) {
  pixels.clear();
  pixels.show();
}

void beep(int t) {
  tone(BUZZER, t, 60);
  delay(100);
  tone(BUZZER, t, 30);
}

void ring(void) {
  tone(BUZZER, 1567, 1000);
  delay(300);
  tone(BUZZER, 1318, 800);
}
