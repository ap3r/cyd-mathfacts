# Math Facts! - Times Table Quiz

A fun, colorful multiplication quiz game for the ESP32 "Cheap Yellow Display" (CYD) to help kids memorize their times tables!

## Features

- **Times Tables 1-12** - Practice all multiplication facts
- **4 Multiple Choice Answers** - Touch-friendly colorful buttons
- **Confetti Celebrations** - Every correct answer triggers confetti!
- **12 Achievements** - Duolingo-style unlockables to keep kids motivated
- **Streak Tracking** - Build streaks of correct answers
- **Progress Saved** - Stats persist across power cycles

## Quick Install (No Software Required!)

Visit the web flasher page and flash directly from your browser:

**[Flash Math Facts! to Your CYD](https://your-username.github.io/cyd-mathfacts/)**

Requirements:
- ESP32-2432S028 (2.4" Cheap Yellow Display)
- USB data cable (not charge-only)
- Chrome or Edge browser

## Manual Installation

### Using PlatformIO (Recommended)

1. Install [PlatformIO](https://platformio.org/)
2. Clone this repository
3. Open in VS Code with PlatformIO extension
4. Connect your CYD via USB
5. Click Upload or run `pio run -t upload`

### Using Arduino IDE

1. Install ESP32 board support
2. Install libraries:
   - TFT_eSPI
   - XPT2046_Touchscreen
3. Configure TFT_eSPI User_Setup.h for CYD pinout
4. Upload `src/main.cpp`

## Hardware

This project is designed for the **ESP32-2432S028**, commonly known as the "Cheap Yellow Display" or CYD:

- ESP32-WROOM-32 microcontroller
- 2.4" ILI9341 TFT display (320x240)
- XPT2046 resistive touch screen
- USB-C or Micro-USB for power and programming

## Achievements

| Icon | Name | How to Unlock |
|------|------|---------------|
| 1 | First Steps | Answer your first question |
| 5 | Getting Started | Get 5 correct answers |
| 10 | Math Whiz | Get 10 correct answers |
| F | On Fire! | Get a 5 streak |
| U | Unstoppable | Get a 10 streak |
| L | Lightning | Answer in under 2 seconds |
| P | Perfect Round | Get 10/10 in a round |
| T | Table Master | Complete a times table |
| H | Half Way | Complete 6 times tables |
| C | Math Champion | Complete all 12 tables |
| 100 | Century | Get 100 correct answers |
| D | Dedication | Get 50 correct in a row |

## Building

The firmware is automatically built and deployed to GitHub Pages when you push to the main branch. The workflow:

1. Builds the firmware using PlatformIO
2. Extracts bootloader, partition, and firmware binaries
3. Generates the ESP Web Tools manifest
4. Deploys to GitHub Pages

## License

MIT License - Feel free to use, modify, and share!

## Contributing

Contributions welcome! Ideas for improvements:
- Sound effects (CYD has a speaker!)
- Different game modes (speed round, specific tables)
- Parent dashboard showing progress
- More achievement types
