/*
 * The classic game snake
 * author: Vera Link
 * hardware: Arduino nano, 8x8 LED display, 5 LEDs + shift register, 4 push buttons
 */
#include <LedControl.h>
#include <time.h>
#define UP 0
#define DOWN 2
#define RIGHT 1
#define LEFT 3

// parameters: int dataPin, int clkPin, int csPin, int numDevices
LedControl lc = LedControl (12, 11, 10, 1);

// pins for the buttons commanding the direction
int buttonUp = 6;
int buttonRight = 7;
int buttonDown = 8;
int buttonLeft = 9;
// pins for the shift register for the 5 level LEDs
int latchPin = 4;   // orange
int clockPin = 2;   // yellow
int dataPin = 3;    // grey

// several static variables:
static int speedTime[5] = {700, 550, 400, 300, 230}; // the higher the int, the faster the snake moves
static int currentLevel;  // eg. used to iterate through the speedTime-Array
static int score;         // increased after each eaten treat, after certain scores currentLevel increases
static int direction;     // direction in which the snake moves (see defined constants)
static int treatPos;      // treat the snake wants to eats, first digit = x-position, second digit = y-position
static int snakeBody[64]; // snakeBody[0] represents the head, first digit = x-position, second digit = y-position
static int snakeLength;   // eg. used to iterate through the snakeBody-Array


void setup() 
{
  lc.shutdown(0, false);      // false -> normal operation, true -> power-down-mode
  lc.setIntensity(0,15);      // brightness from 1 - 15
  lc.clearDisplay(0);
  Serial.begin(9600);
  randomSeed(analogRead(0));

  pinMode(buttonUp, INPUT);
  pinMode(buttonDown, INPUT);
  pinMode(buttonLeft, INPUT);
  pinMode(buttonRight, INPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  lightTest(); 
}

void loop()
{
  gameReset();
  displayCountDown();
  bool alive = true;
  setSnake();
  setTreat();
  displayMatrix();
  while (alive)
  {
    direction = directionInput();
    alive = nextMove();
    displayMatrix();
  }  
}
    
/* sets LEDs on Matrix on/off according to current positions of the snake and the treat */
void displayMatrix () {
  lc.clearDisplay(0);
  // display the treat:
  lc.setLed(0, treatPos % 10, treatPos / 10, true);
  // display the snakes body:
  for (int i = 0; i < snakeLength; i++)
  {
    lc.setLed(0, snakeBody[i] % 10, snakeBody[i] / 10, true);
  }
}

/* Listens to the 4 push buttons for commands to change the direction 
 * the higher the currentLevel, the more time the player has for his/her move
*/
int directionInput()
{
  int input = direction;
  int waitingTime = speedTime[currentLevel];
  while (waitingTime > 0)
  {
    if (digitalRead(buttonUp) == HIGH) input = UP;
    if (digitalRead(buttonDown) == HIGH) input = DOWN;
    if (digitalRead(buttonRight) == HIGH) input = RIGHT;
    if (digitalRead(buttonLeft) == HIGH) input = LEFT;
    delay(5);
    waitingTime = waitingTime - 5;
  }
  delay(waitingTime);
  return input;
}

/* Moving the snake according to the current direction. 
   Returns true if move is successfull, and false if the snake hits its head 
   */
bool nextMove()
{
  int newHeadPos = -1;
  switch (direction)
  {
     case UP:
      newHeadPos = snakeBody[0] - 1;
      break;
     case RIGHT:
      newHeadPos = snakeBody[0] + 10;
      break;
     case DOWN:
      newHeadPos = snakeBody[0] + 1;
      break;
     case LEFT:
      newHeadPos = snakeBody[0] - 10;
      break;
  }
  // check if snake hits itself (condition 1) or the wall (conditions 1-3):
  if (isSnake(newHeadPos) || newHeadPos < 0 || newHeadPos > 77 || newHeadPos % 10 > 7) 
  {
    // let the hurt head blink a few times
    letBlink(snakeBody[0], 10);
    return false;
  }
  // check if the snake eats a treat
  if (newHeadPos == treatPos)
  {
    score++;
    setTreat();
    snakeLength++;
    // level up after a certain reached score
    if (score == 3 || score == 7 || score == 12 || score == 17) 
    {
      levelUp();
    }
  }
  // update the position of the snakes body parts by iterating through the snakeBody-Array from behind (shifting)
  for (int i = snakeLength - 1; i > 0; i--)
  {
    snakeBody[i] = snakeBody[i - 1];
  }
  snakeBody[0] = newHeadPos;
  return true;
}

/* Creates a new snake in the down right corner. */
void setSnake()
{
  direction = LEFT;
  snakeLength = 3;
  // delete old body parts:
  for (int i = 3; i < 64; i++)
  {
    snakeBody[i] = 0;
  }
  // set new body parts:
  snakeBody[0] = 46;
  snakeBody[1] = 56;
  snakeBody[2] = 66;
}

/* Creates a new treat at a random free place */
void setTreat()
{
  do 
  {
    treatPos = random(0,8) * 10 + random(0,8);
  } while (isSnake(treatPos));
}

/* checks if the given position is already covered by the snake*/
bool isSnake(int pos)
{
  for (int i = 0; i < snakeLength; i++)
  {
    if (snakeBody[i] == pos) return true;
  }
  return false;
}

/* Level up after a certain reached score (=eaten treats). Do so by:
 * Increasing currentLevel (automatically makes the snake moves faster)
 * lighten up one more LED on the hardware controller (via shift register)
*/
void levelUp()
{
  currentLevel++;
  Serial.print("score: "); Serial.println(score);
  Serial.print("level: "); Serial.println(currentLevel);
  int levelLEDs[5] = {0B00000001, 0B00000011, 0B00000111, 0B00001111, 0B00011111}; 
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, levelLEDs[currentLevel]);
  digitalWrite(latchPin, HIGH);
}

/* resets scores and turns on first level LED */
void gameReset()
{
  currentLevel = 0;
  score = 0;
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, 0B00000001);
  digitalWrite(latchPin, HIGH);
}

/* to be called at each new game for player prep*/
void displayCountDown()
{
  byte no3[8] = {0B00000000,0B00111100,0B01100110,0B00000110,0B00011100,0B00000110,0B01100110,0B00111100};
  byte no2[8] = {0B00000000,0B00111100,0B01100110,0B00000110,0B00011100,0b00110000,0B01100110,0B01111110};
  byte no1[8] = {0B00000000,0B00011000,0B00111000,0B00011000,0B00011000,0B00011000,0B00011000,0B01111110};
  byte go[8]  = {0B00000000,0B01110111,0B10000101,0B10110101,0B10010101,0B01110111,0B00000000,0B00000000};
  setMatrix(no3);
  delay(1000);
  setMatrix(no2);
  delay(1000);
  setMatrix(no1);
  delay(1000);
  setMatrix(go);
  delay(1000);
}

/* prints an array of length 8 on the 8x8 LED Matrix */
void setMatrix(byte m[8]) 
{
  for (int i = 0; i < 8; i++) 
  {
    lc.setRow(0, i, m[i]);
  }
}

/* Enlights LED 1 - 5 */
void lightTest()
{
  int tests[6] = {0B00000001, 0B00000011, 0B00000111, 0B00001111, 0B00011111,0B00000000}; 
  for (int i = 0; i < 6; i++) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, tests[i]);
    digitalWrite(latchPin, HIGH);
    delay(150);
  }
}

/* lets a certain LED of the matrix at position pos blink for n times*/
void letBlink(int pos, int n)
{
  for (int i = 0; i < n; i++)
  {
    lc.setLed(0, pos % 10, pos / 10, false);
    delay(80);
    lc.setLed(0, pos % 10, pos / 10, true);
    delay(80);
  }
}
