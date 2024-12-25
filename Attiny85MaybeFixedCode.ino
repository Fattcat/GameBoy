#include <TinyWireM.h>
#include <Tiny4kOLED.h>

// Definície OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Definície pinov tlačidiel
#define BUTTON_ENTER 2
#define BUTTON_UP 3
#define BUTTON_DOWN 4

// Premenné pre menu
int cursorPosition = 0; // Poloha kurzora v menu
const int menuItems = 3; // Počet položiek menu

// Premenné pre DinoGame
int dinoY = SCREEN_HEIGHT - 10; // Vertikálna pozícia hráča
float dinoVelocity = 0;         // Vertikálna rýchlosť hráča
const int dinoJumpForce = 10;    // Zvýšená sila skoku
const float dinoGravity = 1;     // Gravitačná sila

struct DinoObstacle {
  int x;
  int y;
  bool active; // Nové pole na sledovanie aktivity prekážky
};

DinoObstacle dinoObstacle; // Deklarovanie dinoObstacle

bool dinoGameActive = false;
int dinoScore = 0;

// Premenné pre Flappy Bird
int birdY = SCREEN_HEIGHT / 2;
float birdVelocity = 0;
const int birdGravity = 1;
const int birdJump = 6;
bool flappyBirdActive = false;

// Premenné pre Pong
const unsigned long PADDLE_RATE = 64;
const unsigned long BALL_RATE = 16;
const uint8_t PADDLE_HEIGHT = 12;
const uint8_t SCORE_LIMIT = 9;

bool game_over, win;

uint8_t player_score, mcu_score;
uint8_t ball_x = 53, ball_y = 26;
uint8_t ball_dir_x = 1, ball_dir_y = 1;

unsigned long ball_update;
unsigned long paddle_update;

const uint8_t MCU_X = 12;
uint8_t mcu_y = 16;

const uint8_t PLAYER_X = 115;
uint8_t player_y = 16;

// Funkcia na inicializáciu displeja
void setup() {
  pinMode(BUTTON_ENTER, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  // Inicializácia I2C komunikácie
  TinyWireM.begin();
  
  // Inicializácia OLED displeja
  oled.begin();
  oled.clear();
  oled.on();
}

void loop() {
  showMenu();
}

void showMenu() {
  oled.clear();
  oled.setCursor(0, 0);
  oled.print(F("Select a game:"));

  oled.setCursor(10, 20);
  oled.print(cursorPosition == 0 ? "> DinoGame" : "  DinoGame");
  oled.setCursor(10, 30);
  oled.print(cursorPosition == 1 ? "> FlappyBird" : "  FlappyBird");
  oled.setCursor(10, 40);
  oled.print(cursorPosition == 2 ? "> PongGame" : "  PongGame");

  oled.on();

  // Ovládanie kurzora
  if (digitalRead(BUTTON_UP) == LOW) {
    cursorPosition = (cursorPosition - 1 + menuItems) % menuItems;
    delay(200);
  } else if (digitalRead(BUTTON_DOWN) == LOW) {
    cursorPosition = (cursorPosition + 1) % menuItems;
    delay(200);
  } else if (digitalRead(BUTTON_ENTER) == LOW) {
    delay(200);
    if (cursorPosition == 0) {
      startDinoGame();
    } else if (cursorPosition == 1) {
      startFlappyBird();
    } else if (cursorPosition == 2) {
      startPongGame();
    }
  }
}

void showGameOverScreen(int score) {
  oled.clear();
  oled.setFont(FONT8X16);
  oled.setCursor(0, 0);
  oled.print("Game Over");
  oled.setCursor(0, 20);
  oled.print("Score: ");
  oled.print(score);
  oled.on();
  delay(2000);
}

// DinoGame
void startDinoGame() {
  dinoGameActive = true;
  dinoScore = 0;
  dinoY = SCREEN_HEIGHT - 10;
  dinoVelocity = 0;
  dinoObstacle.x = SCREEN_WIDTH;

  while (dinoGameActive) {
    oled.clear();

    // Tlačidlo pre skok
    if (digitalRead(BUTTON_ENTER) == LOW && dinoY == SCREEN_HEIGHT - 10) {
      dinoVelocity = -dinoJumpForce;
    }

    // Aplikácia gravitácie a aktualizácia pozície
    dinoVelocity += dinoGravity;
    dinoY += dinoVelocity;

    // Obmedzenie na zem
    if (dinoY > SCREEN_HEIGHT - 10) {
      dinoY = SCREEN_HEIGHT - 10;
      dinoVelocity = 0;
    }

    // Pohyb prekážky
    dinoObstacle.x -= 4; // Zrýchlenie prekážok
    if (dinoObstacle.x < -10) {
      dinoObstacle.x = SCREEN_WIDTH;
      dinoScore++;
    }

    // Kontrola kolízie
    if (dinoObstacle.x < 18 && dinoObstacle.x > 8 && dinoY > SCREEN_HEIGHT - 20) {
      dinoGameActive = false;
      showGameOverScreen(dinoScore);
      break;
    }

    // Kreslenie hry
    oled.fillRect(5, dinoY, 10, 10, 1); // Dino
    oled.fillRect(dinoObstacle.x, SCREEN_HEIGHT - 10, 10, 10, 1); // Prekážka

    // Skóre
    oled.setCursor(0, 0);
    oled.print(F("Score: "));
    oled.print(dinoScore);

    oled.on();
    delay(30);
  }
}

// Flappy Bird
void startFlappyBird() {
  flappyBirdActive = true;

  int birdScore = 0; // Deklarácia pre skóre Flappy Bird

  birdY = SCREEN_HEIGHT / 2;
  birdVelocity = 0;
  flappyObstacle.x = SCREEN_WIDTH;

  while (flappyBirdActive) {
    oled.clear();

    // Skok
    if (digitalRead(BUTTON_ENTER) == LOW) {
      birdVelocity = -birdJump;
    }

    // Aplikácia gravitácie
    birdVelocity += birdGravity;
    birdY += birdVelocity;

    // Obmedzenie obrazovky
    if (birdY < 0) birdY = 0;
    if (birdY > SCREEN_HEIGHT) {
      flappyBirdActive = false;
      showGameOverScreen(birdScore);
      break;
    }

    // Pohyb prekážky
    flappyObstacle.x -= 3;
    if (flappyObstacle.x < -10) {
      flappyObstacle.x = SCREEN_WIDTH;
      flappyObstacle.gapY = random(10, SCREEN_HEIGHT - flappyObstacle.gapHeight - 10);
      birdScore++;
    }

    // Kontrola kolízie
    if (flappyObstacle.x < 14 && flappyObstacle.x > 6) {
      if (birdY < flappyObstacle.gapY || birdY > flappyObstacle.gapY + flappyObstacle.gapHeight) {
        flappyBirdActive = false;
        showGameOverScreen(birdScore);
        break;
      }
    }

    // Kreslenie prekážok
    oled.fillRect(flappyObstacle.x, 0, 8, flappyObstacle.gapY, 1);
    oled.fillRect(flappyObstacle.x, flappyObstacle.gapY + flappyObstacle.gapHeight, 8, SCREEN_HEIGHT - flappyObstacle.gapY - flappyObstacle.gapHeight, 1);

    // Kreslenie vtáka
    oled.fillRect(8, birdY, 6, 6, 1);

    // Zobrazenie skóre
    oled.setCursor(0, 0);
    oled.print(F("Score: "));
    oled.print(birdScore);

    oled.on();
    delay(30);
  }
}
