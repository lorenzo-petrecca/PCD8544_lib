/* 
 * Questo file di esempio costruisce e stampa sui moduli display, comunemente chiamati "5110",
 * che usano driver PCD8544 un menu di base per le impostazioni di visibilità dello schermo lcd.
 * L'esempio è pensato per l'uso su arduino. Qualora si volesse utilizzare l'esempio per programmare
 * un esp32 bisognerà adattare alcune parti del codice, in particolare la nominazione dei pin per
 * rendere lo sketch compatibile con i gpio del modello esp che si utilizza.
 */
#include <Arduino.h>
#include <SPI.h>
#include <PCD8544.h>
#include <font/mono_5x8px/data.h>
#include <font/mono_5x8px/meta.h>
#include <menu/menu.h>

#define SCK 13
#define MOSI 11
#define LCD_CS 10
#define LCD_DC 9
#define LCD_RST 8
#define LCD_BL 5 // scegliere canale PWM

#define BACK_BTN A0
#define SELECT_BTN A1
#define FORWARD_BTN A2

// ottiene il numero di cifre di un numero
uint8_t getDigits (int num) {
    if (num == 0) return 1;
    uint8_t i = 0;
    while (num != 0) {
        num /= 10;
        i++;
    }
    return i;
};

// Calcola la x di partenza per centrare il testo
uint8_t calcAbscissaToCenter (uint8_t strLen, uint8_t maxLen) {
    return (uint8_t)((uint16_t)(maxLen - (uint16_t)(strLen * (GLYPH_WIDTH + GLYPH_SPACING))) / (uint16_t)2);
}

PCD8544 lcd(SPI, {SCK, MOSI, LCD_CS, LCD_DC, LCD_RST, LCD_BL}, 1000000, SPI_MODE0);
MenuController menu({
    BACK_BTN, 
    FORWARD_BTN, 
    SELECT_BTN, 
    true, // active low = true (pulsante chiude a GND, pullup a VCC) | false (pulsante chiude a VCC, pulldown a GND)
    false, // resistenza interna (pullup/pulldown) = true (resistenza interna) | false (resistenza esterna)
    150 // debounce ms per pulsanti
});


void onBack();
const MenuItem BackItem("Indietro", onBack, nullptr, 0);    // Voce: Indietro

// Impostazioni    ——————————————————————————————————————————————————————————————————————————

void onContrast();
void onBrightness();
void onBias();
void onTC();
bool inContrast = false;
bool inBrightness = false;

const MenuItem ContrastItem ("Contrasto", onContrast, nullptr, 0);  // Voce: Contrasto
const MenuItem BrightnessItem ("Luminosita", onBrightness, nullptr, 0); // Voce: Luminosita
const MenuItem BiasItem ("Bias", onBias, nullptr, 0);   // Voce: Bias
const MenuItem TCItem ("TC", onTC, nullptr, 0); // Voce: TC

// array delle voci di "Impostazioni"
const MenuItem settingsItems[] = {
    BackItem,
    ContrastItem,
    BrightnessItem,
    BiasItem,
    TCItem,
};




// ———————————————————————————————————————————————————————————————————————————————————————————————————

// Main Menu     ——————————————————————————————————————————————————————————————————————————

const MenuItem SettingsSubMenu("Impostazioni", nullptr, settingsItems, sizeof(settingsItems) / sizeof(settingsItems[0]));   // Voce: "Impostazioni"

// array delle voci di "Main Menu"
const MenuItem mainMenuItems[] = {
    BackItem,
    SettingsSubMenu,
};

// ———————————————————————————————————————————————————————————————————————————————————————————————————
// Dichiarazione della root del menu ("Main Menu")
const MenuItem rootMenu("Main Menu", nullptr, mainMenuItems, sizeof(mainMenuItems) / sizeof(mainMenuItems[0]));



/* COMMON SETTING ACTIONS */
// Render setting
void renderSetting (const char* label, uint16_t curVal) {
    lcd.clear();
    delay(25);
    lcd.printStringCentered(label, 0);
    uint8_t valL = getDigits(curVal);
    uint8_t xval = calcAbscissaToCenter((valL + 4), COLUMNS);
    lcd.setCursor(xval, 2);
    lcd.print("- ");
    lcd.print(curVal);
    lcd.print(" +"); 
} 

// Exit action
inline void exitAction () { menu.exitAction(); lcd.softRefresh(); }
/* --------------------------------------- */


/* CONTRAST ACTION */
void contrastLeft () {
    lcd.setContrast(lcd.getContrast() - 1);
    delay(10);
}
void contrastRight () {
    lcd.setContrast(lcd.getContrast() + 1);
    delay(10);
}
void contrastRender () {
    renderSetting("Contrasto", lcd.getContrast());
}

void onContrast() {
    MenuController::Action a;
    a.onLeft = contrastLeft;
    a.onRight = contrastRight;
    a.onSelect = exitAction;
    a.onRender = contrastRender;
    
    menu.enterAction(a);
}

/* --------------------------------------- */

/* BRIGHTNESS ACTION */
void brightnessLeft () {
    lcd.backlightLevel(lcd.getBrightness() - 1);
}
void brightnessRight () {
    lcd.backlightLevel(lcd.getBrightness() + 1);
}
void brightnessRender () {
    renderSetting("Luminosita", lcd.getBrightness());
}
void onBrightness() {
    MenuController::Action a;
    a.onLeft = brightnessLeft;
    a.onRight = brightnessRight;
    a.onSelect = exitAction;
    a.onRender = brightnessRender;

    menu.enterAction(a);
}
/* --------------------------------------- */

/* BIAS ACTION */
void biasLeft () {
    lcd.setBias(lcd.getBias() - 1);
}
void biasRight () {
    lcd.setBias(lcd.getBias() + 1);
}
void biasRender () {
    renderSetting("Bias", lcd.getBias());
}
void onBias() {
    MenuController::Action a;
    a.onLeft = biasLeft;
    a.onRight = biasRight;
    a.onSelect = exitAction;
    a.onRender = biasRender;

    menu.enterAction(a);
}
/* --------------------------------------- */

/* TC ACTION */
void tcLeft () {
    lcd.setTC(lcd.getTempCoeff() - 1);
}
void tcRight () {
    lcd.setTC(lcd.getTempCoeff() + 1);
}
void tcRender () {
    renderSetting("TC", lcd.getTempCoeff());
}
void onTC() {
    MenuController::Action a;
    a.onLeft = tcLeft;
    a.onRight = tcRight;
    a.onSelect = exitAction;
    a.onRender = tcRender;

    menu.enterAction(a);
}
/* --------------------------------------- */

/* Torna indietro */
void onBack () {
    menu.exit();
    menu.displayMenu();
}
/* --------------------------------------- */

void setup() {
    Serial.begin(115200);
    delay(2000);
    lcd.begin(30, 50, 4, 2);
    delay(500);
    lcd.setFont(MONO_5x7);
    menu.attachDisplay(&lcd);
    menu.createMenu(&rootMenu);
    
    menu.displayMenu();
}

void loop() {
    menu.update();
}