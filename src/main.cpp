/*
 * Times Table Quiz - ESP32 Cheap Yellow Display (CYD)
 *
 * A fun, colorful multiplication quiz game for kids!
 * Features:
 * - Times tables 1-12
 * - 4 multiple choice answers
 * - Confetti celebrations
 * - Duolingo-style achievements
 * - Streak tracking
 * - Progress visualization
 *
 * Hardware: ESP32-2432S028 (2.4" CYD)
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <Preferences.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// CYD 2.4" Touch pins (directly defined, not from build flags)
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Touch calibration for CYD 2.4" (landscape mode)
#define TOUCH_MIN_X 200
#define TOUCH_MAX_X 3700
#define TOUCH_MIN_Y 300
#define TOUCH_MAX_Y 3800

// Display dimensions
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Game settings
#define MIN_TABLE 1
#define MAX_TABLE 12
#define ANSWERS_COUNT 4

// Backlight pin - GPIO 27 for CYD (not 21!)
#define TFT_BACKLIGHT 27

// ============================================================================
// COLOR PALETTE - Bright, kid-friendly colors!
// ============================================================================

// Main colors
#define COLOR_BG         0x1082  // Dark blue background
#define COLOR_BG_LIGHT   0x2104  // Slightly lighter blue
#define COLOR_WHITE      0xFFFF
#define COLOR_BLACK      0x0000

// Rainbow colors for confetti and effects
#define COLOR_RED        0xF800
#define COLOR_ORANGE     0xFD20
#define COLOR_YELLOW     0xFFE0
#define COLOR_GREEN      0x07E0
#define COLOR_CYAN       0x07FF
#define COLOR_BLUE       0x001F
#define COLOR_PURPLE     0x780F
#define COLOR_PINK       0xF81F
#define COLOR_MAGENTA    0xF81F

// UI Colors
#define COLOR_CORRECT    0x07E0  // Green
#define COLOR_WRONG      0xF800  // Red
#define COLOR_GOLD       0xFEA0  // Gold for achievements
#define COLOR_SILVER     0xC618  // Silver
#define COLOR_BRONZE     0xBC40  // Bronze

// Button colors (gradient effect)
#define COLOR_BTN_1      0x03FF  // Cyan-ish
#define COLOR_BTN_2      0x07FF  // Cyan
#define COLOR_BTN_3      0xFFE0  // Yellow
#define COLOR_BTN_4      0xFD20  // Orange

// ============================================================================
// OBJECTS
// ============================================================================

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);
Preferences prefs;

// ============================================================================
// GAME STATE
// ============================================================================

enum GameScreen {
    SCREEN_SPLASH,
    SCREEN_MENU,
    SCREEN_QUIZ,
    SCREEN_RESULT,
    SCREEN_ACHIEVEMENT,
    SCREEN_STATS
};

struct Question {
    int num1;
    int num2;
    int correctAnswer;
    int answers[ANSWERS_COUNT];
    int correctIndex;
};

struct GameStats {
    int totalCorrect;
    int totalWrong;
    int currentStreak;
    int bestStreak;
    int perfectRounds;
    int questionsThisRound;
    int correctThisRound;
    unsigned long fastestAnswer;  // in milliseconds
    int tablesCompleted;          // bitmask for tables 1-12
};

struct Achievement {
    const char* name;
    const char* icon;
    const char* description;
    bool unlocked;
};

// Global state
GameScreen currentScreen = SCREEN_SPLASH;
Question currentQuestion;
GameStats stats = {0};
unsigned long questionStartTime = 0;
unsigned long lastTouchTime = 0;
int selectedAnswer = -1;
bool showingFeedback = false;
unsigned long feedbackStartTime = 0;

// Confetti particles
#define MAX_CONFETTI 50
struct Confetti {
    float x, y;
    float vx, vy;
    uint16_t color;
    bool active;
    int size;
};
Confetti confetti[MAX_CONFETTI];
bool confettiActive = false;
unsigned long confettiStartTime = 0;

// Star animation for achievements
#define MAX_STARS 20
struct Star {
    float x, y;
    float angle;
    float speed;
    int size;
    uint16_t color;
    bool active;
};
Star stars[MAX_STARS];

// Achievement definitions
#define NUM_ACHIEVEMENTS 12
Achievement achievements[NUM_ACHIEVEMENTS] = {
    {"First Steps", "1", "Answer your first question!", false},
    {"Getting Started", "5", "Get 5 correct answers!", false},
    {"Math Whiz", "10", "Get 10 correct answers!", false},
    {"On Fire!", "F", "Get a 5 streak!", false},
    {"Unstoppable", "U", "Get a 10 streak!", false},
    {"Lightning", "L", "Answer in under 2 seconds!", false},
    {"Perfect Round", "P", "Get 10/10 in a round!", false},
    {"Table Master", "T", "Complete a full times table!", false},
    {"Half Way", "H", "Complete 6 times tables!", false},
    {"Math Champion", "C", "Complete all 12 times tables!", false},
    {"Century", "100", "Get 100 correct answers!", false},
    {"Dedication", "D", "Get 50 correct in a row!", false}
};

// Rainbow colors array for effects
const uint16_t rainbowColors[] = {
    COLOR_RED, COLOR_ORANGE, COLOR_YELLOW, COLOR_GREEN,
    COLOR_CYAN, COLOR_BLUE, COLOR_PURPLE, COLOR_PINK
};
#define NUM_RAINBOW_COLORS 8

// Button colors for answer options
const uint16_t buttonColors[] = {
    0x03EF,  // Teal
    0x07FF,  // Cyan
    0xFFE0,  // Yellow
    0xFD20   // Orange
};

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void drawSplashScreen();
void drawMenuScreen();
void drawQuizScreen();
void drawResultScreen(bool correct);
void drawAchievementPopup(int achievementIndex);
void drawStatsScreen();

void generateQuestion();
void checkAnswer(int answerIndex);
void checkAchievements();
void saveStats();
void loadStats();

void initConfetti();
void updateConfetti();
void drawConfetti();
void startConfetti();

void initStars();
void updateStars();
void drawStars();

bool getTouchPoint(int &x, int &y);
void handleTouch(int x, int y);
void drawButton(int x, int y, int w, int h, uint16_t color, const char* text, int textSize);
void drawRoundedRect(int x, int y, int w, int h, int r, uint16_t color);
void fillRoundedRect(int x, int y, int w, int h, int r, uint16_t color);
void drawProgressBar(int x, int y, int w, int h, int value, int maxVal, uint16_t color);
void drawCenteredText(const char* text, int y, int size, uint16_t color);
void animateCorrect();
void animateWrong();
uint16_t dimColor(uint16_t color, float factor);

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=== Times Table Quiz ===");

    // Initialize backlight on GPIO 27
    pinMode(TFT_BACKLIGHT, OUTPUT);
    digitalWrite(TFT_BACKLIGHT, HIGH);
    Serial.println("Backlight ON (GPIO 27)");

    // Initialize display
    tft.init();
    tft.setRotation(1);  // Landscape mode

    // Print TFT_eSPI driver info for debugging
    Serial.print("TFT_eSPI ver: ");
    Serial.println(TFT_ESPI_VERSION);
    Serial.print("Display driver: ");
    #if defined(ILI9341_DRIVER) || defined(ILI9341_2_DRIVER)
    Serial.println("ILI9341");
    #elif defined(ST7789_DRIVER)
    Serial.println("ST7789");
    #else
    Serial.println("Unknown");
    #endif
    Serial.printf("Display size: %d x %d\n", tft.width(), tft.height());

    tft.fillScreen(COLOR_BG);
    Serial.println("Display initialized");

    // Quick test - draw something visible
    tft.fillScreen(TFT_BLUE);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(50, 100);
    tft.println("Initializing...");
    delay(500);

    // Initialize touch on VSPI with correct pins
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);
    Serial.println("Touch initialized");

    // Initialize random seed
    randomSeed(analogRead(34) + millis());

    // Load saved stats
    loadStats();

    // Initialize effects
    initConfetti();
    initStars();

    // Show splash screen
    currentScreen = SCREEN_SPLASH;
    tft.fillScreen(COLOR_BG);
    drawSplashScreen();

    Serial.println("Setup complete!");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    static unsigned long lastUpdate = 0;
    unsigned long now = millis();

    // Update effects at 60fps
    if (now - lastUpdate >= 16) {
        lastUpdate = now;

        if (confettiActive) {
            updateConfetti();
            drawConfetti();

            // Stop confetti after 3 seconds
            if (now - confettiStartTime > 3000) {
                confettiActive = false;
                // Redraw current screen
                if (currentScreen == SCREEN_QUIZ) {
                    drawQuizScreen();
                }
            }
        }

        if (currentScreen == SCREEN_ACHIEVEMENT) {
            updateStars();
            drawStars();
        }
    }

    // Handle feedback timeout
    if (showingFeedback && now - feedbackStartTime > 1500) {
        showingFeedback = false;

        // Check if we should show achievement
        static int pendingAchievement = -1;
        for (int i = 0; i < NUM_ACHIEVEMENTS; i++) {
            if (achievements[i].unlocked && pendingAchievement != i) {
                // Check if this was just unlocked
                pendingAchievement = i;
                currentScreen = SCREEN_ACHIEVEMENT;
                drawAchievementPopup(i);
                initStars();
                return;
            }
        }

        // Generate next question
        generateQuestion();
        drawQuizScreen();
    }

    // Handle touch
    int touchX, touchY;
    if (getTouchPoint(touchX, touchY)) {
        // Debounce
        if (now - lastTouchTime > 300) {
            lastTouchTime = now;
            handleTouch(touchX, touchY);
        }
    }
}

// ============================================================================
// TOUCH HANDLING
// ============================================================================

bool getTouchPoint(int &x, int &y) {
    // Check if IRQ indicates touch
    if (!touch.tirqTouched()) {
        return false;
    }

    // Verify actually touched
    if (!touch.touched()) {
        return false;
    }

    TS_Point p = touch.getPoint();

    // Filter out invalid readings (z pressure too low or coords at 0)
    if (p.z < 400 || (p.x == 0 && p.y == 0)) {
        return false;
    }

    // Filter out out-of-range values
    if (p.x < 100 || p.x > 4000 || p.y < 100 || p.y > 4000) {
        return false;
    }

    // Map touch coordinates to screen (landscape)
    x = map(p.x, TOUCH_MIN_X, TOUCH_MAX_X, 0, SCREEN_WIDTH);
    y = map(p.y, TOUCH_MIN_Y, TOUCH_MAX_Y, 0, SCREEN_HEIGHT);

    // Clamp to screen bounds
    x = constrain(x, 0, SCREEN_WIDTH - 1);
    y = constrain(y, 0, SCREEN_HEIGHT - 1);

    return true;
}

void handleTouch(int x, int y) {
    Serial.printf("Touch at (%d, %d) - Screen: %d\n", x, y, currentScreen);

    switch (currentScreen) {
        case SCREEN_SPLASH:
            currentScreen = SCREEN_MENU;
            drawMenuScreen();
            break;

        case SCREEN_MENU:
            // "Play" button area (center of screen)
            if (y >= 80 && y <= 160) {
                stats.questionsThisRound = 0;
                stats.correctThisRound = 0;
                generateQuestion();
                currentScreen = SCREEN_QUIZ;
                drawQuizScreen();
            }
            // "Stats" button area (bottom)
            else if (y >= 180 && y <= 230) {
                currentScreen = SCREEN_STATS;
                drawStatsScreen();
            }
            break;

        case SCREEN_QUIZ:
            if (!showingFeedback) {
                // Check which answer button was pressed
                // Buttons are in 2x2 grid
                int btnWidth = 145;
                int btnHeight = 55;
                int startX = 10;
                int startY = 130;
                int gapX = 10;
                int gapY = 10;

                for (int i = 0; i < 4; i++) {
                    int col = i % 2;
                    int row = i / 2;
                    int btnX = startX + col * (btnWidth + gapX);
                    int btnY = startY + row * (btnHeight + gapY);

                    if (x >= btnX && x <= btnX + btnWidth &&
                        y >= btnY && y <= btnY + btnHeight) {
                        checkAnswer(i);
                        break;
                    }
                }
            }
            break;

        case SCREEN_RESULT:
            currentScreen = SCREEN_QUIZ;
            generateQuestion();
            drawQuizScreen();
            break;

        case SCREEN_ACHIEVEMENT:
            currentScreen = SCREEN_QUIZ;
            generateQuestion();
            drawQuizScreen();
            break;

        case SCREEN_STATS:
            // Back button (top left area)
            if (x < 80 && y < 50) {
                currentScreen = SCREEN_MENU;
                drawMenuScreen();
            }
            break;
    }
}

// ============================================================================
// QUESTION GENERATION
// ============================================================================

void generateQuestion() {
    // Random numbers from 1-12
    currentQuestion.num1 = random(MIN_TABLE, MAX_TABLE + 1);
    currentQuestion.num2 = random(MIN_TABLE, MAX_TABLE + 1);
    currentQuestion.correctAnswer = currentQuestion.num1 * currentQuestion.num2;

    // Generate wrong answers that are plausible
    int wrongAnswers[10];
    int wrongCount = 0;

    // Add some near-miss answers
    if (currentQuestion.correctAnswer > 1) {
        wrongAnswers[wrongCount++] = currentQuestion.correctAnswer - 1;
        wrongAnswers[wrongCount++] = currentQuestion.correctAnswer + 1;
    }
    if (currentQuestion.correctAnswer > 2) {
        wrongAnswers[wrongCount++] = currentQuestion.correctAnswer - 2;
        wrongAnswers[wrongCount++] = currentQuestion.correctAnswer + 2;
    }

    // Add answers from adjacent tables
    wrongAnswers[wrongCount++] = currentQuestion.num1 * (currentQuestion.num2 + 1);
    wrongAnswers[wrongCount++] = currentQuestion.num1 * (currentQuestion.num2 - 1);
    wrongAnswers[wrongCount++] = (currentQuestion.num1 + 1) * currentQuestion.num2;
    wrongAnswers[wrongCount++] = (currentQuestion.num1 - 1) * currentQuestion.num2;

    // Shuffle wrong answers
    for (int i = wrongCount - 1; i > 0; i--) {
        int j = random(0, i + 1);
        int temp = wrongAnswers[i];
        wrongAnswers[i] = wrongAnswers[j];
        wrongAnswers[j] = temp;
    }

    // Place correct answer randomly
    currentQuestion.correctIndex = random(0, ANSWERS_COUNT);

    // Fill answer array
    int wrongIdx = 0;
    for (int i = 0; i < ANSWERS_COUNT; i++) {
        if (i == currentQuestion.correctIndex) {
            currentQuestion.answers[i] = currentQuestion.correctAnswer;
        } else {
            // Make sure we don't duplicate answers
            int answer;
            bool valid;
            do {
                valid = true;
                answer = wrongAnswers[wrongIdx++ % wrongCount];

                // Check it's not the correct answer
                if (answer == currentQuestion.correctAnswer) valid = false;
                if (answer <= 0) valid = false;

                // Check it's not already used
                for (int j = 0; j < i; j++) {
                    if (currentQuestion.answers[j] == answer) valid = false;
                }
            } while (!valid && wrongIdx < 20);

            currentQuestion.answers[i] = answer;
        }
    }

    questionStartTime = millis();
    stats.questionsThisRound++;

    Serial.printf("Question: %d x %d = %d (index %d)\n",
                  currentQuestion.num1, currentQuestion.num2,
                  currentQuestion.correctAnswer, currentQuestion.correctIndex);
}

// ============================================================================
// ANSWER CHECKING
// ============================================================================

void checkAnswer(int answerIndex) {
    unsigned long answerTime = millis() - questionStartTime;
    bool correct = (answerIndex == currentQuestion.correctIndex);

    showingFeedback = true;
    feedbackStartTime = millis();

    if (correct) {
        stats.totalCorrect++;
        stats.currentStreak++;
        stats.correctThisRound++;

        if (stats.currentStreak > stats.bestStreak) {
            stats.bestStreak = stats.currentStreak;
        }

        if (answerTime < stats.fastestAnswer || stats.fastestAnswer == 0) {
            stats.fastestAnswer = answerTime;
        }

        // Mark this table as practiced
        int tableNum = min(currentQuestion.num1, currentQuestion.num2);
        stats.tablesCompleted |= (1 << tableNum);

        // Check for perfect round
        if (stats.questionsThisRound >= 10 && stats.correctThisRound == stats.questionsThisRound) {
            stats.perfectRounds++;
        }

        animateCorrect();
        startConfetti();
    } else {
        stats.totalWrong++;
        stats.currentStreak = 0;
        animateWrong();
    }

    checkAchievements();
    saveStats();

    // Draw result feedback on screen
    drawResultScreen(correct);
}

// ============================================================================
// ACHIEVEMENTS
// ============================================================================

void checkAchievements() {
    // First Steps - Answer first question
    if (stats.totalCorrect >= 1 && !achievements[0].unlocked) {
        achievements[0].unlocked = true;
    }

    // Getting Started - 5 correct
    if (stats.totalCorrect >= 5 && !achievements[1].unlocked) {
        achievements[1].unlocked = true;
    }

    // Math Whiz - 10 correct
    if (stats.totalCorrect >= 10 && !achievements[2].unlocked) {
        achievements[2].unlocked = true;
    }

    // On Fire - 5 streak
    if (stats.currentStreak >= 5 && !achievements[3].unlocked) {
        achievements[3].unlocked = true;
    }

    // Unstoppable - 10 streak
    if (stats.currentStreak >= 10 && !achievements[4].unlocked) {
        achievements[4].unlocked = true;
    }

    // Lightning - Under 2 seconds
    if (stats.fastestAnswer > 0 && stats.fastestAnswer < 2000 && !achievements[5].unlocked) {
        achievements[5].unlocked = true;
    }

    // Perfect Round - 10/10
    if (stats.perfectRounds >= 1 && !achievements[6].unlocked) {
        achievements[6].unlocked = true;
    }

    // Table Master - Complete one table (answer all 12 questions)
    if (stats.tablesCompleted > 0 && !achievements[7].unlocked) {
        achievements[7].unlocked = true;
    }

    // Half Way - 6 tables
    int tablesCount = 0;
    for (int i = 1; i <= 12; i++) {
        if (stats.tablesCompleted & (1 << i)) tablesCount++;
    }
    if (tablesCount >= 6 && !achievements[8].unlocked) {
        achievements[8].unlocked = true;
    }

    // Math Champion - All 12 tables
    if (tablesCount >= 12 && !achievements[9].unlocked) {
        achievements[9].unlocked = true;
    }

    // Century - 100 correct
    if (stats.totalCorrect >= 100 && !achievements[10].unlocked) {
        achievements[10].unlocked = true;
    }

    // Dedication - 50 streak
    if (stats.bestStreak >= 50 && !achievements[11].unlocked) {
        achievements[11].unlocked = true;
    }
}

// ============================================================================
// PERSISTENCE
// ============================================================================

void saveStats() {
    prefs.begin("mathquiz", false);
    prefs.putInt("correct", stats.totalCorrect);
    prefs.putInt("wrong", stats.totalWrong);
    prefs.putInt("streak", stats.currentStreak);
    prefs.putInt("bestStreak", stats.bestStreak);
    prefs.putInt("perfect", stats.perfectRounds);
    prefs.putULong("fastest", stats.fastestAnswer);
    prefs.putInt("tables", stats.tablesCompleted);

    // Save achievements
    uint32_t achievementBits = 0;
    for (int i = 0; i < NUM_ACHIEVEMENTS; i++) {
        if (achievements[i].unlocked) {
            achievementBits |= (1 << i);
        }
    }
    prefs.putUInt("achieve", achievementBits);
    prefs.end();
}

void loadStats() {
    prefs.begin("mathquiz", true);
    stats.totalCorrect = prefs.getInt("correct", 0);
    stats.totalWrong = prefs.getInt("wrong", 0);
    stats.currentStreak = prefs.getInt("streak", 0);
    stats.bestStreak = prefs.getInt("bestStreak", 0);
    stats.perfectRounds = prefs.getInt("perfect", 0);
    stats.fastestAnswer = prefs.getULong("fastest", 0);
    stats.tablesCompleted = prefs.getInt("tables", 0);

    // Load achievements
    uint32_t achievementBits = prefs.getUInt("achieve", 0);
    for (int i = 0; i < NUM_ACHIEVEMENTS; i++) {
        achievements[i].unlocked = (achievementBits & (1 << i)) != 0;
    }
    prefs.end();

    Serial.printf("Loaded stats: %d correct, %d streak\n", stats.totalCorrect, stats.currentStreak);
}

// ============================================================================
// CONFETTI EFFECTS
// ============================================================================

void initConfetti() {
    for (int i = 0; i < MAX_CONFETTI; i++) {
        confetti[i].active = false;
    }
}

void startConfetti() {
    confettiActive = true;
    confettiStartTime = millis();

    for (int i = 0; i < MAX_CONFETTI; i++) {
        confetti[i].x = random(0, SCREEN_WIDTH);
        confetti[i].y = random(-50, 0);
        confetti[i].vx = random(-30, 30) / 10.0f;
        confetti[i].vy = random(20, 60) / 10.0f;
        confetti[i].color = rainbowColors[random(0, NUM_RAINBOW_COLORS)];
        confetti[i].size = random(3, 8);
        confetti[i].active = true;
    }
}

void updateConfetti() {
    for (int i = 0; i < MAX_CONFETTI; i++) {
        if (confetti[i].active) {
            // Erase old position
            tft.fillRect(confetti[i].x, confetti[i].y,
                        confetti[i].size, confetti[i].size, COLOR_BG);

            // Update position
            confetti[i].x += confetti[i].vx;
            confetti[i].y += confetti[i].vy;
            confetti[i].vy += 0.2f;  // Gravity
            confetti[i].vx *= 0.99f;  // Air resistance

            // Check bounds
            if (confetti[i].y > SCREEN_HEIGHT + 10) {
                // Respawn at top
                confetti[i].x = random(0, SCREEN_WIDTH);
                confetti[i].y = random(-20, 0);
                confetti[i].vy = random(20, 40) / 10.0f;
            }
        }
    }
}

void drawConfetti() {
    for (int i = 0; i < MAX_CONFETTI; i++) {
        if (confetti[i].active) {
            tft.fillRect(confetti[i].x, confetti[i].y,
                        confetti[i].size, confetti[i].size, confetti[i].color);
        }
    }
}

// ============================================================================
// STAR EFFECTS (for achievements)
// ============================================================================

void initStars() {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x = SCREEN_WIDTH / 2;
        stars[i].y = SCREEN_HEIGHT / 2;
        stars[i].angle = random(0, 360) * PI / 180.0f;
        stars[i].speed = random(20, 50) / 10.0f;
        stars[i].size = random(2, 6);
        stars[i].color = rainbowColors[random(0, NUM_RAINBOW_COLORS)];
        stars[i].active = true;
    }
}

void updateStars() {
    for (int i = 0; i < MAX_STARS; i++) {
        if (stars[i].active) {
            // Erase old position
            tft.fillCircle(stars[i].x, stars[i].y, stars[i].size, COLOR_BG);

            // Update position (burst outward)
            stars[i].x += cos(stars[i].angle) * stars[i].speed;
            stars[i].y += sin(stars[i].angle) * stars[i].speed;
            stars[i].speed *= 0.98f;

            // Respawn if out of bounds or stopped
            if (stars[i].x < 0 || stars[i].x > SCREEN_WIDTH ||
                stars[i].y < 0 || stars[i].y > SCREEN_HEIGHT ||
                stars[i].speed < 0.5f) {
                stars[i].x = SCREEN_WIDTH / 2;
                stars[i].y = SCREEN_HEIGHT / 2;
                stars[i].angle = random(0, 360) * PI / 180.0f;
                stars[i].speed = random(20, 50) / 10.0f;
                stars[i].color = rainbowColors[random(0, NUM_RAINBOW_COLORS)];
            }
        }
    }
}

void drawStars() {
    for (int i = 0; i < MAX_STARS; i++) {
        if (stars[i].active) {
            tft.fillCircle(stars[i].x, stars[i].y, stars[i].size, stars[i].color);
        }
    }
}

// ============================================================================
// SCREEN DRAWING
// ============================================================================

void drawSplashScreen() {
    tft.fillScreen(COLOR_BG);

    // Animated rainbow title
    const char* title = "MATH";
    const char* title2 = "FACTS!";

    int y = 60;
    tft.setTextSize(4);

    // Draw each letter in different color
    int x = 80;
    for (int i = 0; i < 4; i++) {
        tft.setTextColor(rainbowColors[i]);
        tft.setCursor(x + i * 40, y);
        tft.print(title[i]);
    }

    x = 50;
    y = 110;
    for (int i = 0; i < 6; i++) {
        tft.setTextColor(rainbowColors[(i + 4) % NUM_RAINBOW_COLORS]);
        tft.setCursor(x + i * 40, y);
        tft.print(title2[i]);
    }

    // Subtitle
    tft.setTextSize(2);
    tft.setTextColor(COLOR_WHITE);
    drawCenteredText("Times Tables 1-12", 170, 2, COLOR_WHITE);

    // Touch to start
    tft.setTextSize(1);
    drawCenteredText("Touch anywhere to start!", 210, 1, COLOR_YELLOW);

    // Draw some decorative stars
    for (int i = 0; i < 15; i++) {
        int sx = random(0, SCREEN_WIDTH);
        int sy = random(0, SCREEN_HEIGHT);
        tft.fillCircle(sx, sy, random(1, 3), rainbowColors[random(0, NUM_RAINBOW_COLORS)]);
    }
}

void drawMenuScreen() {
    tft.fillScreen(COLOR_BG);

    // Title
    tft.setTextSize(3);
    tft.setTextColor(COLOR_YELLOW);
    drawCenteredText("MATH FACTS", 20, 3, COLOR_YELLOW);

    // Play button
    fillRoundedRect(60, 80, 200, 70, 15, COLOR_GREEN);
    tft.setTextColor(COLOR_WHITE);
    tft.setTextSize(3);
    tft.setCursor(120, 100);
    tft.print("PLAY!");

    // Stats button
    fillRoundedRect(60, 170, 200, 50, 10, COLOR_CYAN);
    tft.setTextSize(2);
    tft.setCursor(120, 185);
    tft.print("STATS");

    // Show streak if any
    if (stats.currentStreak > 0) {
        tft.setTextSize(1);
        tft.setTextColor(COLOR_ORANGE);
        char streakText[32];
        sprintf(streakText, "Current streak: %d", stats.currentStreak);
        drawCenteredText(streakText, 230, 1, COLOR_ORANGE);
    }
}

void drawQuizScreen() {
    tft.fillScreen(COLOR_BG);

    // Header with streak
    tft.setTextSize(1);
    tft.setTextColor(COLOR_WHITE);
    tft.setCursor(5, 5);
    tft.printf("Streak: %d", stats.currentStreak);

    // Show fire emoji if on a streak
    if (stats.currentStreak >= 3) {
        tft.setTextColor(COLOR_ORANGE);
        tft.setCursor(100, 5);
        for (int i = 0; i < min(stats.currentStreak / 3, 5); i++) {
            tft.print("*");  // Fire representation
        }
    }

    // Score
    tft.setTextColor(COLOR_WHITE);
    tft.setCursor(SCREEN_WIDTH - 80, 5);
    tft.printf("Score: %d", stats.totalCorrect);

    // Question box
    fillRoundedRect(20, 30, 280, 85, 15, COLOR_BG_LIGHT);

    // Question text
    char questionText[32];
    sprintf(questionText, "%d x %d = ?", currentQuestion.num1, currentQuestion.num2);

    tft.setTextSize(4);
    tft.setTextColor(COLOR_WHITE);

    // Center the question
    int textWidth = strlen(questionText) * 24;  // Approximate width
    int textX = (SCREEN_WIDTH - textWidth) / 2;
    tft.setCursor(textX, 55);
    tft.print(questionText);

    // Answer buttons (2x2 grid)
    int btnWidth = 145;
    int btnHeight = 55;
    int startX = 10;
    int startY = 130;
    int gapX = 10;
    int gapY = 10;

    for (int i = 0; i < 4; i++) {
        int col = i % 2;
        int row = i / 2;
        int btnX = startX + col * (btnWidth + gapX);
        int btnY = startY + row * (btnHeight + gapY);

        // Draw button with gradient effect
        fillRoundedRect(btnX, btnY, btnWidth, btnHeight, 12, buttonColors[i]);

        // Button text
        char answerText[8];
        sprintf(answerText, "%d", currentQuestion.answers[i]);

        tft.setTextSize(3);
        tft.setTextColor(COLOR_BLACK);

        // Center text in button
        int ansLen = strlen(answerText);
        int ansX = btnX + (btnWidth - ansLen * 18) / 2;
        int ansY = btnY + (btnHeight - 21) / 2;
        tft.setCursor(ansX, ansY);
        tft.print(answerText);
    }
}

void drawResultScreen(bool correct) {
    // Semi-transparent overlay effect by drawing a darker box
    fillRoundedRect(30, 30, 260, 80, 15, correct ? COLOR_CORRECT : COLOR_WRONG);

    tft.setTextSize(3);
    tft.setTextColor(COLOR_WHITE);

    if (correct) {
        // Random positive message
        const char* messages[] = {"AWESOME!", "GREAT!", "CORRECT!", "PERFECT!", "YES!"};
        int msgIdx = random(0, 5);
        drawCenteredText(messages[msgIdx], 50, 3, COLOR_WHITE);

        // Show streak
        if (stats.currentStreak > 1) {
            char streakText[32];
            sprintf(streakText, "%d in a row!", stats.currentStreak);
            tft.setTextSize(2);
            drawCenteredText(streakText, 85, 2, COLOR_YELLOW);
        }
    } else {
        drawCenteredText("TRY AGAIN!", 45, 3, COLOR_WHITE);

        // Show correct answer
        char correctText[32];
        sprintf(correctText, "%d x %d = %d",
                currentQuestion.num1, currentQuestion.num2, currentQuestion.correctAnswer);
        tft.setTextSize(2);
        drawCenteredText(correctText, 85, 2, COLOR_WHITE);
    }

    // Highlight correct answer button
    int btnWidth = 145;
    int btnHeight = 55;
    int startX = 10;
    int startY = 130;
    int gapX = 10;
    int gapY = 10;

    int col = currentQuestion.correctIndex % 2;
    int row = currentQuestion.correctIndex / 2;
    int btnX = startX + col * (btnWidth + gapX);
    int btnY = startY + row * (btnHeight + gapY);

    // Draw border around correct answer
    tft.drawRoundRect(btnX - 2, btnY - 2, btnWidth + 4, btnHeight + 4, 14, COLOR_CORRECT);
    tft.drawRoundRect(btnX - 3, btnY - 3, btnWidth + 6, btnHeight + 6, 14, COLOR_CORRECT);
}

void drawAchievementPopup(int achievementIndex) {
    tft.fillScreen(COLOR_BG);

    // Big celebratory text
    tft.setTextSize(3);
    tft.setTextColor(COLOR_GOLD);
    drawCenteredText("ACHIEVEMENT", 20, 3, COLOR_GOLD);
    drawCenteredText("UNLOCKED!", 55, 3, COLOR_GOLD);

    // Achievement icon (big)
    fillRoundedRect(120, 90, 80, 80, 20, COLOR_GOLD);
    tft.setTextSize(4);
    tft.setTextColor(COLOR_BLACK);
    tft.setCursor(145, 110);
    tft.print(achievements[achievementIndex].icon);

    // Achievement name
    tft.setTextSize(2);
    drawCenteredText(achievements[achievementIndex].name, 185, 2, COLOR_WHITE);

    // Description
    tft.setTextSize(1);
    drawCenteredText(achievements[achievementIndex].description, 210, 1, COLOR_YELLOW);

    // Tap to continue
    drawCenteredText("Tap to continue", 230, 1, COLOR_WHITE);
}

void drawStatsScreen() {
    tft.fillScreen(COLOR_BG);

    // Back button hint
    tft.setTextSize(1);
    tft.setTextColor(COLOR_WHITE);
    tft.setCursor(5, 5);
    tft.print("< Back");

    // Title
    tft.setTextSize(2);
    drawCenteredText("YOUR STATS", 5, 2, COLOR_YELLOW);

    // Stats
    int y = 35;
    int lineHeight = 22;
    tft.setTextSize(1);

    // Correct answers
    tft.setTextColor(COLOR_GREEN);
    tft.setCursor(20, y);
    tft.printf("Correct Answers: %d", stats.totalCorrect);
    y += lineHeight;

    // Wrong answers
    tft.setTextColor(COLOR_RED);
    tft.setCursor(20, y);
    tft.printf("Wrong Answers: %d", stats.totalWrong);
    y += lineHeight;

    // Accuracy
    tft.setTextColor(COLOR_WHITE);
    int total = stats.totalCorrect + stats.totalWrong;
    int accuracy = total > 0 ? (stats.totalCorrect * 100 / total) : 0;
    tft.setCursor(20, y);
    tft.printf("Accuracy: %d%%", accuracy);
    y += lineHeight;

    // Best streak
    tft.setTextColor(COLOR_ORANGE);
    tft.setCursor(20, y);
    tft.printf("Best Streak: %d", stats.bestStreak);
    y += lineHeight;

    // Fastest answer
    tft.setTextColor(COLOR_CYAN);
    tft.setCursor(20, y);
    if (stats.fastestAnswer > 0) {
        tft.printf("Fastest Answer: %.1fs", stats.fastestAnswer / 1000.0);
    } else {
        tft.print("Fastest Answer: --");
    }
    y += lineHeight;

    // Perfect rounds
    tft.setTextColor(COLOR_GOLD);
    tft.setCursor(20, y);
    tft.printf("Perfect Rounds: %d", stats.perfectRounds);
    y += lineHeight + 5;

    // Achievements section
    tft.setTextColor(COLOR_YELLOW);
    tft.setTextSize(1);
    tft.setCursor(20, y);
    tft.print("ACHIEVEMENTS:");
    y += 15;

    // Draw achievement icons
    int achX = 20;
    int achY = y;
    int iconSize = 25;
    int gap = 5;
    int perRow = 8;

    for (int i = 0; i < NUM_ACHIEVEMENTS; i++) {
        int row = i / perRow;
        int col = i % perRow;
        int x = achX + col * (iconSize + gap);
        int boxY = achY + row * (iconSize + gap);

        if (achievements[i].unlocked) {
            fillRoundedRect(x, boxY, iconSize, iconSize, 5, COLOR_GOLD);
            tft.setTextColor(COLOR_BLACK);
        } else {
            fillRoundedRect(x, boxY, iconSize, iconSize, 5, 0x4208);  // Gray
            tft.setTextColor(0x8410);  // Light gray
        }

        tft.setTextSize(1);
        tft.setCursor(x + 8, boxY + 8);
        tft.print(achievements[i].icon);
    }

    // Count unlocked
    int unlockedCount = 0;
    for (int i = 0; i < NUM_ACHIEVEMENTS; i++) {
        if (achievements[i].unlocked) unlockedCount++;
    }

    tft.setTextColor(COLOR_WHITE);
    tft.setCursor(20, 220);
    tft.printf("Unlocked: %d/%d", unlockedCount, NUM_ACHIEVEMENTS);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void drawCenteredText(const char* text, int y, int size, uint16_t color) {
    tft.setTextSize(size);
    tft.setTextColor(color);
    int textWidth = strlen(text) * 6 * size;  // Approximate
    int x = (SCREEN_WIDTH - textWidth) / 2;
    tft.setCursor(x, y);
    tft.print(text);
}

void fillRoundedRect(int x, int y, int w, int h, int r, uint16_t color) {
    tft.fillRoundRect(x, y, w, h, r, color);
}

void drawProgressBar(int x, int y, int w, int h, int value, int maxVal, uint16_t color) {
    tft.drawRect(x, y, w, h, COLOR_WHITE);
    int fillWidth = (w - 2) * value / maxVal;
    tft.fillRect(x + 1, y + 1, fillWidth, h - 2, color);
}

void animateCorrect() {
    // Quick green flash
    for (int i = 0; i < 3; i++) {
        tft.fillScreen(COLOR_CORRECT);
        delay(30);
        drawQuizScreen();
        delay(30);
    }
}

void animateWrong() {
    // Quick shake effect (red flash)
    for (int i = 0; i < 2; i++) {
        tft.fillScreen(COLOR_WRONG);
        delay(50);
        drawQuizScreen();
        delay(50);
    }
}

uint16_t dimColor(uint16_t color, float factor) {
    uint8_t r = ((color >> 11) & 0x1F) * factor;
    uint8_t g = ((color >> 5) & 0x3F) * factor;
    uint8_t b = (color & 0x1F) * factor;
    return (r << 11) | (g << 5) | b;
}
