// Driver for addressable RGB LED Strip
#include "Freenove_WS2812_Lib_for_ESP32.h"

#define LEDS_COUNT  60
#define LEDS_PIN	5
#define CHANNEL		0

#define INIT_COM           0x00
#define SINGLE_COLOR_COM   0x01
#define NEW_COLOR_LIST_COM 0x02
#define FADE_EFFECT_COM    0x03
#define CYCLE_EFFECT_COM   0x04

typedef struct {
  byte red;
  byte green;
  byte blue;
} RGB_t;

enum class Effect {NONE, FADE, CYCLE};

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

void setup() {
  strip.begin();
  randomSeed(analogRead(0));
  Serial.begin(115200);
}

byte fade_index = 0;
byte led_cycle_index = 0;
Effect cur_effect = Effect::NONE;
RGB_t intermediate_color = {0, 0, 0};
//RGB_t* fade_colors = new RGB_t[3] { {0x50, 0, 0}, {0, 0x50, 0}, {0, 0, 0x50} };
RGB_t* fade_colors = new RGB_t[1] { {0, 0, 0} };

void loop() {
  if (Serial.available()) {
    byte header_byte = Serial.read();
    switch (header_byte) {
      case INIT_COM:
        strip.begin();
        turn_off_leds();
        cur_effect = Effect::NONE;
        delete[] fade_colors;
        fade_colors = new RGB_t[1] { {0, 0, 0} };
        
        break;
      case SINGLE_COLOR_COM:
        cur_effect = Effect::NONE;
        delete[] fade_colors;
        fade_colors = new RGB_t[1] { get_serial_color() };
        break;
      case NEW_COLOR_LIST_COM:
        read_in_color_list();
        break;
      case FADE_EFFECT_COM:
        turn_off_leds();
        cur_effect = Effect::FADE;
        break;
      case CYCLE_EFFECT_COM:
        turn_off_leds();
        cur_effect = Effect::CYCLE;
      default:
        break;
    }
  }
  do_color_effect();
}

void read_in_color_list() {
  byte data_size = Serial.read();
  delete[] fade_colors;
  fade_colors = new RGB_t[data_size];
  for (int i = 0; i < data_size; i++) {
    fade_colors[i] = get_serial_color();
  }
}

void do_color_effect() {
  switch (cur_effect) {
    case Effect::NONE:
      Serial.println("Doing effect: none");
      do_effect_none();
      break;
    case Effect::FADE:
      Serial.println("Doing effect: Fade");
      do_effect_fade();
      break;
    case Effect::CYCLE:
      Serial.println("Doing effect: cycle");
      do_effect_cycle();
      break;
    default:
      Serial.println("No effect being done. How did you get an invalid effect?");
      break;
  }
}

void do_effect_none() {
  for (int i = 0; i < LEDS_COUNT; i++) {
    Serial.print("Effect colors: ");
    Serial.print(fade_colors[0].red);
    Serial.print(" ");
    Serial.print(fade_colors[0].green);
    Serial.print(" ");
    Serial.println(fade_colors[0].blue);
    strip.setLedColorData(i, fade_colors[0].red, fade_colors[0].green, fade_colors[0].blue);
  }
  strip.show();
}

void do_effect_fade() {
  
}

void do_effect_cycle() {
  RGB_t cur_color = fade_colors[fade_index];
  Serial.print("Cycling color: ");
  Serial.print(cur_color.red);
  Serial.print(" ");
  Serial.print(cur_color.green);
  Serial.print(" ");
  Serial.println(cur_color.blue);
  Serial.println(led_cycle_index);
  strip.setLedColorData(led_cycle_index, cur_color.red, cur_color.green, cur_color.blue);
  if (led_cycle_index >= LEDS_COUNT) {
    led_cycle_index = -1;
    fade_index = (fade_index + 1) % (sizeof(fade_colors) - 1);
  }
  led_cycle_index += 1;
  strip.show();
  delay(10);
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

void turn_off_leds() {
  for (int i = 0; i < LEDS_COUNT; i++) {
    strip.setLedColorData(i, 0, 0, 0);
  }
  strip.show();
}

void set_solid_color(RGB_t color) {
  for (int i = 0; i < LEDS_COUNT; i++) {
    strip.setLedColorData(i, color.red, color.green, color.blue);
  }
  strip.show();
}
