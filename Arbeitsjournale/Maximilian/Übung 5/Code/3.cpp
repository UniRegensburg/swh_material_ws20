#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_I2CDevice.h>
#include <SPI.h>
#include "pitches.h" //copied the notes from the file last time. Just using the file now

#define LED_PIN 7 //Pin Connect on Arduino
#define LED_COUNT 5 //max number of points on LED strip
#define SPEAKER_PIN 8
#define TEMPO 140
#define CIRCLE_PIN 6
#define CROSS_PIN 5
#define SQUARE_PIN 4
#define CIRCLE 0
#define CROSS 1
#define SQUARE 2

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800); // LED strip
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();                  // LED matrix

/*
byte repesentations of the matrix LED's, generated by: 
https://chrome.google.com/webstore/detail/led-byte-generator/ecbmkegibbgngcjkcihanekjkmlhdoom?hl=en-GB
*/
byte matCircle[] = {B00111100, B01000010, B10000001, B10000001, B10000001, B10000001, B01000010, B00111100},
     matCross[] = {B10000001, B01000010, B00100100, B00011000, B00011000, B00100100, B01000010, B10000001},
     matSquare[] = {B11111111, B10000001, B10000001, B10000001, B10000001, B10000001, B10000001, B11111111};

//attempted to build victory fanfare
int melody[] = {
    NOTE_C5,
    NOTE_C5,
    NOTE_C5,
    NOTE_C5,
    NOTE_GS4,
    NOTE_AS4,
    NOTE_C5,
    NOTE_AS4,
    NOTE_C5,
};
int durations[] = {8, 8, 8, 2, 2, 2, 4, 8, 1};

int shape;
int points = 0;
int reactionTime = 3000; //player has 3 seconds to make the right choice
long lastMillis = 0;

bool wait = false;

void resetGame()
{
  matrix.clear();
  matrix.writeDisplay();
  points = 0;
  strip.clear();
  strip.show();
  wait = false;
}

void updatePoints(uint32_t color, int ledNum)
{
  strip.setPixelColor(ledNum, color);
  strip.show();
}

//tried to imitate "Wonrg-Choice" game show sound
void playMoeoep(){
        tone(SPEAKER_PIN, NOTE_G2, 500);
        delay(400);
        tone(SPEAKER_PIN, NOTE_C1, 700);
}

void playMelody()
{
  int wholenote = (60000 * 2) / TEMPO,
      noteDur = 0, noteDuration = 0,
      songLength = sizeof(melody) / sizeof(melody[0]);

  for (int thisNote = 0; thisNote < songLength; thisNote++)
  {
    // calculates the duration of each note
    noteDur = durations[thisNote];
    if (noteDur > 0)
    { //regular note
      noteDuration = (wholenote) / noteDur;
    }
    else if (noteDur < 0)
    { //dotted note (neg value)
      noteDuration = (wholenote) / abs(noteDur);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    //actually play notes
    tone(SPEAKER_PIN, melody[thisNote]);

    //delay needed, otherwise note would be cancelled immediately
    delay(noteDuration);

    noTone(SPEAKER_PIN); //stop current note
  }
}

void drawShape(int shape)
{
  if (shape == 0)
  {
    matrix.drawBitmap(0, 0, matCircle, 8, 8, LED_ON);
    matrix.writeDisplay();
  }
  else if (shape == 1)
  {
    matrix.drawBitmap(0, 0, matCross, 8, 8, LED_ON);
    matrix.writeDisplay();
  }
  else if (shape == 2)
  {
    matrix.drawBitmap(0, 0, matSquare, 8, 8, LED_ON);
    matrix.writeDisplay();
  }
}

bool rightChoice(int ciB, int crB, int sB)
{
  if ((shape == CIRCLE && ciB == HIGH) || (shape == CROSS && crB == HIGH) || (shape == SQUARE && sB == HIGH))
  {
    return true;
  }
  else
  {
    return false;
  }
}

void setup()
{
  pinMode(SPEAKER_PIN, OUTPUT);
  Serial.begin(9600);

  matrix.begin(0x70);      //standard i2c adress
  strip.begin();           // INITIALIZE NeoPixel strip object
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop()
{
  shape = rand() % 3;

  int waitTime = (rand() % 20) * 500; //wait time between 0 and 10 seconds

  delay(waitTime); //purposefully use delay to avoid unwanted input
  wait = true;
  drawShape(shape);
  lastMillis = millis();

//remain in wait-state until proper input
  while (wait)
  {

//if player to slow, restart game
    if(millis() - lastMillis > reactionTime){
      playMoeoep();
      resetGame();
    }

    int circleButton = digitalRead(CIRCLE_PIN);
    int squareButton = digitalRead(SQUARE_PIN);
    int crossButton = digitalRead(CROSS_PIN);

    if (circleButton == 1 ^ squareButton == 1 ^ crossButton == 1)
    {
      if (rightChoice(circleButton, crossButton, squareButton))
      {
        points++;
        playMelody();
        updatePoints(strip.Color((rand() % 255), (rand() % 255), (rand() % 255)), points - 1);
        wait = false;
      }
      else
      {
        playMoeoep();
        resetGame();
      }
    }
  }

  if (points >= 5)
  {
    resetGame();
  }
  else
  {
    matrix.clear();
    matrix.writeDisplay();
  }
}
