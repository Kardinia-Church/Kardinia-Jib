/**
    Main lcd class
    Responsible for handling the lcds
**/

#ifndef LCD_HANDLER
#define LCD_HANDLER

#include <Wire.h>
#include "MicroLCD/MicroLCD.h"

#define LCD_WIDTH 64
#define LCD_HEIGHT 32

class LCD {
    private:
    int _addr;
    LCD_SSD1306 _LCD;
    String _lines[4] = {"**$$", "**$$", "**$$", "**$$"};
    int _fonts[4] = {FONT_SIZE_MEDIUM, FONT_SIZE_MEDIUM, FONT_SIZE_MEDIUM, FONT_SIZE_MEDIUM};
    String _displayedLinesText[4] = {"", "", "", ""};
    int _displayedLinesFont[4] = {FONT_SIZE_MEDIUM, FONT_SIZE_MEDIUM, FONT_SIZE_MEDIUM, FONT_SIZE_MEDIUM};
    int _yPos = 0;
    int _currentLine = 0;

    int getFontHeight(int font) {
        switch(font) {
            case FONT_SIZE_SMALL: {return 1;}
            case FONT_SIZE_MEDIUM: {return 2;}
            case FONT_SIZE_LARGE: {return 2;}
            case FONT_SIZE_XLARGE: {return 2;}
        }
    }

    String getBlankingString(int font) {
        switch(font) {
            case FONT_SIZE_SMALL: {return "                      ";}
            case FONT_SIZE_MEDIUM: {return "               ";}
            case FONT_SIZE_LARGE: {return "               ";}
            case FONT_SIZE_XLARGE: {return "               ";}
        } 
    }

    public:
    LCD(int address) {
        _addr = address;
    }

    //Flash a dot in the left top corner to reset the LCD (fixed a issue where the lcd would sometimes not start correctly)
    void reset() {
        Serial.println(" Complete");
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

    void clear() {
        _displayedLinesText[0] = "**$$";
        _displayedLinesText[1] = "**$$";
        _displayedLinesText[2] = "**$$";
        _displayedLinesText[3] = "**$$";
        _displayedLinesFont[0] = 0;
        _displayedLinesFont[1] = 0;
        _displayedLinesFont[2] = 0;
        _displayedLinesFont[3] = 0;
        _currentLine = 0;
        _yPos = 0;
        _LCD.clear();
    }

    //Setup the lcd
    void initalize() {
        Serial.print("Setup LCD At Addr: " + (String)_addr);
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

    //Set the text that will be shown on the next update()
    void setTextToShow(String line1="", String line2="", String line3="", String line4=""){
        _lines[0] = line1;
        _lines[1] = line2;
        _lines[2] = line3;
        _lines[3] = line4;
    }

    //Set the text that will be shown on the next update()
    void setTextToShow(String line1, String line2, String line3, String line4, int fontSize1, int fontSize2, int fontSize3, int fontSize4){
        _lines[0] = line1;
        _lines[1] = line2;
        _lines[2] = line3;
        _lines[3] = line4;
        _fonts[0] = fontSize1;
        _fonts[1] = fontSize2;
        _fonts[2] = fontSize3;
        _fonts[3] = fontSize4;
    }

    //Show text
    void showText(String line1="", String line2="", String line3="", String line4="") {
        clear();
        _currentLine = 0;
        _yPos = 0;
        setTextToShow(line1, line2, line3, line4, FONT_SIZE_MEDIUM, FONT_SIZE_MEDIUM, FONT_SIZE_SMALL, FONT_SIZE_SMALL);
        while(update());
    }

    //Show text
    void showText(String line1, String line2, String line3, String line4, int fontSize1, int fontSize2, int fontSize3, int fontSize4) {
        clear();
        _currentLine = 0;
        _yPos = 0;
        setTextToShow(line1, line2, line3, line4, fontSize1, fontSize2, fontSize3, fontSize4);
        while(update());
    }

    //Show an error
    void showError(String errorLine1="Unknown", String errorLine2="", String errorLine3="") {
        clear();
        _currentLine = 0;
        _yPos = 0;
        setTextToShow("Error!", errorLine1, errorLine2, errorLine3, FONT_SIZE_MEDIUM, FONT_SIZE_MEDIUM, FONT_SIZE_SMALL, FONT_SIZE_SMALL);
        while(update());
    }
    
    //Update the display
    bool update() {
        if(_lines[_currentLine] != _displayedLinesText[_currentLine] || _fonts[_currentLine] != _displayedLinesFont[_currentLine]) {
            //Clear the line first
            _LCD.setFontSize(_displayedLinesFont[_currentLine]);
            _LCD.setCursor(0, _yPos);
            _LCD.print(getBlankingString(_displayedLinesFont[_currentLine]));

            if(_fonts[_currentLine] != _displayedLinesFont[_currentLine]) { _displayedLinesFont[_currentLine] = _fonts[_currentLine];}
            if(_lines[_currentLine] != _displayedLinesText[_currentLine]) { _displayedLinesText[_currentLine] = _lines[_currentLine];}
            _LCD.setFontSize(_fonts[_currentLine]);
            _LCD.setCursor(10, _yPos);
            _LCD.println(_lines[_currentLine]);
        }

        _yPos += getFontHeight(_fonts[_currentLine]);
        _currentLine++;
        if(_currentLine >= 4){_currentLine = 0; _yPos = 0;}
        return _currentLine != 0;
    }
};

#endif
