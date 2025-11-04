#include "menu.h"
#include "../PCD8544.h"

void MenuController::scanButtons_ () {
    // Funzione per leggere lo stato dei pulsanti, tenendo conto di activeLow
    auto rd = [&](Btn& btn) {
        bool raw = digitalRead(btn.pin);
        return _pins.activeLow ? !raw : raw;
    };

    // Funzione interna per gestire ciascun pulsante (con debounce)
    auto step = [&](Btn& btn, void (MenuController::*onPress)()) {
        bool now = rd(btn); // Legge il pulsante (in questo istante)
        unsigned long m = millis(); // tempo attuale
        if (now != btn.last && (m - btn.t) >= _pins.debounceMs) {
            btn.last = now; // aggiorna ultimo valore pulsante
            btn.t = m;  // aggiorna tempo pulsante

            // Evento di pressione: non era premuto -> ora lo è
            if (now && !btn.state) {
                btn.state = true;   // aggiorna stato pulsante = premuto
                (this->*onPress)();  // chiama l'azione associata al pulsante
            } else if (!now && btn.state) {  // Evento di rilascio: era premuto -> ora non lo è
                btn.state = false;
            }
        }
    };

    // Applica il controllo ai pulsanti
    step(_bBack, &MenuController::onPressBack_);
    step(_bFwd, &MenuController::onPressForward_);
    step(_bSel, &MenuController::onPressSelect_);
}


void MenuController::displayMenu () {
    if (!_lcd || !_current) return;

    _lcd->clear();
    delay(2);
    //_lcd->fillRow(_cursor);
    _lcd->drawStraightLine(0, 83, 10, true, 1);
    _lcd->printStringCentered(_current->label, 0, true);
    uint8_t firstItemRow = 2;
    uint8_t maxItemsInMenu = PAGES - firstItemRow;
    int16_t scroll = 0;
    
    for (uint8_t item = 0; item < _current->childCount; item++) {
        
        scroll = maxItemsInMenu - 1 - _cursor;
        scroll = scroll < 0 ? - scroll : 0;
        if (item >= maxItemsInMenu) return;
        
        _lcd->setCursor((item == _cursor || (scroll > 0 && item == maxItemsInMenu - 1)) 
                        ? 0 : 7, item + firstItemRow);
        if (item == _cursor || (scroll > 0 && item == maxItemsInMenu - 1)) _lcd->print("> ");
        _lcd->print(_current->children[item + scroll].label);
    }
}


void MenuController::enterAction (Action& a) {
    _act = a;
    _mode = Mode::ACTION;
    if (_act.onRender) _act.onRender();
}

void MenuController::exitAction () {
    _mode = Mode::MENU;
    displayMenu();
}