#include <Arduino.h>
#include <Wire.h>  
#define LGFX_ESP32_S3_BOX_V3
#include <LGFX_AUTODETECT.hpp>
#include <LovyanGFX.hpp>

static LGFX lcd;

// RGB LED pins
#define RGB_RED   39
#define RGB_GREEN 40
#define RGB_BLUE  41

// Display settings
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// UI Layout Constants
#define HEADER_HEIGHT 50
#define BUTTON_MARGIN 15
#define BUTTON_ROWS 2
#define BUTTON_COLS 3

// Calculate button dimensions
#define BUTTON_WIDTH  ((SCREEN_WIDTH - (BUTTON_MARGIN * (BUTTON_COLS + 1))) / BUTTON_COLS)
#define BUTTON_HEIGHT ((SCREEN_HEIGHT - HEADER_HEIGHT - (BUTTON_MARGIN * (BUTTON_ROWS + 1))) / BUTTON_ROWS)

// Toggle button settings
#define TOGGLE_WIDTH 70
#define TOGGLE_HEIGHT 40
#define TOGGLE_X (SCREEN_WIDTH - TOGGLE_WIDTH - 15)
#define TOGGLE_Y ((HEADER_HEIGHT - TOGGLE_HEIGHT) / 2)

// Color definitions
#define COLOR_BG      0x1082
#define COLOR_HEADER  0x2124
#define COLOR_WHITE   0xFFFF
#define COLOR_BLACK   0x0000
#define COLOR_GOLD    0xFFE0

// State variables
struct ColorRGB {
  uint8_t r, g, b;
};

struct TouchPoint {
  int16_t x, y;
  bool touched;
};

// 6 predefined colors using LovyanGFX system colors
ColorRGB colorButtons[6] = {
  {255, 0, 0},    // RED
  {0, 255, 0},    // GREEN  
  {0, 0, 255},    // BLUE
  {255, 255, 150}, // WHITE
  {255, 150, 0},  // YELLOW
  {255, 0, 255}   // PURPLE
};

String colorNames[6] = {"RED", "GREEN", "BLUE", "WHITE", "YELLOW", "PURPLE"};
uint32_t systemColors[6] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_WHITE, TFT_YELLOW, TFT_MAGENTA};

bool ledOn = true;
int selectedColor = 0; // Default to RED
TouchPoint lastTouch = {0, 0, false};

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("Simple RGB LED Controller Starting...");
  
  // Initialize display
  Serial.println("Initializing display...");
  lcd.init();
  lcd.setBrightness(255);
  
  // Test display
  lcd.clear(TFT_BLACK);
  lcd.setFont(&fonts::FreeSans12pt7b);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextDatum(middle_center);
  lcd.drawString("RGB Controller", SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
  delay(2000);
  
  // Initialize RGB LED pins
  Serial.println("Initializing RGB LED...");
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);
  
  // Turn off LED initially
  setLEDColor(0, 0, 0);
  
  // Draw UI
  drawUI();
  updateLED();
  
  Serial.println("Setup complete!");
}

void loop() {
  TouchPoint touch = readTouch();
  
  if (touch.touched && !lastTouch.touched) {
    handleTouch(touch.x, touch.y);
  }
  
  lastTouch = touch;
  delay(50);
}

TouchPoint readTouch() {
  TouchPoint point = {0, 0, false};
  
  uint16_t x, y;
  if (lcd.getTouch(&x, &y)) {
    point.x = x;
    point.y = y;
    point.touched = true;
  }
  
  return point;
}

void drawUI() {
  // Clear screen
  lcd.clear(COLOR_BG);
  
  // Draw header
  drawHeader();
  
  // Draw color buttons
  drawColorButtons();
}

void drawHeader() {
  // Header background
  lcd.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_HEADER);
  
  // Title
  lcd.setFont(&fonts::Font4);
  lcd.setTextColor(COLOR_WHITE);
  lcd.setTextDatum(middle_left);
  lcd.drawString("RGB LED Control", 15, HEADER_HEIGHT/2);
  
  // ON/OFF Toggle
  drawToggle();
}

void drawToggle() {
  uint32_t sliderX = ledOn ? TOGGLE_X + TOGGLE_WIDTH - 37 : TOGGLE_X + 3;

  // Toggle background - GREEN when ON, RED when OFF
  if (ledOn) {
    lcd.fillRoundRect(TOGGLE_X, TOGGLE_Y, TOGGLE_WIDTH, TOGGLE_HEIGHT, TOGGLE_HEIGHT/2, TFT_GREEN);
  } else {
    lcd.fillRoundRect(TOGGLE_X, TOGGLE_Y, TOGGLE_WIDTH, TOGGLE_HEIGHT, TOGGLE_HEIGHT/2, TFT_RED);
  }
  
  // Toggle slider
  lcd.fillCircle(sliderX + 17, TOGGLE_Y + TOGGLE_HEIGHT/2, 17, COLOR_WHITE);
  
  // Add shadow effect
  lcd.drawCircle(sliderX + 17, TOGGLE_Y + TOGGLE_HEIGHT/2, 17, COLOR_BLACK);
}

void drawColorButtons() {
  for (int i = 0; i < 6; i++) {
    int row = i / BUTTON_COLS;
    int col = i % BUTTON_COLS;
    
    int x = BUTTON_MARGIN + col * (BUTTON_WIDTH + BUTTON_MARGIN);
    int y = HEADER_HEIGHT + BUTTON_MARGIN + row * (BUTTON_HEIGHT + BUTTON_MARGIN);
    
    drawColorButton(i, x, y);
  }
}

void drawColorButton(int colorIndex, int x, int y) {
  // Draw border with layered effect
  if (colorIndex == selectedColor) {
    // Selected - 6px silver border only
    for (int i = 0; i < 6; i++) {
      lcd.drawRoundRect(x - i, y - i, BUTTON_WIDTH + 2*i, BUTTON_HEIGHT + 2*i, 12, TFT_SILVER);
    }
  } else {
    // Non-selected - 2px silver inner + 4px black outer
    // First draw 2px Silver inner border
    for (int i = 0; i < 2; i++) {
      lcd.drawRoundRect(x - i, y - i, BUTTON_WIDTH + 2*i, BUTTON_HEIGHT + 2*i, 12, TFT_SILVER);
    }
    // Then draw 2px Black outer border  
    for (int i = 2; i < 6; i++) {
      lcd.drawRoundRect(x - i, y - i, BUTTON_WIDTH + 2*i, BUTTON_HEIGHT + 2*i, 12, TFT_BLACK);
    }
  }
  
  // Draw button background
  if (colorIndex == 0) {
    // RED
    lcd.fillRoundRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, 12, TFT_RED);
  } else if (colorIndex == 1) {
    // GREEN
    lcd.fillRoundRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, 12, TFT_GREEN);
  } else if (colorIndex == 2) {
    // BLUE
    lcd.fillRoundRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, 12, TFT_BLUE);
  } else if (colorIndex == 3) {
    // WHITE
    lcd.fillRoundRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, 12, TFT_WHITE);
  } else if (colorIndex == 4) {
    // YELLOW
    lcd.fillRoundRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, 12, TFT_YELLOW);
  } else if (colorIndex == 5) {
    // PURPLE/MAGENTA
    lcd.fillRoundRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, 12, TFT_MAGENTA);
  }
  
  
  lcd.setFont(&fonts::FreeSansBold9pt7b);if (colorIndex == 3 || colorIndex == 4) {
    // Black text for WHITE and YELLOW
    lcd.setTextColor(TFT_BLACK);
  } else {
    // White text for all other colors
    lcd.setTextColor(TFT_WHITE);
  }
  lcd.setTextDatum(middle_center);
  lcd.drawString(colorNames[colorIndex], x + BUTTON_WIDTH/2, y + BUTTON_HEIGHT/2);
  
}

void handleTouch(int x, int y) {
  Serial.printf("Touch detected: X=%d, Y=%d\n", x, y);
  
  // Check if toggle was touched
  if (x >= TOGGLE_X && x <= TOGGLE_X + TOGGLE_WIDTH && 
      y >= TOGGLE_Y && y <= TOGGLE_Y + TOGGLE_HEIGHT) {
    ledOn = !ledOn;
    drawToggle();
    updateLED();
    Serial.println(ledOn ? "LED turned ON" : "LED turned OFF");
    return;
  }
  
  // Check if any color button was touched
  for (int i = 0; i < 6; i++) {
    int row = i / BUTTON_COLS;
    int col = i % BUTTON_COLS;
    
    int buttonX = BUTTON_MARGIN + col * (BUTTON_WIDTH + BUTTON_MARGIN);
    int buttonY = HEADER_HEIGHT + BUTTON_MARGIN + row * (BUTTON_HEIGHT + BUTTON_MARGIN);
    
    if (x >= buttonX && x <= buttonX + BUTTON_WIDTH && 
        y >= buttonY && y <= buttonY + BUTTON_HEIGHT) {
      
      // Update selection
      selectedColor = i;
      
      // Redraw ALL color buttons to ensure proper border clearing
      drawColorButtons();
      
      // Update LED
      updateLED();
      
      Serial.printf("Color selected: %s (R:%d G:%d B:%d)\n", 
                   colorNames[i].c_str(), 
                   colorButtons[i].r, 
                   colorButtons[i].g, 
                   colorButtons[i].b);
      return;
    }
  }
}

void updateLED() {
  if (ledOn) {
    ColorRGB color = colorButtons[selectedColor];
    setLEDColor(color.r, color.g, color.b);
  } else {
    setLEDColor(0, 0, 0);
  }
}

void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  // Common cathode RGB LED
  analogWrite(RGB_RED, r);
  analogWrite(RGB_GREEN, g);
  analogWrite(RGB_BLUE, b);
}