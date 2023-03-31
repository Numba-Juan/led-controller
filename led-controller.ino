// Driver for addressable RGB LED Strip
#include "Freenove_WS2812_Lib_for_ESP32.h"

// Defines for used pins and ADC channels
#define LEDS_COUNT  60
#define LEDS_PIN	  5
#define LED_CHANNEL 0

#define MIC_PIN     36

#define BUTTON_PIN  34

// Define bytes used for communication packets
#define INIT_COM           0x00
#define SINGLE_COLOR_COM   0x01
#define NEW_COLOR_LIST_COM 0x02
#define FADE_EFFECT_COM    0x03
#define CYCLE_EFFECT_COM   0x04

// Container for RGB bytes
typedef struct {
  byte red;
  byte green;
  byte blue;
} RGB_t;

// Available effects
enum class Effect {NONE, FADE, CYCLE};

// Initialize LED strip
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, LED_CHANNEL, TYPE_GRB);

// Setup pins, interrupt, and serial communication
void setup() {
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), change_effect_button, FALLING);
  strip.begin();
  Serial.begin(115200);
}

// Globals for indicies for incrementing in main loop
byte fade_index = 0;
byte fade_map_index = 0;
byte led_cycle_index = 0;

// Store current effect
Effect cur_effect = Effect::NONE;

// Store available colors
byte num_fade_colors = 1;
RGB_t* fade_colors = new RGB_t[num_fade_colors] { {0, 50, 0} };

// Main program loop
void loop() {
  // Check if there is anything in the serial buffer
  if (Serial.available()) {
    byte header_byte = Serial.read();
    switch (header_byte) {
      // Reinitialize the LED strip  
      case INIT_COM:
        strip.begin();
        turn_off_leds();
        cur_effect = Effect::NONE;
        delete[] fade_colors;
        fade_colors = new RGB_t[1] { {0, 0, 0} };
        num_fade_colors = 1;
        break;
      // Change to a single color
      case SINGLE_COLOR_COM:
        cur_effect = Effect::NONE;
        delete[] fade_colors;
        fade_colors = new RGB_t[1] { get_serial_color() };
        num_fade_colors = 1;
        break;
      // Give new list of colors to apply to the effect
      case NEW_COLOR_LIST_COM:
        read_in_color_list();
        break;
      // Change to the fade effect
      case FADE_EFFECT_COM:
        turn_off_leds();
        cur_effect = Effect::FADE;
        break;
      // Change to the cycle effect
      case CYCLE_EFFECT_COM:
        turn_off_leds();
        cur_effect = Effect::CYCLE;
      default:
        break;
    }
  }
  // Apply the current color effect
  do_color_effect();
}

// Interrupt handler for push button
void change_effect_button() {
  // Cycle between effects in the enum class for effects
  switch (cur_effect) {
    case Effect::NONE:
      cur_effect = Effect::CYCLE;
      break;
    case Effect::CYCLE:
      cur_effect = Effect::FADE;
      break;
    case Effect::FADE:
      cur_effect = Effect::NONE;
      break;
  }
  // Debounce switch
  delayMicroseconds(50000);
}

// Read a new list of colors from the serial buffer
void read_in_color_list() {
  // Get the variable packet size
  num_fade_colors = Serial.read();
  // Overwrite the previous colors
  delete[] fade_colors;
  fade_colors = new RGB_t[num_fade_colors];
  for (int i = 0; i < num_fade_colors; i++) {
    fade_colors[i] = get_serial_color();
  }
}

// Apply the current selected color effect
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

// Show the first color in the color list
void do_effect_none() {
  Serial.print("Effect colors: ");
  Serial.print(fade_colors[0].red);
  Serial.print(" ");
  Serial.print(fade_colors[0].green);
  Serial.print(" ");
  Serial.println(fade_colors[0].blue);
  for (int i = 0; i < LEDS_COUNT; i++) {
    strip.setLedColorData(i, fade_colors[0].red, fade_colors[0].green, fade_colors[0].blue);
  }
  strip.show();
  delay(10);
}

// Fade between all colors in the list using linear interpolation
void do_effect_fade() {
  fade_map_index++;
  if (fade_map_index > 100) {
    fade_map_index = 0;
    fade_index = (fade_index + 1) % num_fade_colors;
  }
  byte target_fade = (fade_index + 1) % num_fade_colors;
  // Linear interpolation for fading colors
  byte r = map(fade_map_index, 0, 100, fade_colors[fade_index].red, fade_colors[target_fade].red);
  byte g = map(fade_map_index, 0, 100, fade_colors[fade_index].green, fade_colors[target_fade].green);
  byte b = map(fade_map_index, 0, 100, fade_colors[fade_index].blue, fade_colors[target_fade].blue);
  volume_based_lighting(r, g, b);
  strip.show();
  delay(10);
}

// Use analog input to change the number of LEDs lit up
void volume_based_lighting(byte r, byte g, byte b) {
  int mic_amplitude = analogRead(MIC_PIN);
  Serial.println(mic_amplitude);
  // Get number of LEDs to light up
  byte length_from_mid = map(mic_amplitude, 0, 4095, 1, LEDS_COUNT / 2);
  for (int i = 0; i < LEDS_COUNT; i++) {
     strip.setLedColorData(i, 0, 0, 0);
  }
  for (int i = (LEDS_COUNT / 2) - length_from_mid; i < (LEDS_COUNT / 2) + length_from_mid; i++) {
    strip.setLedColorData(i, r, g, b);
  }
  delay(2);
}

// Cycle between the colors one LED at a time
void do_effect_cycle() {
  RGB_t cur_color = fade_colors[fade_index];
  Serial.print("Cycling color: ");
  Serial.print(cur_color.red);
  Serial.print(" ");
  Serial.print(cur_color.green);
  Serial.print(" ");
  Serial.println(cur_color.blue);
  Serial.println(led_cycle_index);
  if (led_cycle_index >= LEDS_COUNT) {
    led_cycle_index = -1;
    fade_index = (fade_index + 1) % num_fade_colors;
  }
  strip.setLedColorData(led_cycle_index, cur_color.red, cur_color.green, cur_color.blue);
  led_cycle_index += 1;
  strip.show();
  delay(10);
}

// Get the next three byte from the serial buffer and return a packed RGB value
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

// Turn off every LED
void turn_off_leds() {
  for (int i = 0; i < LEDS_COUNT; i++) {
    strip.setLedColorData(i, 0, 0, 0);
  }
  strip.show();
}

// Show the same color on all LEDs
void set_solid_color(RGB_t color) {
  for (int i = 0; i < LEDS_COUNT; i++) {
    strip.setLedColorData(i, color.red, color.green, color.blue);
  }
  strip.show();
}
