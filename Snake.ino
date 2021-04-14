#include <LedControl.h>
//#include <ArrayField.h>
#include <time.h>
#define DIM 10
#define UP 0
#define DOWN 2
#define RIGHT 1
#define LEFT 3

// LedControl(int dataPin, int clkPin, int csPin, int numDevices=1);
LedControl lc = LedControl (12, 11, 10, 1);
// input and output definitions
int buttonUp = 6;
int buttonRight = 7;
int buttonDown = 8;
int buttonLeft = 9;
int latchPin = 4; // before: 8 (orange)
int clockPin = 2; // before: 12 (yellow)
int dataPin = 3; // before 11 (grey)

// constants for the field contents
const int WALL = -1;        // walls of the field
const int TREAT = 9;        // treat the snake wants to eat
const int HEAD = 1;

// several static variables:
static int lives = 1;
static int speedTime = 700;
static int score = 0;
static int currentLevel = 0;
static int direction;       // direction in which the snake moves (0 = up, 1 = right, 2 = down, 3 = left)
static int treatPosX;
static int treatPosY;       
static int headPosX;      // column
static int headPosY;      // row

static int field[DIM][DIM]= { {WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL},
                              {WALL,0,0,0,0,0,0,0,0,WALL},
                              {WALL,0,0,0,0,0,0,0,0,WALL},
                              {WALL,0,0,0,0,0,0,0,0,WALL},
                              {WALL,0,0,0,0,0,0,0,0,WALL},
                              {WALL,0,0,0,0,0,0,0,0,WALL},
                              {WALL,0,0,0,0,0,0,0,0,WALL},
                              {WALL,0,0,0,0,0,0,0,0,WALL},
                              {WALL,0,0,0,0,0,0,0,0,WALL},
                              {WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL}};


void setup() {
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
  displayCountDown();
  
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, 0B00000001);
  digitalWrite(latchPin, HIGH);

}

void loop()
{
  lives = 1;
  setSnake();
  setTreat();
  displayMatrix();
  while (lives == 1)
  {
    direction = directionInput();
    slither();
    displayMatrix();
  }
}

// sets LEDs on Matrix on/off according to current state of the field. 
void displayMatrix () {
  lc.clearDisplay(0);
  for (int i = 0; i < 8; i++)
  {
     for (int j = 0; j < 8; j++)
     {
       if(field[i+1][j+1] == 0)
       {
          continue;
       }
       lc.setLed(0,i,j,true);
     }
  }
}

// listens to the push buttons for orders to change the direction
int directionInput()
{
  int input = direction;
  int waitingTime = speedTime;
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

void slither()
{
  // save the new position temporarily:
  int newHeadPosX = headPosX;
  int newHeadPosY = headPosY;
  // updates the new position according to the current chosen direction:
  switch (direction)
  {
    case UP: 
      newHeadPosY--;
      break;
    case RIGHT: 
      newHeadPosX++;
      break;
    case DOWN:
      newHeadPosY++;
      break;
    case LEFT:
      newHeadPosX--;
      break;
  }
  switch (field[newHeadPosY][newHeadPosX])
  {
    case WALL:  // snake hit the wall. The loop lets the snakes head blink to symbolize the injury
      for (int i = 0; i < 10; i ++)
      {
        lc.setLed(0,headPosY-1,headPosX-1,false);
        delay(100);
        lc.setLed(0,headPosY-1,headPosX-1,true);
        delay(100);
      }
      lives--;
      speedTime = 700;
      score = 0;
      currentLevel = 0;
      lightTest(); 
      break;
    case 0:     // snake has free place to move in the current direction. The heads position gets updated
      field[headPosY][headPosX] = 0;
      headPosY = newHeadPosY;
      headPosX = newHeadPosX;
      field[headPosY][headPosX] = HEAD;
      break; 
    case TREAT:   // snake eats the treat. FOR NOW: end game so that a new snake/treat is created
      score++;
      levelUp();
      lives--;
      break;
    default:
      break;
  }
}

void setSnake()
{
  field[headPosY][headPosX] = 0;
  direction = random(0,4);
  do 
  {
    headPosY = random(3,7);  // row
    headPosX = random(3,7);  // column
  } while (field[headPosY][headPosX] != 0);
  field[headPosY][headPosX] = HEAD;
}

void setTreat()
{
  field[treatPosY][treatPosX] = 0;
  do 
  {
    treatPosY = random(1,9);  // row
    treatPosX = random(1,9);  // column
  } while (field[treatPosY][treatPosX] != 0);
  field[treatPosY][treatPosX] = TREAT;
}

// always level up after 3 eaten treats.
void levelUp()
{
  Serial.print("score: "); Serial.println(score);
  if ((score % 3) == 0) 
  {
    currentLevel++;
    Serial.print("level: "); Serial.println(currentLevel);
    int levels[5] = {0B00000001, 0B00000011, 0B00000111, 0B00001111, 0B00011111}; 
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, levels[currentLevel]);
    digitalWrite(latchPin, HIGH);
    speedTime = speedTime - 150;
  }
}


void displayCountDown()
{
  byte no3[8] = {0B00000000,0B00111100,0B01100110,0B00000110,0B00011100,0B00000110,0B01100110,0B00111100};
  byte no2[8] = {0B00000000,0B00111100,0B01100110,0B00000110,0B00011100,0b00110000,0B01100110,0B01111110};
  byte no1[8] = {0B00000000,0B00011000,0B00111000,0B00011000,0B00011000,0B00011000,0B00011000,0B01111110};
  byte go[8]  = {0B00000000,0B01110111,0B10000101,0B10110101,0B10010101,0B01110111,0B00000000,0B00000000};
  
  setMatrix(no3, 0);
  delay(1000);
  setMatrix(no2, 0);
  delay(1000);
  setMatrix(no1, 0);
  delay(1000);
  setMatrix(go, 0);
  delay(1000);

}

// m[8] = array to be printed
// byte mode = 1 inverts the colors
void setMatrix(byte m[8], byte mode) {
  int x;
  if (mode == 1) x = 0B11111111;
  if (mode == 0) x = 0B00000000;
  for (int i = 0; i < 8; i++) {
    lc.setRow(0, i, m[i] ^ x);
  }
}

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
/*

void decreaseLive()
{
  int currentLed;
  switch (lives)
  {
    case 3: currentLed = led3; break;
    case 2: currentLed = led2; break;
    case 1: currentLed = led1; break;
  }
  pinMode(currentLed, LOW);
  lives--;
}

 */
