#include "PCD8544.h"
#include "font/FontCompact.h"

/*
 *  Function: begin   
 *  Desc: Inizializza SPI e il display con reset e impostazioni base
 */
void PCD8544::begin (uint16_t blLevel /*0..100*/,
    uint16_t contrastLevel /*0..100*/,
    uint16_t biasLevel /*0..7*/,
    uint16_t tcLevel /*0..3*/
    ) {
    pinMode(_pins.cs, OUTPUT);
    ceHigh();
    pinMode(_pins.dc, OUTPUT);
    pinMode(_pins.rst, OUTPUT);
    if (_pins.bl >= 0) pinMode(_pins.bl, OUTPUT);
    backlightLevel(0);
    
    if (_pins.sclk >= 0 && _pins.mosi >= 0) {
        _spi.begin(_pins.sclk, -1, _pins.mosi);
    } else {
        _spi.begin();
    }

    
    delay(50);

    hwReset();
    delay(5);

    addressing.setFromLevel(0);
    delay(5);

    transaction([&] {
        changeSetting(tempCoeff, tcLevel);
        changeSetting(bias, biasLevel);
        changeSetting(contrast, contrastLevel);
        write(DISPLAY_ON, WRITING_MODE::CMD);
    });


    delay(2);
    clear();

    #if defined(ARDUINO_ARCH_ESP32)
        if (_pins.bl >= 0) {
            ledcSetup(_blChannel, 20000, 8);
            ledcAttachPin(_pins.bl, _blChannel);
        }
    #endif
    backlightLevel(blLevel);

    delay(1);
    softRefresh();
}

/*
 *  Function: write   
 *  Desc: Trasferisce un byte in SPI a seconda della modalità scelta (command o data)
 */
void PCD8544::write (uint8_t b, WRITING_MODE mode) {
    switch (mode) {
    case WRITING_MODE::CMD:
        dcCmd();
        break;
    case WRITING_MODE::DATA:
    default:
        dcData();
        break;
    }
    ceLow();
    _spi.transfer(b);
    ceHigh();
}
/*
 *  Function: write   
 *  Desc: Trasferisce un buffer di lunghezza specificata in SPI in modalità DATA
 */
void PCD8544::write (const uint8_t* buf, size_t len) {
    dcData(); ceLow();
    while (len--) {
        _spi.transfer(*buf++);
    }
    ceHigh();
}
/*
 *  Function: write_P   
 *  Desc: Trasferisce un buffer di lunghezza specificata in SPI in modalità DATA da sorgente in flash/PROGMEM (no RAM)
 */
void PCD8544::write_P (const uint8_t* src, size_t len, const bool invert) {
    dcData(); ceLow();
    while (len--) {
        if (!invert) _spi.transfer(FONT_READ_U8(src++));
        else {
            uint8_t b = FONT_READ_U8(src++);
            _spi.transfer(b ^= 0xFF);
        }
    }
    ceHigh();
}
/*
 *  Function: writeZeros   
 *  Desc: Trasferisce in SPI in modalità DATA n 0x00.
 */
void PCD8544::writeZeros (uint8_t n, const bool invert) {
    if (!n) return;
    dcData(); ceLow();
    while (n--) {
        if (!invert) _spi.transfer(0x00);
        else _spi.transfer(0xFF);
    }
    ceHigh();
}

/*
 *  Function: setContrast   
 *  Desc: Imposta il contrasto ed aggiorna il valore corrente
 */
void PCD8544::setContrast (uint16_t level) {
    transaction([&] {
        changeSetting(contrast, level);
    });
}
/*
 *  Function: setContrastLevels   
 *  Desc: Imposta il numero di livelli per l'impostazione di contrasto
 */
void PCD8544::setContrastLevels (uint16_t lvls) {
    setSettingLevels(contrast, lvls);
}
/*
 *  Function: setBias   
 *  Desc: Imposta il bias ed aggiorna il valore corrente
 */
void PCD8544::setBias (uint16_t level) {
    transaction([&] {
        changeSetting(bias, level);
    });
}
/*
 *  Function: setBiasLevels   
 *  Desc: Imposta il numero di livelli per l'impostazione di bias
 */
void PCD8544::setBiasLevels (uint16_t lvls) {
    setSettingLevels(bias, lvls);
}
/*
 *  Function: setTC   
 *  Desc: Imposta il coefficiente di temperatura ed aggiorna il valore corrente
 */
void PCD8544::setTC (uint16_t level) {
    transaction([&] {
        changeSetting(tempCoeff, level);
    });
}
/*
 *  Function: setTCLevels   
 *  Desc: Imposta il numero di livelli per l'impostazione di coefficiente di temperatura
 */
void PCD8544::setTCLevels (uint16_t lvls) {
    setSettingLevels(tempCoeff, lvls);
}
/*
 *  Function: setAddressing   
 *  Desc: Imposta il verso di indirizzamento del cursore, orizzontale (0) o verticale (1)
 */
void PCD8544::setAddressing(uint8_t level) {
    addressing.setFromLevel(level);                 // 0..1
    write(addressing.current, WRITING_MODE::CMD);   // invia 0x20 o 0x22
}


/*
 *  Function: backlightLevel   
 *  Desc: Imposta l'intensità di luce tramite il livello di pwm passato alla funzione (0% - 100%).
 *  Per poter utilizzare questa funzione è necessario aver impostato il pin relativo alla backlight
 *  nella dichiarazione dell'istanza della classe PCD8544.
 *  
 *  N.B. Il minimo ed il massimo della luminosità dipendono da come è stato configurato l'hardware.
 *    - Se si usa il pin pwm per alimentare la backlight del display allora il funzionamento sarà active HIGH
 *    - Se si usa il pin per comandare un transistor NPN che controlla la luminosità, il funzionamento sarà actvie LOW.
 *    - Se si usa il pin per comandare un transistor PNP che controllà la luminostità il funzionamento sarà active HIGH.
 *  
 *    Qualora il funzionamento sarà active LOW, allora 0 corrisponderà alla luminostià massima; per evitare
 *    ciò, e mantenere il classico e più intuitivo livello di luminosità in cui la luminosità minima corrisponde
 *    al livello 0 e la luminosità massima al livello 100% (255), allora bisognerà usare la funzione invertedBacklightLevel
 *    per invertire i livelli di luminosità.
 *
 *  Si consiglia di utilizzare una resistenza da circa 470 ohm in serie al pin relativo all'illuminazione del modulo.
 */
void PCD8544::backlightLevel (uint16_t level) {
    if (_pins.bl < 0) return;
    backlight.setFromLevel(level);  // salva livello e calcola current = min + x
    uint8_t pwm = backlight.current; // qui current lo usiamo come PWM 0..255
    if (_blInverted) pwm = 255 - pwm;
    #if defined(ARDUINO_ARCH_ESP32)
        ledcWrite(_blChannel, pwm);
    #else
        analogWrite(_pins.bl, pwm);
    #endif
}
/*
 *  Function: setBacklightLevels   
 *  Desc: Imposta il numero di livelli per l'impostazione di backlight
 */
void PCD8544::setBacklightLevels (uint16_t lvls) {
    setSettingLevels(backlight, lvls);
}

/*
 *  Function: invertedBacklightLevel   
 *  Desc: (SOLO ESP32) Configura il canale pwm per la backlight. Di default è usato il canale 6. È possibile
 *  cambiare il canale con questa funzioni. Ed è possibile anche modificare la frequenza e la risoluzione, ma
 *  si sconsiglia di toccare queste impostazioni poichè i livelli di backlight della libreria sono 255 e non 1023
 *  o altri valori. La possibilità di modificare questi altri due valori è stata lasciata per utenti esperti.
 */
void PCD8544::configureBacklightPWM (uint8_t channel, uint32_t freq, uint8_t resolutionBits) {
    _blChannel = channel;
    ledcSetup(_blChannel, freq, resolutionBits);
    ledcAttachPin(_pins.bl, _blChannel);
}

/*
 *  Function: invertedBacklightLevel   
 *  Desc: Imposta se i livelli di intensità di backlight del modulo display devono essere invertiti o meno:
 *  - false = 0 (min) - 255 (max)
 *  - true = 0 (max) - 255 (min)
 */
void PCD8544::invertedBacklightLevel (bool inv) {
    _blInverted = inv;
}

/*
 *  Function: setXY   
 *  Desc: Imposta il cursore
 */
void PCD8544::setXY (uint8_t x, uint8_t y) {
    if (x > 83) x = 83;
    if (y > 5) y = 5;
    write((0x40 | y), WRITING_MODE::CMD);
    write((0x80 | x), WRITING_MODE::CMD);
}

/*
 *  Function: setCursor   
 *  Desc: Imposta il cursore
 */
void PCD8544::setCursor (uint8_t x, uint8_t y) {
    transaction([&] {
        setXY(x, y);
    });
}


/*
 *  Function: clear   
 *  Desc: Pulisce il display (imposta tutti i byte a 0)
 */
void PCD8544::clear () {
    transaction([&] {
        for (uint8_t page = 0; page < PAGES; page++) {
            setXY(0, page);
            ceLow();
            dcData();
            for (uint8_t col = 0; col < COLUMNS; ++col) {
                _spi.transfer(0x00);    // 84 byte consecutivi
            }
            ceHigh();
        }
    });
    setCursor(0, 0);
}

/*
 *  Function: powerDown   
 *  Desc: Spegne il display e resetta la RAM del driver.
 */
void PCD8544::powerDown () {
    transaction([&] {
        write(POWER_DOWN, WRITING_MODE::CMD);
    });
}

/*
 *  Function: standby   
 *  Desc: Mette in standby il display (blank), lo schermo diventa vuoto, ma non viene cancellato
 *        il buffer in RAM del driver.
 */
void PCD8544::standby () {
    transaction([&] {
        write(BLANK, WRITING_MODE::CMD);
    });
}

/*
 *  Function: standby   
 *  Desc: Accende il display
 */
void PCD8544::displayOn () {
    transaction([&] {
        write(DISPLAY_ON, WRITING_MODE::CMD);
    });
}

/*
 *  Function: setFont   
 *  Desc: Imposta il font da usare.
 */
void PCD8544::setFont (const pcd8544::FontInfo& f) {
    if (!f.data) return;
    if (f.first > f.last) return;
    if (f.gWidth == 0) return;

    _font = f;
    _fontReady = true;
}

/*
 *  Function: drawChar   
 *  Desc: Prende in input un carattere e lo trasferisce in SPI al driver seguendo lo schema di caratteri
 *      definito dal font in uso.
 *      N.B.
 *      È necessario che sia presente la cartella "/font" con i relativi file generali e che sia presente la
 *      cartella del font da usare, e che il font sia stato correttamente settato tramite il metodo setFont.
 */
void PCD8544::drawChar (char c, const bool inverted) {
    if (!_fontReady) return;
    uint8_t uc = (uint8_t)c;
    if (uc < _font.first || uc > _font.last) {
        uc = (uint8_t)'?';
    }
    const uint16_t index = (uint16_t)(uc - _font.first) * _font.gWidth;
    write_P(_font.data + index, _font.gWidth, inverted);
    writeZeros(_font.gSpacing, inverted);
}


/*
 *  Function: print   
 *  Desc: Prende in input una stringa e la stampa sul display. È possibile scegliere se evidenziare la stringa o meno.
 *      N.B.
 *      È necessario che sia presente la cartella "/font" con i relativi file generali e che sia presente la
 *      cartella del font da usare, e che il font sia stato correttamente settato tramite il metodo setFont.
 */
void PCD8544::print (const char* str, const bool highlighted) {
    if (!str || !_fontReady) return;
    while (*str) drawChar(*str++, highlighted);
}
void PCD8544::print (char c, const bool highlighted) {
    char str[2] = {c, '\0'};
    print(str, highlighted);
}
void PCD8544::print (int value, const bool highlighted) {
    char str[12];
    itoa(value, str, 10);
    print(str, highlighted);
}
void PCD8544::print (unsigned int value, const bool highlighted) {
    char str[12];
    itoa(value, str, 10);
    print(str, highlighted);
}
void PCD8544::print (float value, const uint8_t decimals,  const bool highlighted) {
    char str[20];
    dtostrf(value, 0, decimals, str);
    print(str, highlighted);
}


void PCD8544::print(const __FlashStringHelper* fstr, bool highlighted) {
#if defined(ARDUINO_ARCH_AVR)
  const char* p = reinterpret_cast<const char*>(fstr);
  char c;
  while ((c = pgm_read_byte(p++))) {
    drawChar(c); // o drawCharHighlighted se usi 'highlighted'
  }
#else
  // Su ESP32 (e molte altre), la flash è memory-mapped: puoi leggerla come un C-string normale
  const char* p = reinterpret_cast<const char*>(fstr);
  while (*p) {
    drawChar(*p++); // idem: gestisci 'highlighted' se vuoi
  }
#endif
}


/*
 *  Function: fillRow   
 *  Desc: Colora interamente la riga selezionata (8x84 px)
 */
void PCD8544::fillRow (uint8_t y) {
    transaction([&] {
        setXY(0, y);
        dcData(); ceLow();
        for (uint8_t x = 0; x < COLUMNS; x++) {
            _spi.transfer(0xFF);
        }
        ceHigh();
    });
}

/*
 *  Function: drawStraightLine   
 *  Desc: Permette di creare una linea dritta, orizzontale o verticale.
 *  Parametri:
 *      - horizontal: true disegna linea orizzontale, false verticale
 *      - c1: rappresenta la prima coordinata (inizio) della linea relativa alla sua direzione
 *      - c2: rappresenta la seconda coordinata (fine) della linea relativa alla sua direzione
 *      - oc: rappresenta la coordinata complementare. Se si è scelto il verso orizzontale rappresenterà
 *            la y, altrimenti la x.
 *      - borderWidth: rappresenta lo spessore della linea in pixel (da 0 a 8).
 * 
 *  N.B.
 *  In questa funzione le y hanno come unità di misura i pixel e non le pagine ("righe") del display. Pertanto ogni y può assumere un 
 *  valore da 0 a 48 px e NON da 0 a 6. Inoltre la funzione per poter funzionare correttamente è necessario tener conto delle 
 *  dimensioni massime del display (in pixel).
 */

void PCD8544::drawStraightLine (uint8_t c1, uint8_t c2, uint8_t oc, bool horizontal, uint8_t borderWidth) {
    if (borderWidth == 0) return;
    const uint8_t HEIGHT = PAGES * 8;

    if (horizontal) {
        // --- linea orizzontale: X = c1..c2, Y = oc (spessore = borderWidth in pixel)
        if (oc >= HEIGHT) oc = HEIGHT - 1;
        if (c1 > c2) { uint8_t t = c1; c1 = c2; c2 = t; }
        if (c1 >= COLUMNS) return;
        if (c2 >= COLUMNS) c2 = COLUMNS - 1;

        const uint8_t page   = oc >> 3;          // 0..5
        const uint8_t yRest  = oc & 7;           // 0..7

        // maschera sulla pagina corrente
        uint8_t lowMask = ((uint8_t)0xFFu >> (8 - borderWidth));
        lowMask = (uint8_t)( (lowMask << yRest) & 0xFFu );

        // se lo spessore "sfonda" la pagina, prepara anche la maschera sulla pagina successiva
        uint8_t highMask = 0;
        if (yRest + borderWidth > 8) {
            const uint8_t spill = (uint8_t)(yRest + borderWidth - 8); // 1..7
            highMask = (uint8_t)(0xFFu >> (8 - spill));               // bit bassi
        }

        transaction([&] {
            // pagina corrente
            setXY(c1, page);
            dcData(); ceLow();
            for (uint8_t x = c1; x <= c2; ++x) _spi.transfer(lowMask);
            ceHigh();

            // eventuale pagina successiva
            if (highMask && page + 1 < (HEIGHT / 8)) {
                setXY(c1, (uint8_t)(page + 1));
                dcData(); ceLow();
                for (uint8_t x = c1; x <= c2; ++x) _spi.transfer(highMask);
                ceHigh();
            }
        });

    } else {
        // --- linea verticale: Y = c1..c2, X = oc (spessore = borderWidth in colonne)
        if (oc >= COLUMNS) return;
        if (c1 > c2) { uint8_t t = c1; c1 = c2; c2 = t; }
        if (c2 >= HEIGHT) c2 = HEIGHT - 1;

        const uint8_t firstPage = c1 >> 3;       // pagina di partenza
        const uint8_t lastPage  = c2 >> 3;       // pagina di arrivo

        transaction([&] {
            for (uint8_t page = firstPage; page <= lastPage && page < (HEIGHT/8); ++page) {
                // bit di inizio/fine all'interno della pagina
                const uint8_t startBit = (page == firstPage) ? (c1 & 7) : 0;
                const uint8_t endBit   = (page == lastPage)  ? (c2 & 7) : 7;

                // maschera per i bit da startBit a endBit (inclusi)
                uint8_t mask = (uint8_t)(0xFFu << startBit);
                mask &= (uint8_t)(0xFFu >> (7 - endBit));

                // disegna la stessa maschera su 'borderWidth' colonne adiacenti
                const uint8_t xEnd = (uint8_t)min<int>(oc + borderWidth - 1, COLUMNS - 1);
                for (uint8_t x = oc; x <= xEnd; ++x) {
                    setXY(x, page);
                    dcData(); ceLow();
                    _spi.transfer(mask);
                    ceHigh();
                }
            }
        });
    }
}
/*void PCD8544::drawStraightLine (const uint8_t c1, const uint8_t c2, const uint8_t oc, const bool horizontal, const uint8_t borderWidth) {
    if (horizontal) {
        transaction([&] {
            uint8_t yRest = (uint8_t)(oc % (uint8_t)8);
            uint8_t y = (uint8_t)((oc / (uint8_t)8));

            setXY(c1, y);
            dcData(); ceLow();
            uint8_t lineLen = c2 - c1 < 0 ? COLUMNS - c1 + c2 : c2 - c1;
            for (uint8_t x = 0; x < lineLen; x++) {
                uint8_t line = (uint8_t)pow((uint8_t)2, borderWidth) - 1;
                _spi.transfer(0xFF & (line << yRest)); 
            }
        });
    }
}*/





/*
 *  Function: drawStraightLine   
 *  Desc: Questa funzione permette di disegnare all'interno di uno spazio (rect) pre-determinato, passando un buffer di byte.
 *  Parametri: 
 *      - x: coordinata x di inizio del rettangolo (vertice in alto a sinistra)
 *      - y: coordinata y di inizio del rettangolo (vertice in alto a sinistra)
 *      - width: larghezza del rettangolo
 *      - height: altezza del rettangolo
 *      - buff: buffer di byte da stampare sul display
 */
void PCD8544::drawInRect (const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height, const uint8_t* buff) {
    uint8_t maxY = PAGES * 8;
    if (x + width > COLUMNS || y + height > maxY) return;
    if (width == 0 || height == 0) return;

    // Creazione rettangolo     —————————————————————————————————————————————————————————————————————
    uint8_t pageStart = y / 8; // Pagina di inizio
    uint16_t yEnd = uint16_t(y) + uint16_t(height) - 1;   // sicuro anche se height=1
    uint8_t  pageEnd = yEnd / 8;    // Pagina di fine
   
    const uint8_t pagesInRect = pageEnd - pageStart + 1;
    
    size_t buffSize = size_t(width) * pagesInRect;  // dimensione del buffer
    if (buffSize > COLUMNS * PAGES) return;
    // ——————————————————————————————————————————————————————————————————————————————————————————————

    uint8_t shift = y & 7; // (y % 8) shift verticale
    const uint8_t hmask = (height == 8) ? 0xFF : (uint8_t)((1u << height) - 1u);    // maschera altezza
    // invio dati
    transaction([&] {
        
        // pagina bassa
        setXY(x, pageStart); 
        dcData(); ceLow();
        for (uint8_t c=0; c<width; ++c) _spi.transfer( (buff[c] & hmask) << shift );
        ceHigh();

        // spill su pagina successiva
        if (shift && (pageStart+1) < PAGES) {
          setXY(x, pageStart+1); 
          dcData(); ceLow();
          for (uint8_t c=0; c<width; ++c) _spi.transfer( (buff[c] & hmask) >> (8 - shift) );
          ceHigh();
        }
        
    });
    
}