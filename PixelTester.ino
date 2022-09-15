#include "settings.h"
#include "system.h"

#define FLASH_INTERVAL 50 //

// Color arrays to show/flash
const UG_COLOR allcolors[] = {C_BLACK, C_RED, C_LIME, C_BLUE, C_YELLOW, C_MAGENTA, C_CYAN, C_WHITE};
const UG_COLOR rgbcolors[] = {C_RED, C_LIME, C_BLUE};

// Menu items
boolean testing = false, flashing = false;

// Global variables
long lastFrame; // to count time
unsigned short color_pos = 0; // to cycle color arrays
unsigned int flash_timer = 0; // to flash the screen every FLASH_INTERVAL milliseconds
boolean flashing_all = false, flashing_rgb = false; // to decide what colors to flash
boolean hidden_gui = false; // to hide the GUI

void setup() {
  // I set up the Pocuter library...
  pocuter = new Pocuter();
  pocuter->begin(PocuterDisplay::BUFFER_MODE_DOUBLE_BUFFER);
  pocuter->Display->continuousScreenUpdate(false);
  // ... I maximize the display brightness...
  pocuter->Display->setBrightness(15);
  // ... I get the system color...
  pocuterSettings.systemColor = getSetting("GENERAL", "SystemColor", C_LIME);
  // ... I disable the double clicks...
  disableDoubleClick(BUTTON_A);
  disableDoubleClick(BUTTON_B);
  disableDoubleClick(BUTTON_C);
  // ... and I start counting time
  lastFrame = micros();
}

void loop() {
  // I get the delta time...
  dt = (micros() - lastFrame) / 1000.0; // in milliseconds
  lastFrame = micros();
  // ... and I get the button presses
  updateInput();

  // I catch the command to go back to the main app
  if (ACTION_BACK_TO_MENU) {
    pocuter->OTA->setNextAppID(1);
    pocuter->OTA->restart();
  }

  // If I am in the testing screen...
  if (testing) {
    // ... I set up the buttons to go back, decrease or increase the array position...
    if (ACTION_GOTO_MENU) testing = false;
    if (ACTION_COLOR_PREV) color_pos--;
    if (ACTION_COLOR_NEXT) color_pos++;
    // ... and I ensure the position does not exceed the size of the array
    color_pos = color_pos % (sizeof allcolors / sizeof allcolors[0]);
  }
  // If I am in the flashing screen...
  else if (flashing) {
    // ... if I am not flashing...
    if (!(flashing_all || flashing_rgb)) {
      // ... I set up the buttons to go back or start the two modes of flashing...
      if (ACTION_GOTO_MENU) flashing = false;
      if (ACTION_FLASH_RGB) {
        flash_timer = 0;
        flashing_rgb = true;
      }
      if (ACTION_FLASH_ALL)  {
        flash_timer = 0;
        flashing_all = true;
      }
    }
    // ... otherwise, if I am flashing...
    else {
      // ... I set up the buttons to always go back...
      if (ACTION_FLASH_STOP) {
        flashing_rgb = false;
        flashing_all = false;
      };
      // ... I increase the flash counter...
      flash_timer += dt;
      // ... until it reaches the flash interval...
      if (flash_timer >= FLASH_INTERVAL) {
        // ... then I reset the counter...
        flash_timer = 0;
        // ... I increase the array position
        color_pos++;
        // ... and I ensure the position does not exceed the size of the relevant array
        if (flashing_all) {
          color_pos = color_pos % (sizeof allcolors / sizeof allcolors[0]);
        }
        if (flashing_rgb) {
          color_pos = color_pos % (sizeof rgbcolors / sizeof rgbcolors[0]);
        }
      }
    }
  }
  // If I am in the main screen...
  else {
    // ... I set up the buttons to toggle the GUI or to enter the menu items
    if (ACTION_GOTO_TEST) {
      color_pos = 0;
      testing = true;
    }
    if (ACTION_GOTO_FLASH) {
      color_pos = 0;
      flashing = true;
    }
    if (ACTION_TOGGLE_GUI) hidden_gui = !hidden_gui;
  }

  // I set up the GUI...
  UGUI* gui = pocuter->ugui;
  // ... and I get the display size
  uint16_t sizeX;
  uint16_t sizeY;
  pocuter->Display->getDisplaySize(sizeX, sizeY);

  // If I am in the testing screen...
  if (testing) {
    // ... I fill the screen with the correct color...
    gui->UG_FillScreen(allcolors[color_pos]);
    // ... and I display the GUI if needed
    if (!hidden_gui) {
      drawButtonText(gui, "Prev", BUTTON_A, sizeX, sizeY);
      drawButtonText(gui, "Next", BUTTON_B, sizeX, sizeY);
      drawButtonText(gui, "Menu", BUTTON_C, sizeX, sizeY);
    }
  }
  // If I am in the flashing screen...
  else if (flashing) {
    // ... if I am flashing I fill the screen with the correct color...
    if (flashing_all) {
      gui->UG_FillScreen(allcolors[color_pos]);
    } else if (flashing_rgb) {
      gui->UG_FillScreen(rgbcolors[color_pos]);
    }
    // ... otherwise, if I am not flashing...
    else {
      // ... I display the flash warning...
      gui->UG_FillScreen(C_BLACK);
      gui->UG_SetForecolor(C_RED);
      gui->UG_FontSelect(&FONT_POCUTER_5X8);
      gui->UG_PutStringSingleLine(0, 0, "WARNING!");
      gui->UG_FontSelect(&FONT_POCUTER_4X6);
      gui->UG_PutStringSingleLine(0, 14, "The display will flash!");
      gui->UG_SetForecolor(pocuterSettings.systemColor);
      gui->UG_PutString(0, 26, "To stop the sequence, press any button");
      // ... and I display the GUI if needed
      if (!hidden_gui) {
        drawButtonText(gui, "RGB", BUTTON_A, sizeX, sizeY);
        drawButtonText(gui, "All", BUTTON_B, sizeX, sizeY);
        drawButtonText(gui, "Menu", BUTTON_C, sizeX, sizeY);
      }
    }
  }
  // If I am in the main screen...
  else {
    // ... I display the name of the app and a short summary...
    gui->UG_FillScreen(C_BLACK);
    gui->UG_SetForecolor(pocuterSettings.systemColor);
    gui->UG_FontSelect(&FONT_POCUTER_5X8);
    gui->UG_PutStringSingleLine(0, 0, "PixelTester");
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    gui->UG_PutString(0, 14, "Test changes the display color. Flash tries to fix dead/stuck pixels.");
    // ... and I display the GUI
    drawButtonText(gui, "Test", BUTTON_A, sizeX, sizeY);
    drawButtonText(gui, "Flash", BUTTON_B, sizeX, sizeY);
    if (hidden_gui) {
      drawButtonText(gui, "Show GUI", BUTTON_C, sizeX, sizeY);
    } else {
      drawButtonText(gui, "Hide GUI", BUTTON_C, sizeX, sizeY);
    }
  }

  // Finally, I update the screen
  pocuter->Display->updateScreen();
}


void drawButtonText(UGUI * gui, const char *str, int corner, int sizeX, int sizeY) {
  gui->UG_FontSelect(&FONT_POCUTER_4X6);
  gui->UG_SetForecolor(pocuterSettings.systemColor);
  if (corner == 0) {
    gui->UG_FillFrame(sizeX - gui->UG_StringWidth(str) - 3, 0, sizeX, 10, C_BLACK);
    gui->UG_DrawFrame(sizeX - gui->UG_StringWidth(str) - 3, 0, sizeX, 10, pocuterSettings.systemColor);
    gui->UG_PutString(sizeX - gui->UG_StringWidth(str) - 1, -1, str);
  } else if (corner == 1) {
    gui->UG_FillFrame(sizeX - gui->UG_StringWidth(str) - 3, sizeY - 11, sizeX, sizeY - 1, C_BLACK);
    gui->UG_DrawFrame(sizeX - gui->UG_StringWidth(str) - 3, sizeY - 11, sizeX, sizeY - 1, pocuterSettings.systemColor);
    gui->UG_PutString(sizeX - gui->UG_StringWidth(str) - 1, sizeY - 12, str);
  } else if (corner == 2) {
    gui->UG_FillFrame(-1, sizeY - 11, gui->UG_StringWidth(str) + 2, sizeY - 1, C_BLACK);
    gui->UG_DrawFrame(-1, sizeY - 11, gui->UG_StringWidth(str) + 2, sizeY - 1, pocuterSettings.systemColor);
    gui->UG_PutString(1, sizeY - 12, str);
  }
}
