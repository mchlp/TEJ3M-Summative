#include <Charliplexing.h>
#include <LiquidCrystal_I2C.h>

const int CARD_READER_PIN = A1;
const int BUTTON_PIN = A0;

const int NO_CARD = 812;
const int TEST_MODE_CARD = 794;
const int CROSSY_ROAD_CARD = 556;

const int CARD_READ_DELTA = 10;

const int MATRIX_WIDTH = 14;
const int MATRIX_HEIGHT = 9;

boolean gameStarted = false;
int lastCardValue = 0;
bool ledStates[14][9];

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  pinMode(CARD_READER_PIN, INPUT);
  LedSign::Init();
  lcd.begin();
  lcd.backlight();
  Serial.begin(9600);
  while (! Serial);
}

void loop() {

  int cardReaderValue = analogRead(CARD_READER_PIN);
  //Serial.println(cardReaderValue);

  // checks if the card has been changed 
  if (checkNewCard()) {
    lastCardValue = analogRead(CARD_READER_PIN);
    lcd.clear();
    memset(ledStates, false, sizeof(ledStates));
    
    if (checkCard(cardReaderValue, NO_CARD, CARD_READ_DELTA)) {
      // no card
      lcd.print("No Card");
    } else if (checkCard(cardReaderValue, TEST_MODE_CARD, CARD_READ_DELTA)) {
      // test mode
      lcd.print("Test Mode");
      memset(ledStates, true, sizeof(ledStates));
      updateLED();
      while (!checkNewCard());  
      memset(ledStates, false, sizeof(ledStates));
      updateLED();
    } else if (checkCard(cardReaderValue, CROSSY_ROAD_CARD, CARD_READ_DELTA)) {
      // crossy road
      lcd.print("Crossy Road!");
      delay(1000);
      int lines[] = {2,5,6,8,9,14};
      int gap = 5;
      int score = 0;
      int btnRead;
      int curPos[] = {0,0};
      short btnUpDown = 0;
      short btnLeftRight = 0;
      int linePos[sizeof(lines)];
      unsigned long UPDATE_INTERVAL = 500;
      unsigned long lastUpdateTime = 0;
      
      for (int i=0; i<(sizeof(linePos)/sizeof(linePos[0])); i++) {
        linePos[i] = rand()%MATRIX_HEIGHT;
      }
      
      while (!checkNewCard()) {

        // player controls

        btnRead = analogRead(BUTTON_PIN);
        Serial.println(btnRead);
      
        if (btnRead > 05 && btnRead < 80) {
          btnUpDown = -1;
        } else if (btnRead > 200 && btnRead < 210) {
          btnUpDown = -1;
        } else if (btnRead > 500 && btnRead < 510) {
          btnUpDown = -1;
        } else if (btnRead > 45 && btnRead < 55) {
          btnUpDown = 1;
        }

        if (millis()-lastUpdateTime > UPDATE_INTERVAL) {
          memset(ledStates, false, sizeof(ledStates));
          
          // display LEDs
          for (int i=0; i<sizeof(lines); i++) {
            for (int y=0; y<MATRIX_HEIGHT; y++) {
              if (y > linePos[i] && y < linePos[i] + gap) {
                ledStates[lines[i]][y] = false;
              } else {
                ledStates[lines[i]][y] = true;
              }
            }
            linePos[i] = (linePos[i]+1)%MATRIX_HEIGHT;
          }

          // reset stuff
          if (ledStates[(curPos[0]+btnLeftRight)%MATRIX_WIDTH][(curPos[1]+btnUpDown)%MATRIX_HEIGHT]) {
            lcd.print("GAME OVER!");
            lcd.setCursor(0, 1);
            lcd.print("Score: ");
            lcd.print(score);
            delay(1000);
            break;
          } else {
            curPos[1] = (curPos[1]+btnUpDown)%MATRIX_HEIGHT;
            curPos[0] = (curPos[0]+btnLeftRight)%MATRIX_WIDTH;

            if (curPos[0] >= MATRIX_WIDTH-1) {
              score++;
              lcd.print("You Win!");
              lcd.setCursor(0, 1);
              lcd.print("Score: ");
              lcd.print(score);
              delay(1000);
              break;
            }
          }
          ledStates[curPos[0]][curPos[1]] = true;
          lastUpdateTime = millis();
          btnUpDown=0;
          btnLeftRight=0;
          score = curPos[0];
  
          // update LED and LCD
          updateLED();
          lcd.clear();
          lcd.print("Score: ");
          lcd.print(score);
        }
        delay(50);
        lcd.clear();
      }
    } else {
      // unrecognized card
      lcd.print("Invalid card");
    }
  }

  delay(250);
}

void updateLED() {
  for (int x=0; x<MATRIX_WIDTH; x++) {
    for (int y=0; y<MATRIX_HEIGHT; y++) {
      LedSign::Set(x,y,ledStates[x][y]);
      //LedSign::Set(x,y,true);
    }
  }
}

boolean checkNewCard() {
  if (!checkCard(analogRead(CARD_READER_PIN), lastCardValue, 20)) {
    return true;
  }
  return false;
}

boolean checkCard(int readValue, int cardValue, int delta) {
  if (abs(cardValue-readValue) <= delta) {
    return true;
  } else {
    return false;
  }
}

