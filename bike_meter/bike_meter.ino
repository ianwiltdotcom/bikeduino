#include <MCUFRIEND_kbv.h>
#include <FreeDefaultFonts.h>
#include <TouchScreen.h>
#include <EEPROM.h>
#include "colors.h"
#include "addresses.h"
#include "menus.h"
MCUFRIEND_kbv tft;

String versionNumber = "1.0a";

const int XP = 6, XM = A2, YP = A1, YM = 7; // TOUCHSCREEN CALIBRATION
const int TS_LEFT = 132, TS_RT = 917, TS_TOP = 948, TS_BOT = 169;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;

int menu = MENU_HOME; // PUBLIC VARIABLES
int xpos, ypos;
unsigned long btntimercurrent;
long btntimerlast;
int barcolor, textcolor, ws;
bool metric;
double spd, dist, avgspd, odometer, diameter;

void setup() {
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(0);
  // Settings loading
  loadColorBar();
  EEPROM.get(ADD_METRICMODE, metric);
  EEPROM.get(ADD_WHEELDIAMETER, diameter);
  drawMenu(menu);
}

void loop() {
  int xpos, ypos;
  tp = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  xpos = map(tp.x, TS_LEFT, TS_RT, 0, tft.width());
  ypos = map(tp.y, TS_TOP, TS_BOT, 0, tft.height());
  btntimercurrent = millis();
  if (tp.z > 200 && tp.z < 1000) { // If a touch was actually detected
    if (xpos > 0 && xpos < 240 && ypos > 0 && ypos < 20) { // For when you touch the menu bar to return
      if (menu == 5) { // Reset the color to saved if in themes menu
        loadColorBar();
      }
      loadMenu(MENU_HOME);
    }
    int i = 0;
    switch (menu) { // PUT ALL BUTTON DETECTION HERE
      case MENU_HOME:
        for (int x = 0; x < 4; x++) {
          if (xpos > 0 + 63 * x && xpos < 50 + 63 * x && ypos > 255 && ypos < 305) {
            switch (i) {
              case 0: loadMenu(1); break;
              case 1: loadMenu(2); break;
              case 2: loadMenu(3); break;
              case 3: loadMenu(4); break;
            }
          }
          i++;
        }
        break;
      case MENU_OPTIONS:
        for (int y = 0; y < 5; y++) {
          if (xpos > 0 && xpos < 240 && ypos > 40 + 51 * y && ypos < 90 + 51 * y) {
            switch (i) {
              case 0: loadMenu(5); break;
              case 1:
                metric = !metric;
                EEPROM.put(ADD_METRICMODE, metric);
                loadMenu(3);
                break;
              case 2:
                ws = 0;
                loadMenu(6);
                break;
              case 3: break;
              case 4: loadMenu(7); break;
            }
          }
          i++;
        }
        break;
      case MENU_THEME:
        for (int y = 0; y < 2; y++) {
          for (int x = 0; x < 6; x++) {
            if (xpos > 0 + 40 * x && xpos < 40 + 40 * x && ypos > 60 + 40 * y && ypos < 100 + 40 * y) {
              switch (i) {
                case 0: barcolor = RED; break;
                case 1: barcolor = ORANGE; break;
                case 2: barcolor = YELLOW; break;
                case 3: barcolor = GREEN; break;
                case 4: barcolor = BLUE; break;
                case 5: barcolor = CYAN; break;
                case 6: barcolor = MAGENTA; break;
                case 7: barcolor = PURPLE; break;
                case 8: barcolor = BROWN; break;
                case 9: barcolor = WHITE; break;
                case 10: barcolor = GREY; break;
                case 11: barcolor = BLACK; break;
              }
            }
            i++;
          }
        }
        if (xpos > 0 && xpos < 120 && ypos > 180 && ypos < 220) textcolor = WHITE;
        if (xpos > 120 && xpos < 240 && ypos > 180 && ypos < 220) textcolor = BLACK;
        tft.fillRect(0, 0, tft.width(), 20, barcolor);
        tft.setCursor(2, 2);
        printText("Options > Theme", textcolor, 2);
        if (xpos > 40 && xpos < 200 && ypos > 240 && ypos < 280) {
          EEPROM.write(ADD_MENUBARCOLORA, barcolor);
          EEPROM.write(ADD_MENUBARCOLORB, barcolor >> 8);
          EEPROM.write(ADD_MENUBARTEXTA, textcolor);
          EEPROM.write(ADD_MENUBARTEXTB, textcolor >> 8);
          loadMenu(3);
        }
        break;
      case MENU_WHEEL:
        for (int y = 0; y < 3; y++) {
          for (int x = 0; x < 4; x++) {
            if (i == 11) continue;
            if (xpos > 25 + 50 * x && xpos < 25 + 50 * x + 40 && ypos > 80 + 50 * y && ypos < 80 + 50 * y + 40 && btntimercurrent - btntimerlast > 250L) {
              btntimerlast = btntimercurrent;
              if (i == 10) ws = 0;
              else {
                if (String(ws).length() < 4) {
                  if (ws == 0) ws = i;
                  else ws = (ws * 10) + i;
                }
              }
            }
            i++;
          }
        }
        tft.setCursor(95, 240);
        tft.setTextColor(WHITE, BLACK);
        tft.print(String(ws) + "mm   ");
        if (xpos > 40 && xpos < 200 && ypos > 270 && ypos < 310) {
          diameter = (ws / PI) / 25.4;
          EEPROM.put(ADD_WHEELDIAMETER, diameter);
          loadMenu(3);
        }
        break;
    }
  }
}

void loadColorBar() {
  textcolor = EEPROM.read(ADD_MENUBARTEXTA) + (EEPROM.read(ADD_MENUBARTEXTB) << 8);
  barcolor = EEPROM.read(ADD_MENUBARCOLORA) + (EEPROM.read(ADD_MENUBARCOLORB) << 8);
}

void loadMenu(int m) {
  menu = m;
  drawMenu(menu);
  delay(250);
}

void printText(String t, int c, int s) {
  tft.setTextSize(s);
  tft.setTextColor(c);
  tft.print(t);
}

void drawInfo() {
  if (menu == MENU_HOME) {
    tft.setCursor(5, 120);
    tft.setFont(&FreeSevenSegNumFont);
    double spdd;
    spdd = spd;
    if (metric) spdd *= 1.609;
    if (spd < 10) printText("0", DGREY, 2);
    printText(String(int(spdd)), WHITE, 2);
    printText(String(int((spdd - int(spdd)) * 10)), WHITE, 1);
    tft.setFont();
    if (metric) printText(" KM/H", WHITE, 2);
    else printText(" MPH", WHITE, 2);
    tft.setCursor(2, 143);
    printText("Distance", BLACK, 2);
    tft.setCursor(2, 164);
    if (metric) printText(String(dist * 1.609, 1), BLACK, 2);
    else printText(String(dist, 1), BLACK, 2);
    if (metric) printText(" km ", BLACK, 2);
    else printText(" mi ", BLACK, 2);
    tft.setCursor(tft.width() / 2 + 2, 143);
    printText("Time", BLACK, 2);
    tft.setCursor(tft.width() / 2 + 2, 164);
    printText("00:00:00", BLACK, 2);
    tft.setCursor(2, 194);
    printText("Odometer", BLACK, 2);
    tft.setCursor(2, 215);
    if (metric) printText(String(odometer *  1.609, 0), BLACK, 2);
    else printText(String(odometer, 0), BLACK, 2);
    if (metric) printText(" km ", BLACK, 2);
    else printText(" mi ", BLACK, 2);
    tft.setCursor(tft.width() / 2 + 2, 194);
    printText("Avg. Spd.", BLACK, 2);
    tft.setCursor(tft.width() / 2 + 2, 215);
    if (metric) printText(String(avgspd * 1.609, 1), BLACK, 2);
    else printText(String(avgspd, 1), BLACK, 2);
    if (metric) printText(" KM/H ", BLACK, 2);
    else printText(" MPH", BLACK, 2);
  }
}

void drawMenu(int m) {
  tft.fillScreen(BLACK);
  tft.fillRect(0, 0, tft.width(), 20, barcolor);
  tft.setCursor(2, 2);
  int i = 0;
  switch (m) {
    case MENU_HOME:
      printText("Bikeduino", textcolor, 2);

      tft.drawRect(0, 21, tft.width(), 120, WHITE);
      tft.fillRect(0, 142, tft.width(), 50, GREY);
      tft.fillRect(0, 193, tft.width(), 50, GREY);
      tft.drawLine(tft.width() / 2, 142, tft.width() / 2, 243, BLACK);

      tft.drawRect(0, 255, 50, 50, GREEN);
      tft.drawRect(7, 255 + 7, 2, 35, WHITE);
      tft.drawRect(7, 255 + 41, 35, 2, WHITE);
      tft.drawLine(9, 255 + 40, 24, 255 + 14, GREEN);
      tft.drawLine(24, 255 + 15, 32, 255 + 29, GREEN);
      tft.drawLine(32, 255 + 29, 42, 255 + 12, GREEN);

      tft.drawRect(63, 255, 50, 50, YELLOW);
      tft.drawCircle(63 + 25, 255 + 25, 17, WHITE);
      tft.drawCircle(63 + 25, 255 + 25, 16, WHITE);
      tft.fillRect(63 + 7, 255 + 7, 20, 20, BLACK);
      tft.fillTriangle(63 + 26, 255 + 4, 63 + 26, 255 + 13, 63 + 18, 255 + 8, WHITE);

      tft.drawRect(126, 255, 50, 50, BLUE);
      tft.drawRect(126 + 7, 255 + 7, 35, 35, WHITE);
      tft.drawRect(126 + 8, 255 + 8, 33, 33, WHITE);
      for (int x = 0; x < 6; x++) tft.drawLine(126 + 10, 255 + 11 + 5 * x, 126 + 10 + 27, 255 + 11 + 5 * x, BLUE);

      tft.drawRect(190, 255, 50, 50, RED);
      tft.drawLine(190 + 9, 255 + 9, 190 + 40, 255 + 40, WHITE);
      tft.drawLine(190 + 8, 255 + 41, 190 + 40, 255 + 9, WHITE);
      tft.drawLine(190 + 9, 255 + 10, 190 + 39, 255 + 40, WHITE);
      tft.drawLine(190 + 10, 255 + 9, 190 + 40, 255 + 39, WHITE);
      tft.drawLine(190 + 8, 255 + 40, 190 + 39, 255 + 9, WHITE);
      tft.drawLine(190 + 9, 255 + 41, 190 + 40, 255 + 10, WHITE);
      drawInfo();
      break;
    case MENU_GRAPH:
      printText("Graph", textcolor, 2);
      break;
    case MENU_RESET:
      printText("Reset", textcolor, 2);
      break;
    case MENU_OPTIONS:
      printText("Options", textcolor, 2);
      for (int x = 0; x < 5; x++) tft.fillRect(0, 40 + 51 * x, tft.width(), 50, GREY);
      tft.setCursor(2, 55);
      printText("Menu Bar Theme", BLACK, 2);
      tft.setCursor(2, 106);
      printText("Units: ", BLACK, 2);
      if (metric) printText("Metric", BLACK, 2);
      else printText("Imperial", BLACK, 2);
      tft.setCursor(2, 157);
      printText("Wheel Size: " + String(diameter) + "\"", BLACK, 2);
      tft.setCursor(2, 208);
      printText("Settings 4", BLACK, 2);
      tft.setCursor(2, 259);
      printText("About Bikeduino", BLACK, 2);
      break;
    case MENU_STOP:
      printText("Stop", textcolor, 2);
      break;
    case MENU_THEME:
      printText("Options > Theme", textcolor, 2);
      tft.setCursor(2, 40);
      printText("Menu Bar Color", WHITE, 2);
      tft.setCursor(2, 160);
      printText("Menu Bar Text Color", WHITE, 2);
      tft.fillRect(0, 60, 40, 40, RED);
      tft.fillRect(40, 60, 40, 40, ORANGE);
      tft.fillRect(80, 60, 40, 40, YELLOW);
      tft.fillRect(120, 60, 40, 40, GREEN);
      tft.fillRect(160, 60, 40, 40, BLUE);
      tft.fillRect(200, 60, 40, 40, CYAN);
      tft.fillRect(0, 100, 40, 40, MAGENTA);
      tft.fillRect(40, 100, 40, 40, PURPLE);
      tft.fillRect(80, 100, 40, 40, BROWN);
      tft.fillRect(120, 100, 40, 40, WHITE);
      tft.fillRect(160, 100, 40, 40, GREY);
      tft.fillRect(200, 100, 40, 40, DGREY);
      tft.fillRect(0, 180, 120, 40, WHITE);
      tft.fillRect(120, 180, 120, 40, DGREY);
      tft.drawRect(40, 240, 160, 40, YELLOW);
      tft.setCursor(95, 252);
      printText("SAVE", WHITE, 2);
      tft.setCursor(0, 300);
      printText("Note: if you tap the home bar, your", WHITE, 1);
      tft.setCursor(0, 308);
      printText("choice will not be saved.", WHITE, 1);
      break;
    case MENU_WHEEL:
      printText("Options > Wheel", textcolor, 2);
      tft.setCursor(2, 40);
      printText("Measure the distance your front tire", WHITE, 1);
      tft.setCursor(2, 50);
      printText("travels in one rotation. Input the", WHITE, 1);
      tft.setCursor(2, 60);
      printText("distance in millimeters below.", WHITE, 1);
      for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 4; x++) {
          if (i == 11) continue;
          tft.drawRect(25 + 50 * x, 80 + 50 * y, 40, 40, WHITE);
          tft.setCursor(40 + 50 * x, 90 + 50 * y);
          if (i == 10) printText("-", WHITE, 2);
          else printText(String(i), WHITE, 2);
          i++;
        }
      }
      tft.drawRect(40, 270, 160, 40, YELLOW);
      tft.setCursor(95, 282);
      printText("SAVE", WHITE, 2);
      tft.setCursor(95, 240);
      printText("0mm", WHITE, 2);
      break;
    case MENU_ABOUT:
      printText("Options > About", textcolor, 2);
      tft.setCursor(2, 40);
      printText("Bikeduino", barcolor, 3);
      tft.setCursor(2, 65);
      printText("(c) 2019 Ian Wilt", WHITE, 2);
      tft.setCursor(2, 100);
      printText("Release " + versionNumber, WHITE, 2);
      tft.setCursor(2, 120);
      printText("Download new updates from GitHub!", WHITE, 1);
      tft.setCursor(2, 130);
      printText("https://github.com/watermeloninja", WHITE, 1);
      tft.setCursor(2, 140);
      printText("  /bikeduino/releases", WHITE, 1);
      tft.setCursor(2, 200);
      printText("Special thanks:", WHITE, 2);
      tft.setCursor(2, 220);
      printText("adafruit - GFX & TouchScreen library", WHITE, 1);
      tft.setCursor(2, 230);
      printText("prenticedavid - MCUFRIEND_kbv library", WHITE, 1);
      break;
  }
}
