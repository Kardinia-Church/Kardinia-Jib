/**
    Main lcd class
    Responsible for handling the lcds
**/

#ifndef LCD_HANDLER
#define LCD_HANDLER

#include <Wire.h>
#include <MicroLCD.h>

#define LCD_WIDTH 64
#define LCD_HEIGHT 32

class LCD {
    private:
    int _addr;
    LCD_SSD1306 _LCD;
    public:
    LCD(int address) {
        _addr = address;
    }

    //Flash a dot in the left top corner to reset the LCD (fixed a issue where the lcd would sometimes not start correctly)
    void reset() {
        Serial.println("[INFO] Reset");
        _LCD.clear();
        _LCD.print(".");
        delay(100);
        _LCD.clear();
        _LCD.print(".");
        delay(100);
        _LCD.clear();
        _LCD.print(".");
        delay(100);
        _LCD.clear();
        _LCD.print(".");
        delay(100);
    }

    void clear() {_LCD.clear();}

    //Setup the lcd
    void initalize() {
        Serial.println("[INFO] Setup LCD At Addr: " + (String)_addr);
        _LCD.begin(SSD1306_SWITCHCAPVCC,_addr);
        delay(1000);
        reset();
        clear();
    }

    //Show the startup screen
    void showStartup(String version = "") {
        _LCD.clear();
        _LCD.setFontSize(FONT_SIZE_LARGE);
        _LCD.println("Booting");
        _LCD.setFontSize(FONT_SIZE_MEDIUM);
        _LCD.println("Please Wait");
        _LCD.setFontSize(FONT_SIZE_SMALL);
        _LCD.println(version);
    }

    //Show an error
    void showError(String errorLine1="Unknown", String errorLine2="", String errorLine3="") {
        _LCD.clear();
        _LCD.setFontSize(FONT_SIZE_LARGE);
        _LCD.println("Error!");
        _LCD.setFontSize(FONT_SIZE_MEDIUM);
        _LCD.println(errorLine1);
        _LCD.setFontSize(FONT_SIZE_SMALL);
        _LCD.println(errorLine2);
        _LCD.setFontSize(FONT_SIZE_SMALL);
        _LCD.println(errorLine3);
    }

    //Show a value
    void showValue(String title, String value) {
        _LCD.clear();
        _LCD.setCursor((LCD_WIDTH - title.length()) / 2, 5);
        _LCD.setFontSize(FONT_SIZE_XLARGE);
        _LCD.println(title);
        _LCD.setCursor(0, 5);
        _LCD.setFontSize(FONT_SIZE_LARGE);
        _LCD.println(value);
    }
};

#endif
