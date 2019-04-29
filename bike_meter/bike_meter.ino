#include <MCUFRIEND_kbv.h>
#include <FreeDefaultFonts.h>
#include <TouchScreen.h>
#include <EEPROM.h>
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
                case 0: barcolor = TFT_RED; break;
                case 1: barcolor = TFT_ORANGE; break;
                case 2: barcolor = TFT_YELLOW; break;
                case 3: barcolor = TFT_GREEN; break;
                case 4: barcolor = TFT_BLUE; break;
                case 5: barcolor = TFT_CYAN; break;
                case 6: barcolor = TFT_MAGENTA; break;
                case 7: barcolor = TFT_PURPLE; break;
                case 8: barcolor = TFT_BROWN; break;
                case 9: barcolor = TFT_WHITE; break;
                case 10: barcolor = TFT_DARKGREY; break;
                case 11: barcolor = TFT_BLACK; break;
              }
            }
            i++;
          }
        }
        if (xpos > 0 && xpos < 120 && ypos > 180 && ypos < 220) textcolor = TFT_WHITE;
        if (xpos > 120 && xpos < 240 && ypos > 180 && ypos < 220) textcolor = TFT_BLACK;
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
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
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
    if (spd < 10) printText("0", 0x0841, 2);
    printText(String(int(spdd)), TFT_WHITE, 2);
    printText(String(int((spdd - int(spdd)) * 10)), TFT_WHITE, 1);
    tft.setFont();
    if (metric) printText(" KM/H", TFT_WHITE, 2);
    else printText(" MPH", TFT_WHITE, 2);
    tft.setCursor(2, 143);
    printText("Distance", TFT_BLACK, 2);
    tft.setCursor(2, 164);
    if (metric) printText(String(dist * 1.609, 1), TFT_BLACK, 2);
    else printText(String(dist, 1), TFT_BLACK, 2);
    if (metric) printText(" km ", TFT_BLACK, 2);
    else printText(" mi ", TFT_BLACK, 2);
    tft.setCursor(tft.width() / 2 + 2, 143);
    printText("Time", TFT_BLACK, 2);
    tft.setCursor(tft.width() / 2 + 2, 164);
    printText("00:00:00", TFT_BLACK, 2);
    tft.setCursor(2, 194);
    printText("Odometer", TFT_BLACK, 2);
    tft.setCursor(2, 215);
    if (metric) printText(String(odometer *  1.609, 0), TFT_BLACK, 2);
    else printText(String(odometer, 0), TFT_BLACK, 2);
    if (metric) printText(" km ", TFT_BLACK, 2);
    else printText(" mi ", TFT_BLACK, 2);
    tft.setCursor(tft.width() / 2 + 2, 194);
    printText("Avg. Spd.", TFT_BLACK, 2);
    tft.setCursor(tft.width() / 2 + 2, 215);
    if (metric) printText(String(avgspd * 1.609, 1), TFT_BLACK, 2);
    else printText(String(avgspd, 1), TFT_BLACK, 2);
    if (metric) printText(" KM/H ", TFT_BLACK, 2);
    else printText(" MPH", TFT_BLACK, 2);
  }
}

void drawMenu(int m) {
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, tft.width(), 20, barcolor);
  tft.setCursor(2, 2);
  int i = 0;
  switch (m) {
    case MENU_HOME:
      printText("Bikeduino", textcolor, 2);

      tft.drawRect(0, 21, tft.width(), 120, TFT_WHITE);
      tft.fillRect(0, 142, tft.width(), 50, TFT_DARKGREY);
      tft.fillRect(0, 193, tft.width(), 50, TFT_DARKGREY);
      tft.drawLine(tft.width() / 2, 142, tft.width() / 2, 243, TFT_BLACK);

      tft.drawRect(0, 255, 50, 50, TFT_GREEN);
      tft.drawRect(7, 255 + 7, 2, 35, TFT_WHITE);
      tft.drawRect(7, 255 + 41, 35, 2, TFT_WHITE);
      tft.drawLine(9, 255 + 40, 24, 255 + 14, TFT_GREEN);
      tft.drawLine(24, 255 + 15, 32, 255 + 29, TFT_GREEN);
      tft.drawLine(32, 255 + 29, 42, 255 + 12, TFT_GREEN);

      tft.drawRect(63, 255, 50, 50, TFT_YELLOW);
      tft.drawCircle(63 + 25, 255 + 25, 17, TFT_WHITE);
      tft.drawCircle(63 + 25, 255 + 25, 16, TFT_WHITE);
      tft.fillRect(63 + 7, 255 + 7, 20, 20, TFT_BLACK);
      tft.fillTriangle(63 + 26, 255 + 4, 63 + 26, 255 + 13, 63 + 18, 255 + 8, TFT_WHITE);

      tft.drawRect(126, 255, 50, 50, TFT_BLUE);
      tft.drawRect(126 + 7, 255 + 7, 35, 35, TFT_WHITE);
      tft.drawRect(126 + 8, 255 + 8, 33, 33, TFT_WHITE);
      for (int x = 0; x < 6; x++) tft.drawLine(126 + 10, 255 + 11 + 5 * x, 126 + 10 + 27, 255 + 11 + 5 * x, TFT_BLUE);

      tft.drawRect(190, 255, 50, 50, TFT_RED);
      tft.drawLine(190 + 9, 255 + 9, 190 + 40, 255 + 40, TFT_WHITE);
      tft.drawLine(190 + 8, 255 + 41, 190 + 40, 255 + 9, TFT_WHITE);
      tft.drawLine(190 + 9, 255 + 10, 190 + 39, 255 + 40, TFT_WHITE);
      tft.drawLine(190 + 10, 255 + 9, 190 + 40, 255 + 39, TFT_WHITE);
      tft.drawLine(190 + 8, 255 + 40, 190 + 39, 255 + 9, TFT_WHITE);
      tft.drawLine(190 + 9, 255 + 41, 190 + 40, 255 + 10, TFT_WHITE);
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
      for (int x = 0; x < 5; x++) tft.fillRect(0, 40 + 51 * x, tft.width(), 50, TFT_DARKGREY);
      tft.setCursor(2, 55);
      printText("Menu Bar Theme", TFT_BLACK, 2);
      tft.setCursor(2, 106);
      printText("Units: ", TFT_BLACK, 2);
      if (metric) printText("Metric", TFT_BLACK, 2);
      else printText("Imperial", TFT_BLACK, 2);
      tft.setCursor(2, 157);
      printText("Wheel Size: " + String(diameter) + "\"", TFT_BLACK, 2);
      tft.setCursor(2, 208);
      printText("Settings 4", TFT_BLACK, 2);
      tft.setCursor(2, 259);
      printText("About Bikeduino", TFT_BLACK, 2);
      break;
    case MENU_STOP:
      printText("Stop", textcolor, 2);
      break;
    case MENU_THEME:
      printText("Options > Theme", textcolor, 2);
      tft.setCursor(2, 40);
      printText("Menu Bar Color", TFT_WHITE, 2);
      tft.setCursor(2, 160);
      printText("Menu Bar Text Color", TFT_WHITE, 2);
      tft.fillRect(0, 60, 40, 40, TFT_RED);
      tft.fillRect(40, 60, 40, 40, TFT_ORANGE);
      tft.fillRect(80, 60, 40, 40, TFT_YELLOW);
      tft.fillRect(120, 60, 40, 40, TFT_GREEN);
      tft.fillRect(160, 60, 40, 40, TFT_BLUE);
      tft.fillRect(200, 60, 40, 40, TFT_CYAN);
      tft.fillRect(0, 100, 40, 40, TFT_MAGENTA);
      tft.fillRect(40, 100, 40, 40, TFT_PURPLE);
      tft.fillRect(80, 100, 40, 40, TFT_BROWN);
      tft.fillRect(120, 100, 40, 40, TFT_WHITE);
      tft.fillRect(160, 100, 40, 40, TFT_DARKGREY);
      tft.fillRect(200, 100, 40, 40, 0x0841);
      tft.fillRect(0, 180, 120, 40, TFT_WHITE);
      tft.fillRect(120, 180, 120, 40, 0x0841);
      tft.drawRect(40, 240, 160, 40, TFT_YELLOW);
      tft.setCursor(95, 252);
      printText("SAVE", TFT_WHITE, 2);
      tft.setCursor(0, 300);
      printText("Note: if you tap the home bar, your", TFT_WHITE, 1);
      tft.setCursor(0, 308);
      printText("choice will not be saved.", TFT_WHITE, 1);
      break;
    case MENU_WHEEL:
      printText("Options > Wheel", textcolor, 2);
      tft.setCursor(2, 40);
      printText("Measure the distance your front tire", TFT_WHITE, 1);
      tft.setCursor(2, 50);
      printText("travels in one rotation. Input the", TFT_WHITE, 1);
      tft.setCursor(2, 60);
      printText("distance in millimeters below.", TFT_WHITE, 1);
      for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 4; x++) {
          if (i == 11) continue;
          tft.drawRect(25 + 50 * x, 80 + 50 * y, 40, 40, TFT_WHITE);
          tft.setCursor(40 + 50 * x, 90 + 50 * y);
          if (i == 10) printText("-", TFT_WHITE, 2);
          else printText(String(i), TFT_WHITE, 2);
          i++;
        }
      }
      tft.drawRect(40, 270, 160, 40, TFT_YELLOW);
      tft.setCursor(95, 282);
      printText("SAVE", TFT_WHITE, 2);
      tft.setCursor(95, 240);
      printText("0mm", TFT_WHITE, 2);
      break;
    case MENU_ABOUT:
      printText("Options > About", textcolor, 2);
      tft.setCursor(2, 40);
      printText("Bikeduino", barcolor, 3);
      tft.setCursor(2, 65);
      printText("(c) 2019 Ian Wilt", TFT_WHITE, 2);
      tft.setCursor(2, 100);
      printText("Release " + versionNumber, TFT_WHITE, 2);
      tft.setCursor(2, 120);
      printText("Download new updates from GitHub!", TFT_WHITE, 1);
      tft.setCursor(2, 130);
      printText("https://github.com/watermeloninja", TFT_WHITE, 1);
      tft.setCursor(2, 140);
      printText("  /bikeduino/releases", TFT_WHITE, 1);
      tft.setCursor(2, 200);
      printText("Special thanks:", TFT_WHITE, 2);
      tft.setCursor(2, 220);
      printText("adafruit - GFX & TouchScreen library", TFT_WHITE, 1);
      tft.setCursor(2, 230);
      printText("prenticedavid - MCUFRIEND_kbv library", TFT_WHITE, 1);
      break;
  }
}
