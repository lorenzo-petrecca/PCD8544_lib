#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "font/FontInfo.h"

/*
 * ** PCD8544_lib **
 * Questa libreria è progettata per gestire il driver PCD8544, utilizzato in display LCD, 
 * come per il classico modulo display, che si trova in commercio, spesso con il nome Nokia 5110,
 * poichè questo tipo di display LCD con questo tipo di driver fu utilizzato per la prima volta per l'appunto
 * nel cellulare Nokia 5110.
 * Questo driver gestisce display LCD tipicamente 84x48 pixel. Pertanto questa libreria è progettata per gestire
 * display di queste dimensioni.
 * Il driver suddivide il display in 6 righe (Y), dette pagine ("pages"), da 0 a 5, e ogni pagina è formata da
 * 8 righe (1 byte). Il display è anche suddiviso dal driver in 84 colonne (X). Grazie a questo tipo di
 * suddivisione è possibile scrivere ogni colonna di ogni pagina tramite un byte.
 */
#define PAGES 6
#define COLUMNS 84
#define MAX_BUFFER (PAGES*COLUMNS)

// bitmask del Function Set (PCD8544)
constexpr uint8_t FUNCTION_SET = 0x20;     // base
constexpr uint8_t FS_PD = 1 << 2;          // Power-down
constexpr uint8_t FS_V  = 1 << 1;          // Vertical addressing
constexpr uint8_t FS_H  = 1 << 0;          // Extended instruction set

// scorciatoie
constexpr uint8_t BASIC  = FUNCTION_SET;               // 0x20 (H=0)
constexpr uint8_t EXTENDED = FUNCTION_SET | FS_H;      // 0x21
constexpr uint8_t BASIC_HORIZONTAL_ADDRESSING = FUNCTION_SET;           // 0x20 (V=0)
constexpr uint8_t BASIC_VERTICAL_ADDRESSING   = FUNCTION_SET | FS_V;    // 0x22 (V=1)
constexpr uint8_t POWER_DOWN = FUNCTION_SET | FS_PD;   // 0x24 (PD=1)


enum DISPLAY_CONTROL : uint8_t {
    BLANK = 0x08,
    DISPLAY_ON = 0x0C,
    ALL_SEGMENTS_ON = 0x09,
    INVERSE = 0x0D
};

class PCD8544 {
public:
    struct Pins {
        int8_t sclk;
        int8_t mosi;
        int8_t cs;
        int8_t dc;
        int8_t rst;
        int8_t bl;
    };

    struct SettingItem {
        const uint8_t  defaultValue;   // valore registro di default (es. 0xB0)
        const uint8_t  minimum;        // minimo registro (es. 0x80)
        const uint8_t  delta;          // ampiezza (es. 127 -> max = min+delta)
        uint16_t       decimalLevels;  // es. contrast=100, bias=7, tc=3
        uint16_t       decimalCurrent; // livello “umano” (0..decimalLevels)
        uint8_t        current;        // valore registro corrente (min..min+delta)

        constexpr SettingItem (uint8_t def, uint8_t min, uint8_t d, uint16_t dl)
            : defaultValue(def), minimum(min), delta(d), decimalLevels(dl), decimalCurrent(0), current(def) {}

        // Converte un livello "umano" (0..decimalLevels) in valore registro (min..min+delta)
        uint8_t mapLevelToReg(uint16_t lvl) const {
            if (lvl > decimalLevels) lvl = decimalLevels;
            // x in [0..delta]
            const uint32_t x = (uint32_t)lvl * (uint32_t)delta / (uint32_t)decimalLevels;
            return (uint8_t)(minimum + (uint8_t)x);
        }

        // Imposta DA REGISTRO (solo per casi in cui si passa già il valore assoluto)
        bool setCurrent(uint8_t regVal) {
            if (regVal < minimum || regVal > (uint8_t)(minimum + delta)) return false;
            current = regVal;
            // aggiorna un livello coerente
            const uint32_t x = (uint32_t)(regVal - minimum);
            decimalCurrent = (uint16_t)( (x * (uint32_t)decimalLevels) / (uint32_t)delta );
            return true;
        }

        void setNumberOfLevels(uint16_t levels) { decimalLevels = levels ? levels : 1; }

        // Helper per set da livello “umano”
        void setFromLevel(uint16_t lvl) {
            decimalCurrent = (lvl > decimalLevels) ? decimalLevels : lvl;
            current = mapLevelToReg(decimalCurrent);
        }

        // ottieni valore corrente (format: 0 = human format (decimal) | 1 = register format)
        uint16_t getCurrentValue (uint8_t format = 0) {
            return format == 0 ? decimalCurrent : (uint16_t)current;
        }

        // Ottieni valore minimo (format: 0 = human format (decimal) | 1 = register format)
        inline uint8_t getMinimum (uint8_t format = 0) { return format == 0 ? 0 : minimum; }
        // Ottieni valore massimo (format: 0 = human format (decimal) | 1 = register format)
        inline uint16_t getMaximum (uint8_t format = 0) { return format == 0 ? decimalLevels : minimum + delta; }
    };


    PCD8544 (SPIClass& spi, Pins pins, uint32_t spiHz = 2000000, uint8_t spiMode = SPI_MODE0)
        : _spi(spi), _pins(pins), _spiHz(spiHz), _spiMode(spiMode) {}

    void begin (uint16_t blLevel = backlight.defaultValue, uint16_t contrastLevel = contrast.defaultValue,  uint16_t biasLevel = bias.defaultValue, uint16_t tcLevel = tempCoeff.defaultValue);
    void setContrast (uint16_t level);
    void setContrastLevels (uint16_t lvls);
    void setBias (uint16_t level);
    void setBiasLevels (uint16_t lvls);
    void setTC (uint16_t level);
    void setTCLevels (uint16_t lvls);
    void setAddressing (uint8_t level = 0);
    void backlightLevel (uint16_t level);
    void setBacklightLevels (uint16_t lvls);
    #if defined(ARDUINO_ARCH_ESP32)
        void configureBacklightPWM (uint8_t channel = 6, uint32_t freq = 20000, uint8_t resolutionBits = 8);
    #else
        inline bool configBacklightPwm(uint8_t, uint32_t, uint8_t) { return true; }
    #endif
    void invertedBacklightLevel (bool inv = true);
    void clear ();
    void setCursor (uint8_t x, uint8_t y);
    void powerDown ();
    void standby ();
    void displayOn ();
    inline void softRefresh () {
        transaction([&] {
            write(POWER_DOWN, WRITING_MODE::CMD);
            delay(2);
            write(BASIC, WRITING_MODE::CMD);
            write(DISPLAY_ON, WRITING_MODE::CMD);
        });
    };
    void setFont (const pcd8544::FontInfo& f);
    void print (const char* str, const bool highlighted = false);
    void print (char c, const bool highlighted = false);
    void print (int value, const bool highlighted = false);
    void print (unsigned int value, const bool highlighted = false);
    void print (float value, const uint8_t decimals = 2,  const bool highlighted = false);
    void print (const __FlashStringHelper* fstr, const bool highlighted = false);
    void fillRow (uint8_t row);
    inline void printStringCentered (const char* str, const uint8_t notToCenterCoordinate, const bool horizontalAlignment = true, const bool highlighted = false) {
        uint16_t len = strlen(str);
        if (len == 0) return;
        if (horizontalAlignment && len >= COLUMNS) return print(str, highlighted);
        uint8_t x, y = 0;
        if (horizontalAlignment) x = (uint8_t)((uint16_t)(COLUMNS - (uint16_t)(len * (_font.gWidth + _font.gSpacing))) / (uint16_t)2);
        else y = PAGES / 2 - 1;
        horizontalAlignment ? y = notToCenterCoordinate : x = notToCenterCoordinate;
        
        setCursor(x, y);
        print(str, highlighted);
    }
    void drawStraightLine (const uint8_t c1, const uint8_t c2, const uint8_t oc, const bool horizontal, const uint8_t borderWidth);
    void drawInRect (const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height, const uint8_t* buff);

    inline uint16_t getContrast (uint8_t format = 0) {
        return contrast.getCurrentValue(format);
    };
    inline uint16_t getBrightness (uint8_t format = 0) {
        return backlight.getCurrentValue(format);
    };
    inline uint16_t getBias (uint8_t format = 0) {
        return bias.getCurrentValue(format);
    };
    inline uint16_t getTempCoeff (uint8_t format = 0) {
        return tempCoeff.getCurrentValue(format);
    };
    inline uint16_t getAddressing (uint8_t format = 0) {
        return addressing.getCurrentValue(format);
    };
    inline uint8_t getContrastMinValue (uint8_t format = 0) { return contrast.getMinimum(format); }
    inline uint8_t getBiasMinValue (uint8_t format = 0) { return bias.getMinimum(format); }
    inline uint8_t getBrightnessMinValue (uint8_t format = 0) { return backlight.getMinimum(format); }
    inline uint8_t getTempCoeffMinValue (uint8_t format = 0) { return tempCoeff.getMinimum(format); }
    inline uint8_t getContrastMaxValue (uint8_t format = 0) { return contrast.getMaximum(format); }
    inline uint8_t getBiasMaxValue (uint8_t format = 0) { return bias.getMaximum(format); }
    inline uint8_t getBrightnessMaxValue (uint8_t format = 0) { return backlight.getMaximum(format); }
    inline uint8_t getTempCoeffMaxValue (uint8_t format = 0) { return tempCoeff.getMaximum(format); }

private:
    SPIClass& _spi;
    Pins _pins;
    uint32_t _spiHz;
    uint8_t _spiMode;
    pcd8544::FontInfo _font {0,0,0,0,0,0,nullptr};
    bool _fontReady = false;
    static inline SettingItem tempCoeff{0x05, 0x04, 3, 3};
    static inline SettingItem bias {0x14, 0x10, 7, 7};
    static inline SettingItem contrast {0xB0, 0x80, 127, 100};
    static inline SettingItem addressing {BASIC_HORIZONTAL_ADDRESSING, BASIC, 0x02, 1};
    static inline SettingItem backlight {127, 0, 255, 100};
    uint8_t _blChannel = 6;
    bool _blInverted = false;

    enum class WRITING_MODE {
        CMD,
        DATA
    };

    template <class F>
    inline void transaction (F&& f) {
        _spi.beginTransaction(SPISettings(_spiHz, MSBFIRST, _spiMode));
        f();
        _spi.endTransaction();
    }

    inline void ceHigh () { digitalWrite(_pins.cs, HIGH); }
    inline void ceLow () { digitalWrite(_pins.cs, LOW); }
    inline void dcData () { digitalWrite(_pins.dc, HIGH); }
    inline void dcCmd () { digitalWrite(_pins.dc, LOW); }
    inline void hwReset () {
        delay(50);
        digitalWrite(_pins.rst, LOW);
        delay(10);
        digitalWrite(_pins.rst, HIGH);
        delay(10);
    }
    inline void changeSetting (SettingItem& setting, uint16_t level) {
        setting.setFromLevel(level);    // converte 0..levels -> valore registro
        write(EXTENDED, WRITING_MODE::CMD); // Passa in modalità estesa
        write(setting.current, WRITING_MODE::CMD);  // Invia byte corrente del valore in formato registro
        write(addressing.current, WRITING_MODE::CMD);   // torna al BASIC
        write(DISPLAY_ON, WRITING_MODE::CMD); // forza refresh
    }
    inline void setSettingLevels (SettingItem& setting, uint16_t levels) {
        setting.setNumberOfLevels(levels);
    }
    
    void write (uint8_t b, WRITING_MODE mode);
    void write (const uint8_t* buf, size_t len);
    void write_P (const uint8_t* src, size_t len, const bool invert = false);
    void writeZeros (uint8_t n, const bool invert);
    void setXY (uint8_t x, uint8_t y);
    void drawChar (char c, const bool inverted = false);
};