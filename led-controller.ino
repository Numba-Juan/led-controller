// Driver for addressable RGB LED Strip
#include "Freenove_WS2812_Lib_for_ESP32.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define LEDS_COUNT  60
#define LEDS_PIN	5
#define CHANNEL		0

typedef struct {
  byte red;
  byte green;
  byte blue;
} RGB_t;

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

void setup() {
  strip.begin();
  strip.setBrightness(10);
  randomSeed(analogRead(0));
  Serial.begin(115200);
  set_solid_color(0, 0, 0);
}

byte fade_index = 0;
RGB_t* fade_colors = null;

void loop() {
  if (Serial.available() >= 3) {
    RGB_t new_color = get_serial_color();
    set_solid_color(new_color);
  }
}



void set_solid_color(byte r, byte g, byte b) {
  for (int i = 0; i < LEDS_COUNT; i++) {
    strip.setLedColorData(i, r, g , b);
  }
  strip.show();
}

RGB_t get_serial_color() {
  byte temp_buf[3];
  Serial.readBytes(temp_buf, 3);
  Serial.write("Red:");
  Serial.print(temp_buf[0]);
  Serial.write(", Green:");
  Serial.print(temp_buf[1]);
  Serial.write(", Blue:");
  Serial.print(temp_buf[2]);
  RGB_t color = {temp_buf[0], temp_buf[1], temp_buf[2]};
  return color;
}

void set_solid_color(RGB_t color) {
  for (int i = 0; i < LEDS_COUNT; i++) {
    strip.setLedColorData(i, color.red, color.green, color.blue);
  }
  strip.show();
}
