/*************************************************

 * WORDUINO: an Arduino-based game inspired by Wordle. 
 This program tries to emulate the Wordle game with the Arduino environment. 

 author: Chiara Giacanelli 
 the code of the winning and starting melodies were taken from https://github.com/robsoncouto/arduino-songs/blob/master/pacman/pacman.ino
 the project was inspired by the video: https://www.youtube.com/watch?v=N5DC3wrDHUE&t=186s but the code and the usage of more components is original

 date of creation: December 2022

 *************************************************/

#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_E6  1319
#define NOTE_G6  1568

// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 7, d7 = 6;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int numRows = 2;
const int numCols = 16;

//constants for the joystick
const int joy_button = 8;
int LCDpos_start = 5; //position of the cursor
bool flip = true; //allows to blink the cursor on and off 

//for the changing of variables when the joystick is moving
const int X_read = 0;
const int Y_read= 1;

//display of alphabet
String alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//buzzer
const int buzzer = 13;

/*notes of the melodies*/
int startingMelody[] = {

  NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
  NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
  NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

  NOTE_B4, 16,  NOTE_B5, 16,  NOTE_FS5, 16,   NOTE_DS5, 16,  NOTE_B5, 32,  //2
  NOTE_FS5, -16, NOTE_DS5, 8,  NOTE_DS5, 32, NOTE_E5, 32,  NOTE_F5, 32,
  NOTE_F5, 32,  NOTE_FS5, 32,  NOTE_G5, 32,  NOTE_G5, 32, NOTE_GS5, 32,  NOTE_A5, 16, NOTE_B5, 8

};

int winningMelody[] = {
  NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_GS4, NOTE_AS4, NOTE_C5, NOTE_AS4, NOTE_C5

};

int delayWinningMelody[] = {133, 133, 133, 400, 400, 400, 133, 266, 1200};

int losingMelody[] = {
  NOTE_C5,-4, NOTE_G4,-4, NOTE_E4,4, //45
  NOTE_A4,-8, NOTE_B4,-8, NOTE_A4,-8, NOTE_GS4,-8, NOTE_AS4,-8, NOTE_GS4,-8,
  NOTE_G4,8, NOTE_D4,8, NOTE_E4,-2, 
};

/*to control when the music will be played*/
bool musicOn = true; 
bool finalround = false;

//flag to control which page is to be displayed
int start_game = 0;
//coordinates of the joystick of the game to move in the alphabet
int joystickXPos = 0;
int joystickYPos = 0;

//flag to control which page of the game is to be displayed (alphabet or insert word)
int state_alpha = 0;
//index of where to put the letter when selected 
int letterPos = 0; 
//array to insert the words put by the user in the game
String wordsTried[] = {"_____", "_____", "_____", "_____", "_____", "_____"};

/*variables to control the 1-digit 7 segment LED display*/
const int one_digit_1 = 9, one_digit_2 = 10, one_digit_3 = 1, one_digit_4 = 0, one_digit_5 = A2, one_digit_6 = A3, one_digit_7 = A4, one_digit_8 = A5;
const int one_digit_led[] = {9, 10, 3, 2, A2, A3, A4, A5};

/*rounds*/
int rounds = 6;

/*words*/ 
String wordsToGuess[] = {"APPLE", "RAINY", "ADULT", "CHAIR", "BREAD", "EARTH", "FRESH", "GUEST", "LUCKY", "QUEEN", "WOMAN", "CLICK", "DRAMA", "ENJOY", "BEAST", "FLAME", "OCEAN", "RADIO", "VIRUS", "STONE"};
String wordToGuess= "_____";
//words to be displayed at the right of the screen if they are present in the word
String presentLetters = ""; 


void setup() {

  Serial.begin(9600);
  lcd.begin(numCols, numRows); // set up the LCD's number of columns and rows
  pinMode(joy_button, INPUT);
  digitalWrite(joy_button, HIGH);  //otherwise I cannot use it because it will always be zero
  pinMode(buzzer, OUTPUT);

  randomSeed(analogRead(0)); // I initialize the random seed

  for (int i = 0; i < 8; i++) {
    pinMode(one_digit_led[i], OUTPUT);
  }
  
  
}

void melody(int melody[]) {

  /*code taken from https://github.com/robsoncouto/arduino-songs/blob/master/pacman/pacman.ino*/
    int tempo = 105;
    if (rounds == 1) {
      tempo = 200;
    }
    int notes = sizeof(startingMelody) / sizeof(melody[0]) / 2;
    int wholenote = (60000 * 4) / tempo;
    int divider = 0, noteDuration = 0;

    for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }
    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, melody[thisNote], noteDuration * 0.9);
    // Wait for the specief duration before playing the next note.
    delay(noteDuration);
    // stop the waveform generation before the next note.
    noTone(buzzer);
  } 
}

/*function to play the music at the beginning, when winning or losing*/
void music() {
  
  if (musicOn && rounds == 6 && start_game) {
    melody(startingMelody);
    musicOn = !musicOn; //the music is played once

  } else if (finalround && musicOn) {
    //winning
    if (wordsTried[6 - rounds] == wordToGuess) {
      for (int i = 0; i< 9; i++) {
        tone(buzzer, winningMelody[i], delayWinningMelody[i]);
        delay(delayWinningMelody[i]);
      }
      musicOn = !musicOn;
    } else { /*losing*/
      melody(losingMelody);
      musicOn = !musicOn;
    }
  }
  
}

/*function to display the numbers on the 1-digit 7 segment LED display*/
void displayRounds() 
{ 
  if (rounds == 1) {

    digitalWrite(A5,HIGH);
    digitalWrite(2,HIGH);

  } else if (rounds == 2) {
    digitalWrite(3,HIGH);
    digitalWrite(2,HIGH);
    digitalWrite(9,HIGH);
    digitalWrite(A3,HIGH);
    digitalWrite(A2,HIGH);

  } else if (rounds == 3) {
    
    digitalWrite(2,HIGH);
    digitalWrite(3,HIGH); 
    digitalWrite(9,HIGH);
    digitalWrite(A2,HIGH);
    digitalWrite(A5,HIGH);

  } else if (rounds == 4) {

    digitalWrite(2,HIGH);
    digitalWrite(9,HIGH);
    digitalWrite(10,HIGH);
    digitalWrite(A5,HIGH);

  } else if (rounds == 5) {

    digitalWrite(3,HIGH);
    digitalWrite(9,HIGH);
    digitalWrite(10,HIGH);
    digitalWrite(A5,HIGH);
    digitalWrite(A2,HIGH);

  } else if (rounds == 6) {
    digitalWrite(3,HIGH);
    digitalWrite(9,HIGH);
    digitalWrite(10,HIGH);
    digitalWrite(A2,HIGH);
    digitalWrite(A3,HIGH);  
    digitalWrite(A5,HIGH);  

  } else {
    clearOneSegmentDisplay();
  }
} 


/*page displayed if the usre clicks N*/
void LCDNo() {
  lcd.clear();
  lcd.print("OKAY, GOODBYE!");
  delay(10000);
  lcd.clear();
  delay(10);
}

/*function that displays the alphabet in the game*/
void displayAlphabet() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(alphabet.substring(0, 13)); //divided in two rows
  lcd.setCursor(0,1);
  lcd.print(alphabet.substring(13, 32));
   
  if (flip) {
    lcd.setCursor(joystickXPos, joystickYPos);
    lcd.print("_");
  }
  flip = !flip;

  delay(400);


}

/*function that controls the joystick in the game*/
void joystickGame() {
  
  int j = 6;
  /*adjusting the length of the X-axis limit when there's the alphabet*/
  if (state_alpha) {
    j = 12;
  }

  /*commands to make the joystick move left and right*/
  if (analogRead(X_read) < 175 && joystickXPos > 0 && joystickXPos <= j) { //go left
    joystickXPos--;
  } else {
    if (analogRead(X_read) > 530 && joystickXPos < j && joystickXPos >= 0) 
      joystickXPos++;
      if (joystickXPos > 6 && !state_alpha) { //otherwise it will show the alphabet in wrong ways
        joystickXPos = 6;
      }
  }

  /*commands to make the joystick move up and down*/
  if (analogRead(Y_read) < 175 && joystickYPos == 1) { //go up
    joystickYPos = 0;
  } else {
    if (analogRead(Y_read) > 530 && joystickYPos == 0 && state_alpha) 
      joystickYPos = 1;
  }

  /*onclick*/
  if (digitalRead(joy_button) == 0) {
    if (!(joystickXPos == 5 && joystickYPos == 0 && !state_alpha)) {
      if (state_alpha) {
        wordsTried[6-rounds][letterPos] = alphabet[joystickXPos + joystickYPos * 13];
      } else if (joystickXPos == 6 && joystickYPos == 0 && !state_alpha) {
        state_alpha = !state_alpha; //not to display alphabet
        if (rounds == 1 || wordsTried[6 - rounds] == wordToGuess) {
          finalround = true;
          musicOn = true;
        }
        controlWord();
       
      }
    
    state_alpha= !state_alpha;
    joystickXPos = 0;
    joystickYPos = 0;
    
    if (state_alpha) {
      displayAlphabet();
    }
  }
  }
}

/*function that displays the to-be guessed word*/
void displayWord() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(wordsTried[6- rounds]);
  lcd.setCursor(6, 0);
  lcd.print(">");
  lcd.setCursor(10, 0);
  lcd.print(presentLetters);
  if (rounds < 6) { //you show the word put in the round before
    lcd.setCursor(0, 1);
    lcd.print(wordsTried[6- rounds-1]);

  }
  
 
  if (flip) {
    lcd.setCursor(joystickXPos, 0); //the word is chosen just in the first line so I put 0 as y coordinate
    letterPos = joystickXPos; //I save the last position of the letter to change it
    lcd.print(" ");
  }
  flip = !flip;
  delay(400);  

}

/*function that displays the letters present in the word but that aren't in the right position*/
void displayPresentLetters() {
    for (int i = 0; i < 5; i++) { 
      //if the letter is in the right position you put it in the to be guessed word of the next round
      if (wordsTried[6 - rounds][i] == wordToGuess[i]) {  
        wordsTried[6 - (rounds-1)][i] = wordToGuess[i];
        //else you display it just once
      } else if (wordToGuess.indexOf(wordsTried[6 - rounds][i]) != -1 && wordToGuess.indexOf(wordToGuess[i]) != wordToGuess.indexOf(wordsTried[6 - rounds][i]) ) {
        if (presentLetters.indexOf(wordsTried[6 - rounds][i]) == -1) {
           presentLetters += wordsTried[6 - rounds][i];
        }
       
      } 
    }
    
}


/*function that controls whether the word put is correct*/
void controlWord() {
  lcd.clear();

  if (wordsTried[6 - rounds] == wordToGuess) {
    lcd.print("WIN!");
    delay(4000);
    lcd.clear();
    //reinitialize all the variables
    start_game = false;
    rounds = 6;
    lcd.clear();
    wordToGuess= "_____";
    presentLetters = "";
    for (int i = 0; i< 6; i++) {
      wordsTried[i] = "_____";
    }

  } else if (rounds == 1) {
    lcd.print("OPS, YOU LOSE!");
    lcd.setCursor(0,1);
    lcd.print("THE WORD WAS...");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(wordToGuess);
    delay(2000);
    lcd.clear();
    //reinitilize all the variables
    start_game = false;
    rounds = 6;
    wordToGuess= "_____";
    presentLetters = "";
    for (int i = 0; i< 6; i++) {
      wordsTried[i] = "_____";
    }
  } else {
    presentLetters = "";
    displayPresentLetters();
    rounds--;
    delay(800);
  }

}

/*this is the function that defines the two pages of the game*/
void startGame() {
  if (state_alpha) {
    displayAlphabet();
  } else {
    displayWord();
  }
  Serial.print(wordToGuess);
}

//joystick routine on the homepage
void checkJoystick() {

  if (analogRead(X_read) < 175 && LCDpos_start == 10) { //go left
    LCDpos_start = 5;
  } else {
    if (analogRead(X_read) > 530 && LCDpos_start == 5) 
      LCDpos_start = 10;
  }
  if (digitalRead(joy_button) == 0 && LCDpos_start == 10) {
    LCDNo();
  } else if (digitalRead(joy_button) == 0 && LCDpos_start == 5) {
    int randInt = random(19);
    wordToGuess = wordsToGuess[randInt];
    startGame();
    start_game= 1;

  }

}

/*this function turns off the one segment display*/
void clearOneSegmentDisplay() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(one_digit_led[i], LOW);
  }

}

void LCD_start() {
  lcd.setCursor(1, 0);
  lcd.print("READY TO PLAY?");
  lcd.setCursor(5, 1); //start of the next line (1)
  lcd.print("Y");
  lcd.setCursor(10, 1);
  lcd.print("N");

  /*this conditional checks the position of 
  the joystick and turns the cursor on and off among its position*/
  if (flip) {
    lcd.setCursor(LCDpos_start, 1);
    lcd.print("_");
  }
  flip = !flip;

  delay(400);

}


/*this function enables the whole Arduino program to execute*/
void loop() {
  /*section that controls the one segment display, that's always active and shows the rounds left*/
  clearOneSegmentDisplay();
  displayRounds();
  
  if (start_game) { //if the game starts (so the user presses Y, the page changes into the one in which there's the game)
    joystickGame();
    music(); 
    startGame();

  } else { //homepage
    checkJoystick();
    LCD_start();
  }

}