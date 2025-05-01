#include <TinyWireM.h>
#include <U8g2lib.h>

// Inicializácia I2C OLED displeja pre ATtiny85
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Rozmery obrazovky a objektov
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 3
#define BALL_SIZE 3
#define BRICK_WIDTH 20
#define BRICK_HEIGHT 5

// Tlačidlá
#define BUTTON_LEFT 3
#define BUTTON_RIGHT 4

// Herné objekty
int paddleX;
int ballX, ballY;
int ballVX, ballVY;
unsigned long lastBallMove;
int ballSpeed = 50; // ms medzi pohybmi
int score = 0;
bool gameOver = false;
unsigned long gameOverTime = 0;

// Tehly: 6 stĺpcov × 3 riadky
bool bricks[6][3];

void setup() {
  TinyWireM.begin();
  u8g2.begin();

  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);

  resetGame();
}

void loop() {
  if (gameOver) {
    drawGameOver();
    if (millis() - gameOverTime >= 5000) {
      resetGame();
    }
    return;
  }

  handleInput();
  updateGame();
  drawGame();
  delay(10);
}

void resetGame() {
  // Reset objektov
  paddleX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;
  ballVX = 1;
  ballVY = -1;
  score = 0;
  gameOver = false;
  lastBallMove = millis();

  // Reset tehál
  for (int i = 0; i < 6; i++)
    for (int j = 0; j < 3; j++)
      bricks[i][j] = true;
}

void handleInput() {
  if (digitalRead(BUTTON_LEFT) == LOW && paddleX > 0)
    paddleX -= 2;
  if (digitalRead(BUTTON_RIGHT) == LOW && paddleX < SCREEN_WIDTH - PADDLE_WIDTH)
    paddleX += 2;
}

void updateGame() {
  if (millis() - lastBallMove > ballSpeed) {
    ballX += ballVX;
    ballY += ballVY;

    // Odraz od okrajov
    if (ballX <= 0 || ballX >= SCREEN_WIDTH - BALL_SIZE)
      ballVX *= -1;
    if (ballY <= 0)
      ballVY *= -1;

    // Kolízia s platformou
    if (ballY + BALL_SIZE >= SCREEN_HEIGHT - PADDLE_HEIGHT &&
        ballX + BALL_SIZE >= paddleX &&
        ballX <= paddleX + PADDLE_WIDTH) {
      ballVY *= -1;
      ballY = SCREEN_HEIGHT - PADDLE_HEIGHT - BALL_SIZE;
    }

    // Kolízia s tehlami
    int col = ballX / BRICK_WIDTH;
    int row = ballY / BRICK_HEIGHT;
    if (col >= 0 && col < 6 && row >= 0 && row < 3 && bricks[col][row]) {
      bricks[col][row] = false;
      ballVY *= -1;
      score++;
    }

    // Gulička spadla pod obrazovku
    if (ballY > SCREEN_HEIGHT) {
      gameOver = true;
      gameOverTime = millis();
    }

    lastBallMove = millis();
  }
}

void drawGame() {
  u8g2.clearBuffer();

  // Tehly
  for (int i = 0; i < 6; i++)
    for (int j = 0; j < 3; j++)
      if (bricks[i][j])
        u8g2.drawBox(i * BRICK_WIDTH, j * BRICK_HEIGHT, BRICK_WIDTH - 1, BRICK_HEIGHT - 1);

  // Platforma
  u8g2.drawBox(paddleX, SCREEN_HEIGHT - PADDLE_HEIGHT, PADDLE_WIDTH, PADDLE_HEIGHT);

  // Gulička
  u8g2.drawBox(ballX, ballY, BALL_SIZE, BALL_SIZE);

  // Skóre
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.setCursor(0, 10);
  u8g2.print("Score:");
  u8g2.print(score);

  u8g2.sendBuffer();
}

void drawGameOver() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.setCursor(30, 30);
  u8g2.print("GAME OVER");

  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.setCursor(35, 45);
  u8g2.print("Score: ");
  u8g2.print(score);

  u8g2.sendBuffer();
}