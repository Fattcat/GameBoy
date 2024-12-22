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

// Premenné pre menu
int cursorPosition = 0; // Poloha kurzora v menu
const int menuItems = 2; // Počet položiek menu

// ----- Premenné pre DinoGame -----
int dinoY = SCREEN_HEIGHT - 10; // Vertikálna pozícia hráča
float dinoVelocity = 0;         // Vertikálna rýchlosť hráča
const int dinoJumpForce = 5;    // Zvýšená sila skoku
const float dinoGravity = 0.6;  // Gravitačná sila

struct DinoObstacle {
  int x;
  int y;
  bool active; // Nové pole na sledovanie aktivity prekážky
};

DinoObstacle obstacles[3]; // Pole pre prekážky
bool dinoGameActive = false;
int dinoScore = 0;

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

  // Inicializácia prekážok
  for (int i = 0; i < 3; i++) {
    obstacles[i] = {SCREEN_WIDTH + (i * 40), SCREEN_HEIGHT - 10, true}; // Rozmiestnenie prekážok
  }
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
    }
  }
}

// ----- DinoGame -----
void startDinoGame() {
  dinoGameActive = true;
  dinoScore = 0;
  dinoY = SCREEN_HEIGHT - 10;
  dinoVelocity = 0;

  // Inicializácia prekážok
  for (int i = 0; i < 3; i++) {
    obstacles[i] = {SCREEN_WIDTH + (i * 40), SCREEN_HEIGHT - 10, true}; // Rozmiestnenie prekážok
  }

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

    // Aktualizácia pozície prekážok
    static int obstacleSpawnCounter = 0; // Počítadlo pre generovanie prekážok
    obstacleSpawnCounter++;

    for (int i = 0; i < 3; i++) {
      if (obstacles[i].active) {
        obstacles[i].x -= 6; // Zvýšená rýchlosť pohybu prekážok
        if (obstacles[i].x < -10) {
          obstacles[i].x = SCREEN_WIDTH + random(20, 60); // Reset pozície s náhodným rozostupom
          // Zabezpečenie, že prekážky sa objavia minimálne 10 pixelov od Dína
          while (obstacles[i].x < 18) {
            obstacles[i].x = SCREEN_WIDTH + random(20, 60);
          }
          dinoScore++;
        }
      }
    }

    // Zobrazenie hráča
    display.fillRect(10, dinoY, 10, 10, SSD1306_WHITE);

    // Zobrazenie prekážok
    for (int i = 0; i < 3; i++) {
      if (obstacles[i].active) {
        display.fillRect(obstacles[i].x, obstacles[i].y, 10, 10, SSD1306_WHITE);
      }
    }

    // Detekcia kolízií
    for (int i = 0; i < 3; i++) {
      if (obstacles[i].active && obstacles[i].x < 18 && obstacles[i].x > 8 && dinoY > SCREEN_HEIGHT - 20) {
        dinoGameActive = false;
        showGameOverScreen(dinoScore);
        break;
      }
    }

    display.display();
    delay(50); // Rýchlosť hry
  }
}

void showGameOverScreen(int score) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Game Over!"));
  display.setTextSize(1);
  display.print(F("Score: "));
  display.println(score);
  display.display();
  delay(2000); // Zobrazenie obrazovky konca hry
  dinoGameActive = false; // Ukončenie hry
}

void startFlappyBird() {
  // Implementácia FlappyBird hry
}
