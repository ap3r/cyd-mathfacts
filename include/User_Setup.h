// User_Setup.h for ESP32-2432S028 (Cheap Yellow Display 2.4")
// Based on working bongo_cat config

#define USER_SETUP_INFO "ESP32-2432S028"

// ##################################################################################
// Display driver
// ##################################################################################
#define ILI9341_2_DRIVER

// ##################################################################################
// Display dimensions
// ##################################################################################
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

// ##################################################################################
// IMPORTANT: Color inversion - required for CYD!
// ##################################################################################
#define TFT_INVERSION_ON

// ##################################################################################
// ESP32 pins - matching bongo_cat working config
// ##################################################################################
#define ESP32_DMA

#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1
#define TFT_BL   27  // IMPORTANT: Backlight is GPIO 27, NOT 21!

// ##################################################################################
// Touch pins - XPT2046 on separate SPI bus
// ##################################################################################
#define TOUCH_CS   33
#define TOUCH_MISO 39
#define TOUCH_MOSI 32
#define TOUCH_CLK  25

// ##################################################################################
// Backlight control
// ##################################################################################
#define TFT_BACKLIGHT_ON HIGH

// ##################################################################################
// Fonts
// ##################################################################################
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

// ##################################################################################
// SPI configuration
// ##################################################################################
#define SPI_FREQUENCY       65000000
#define SPI_READ_FREQUENCY  80000000
#define SPI_TOUCH_FREQUENCY  2500000

// IMPORTANT: Use HSPI for display (pins 12,13,14 are HSPI)
// This frees up VSPI for the touch controller
#define USE_HSPI_PORT
