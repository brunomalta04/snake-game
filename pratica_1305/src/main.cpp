#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

// --- Objeto BluetoothSerial ---
BluetoothSerial SerialBT;

// --- Configurações dos Pinos ---
#define BUZZER_PIN   15
#define LED_PIN      26
#define NUM_LEDS     25
#define WIDTH        5
#define HEIGHT       5

#define BTN_UP       27
#define BTN_DOWN     14
#define BTN_LEFT     12
#define BTN_RIGHT    13

// --- Objeto NeoPixel ---
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const uint32_t COLOR_BLACK          = Adafruit_NeoPixel::Color(255, 255, 0);
const uint32_t COLOR_SNAKE          = Adafruit_NeoPixel::Color(0, 255, 0);
const uint32_t COLOR_FOOD           = Adafruit_NeoPixel::Color(0, 0, 255);
const uint32_t COLOR_GAMEOVER       = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t COLOR_COUNTDOWN      = Adafruit_NeoPixel::Color(255, 255, 0);
const uint32_t COLOR_RESTART_PROMPT = Adafruit_NeoPixel::Color(0, 255, 255);

// --- Enumerações e Estruturas ---
enum Direction { UP, DOWN, LEFT, RIGHT, NONE };
struct Point { int x; int y; };

// --- Variáveis Globais do Jogo ---
Direction currentDirection = RIGHT;
Point snake[NUM_LEDS];
int snakeLength;
Point food;
unsigned long lastMoveTime;
unsigned long currentMoveInterval;
const unsigned long INITIAL_MOVE_INTERVAL = 500;
const unsigned long MIN_MOVE_INTERVAL = 150;
const int DIFFICULTY_INCREASE_STEP = 25;
const int FOOD_FOR_DIFFICULTY_INCREASE = 3;
int foodEatenCounter = 0;
bool gameOver = false;

// --- Protótipos das Funções ---
// O compilador usa isso para saber que as funções existem
int mapXY(int x, int y);
void spawnFood();
void initializeGame();
void showCountdown();
void handleInput();
void updateGameLogic();
void drawGame();
void handleGameOver();

// =========================================================================
//  SETUP
// =========================================================================
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Snake_Game");

  strip.begin();
  strip.setBrightness(50);
  strip.show();

  randomSeed(analogRead(0));

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  initializeGame();
  showCountdown();
}

// =========================================================================
//  LOOP
// =========================================================================
void loop() {
  if (gameOver) {
    handleGameOver();
  } else {
    handleInput();
    if (millis() - lastMoveTime >= currentMoveInterval) {
      lastMoveTime = millis();
      updateGameLogic();
      if (!gameOver) {
        drawGame();
      }
    }
  }
}

// =========================================================================
//  FUNÇÕES AUXILIARES
// =========================================================================

// --- Mapeia coordenadas (x,y) para o índice do LED (serpentina) ---
int mapXY(int x, int y) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
    return -1; // Fora dos limites
  }
  if (y % 2 == 0) { // Linhas pares
    return y * WIDTH + x;
  } else { // Linhas ímpares
    return y * WIDTH + (WIDTH - 1 - x);
  }
}

// --- Gera uma nova posição para a comida ---
void spawnFood() {
  bool onSnake;
  do {
    onSnake = false;
    food.x = random(0, WIDTH);
    food.y = random(0, HEIGHT);
    for (int i = 0; i < snakeLength; i++) {
      if (snake[i].x == food.x && snake[i].y == food.y) {
        onSnake = true;
        break;
      }
    }
  } while (onSnake);
}

// --- Inicializa/Reinicializa as variáveis do jogo ---
void initializeGame() {
  snakeLength = 1;
  snake[0] = {WIDTH / 2, HEIGHT / 2};
  currentDirection = RIGHT;
  spawnFood();
  currentMoveInterval = INITIAL_MOVE_INTERVAL;
  lastMoveTime = millis();
  foodEatenCounter = 0;
  gameOver = false;
  strip.clear();
  strip.show();
}

// --- Mostra uma contagem regressiva simples ---
void showCountdown() {
  strip.clear();
  for (int i = 3; i > 0; i--) {
    int_fast8_t centralPixelX = WIDTH / 2;
    int_fast8_t centralPixelY = HEIGHT / 2;
    tone(BUZZER_PIN, 800, 150);
    strip.setPixelColor(mapXY(centralPixelX, centralPixelY), COLOR_COUNTDOWN);
    strip.show();
    delay(300);
    strip.setPixelColor(mapXY(centralPixelX, centralPixelY), COLOR_BLACK);
    strip.show();
    delay(200);
  }
  delay(500);
  lastMoveTime = millis();
}

// --- Processa a entrada dos botões físicos e comandos Bluetooth ---
void handleInput() {
  if (!digitalRead(BTN_UP) && currentDirection != DOWN) { currentDirection = UP; } 
  else if (!digitalRead(BTN_DOWN) && currentDirection != UP) { currentDirection = DOWN; } 
  else if (!digitalRead(BTN_LEFT) && currentDirection != RIGHT) { currentDirection = LEFT; }
  else if (!digitalRead(BTN_RIGHT) && currentDirection != LEFT) { currentDirection = RIGHT; }
  
  delay(10);
  
  if (SerialBT.available()) {
    char command = SerialBT.read();
    if ((command == 'U' || command == 'u') && currentDirection != DOWN) { currentDirection = UP; } 
    else if ((command == 'D' || command == 'd') && currentDirection != UP) { currentDirection = DOWN; }
    else if ((command == 'L' || command == 'l') && currentDirection != RIGHT) { currentDirection = LEFT; }
    else if ((command == 'R' || command == 'r') && currentDirection != LEFT) { currentDirection = RIGHT; }
  }
}

// --- Atualiza a lógica do jogo (movimento, colisões, comida) ---
void updateGameLogic() {
  if (gameOver) return;
  
  Point newHead = snake[0];

  switch (currentDirection) {
    case UP:    newHead.y--; break;
    case DOWN:  newHead.y++; break;
    case LEFT:  newHead.x--; break;
    case RIGHT: newHead.x++; break;
    case NONE:  return;
  }

  if (newHead.x < 0 || newHead.x >= WIDTH || newHead.y < 0 || newHead.y >= HEIGHT) {
    gameOver = true;
    return;
  }

  for (int i = 0; i < snakeLength - 1; i++) {
    if (newHead.x == snake[i].x && newHead.y == snake[i].y) {
      gameOver = true;
      return;
    }
  }
  
  bool ateComida = (newHead.x == food.x && newHead.y == food.y);
  
  if (ateComida) {
    tone(BUZZER_PIN, 1200, 100);
    if (snakeLength < NUM_LEDS) {
      snakeLength++;
    }
    spawnFood();
    foodEatenCounter++;
    if (foodEatenCounter % FOOD_FOR_DIFFICULTY_INCREASE == 0) {
      if (currentMoveInterval > MIN_MOVE_INTERVAL) {
        currentMoveInterval -= DIFFICULTY_INCREASE_STEP;
        if (currentMoveInterval < MIN_MOVE_INTERVAL) {
          currentMoveInterval = MIN_MOVE_INTERVAL;
        }
      }
    }
  }

  for (int i = snakeLength - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }
  snake[0] = newHead;
}

// --- Desenha a cobra e a comida na matriz de LEDs ---
void drawGame() {
  strip.clear();
  for (int i = 0; i < snakeLength; i++) {
    int idx = mapXY(snake[i].x, snake[i].y);
    if (idx != -1) strip.setPixelColor(idx, COLOR_SNAKE);
  }
  int foodIdx = mapXY(food.x, food.y);
  if (foodIdx != -1) strip.setPixelColor(foodIdx, COLOR_FOOD);
  strip.show();
}

// --- Gerencia o estado de "Game Over" e o reinício ---
void handleGameOver() {
  tone(BUZZER_PIN, 440, 200);
  delay(200);
  tone(BUZZER_PIN, 220, 300);
  delay(300);

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < NUM_LEDS; j++) strip.setPixelColor(j, COLOR_GAMEOVER);
    strip.show();
    delay(250);
    for (int j = 0; j < NUM_LEDS; j++) strip.setPixelColor(j, COLOR_BLACK);
    strip.show();
    delay(250);
  }
  
  strip.clear();
  int_fast8_t centralPixelX = WIDTH / 2;
  int_fast8_t centralPixelY = HEIGHT / 2;
  strip.setPixelColor(mapXY(centralPixelX, centralPixelY), COLOR_RESTART_PROMPT);
  strip.show();
  Serial.println("Game Over! Pressione botão físico ou envie 'S' via Bluetooth para reiniciar.");

  bool restartTriggered = false;
  unsigned long lastInputCheck = millis();

  while (!restartTriggered) {
    if (millis() - lastInputCheck > 50) {
      if (!digitalRead(BTN_UP) || !digitalRead(BTN_DOWN) || !digitalRead(BTN_LEFT) || !digitalRead(BTN_RIGHT)) {
        restartTriggered = true;
        delay(200);
      }
      lastInputCheck = millis();
    }
    if (SerialBT.available()) {
      char btCmd = SerialBT.read();
      if (btCmd == 'S' || btCmd == 's') {
        restartTriggered = true;
      }
    }
    yield();
  }

  initializeGame();
  showCountdown();
}