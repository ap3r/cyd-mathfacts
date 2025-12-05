// User_Setup.h for ESP32-2432S028 (Cheap Yellow Display 2.4")
// This file configures the TFT_eSPI library for the CYD board

#define USER_SETUP_INFO "ESP32-2432S028"

// ##################################################################################
// Display driver - ILI9341 for CYD v1/v2 (use ST7789_DRIVER for v3)
// ##################################################################################
#define ILI9341_2_DRIVER

// ##################################################################################
// Display dimensions
// ##################################################################################
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// ##################################################################################
// ESP32 pins used for the display
// ##################################################################################
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST  -1  // Reset pin (connected to RST on board)
#define TFT_BL   21  // LED back-light (HIGH to turn on)

// ##################################################################################
// Touch screen chip select (directly configured to work with XPT2046)
// ##################################################################################
#define TOUCH_CS 33

// ##################################################################################
// Backlight control
// ##################################################################################
#define TFT_BACKLIGHT_ON HIGH

// ##################################################################################
// Fonts to be available
// ##################################################################################
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

// ##################################################################################
// SPI configuration
// ##################################################################################
#define SPI_FREQUENCY       40000000  // 40 MHz - stable for CYD
#define SPI_READ_FREQUENCY  16000000  // 16 MHz for reading
#define SPI_TOUCH_FREQUENCY  2500000  // 2.5 MHz for touch

// Use hardware SPI
#define USE_HSPI_PORT
