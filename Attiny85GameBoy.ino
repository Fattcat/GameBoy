#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// ----- Definície OLED a pinov -----
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Definície pinov tlačidiel
#define BUTTON_ENTER 2
#define BUTTON_UP 3
#define BUTTON_DOWN 4

// ----- Premenné pre menu -----
int cursorPosition = 0; // Poloha kurzora v menu
const int menuItems = 3; // Počet položiek menu

// ----- Premenné pre DinoGame -----
int dinoY = SCREEN_HEIGHT - 10; // Vertikálna pozícia hráča
float dinoVelocity = 0;         // Vertikálna rýchlosť hráča
const int dinoJumpForce = 10;    // Zvýšená sila skoku
const float dinoGravity = 1;  // Gravitačná sila

struct DinoObstacle {
  int x;
  int y;
  bool active; // Nové pole na sledovanie aktivity prekážky
};

DinoObstacle dinoObstacle; // Deklarovanie dinoObstacle

bool dinoGameActive = false;
int dinoScore = 0;

// ----- Premenné pre Flappy Bird -----
int birdY = SCREEN_HEIGHT / 2;
float birdVelocity = 0;
const int birdGravity = 1;
const int birdJump = 6;
bool flappyBirdActive = false;

// ----- Premenné pre Pong -----
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

// ----- Funkcie -----
void setup() {
  pinMode(BUTTON_ENTER, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  showMenu();
}

void showMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Zobrazenie menu
  display.setCursor(0, 0);
  display.println(F("Select a game:"));

  display.setCursor(10, 20);
  display.print(cursorPosition == 0 ? "> DinoGame" : "  DinoGame");
  display.setCursor(10, 30);
  display.print(cursorPosition == 1 ? "> FlappyBird" : "  FlappyBird");
  display.setCursor(10, 40);
  display.print(cursorPosition == 2 ? "> PongGame" : "  PongGame");

  display.display();

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
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Game Over");
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Score: ");
  display.print(score);
  display.display();
  delay(2000);
}


// ----- DinoGame -----
void startDinoGame() {
  dinoGameActive = true;
  dinoScore = 0;
  dinoY = SCREEN_HEIGHT - 10;
  dinoVelocity = 0;
  dinoObstacle.x = SCREEN_WIDTH;

  while (dinoGameActive) {
    display.clearDisplay();

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
    display.fillRect(5, dinoY, 10, 10, SSD1306_WHITE); // Dino
    display.fillRect(dinoObstacle.x, SCREEN_HEIGHT - 10, 10, 10, SSD1306_WHITE); // Prekážka

    // Skóre
    display.setCursor(0, 0);
    display.print(F("Score: "));
    display.print(dinoScore);

    display.display();
    delay(30);
  }
}

// ----- Flappy Bird -----
// Definuj štruktúru pre prekážky
struct FlappyObstacle {
  int x;
  int gapY;
  const int gapHeight = 16;
};

FlappyObstacle flappyObstacle; // Globálna premenná pre prekážky

void startFlappyBird() {
  flappyBirdActive = true;

  int birdScore = 0; // Deklarácia pre skóre Flappy Bird

  birdY = SCREEN_HEIGHT / 2;
  birdVelocity = 0;
  flappyObstacle.x = SCREEN_WIDTH;

  while (flappyBirdActive) {
    display.clearDisplay();

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

    // Kreslenie hry
    display.fillCircle(10, birdY, 3, SSD1306_WHITE); // Bird
    display.fillRect(flappyObstacle.x, 0, 10, flappyObstacle.gapY, SSD1306_WHITE); // Horná prekážka
    display.fillRect(flappyObstacle.x, flappyObstacle.gapY + flappyObstacle.gapHeight, 10, SCREEN_HEIGHT - flappyObstacle.gapY - flappyObstacle.gapHeight, SSD1306_WHITE); // Dolná prekážka

    // Skóre
    display.setCursor(0, 0);
    display.print(F("Score: "));
    display.print(birdScore);

    display.display();
    delay(30);
  }
}


// ----- Pong -----
void startPongGame() {
  // Inicializácia premennej pre hru
  game_over = false;
  win = false;
  player_score = 0;
  mcu_score = 0;
  ball_x = 53;
  ball_y = 26;
  ball_dir_x = 1;
  ball_dir_y = 1;
  player_y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;
  mcu_y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;

  unsigned long buttonPressStart = 0; // Na sledovanie času stlačenia tlačidla

  while (!game_over) {
    unsigned long time = millis();
    bool update_needed = false;

    // Logika pohybu lopty
    if (time > ball_update) {
      ball_x += ball_dir_x;
      ball_y += ball_dir_y;

      // Kontrola kolízie s bočnými stenami
      if (ball_x == 0 || ball_x == 127) {
        ball_dir_x = -ball_dir_x;
        if (ball_x == 0) {
          mcu_score++;
        } else {
          player_score++;
        }
      }

      // Kontrola kolízie s vertikálnymi stenami
      if (ball_y == 0 || ball_y == 63) {
        ball_dir_y = -ball_dir_y;
      }

      ball_update = time + BALL_RATE;
      update_needed = true;
    }

    // Logika pohybu pálky
    if (time > paddle_update) {
      if (digitalRead(BUTTON_UP) == LOW && player_y > 0) {
        player_y -= 2; // Zvýšime rýchlosť pohybu pálky
      }
      if (digitalRead(BUTTON_DOWN) == LOW && player_y + PADDLE_HEIGHT < SCREEN_HEIGHT) {
        player_y += 2; // Zvýšime rýchlosť pohybu pálky
      }
      paddle_update = time + PADDLE_RATE;
      update_needed = true;
    }

    // Kontrola kolízie lopty s pálkami
    if (ball_x == PLAYER_X - 3 && ball_y >= player_y && ball_y <= player_y + PADDLE_HEIGHT) {
      ball_dir_x = -ball_dir_x; // Lopta sa odrazí od hráčovej pálky
    }

    if (ball_x == MCU_X + 5 && ball_y >= mcu_y && ball_y <= mcu_y + PADDLE_HEIGHT) {
      ball_dir_x = -ball_dir_x; // Lopta sa odrazí od pálky MCU
    }

    // Kontrola tlačidla ENTER
    if (digitalRead(BUTTON_ENTER) == LOW) {
      if (buttonPressStart == 0) { // Začiatok sledovania stlačenia
        buttonPressStart = millis();
      } else if (millis() - buttonPressStart >= 1000) { // Tlačidlo držané viac ako 1 sekundu
        game_over = true; // Vypneme hru
        delay(1000);
        return; // Vrátime sa do hlavného menu
      }
    } else {
      buttonPressStart = 0; // Resetujeme časovač, ak tlačidlo nie je stlačené
    }

    // Zobrazenie
    if (update_needed) {
      display.clearDisplay();
      display.fillRect(MCU_X, mcu_y, 5, PADDLE_HEIGHT, SSD1306_WHITE);
      display.fillRect(PLAYER_X, player_y, 5, PADDLE_HEIGHT, SSD1306_WHITE);
      display.fillRect(ball_x, ball_y, 3, 3, SSD1306_WHITE);
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.print(F("CPU: "));
      display.println(player_score);
      display.setCursor(90, 0);
      display.print(F("Hrac: "));
      display.println(mcu_score);
      display.display();
    }

    // Kontrola, či niektorý hráč dosiahol cieľové skóre
    if (player_score == SCORE_LIMIT || mcu_score == SCORE_LIMIT) {
      game_over = true;
      win = player_score == SCORE_LIMIT;
    }
  }

  // Ukončenie hry Pong
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(win ? "Player Wins!" : "Game Over!");
  display.setTextSize(1);
  display.print(F("Player: "));
  display.println(player_score);
  display.print(F("MCU: "));
  display.println(mcu_score);
  display.display();
  delay(2000);
}
