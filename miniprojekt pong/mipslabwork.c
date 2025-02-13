/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file modified 2017-04-31 by Ture Teknolog

   For copyright and licensing, see file COPYING */

#include <stdint.h>  /* Declarations of uint_32 and the like */
#include <pic32mx.h> /* Declarations of system-specific addresses etc */
#include "mipslab.h" /* Declatations for these labs */
#include <math.h>    /* Declaration for math functions, for ball physics*/
#include <stdio.h>

extern void _enable_interrupt(); /* Lab assistent said we needed this, i think for
activating the assembly code*/

unsigned char display2D[32][128];
unsigned char display1D[512];

volatile int *Port_E = (volatile int *)0xbf886110;
volatile int *E = (volatile int *)0xbf886100;

// paddle starting coordinates
float paddleL_ycords = 13;
float paddleR_ycords = 13;
// boll start koordinater
float ballXCords = 63;   // (128/2) - 1 pga coordinatsystemet
float ballYCords = 15;   // (32/2) - 1 pga coordinatsystemet
float ballSpeedX = -0.5; // legit random variabler
float ballSpeedY = 0;    // y är dock mindre än x pga boll behöver
// Display menu variables
int currentMenu = 1; // menu which has the modes.
int Difficulty = 1;
int mode = 0;                  // mode represents the mode in the game.
int score = 0;                 // first player score.
int score2 = 0;                // second player score.
int finalScore = 0;            // the winner score.
char finalScoreChar = '0';     // beggning of the score which is 0.
char name[] = {'A', 'A', 'A'}; // winner going to decide his name which is 3 capital letters.
int pointer = 0;               // changing the letters when pointer++ to choose the desire letter for the name.
char highScoreName[3] = {};    // winner name.
char highscorelist[3][5] = {}; // include the points and the player who won.
int amountOfGames = 0;         // how many games did they win.
char Score[] = "0";            // no need ?
char highscorelist1[5];
char highscorelist2[5];
char highscorelist3[5];
// inte röra sig lika mycket på Y koordinaterna

int timeoutcount;                                           // Global värdet som räknar mängden av perioder för timer 2
char textstring[] = "text, more text, and even more text!"; // wallah vet ej om vi kan ta bort den?

/* Interrupt Service Routine */
void user_isr(void)
{
  if (IFS(0) & 0x100)
  {                 /* bit 31 till 0 blir 1, tror att vi checkade fel del av manualen?*/
    timeoutcount++; /* Global variable for counting the ammounts of periods*/
    /* Updates display after 10 periods*/
    if (timeoutcount == 10)
    {
      timeoutcount = 0;
    }
    /* Clears the specific spot we are checking in the event-flag,
      which makes the random choice ok*/
    IFSCLR(0) = 0x100;
  }
  /* suprise: Checks interrupt flag of INT2 or switch 2, which is at bit 11
  If it interrupts, increment by 3 occurs*/
  if (IFS(0) & 0x800)
  {
    ballXCords = 63;
    ballYCords = 15;
    ballSpeedX = -0.5;
    ballSpeedY = 0;
    IFSCLR(0) = 0x800;
  }
  return;
}

/* Lab-specific initialization goes here */
void labinit(void)
{
  /*sets bit 0 to 7 in register TRISE as output, makes the 8 LED blink*/
  *E = *E & 0xffffff00;
  /* In the initialization we zero from start, so no LED are turned on in the start*/
  *Port_E = 0x0;
  /* sets bits 11 to 5 as inputs in PORT D, which contains
  data for switches and buttons*/
  TRISD = TRISD | 0x0fe0;
  /* sets bit 1, to enable button 1 as input*/
  TRISF = TRISF | 2;
  /*Stops timer and clears the control register, basically wipes it clean to start fresh*/
  T2CON = 0x0;
  /* Vi startar timer, samtidigt som vi lägger prescaler till 1:256 */
  T2CONSET = 0x8070;
  /*Timer number, håller nuvarande tiden, vi clearar den också*/
  TMR2 = 0x0;
  /* Bestämmer hur mycket Timer behöver räkna tills det blir en period*/
  PR2 = 31250; /* 80 M / 256x10 = 31250 */
  ;

  IEC(0) = 0x900; /*interrupt enabled för timer 2, IEC(0) bit 8*/
  IPC(2) = 0x1F;  /*sätter priority och subpriority
    till högst för timer 2, annars så kan fel ske*/
  /*Programet har enbart 2 interrupt sources, timer 2 och någon switch eller knapp
   (beroende på suprise) så gör det att mer kod blir onödig onödigt*/

  enable_interrupt();
  return;
}

/* This function is called repetitively from the main program */
void labwork(void)
{
  // if statements represents the mode in the menu.
  if (mode == 0)
  {
    menu();
  }
  else if (mode == 1)
  {
    Mode_1v1();
  }
  else if (mode == 2)
  {
    Mode_1vbot();
  }
  else if (mode == 3)
  {
    highscore();
  }
  /*
  For 1 v 1
  paddleL_movement();
  paddleR_movement();
  balls_engine();
  balls_paddels_physics();
  clearPixels();
  displayPixel2D(0, paddleL_ycords, 6, 2);
  displayPixel2D(126, paddleR_ycords, 6, 2);
  displayPixel2D(ballXCords, ballYCords, 2, 2); // Displays ball
  convertPixel2DTo1D();
  display_image(0, display1D);
  */
}

/* Takes objects which we define with size and places them at given coordinates on the 2D array
Is implemented from our understanding of how others have displayed
pixels on their screens, however is fully written by us*/
void displayPixel2D(int xcords, int ycords, int length, int width)
{
  int column, row; // vi definerar först raderna och kolumnerna

  for (row = 0; row < 32; row++)
  {
    for (column = 0; column < 128; column++) // vi tittar genom raderna och kolumnerna
    {
      if (column >= xcords && column <= (xcords + width - 1)) // checkar ifall värdet är i intervallet/arean mellan
      {
        if (row >= ycords && row <= (ycords + length - 1)) // koordinaterna och den angivna längden/bredden
        {
          display2D[row][column] = 0;
        }
      }
    }
  }
}

void convertPixel2DTo1D()
{
  unsigned char page, column, row, element, black;

  for (page = 0; page < 4; page++) // going through all four pages which has 8x128 pixels
  {
    for (column = 0; column < 128; column++) // check all columns in each page
    {
      black = 1;                    // the element's colour became black 1 V
      element = 0;                  // the element's colour became white 0 V
      for (row = 0; row < 8; row++) // check all row in each column
      {
        if (display2D[(8 * page) + row][column]) // check if the pixel is black 1 or white 0
        {
          element = element | black; // make the element binary number 1 to make the element black
        }
        black <<= 1; // shift the black
      }
      display1D[column + page * 128] = element; //
    }
  }
}

// To clear the display, we have to clear both the arrays
void clearPixels()
{
  int element, column, row;
  for (row = 0; row < 32; row++)
  {
    for (column = 0; column < 128; column++)
    {
      display2D[row][column] = 1;
    }
  }
  for (element = 0; element < 512; element++)
  {
    display1D[element] = 255;
  }
}
// control the movement for a player and connect it to a bushbutton.
void paddleL_movement()
{
  if ((getbtns() & 0x4) && (paddleL_ycords < (32 - 6))) // if paddleL_ycords is less than 26, the palyer can move the paddle down.
  {
    paddleL_ycords += 1.5;
  }
  if ((getbtns() & 0x2) && (paddleL_ycords > 0)) // if paddleL_ycords is bigger than 0, the player can move the paddle up. the roof is 0 so it is down by 1.
  {
    paddleL_ycords -= 1.5;
  }
}
// control the movement for a player and connect it to a bushbutton.
void paddleR_movement()
{
  if ((getbtns() & 0x1) && (paddleR_ycords < (32 - 6)))
  {
    paddleR_ycords += 1.5;
  }
  if ((getbtn1() & 0x1) && (paddleR_ycords > 0))
  {
    paddleR_ycords -= 1.5;
  }
}

// ai paddle movement.
void bot_movement()
{
  if (ballXCords <= 42) // if ball is at x coordinat 42
  {
    if (ballYCords < (paddleL_ycords - 2) && (paddleL_ycords > 0)) // if ball y coordinate less than paddle y coordinat - 2
                                                                   // to hit the ball at the head of the paddle the paddle move.
      paddleL_ycords = paddleL_ycords - Difficulty;                // paddle y coordinat decrease depending on the difficulty.
    if (ballYCords > (paddleL_ycords - 2) && (paddleL_ycords < (32 - 6)))
      paddleL_ycords = paddleL_ycords + Difficulty;
  }
}
// balls movement.
void balls_engine()
{
  ballXCords = ballSpeedX + ballXCords;
  ballYCords = ballSpeedY + ballYCords;

  // checks first if ball is in pixel intervall close to wall
  // either roof or floor
  if (((ballYCords >= 0) && (ballYCords <= 1)) || ((ballYCords >= (31 - 2)) && (ballYCords <= 32))) // when it hits 0 to 1 or 29 to 32 it means it hits the y coord for roof.
    ballSpeedY = -(ballSpeedY);
  // change causes elastic hit, only changing Ydirection

  // controlls the speed of the ball
  // makes sure that variables for speed don't increase unexpectedly
  // if this is not implemented, upp and down strikes might make ball so fast
  // that it leaves the field or just makes ball impossible to catch
  if (ballSpeedY > 1 || ballSpeedY < -1)
    ballSpeedY = (ballSpeedY > 0) - (ballSpeedY < 0);
  // Yspeed is 1 or -1, so to correct them i use a makeshift signum function
  if (ballSpeedX > 2)
    ballSpeedX = 2;
  if (ballSpeedX < -2)
    ballSpeedX = -2;

  // if ball for some reason is pressed against the screen, it can bug out
  if (ballYCords > 29)
    ballYCords = 29;
  if (ballYCords < 0)
    ballYCords = 0;
}

void balls_paddels_physics()
{
  // This implements ball physics when interacting with padel
  // Difference is Y-speed is same, X-speed changes
  // The level of change depends on where on the paddle the ball hits
  // In the middle, the angle does not change
  // the farther to the sides, the more the ball angle becomes extreme
  float intersectRelY, normRelY, angle, paddleYCenter, distanceFromCenter;
  // Checks if ball hits left paddle
  if (((ballXCords >= 0) && (ballXCords <= 3)))                                       // checks x axis
    if ((ballYCords >= (paddleL_ycords - 2)) && (ballYCords <= (paddleL_ycords + 6))) // checks y axis
    {
      /*
      intersectRelY = (paddleL_ycords + 3) - ballYCords; // 3 = paddle height / 2
      normRelY = (intersectRelY/3);
      angle = normRelY * ((5 * 3.14)/12);
      ballSpeedX = -(ballSpeedX);
      ballSpeedY = (ballSpeedX + ballSpeedY) * (-sin(angle));
      */
      paddleYCenter = paddleL_ycords + 2;
      distanceFromCenter = paddleYCenter - ballYCords;
      ballSpeedY = ballSpeedY + ((-0.2) * distanceFromCenter);
      ballSpeedX = -(ballSpeedX);
    }

  // Checks if ball hits right paddle
  if (((ballXCords >= 124) && (ballXCords <= 126)))                                   // checks x axis
    if ((ballYCords >= (paddleR_ycords - 2)) && (ballYCords <= (paddleR_ycords + 6))) // checks y axis
    {
      paddleYCenter = paddleR_ycords + 2;
      distanceFromCenter = paddleYCenter - ballYCords;
      ballSpeedY = ballSpeedY + ((-0.2) * distanceFromCenter);
      ballSpeedX = -(ballSpeedX);

      /*
      intersectRelY = (paddleR_ycords + 3) - ballYCords; // 3 = paddle height / 2
      normRelY = (intersectRelY/3);
      angle = normRelY * ((5 * 3.14)/12);
      ballSpeedX = -(ballSpeedX);
      ballSpeedY = (ballSpeedX + ballSpeedY) * (-sin(angle));
      */
    }
}

// when the score going to increase for both players
void score_system()
{ // if ball went out of the screen.
  if (ballXCords > 128)
  {
    score++; // increase the ball.
    game_restart();
  }
  // if ball went out of the screen.
  else if (ballXCords < -2)
  {
    score2++;
    game_restart();
  }
  if (score == 5)
  {
    delay(200);
    finalScore = score;
    finalScoreChar = '0' + finalScore;
    amountOfGames += 1;
    game_restart();
    mode = 3;
  }
  if (score2 == 5)
  {
    delay(200);
    finalScore = score2;
    finalScoreChar = '0' + finalScore;
    amountOfGames += 1;
    game_restart();
    mode = 3;
  }
}
// score against bot.
void score_system_bot()
{
  if (ballXCords > 128)
  {
    score++;
    game_restart();
  }
  else if (ballXCords < -2)
  {
    score2++;
    game_restart();
  }
  if (score == 5)
  {
    delay(200);
    game_restart();

    score2 = 0;
    finalScore = 0;
    string_clear();

    mode = 0;
    currentMenu = 1;
    score = 0;
  }
  if (score2 == 5)
  {
    delay(200);
    finalScore = score2;
    finalScoreChar = '0' + finalScore;
    amountOfGames += 1;
    game_restart();
    mode = 3;
  }
}
// some settings which used in both 1 v 1 mode, and bot mode.
void generalModeSetting()
{
  paddleR_movement();
  balls_engine();
  balls_paddels_physics();
  clearPixels();
  displayPixel2D(0, paddleL_ycords, 6, 2);
  displayPixel2D(126, paddleR_ycords, 6, 2);
  displayPixel2D(ballXCords, ballYCords, 2, 2); // Displays ball
  convertPixel2DTo1D();
  display_image(0, display1D);
}
// restart the game.
void game_restart()
{
  // paddle starting coordinates
  paddleL_ycords = 13;
  paddleR_ycords = 13;
  // boll start koordinater
  ballXCords = 63;   // (128/2) - 1 pga coordinatsystemet
  ballYCords = 15;   // (32/2) - 1 pga coordinatsystemet
  ballSpeedX = -0.5; // legit random variabler
  ballSpeedY = 0;    // y är dock mindre än x pga boll behöver

  clearPixels();
  displayPixel2D(0, paddleL_ycords, 6, 2);      // paddle coordinate, scale.
  displayPixel2D(126, paddleR_ycords, 6, 2);    // sec paddle coordinate, scale.
  displayPixel2D(ballXCords, ballYCords, 2, 2); // Displays ball
  convertPixel2DTo1D();
  display_image(0, display1D);
}

// mode 1 v 1 players play against each other
void Mode_1v1()
{
  paddleL_movement();
  generalModeSetting();
  score_system();
}
// one player play against a bot.
void Mode_1vbot()
{
  bot_movement();
  generalModeSetting();
  score_system_bot();
}
// to clear the strings we have in pages from 0 to 3.
void string_clear()
{
  display_string(0, "");
  display_string(1, "");
  display_string(2, "");
  display_string(3, "");
  display_update();
}

// menu function.
void menu()
{
  if (currentMenu == 1)
  {
    delay(200);
    clearPixels();
    display_string(0, "1 vs 1");
    display_string(1, "1 vs bot");
    display_string(2, "Settings");
    display_string(3, "Highscore");
    display_update();

    if (getbtns() & 0x4)
    { // if bushbotton 4 is on, go to the mode 1.
      currentMenu = 0;
      mode = 1;
    }

    else if (getbtns() & 0x2)
    { // if bushbotton 3 is on, go to the mode 2.
      currentMenu = 0;
      mode = 2;
    }
    else if (getbtns() & 0x1)
    { // if bushbotton 2 is on, go to settings.
      currentMenu = 2;
    }
    else if (getbtn1() & 0x1)
    { // if bushbotton 1 is on, go to highscore.
      currentMenu = 3;
    }
  }

  // if currentMentu =2, we got to the Diffecilty settings.
  if (currentMenu == 2)
  {
    delay(200);
    string_clear();
    display_string(0, "High");
    display_string(1, "On Coke");
    display_string(2, "On Meth");
    display_update();

    if (getbtns() & 0x4)
    { // we choose first difficilty, go back to main manu when currentMenu = 1;
      currentMenu = 1;
      string_clear();
      Difficulty = 1;
      display_update();
    }
    else if (getbtns() & 0x2)
    { // we choose sec difficilty, go back to main manu when currentMenu = 1;

      currentMenu = 1;
      string_clear();
      Difficulty = 2;
      display_update();
    }
    else if (getbtns() & 0x1)
    { // we choose third difficilty, go back to main manu when currentMenu = 1;

      currentMenu = 1;
      string_clear();
      Difficulty = 3;
      display_update();
    }
  }
  // when currentMenu = 3, we got to the highscore in the menu, to check the scores.
  if (currentMenu == 3)
  {
    delay(200);
    string_clear();
    display_string(0, "Newest Highscores");
    display_string(1, highscorelist1);
    display_string(2, highscorelist2);
    display_string(3, highscorelist3);
    display_update();
    if (getbtns() & 0x4)
    {
      delay(200);
      currentMenu = 1;
    }
  }
}
// write the name of the winner player, calculate win games.
void highscore()
{
  int games, k, i, j;

  delay(200);
  string_clear();
  display_string(0, "Write Your Name");
  display_string(1, "Chosen One");
  display_string(2, name);
  display_string(3, "  CON  <  >  ^ ");
  display_update();

  if (getbtn1() & 0x1)
  { // the player choose the desire Letter.
    name[pointer]++;
    if (name[pointer] == '[') // we want to became ASCII in certin value get back to A.
      name[pointer] = 'A';
  }
  if (getbtns() & 0x1)
  { //
    pointer++;
    if (pointer == 3)
      pointer = 2;
  }
  if (getbtns() & 0x2)
  {
    pointer--;
    if (pointer == -1)
      pointer = 0;
  }
  if (getbtns() & 0x4)
  { // check and print the winner names, amount of games the won.
    games = amountOfGames % 4;
    if (games == 0)
      games++;
    for (k = 0; k < 3; k++) // put the choosen letters as a name.
    {
      highscorelist[games - 1][k] = name[k];
    }
    highscorelist[games - 1][3] = '-';
    highscorelist[games - 1][4] = finalScoreChar;

    score = 0;
    score2 = 0;
    finalScore = 0;
    string_clear();

    mode = 0;
    currentMenu = 1;

    for (j = 0; j < 5; j++)
    {
      if (games - 1 == 0)
        highscorelist1[j] = highscorelist[games - 1][j];
      if (games - 1 == 1)
        highscorelist2[j] = highscorelist[games - 1][j];
      if (games - 1 == 2)
        highscorelist3[j] = highscorelist[games - 1][j];
    }
  }
}

/*
int integerToStr() {
    int anInteger = 13765; // or whatever

    if (anInteger == INT_MIN) { // handle corner case
        puts(INTMIN_STR);
        return 0;
    }

    int flag = 0;
    char str[128] = { 0 }; // large enough for an int even on 64-bit
    int i = 126;
    if (anInteger < 0) {
        flag = 1;
        anInteger = -anInteger;
    }

    while (anInteger != 0) {
        str[i--] = (anInteger % 10) + '0';
        anInteger /= 10;
    }

    if (flag) str[i--] = '-';

    printf("The number was: %s\n", str + i + 1);

    return 0;
}
*/
/* if(getbtns() & 0x4){
  display_string(1, letter);
  display_update();
  letter[0]++;




    if(letter[0] > 90){
      letter[0] = "A";
    }

  else if( getbtns() & 0x2){
    name[i] = letter[0];
    i++;

  }
display_update();

 }
}
*/

/*
display_string(0, "Leaderboard: score");

if(score > score2){

  display_string(1, "Player 1: " + score);
  display_string(2, "Player 2: " + score2);
  display_string(3, "Player 1 has highest score ");

}

else if(score2 > score){
  display_string(1, "Player 2: " + score2);
  display_string(2, "Player 1: " + score);
  display_string(3, "Player 2 has highest score ");

}

else{
  display_string(1, "Player 1: " + score);
  display_string(2, "Player 2: " + score2);
  display_string(3, "The result is a tie");
}

if(getbtns() & 0x4 | getbtns() & 0x2| getbtns() & 0x1| getbtn1() & 0x1){
  string_clear();
  display_update();
  mode = current_mode;
}

}

*/
