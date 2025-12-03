#pragma once
#include <stdint.h>
#include <Arduino.h>

class PCD8544;

#define MAX_DEPTH 8

/*  
 *  ### INPUT CONFIG
 *  uint8_t backPin
 *  uint8_t forwardPin
 *  uint8_t selectPin
 *  bool activeLow: true se i pulsanti chiudono a GND, false se chiudono a VCC
 *  bool useInternalPull: true = attiva PULLUP/PULLDOWN | false = resistenza esterna
 *  uint16_t deboounceMs: debounce per pulsanti (es. 30ms)
 *  
 */
struct MenuInputPins {
    uint8_t backPin;
    uint8_t forwardPin;
    uint8_t selectPin;
    bool activeLow; // true se i pulsanti chiudono a GND
    bool useInternalPull; // true = attiva PULLUP/PULLDOWN | false = resistenza esterna
    uint16_t debounceMs; // debounce per pulsanti (es. 30ms)
};


/*  
 *  ### MENU ITEM
 *  const char* label: titolo
 *  void (*onSelect) (): callback opzionale
 *  const MenuItem* children: array di figli
 *  uint8_t childCount = 0: numero figli 
 */
struct MenuItem {
    const char* label;
    void (*onSelect) (); // callback opzionale
    const MenuItem* children; // array di figli
    uint8_t childCount = 0; // numero figli 

    MenuItem (const char* l, 
            void (*fn)() = nullptr, 
            const MenuItem* kids = nullptr, 
            uint8_t n = 0)
        :   label(l), onSelect(fn), children(kids), childCount(n) {}

    // Costruttore di default
    MenuItem()
        : label(nullptr), onSelect(nullptr), children(nullptr), childCount(0) {}
};


class MenuController {
public:
    MenuController (const MenuInputPins& pins)
    : _pins(pins), _bBack(pins.backPin), _bFwd(pins.forwardPin), _bSel(pins.selectPin) {
        // BACK
        pinMode(pins.backPin,
        pins.useInternalPull ?
        #if defined(ARDUINO_ARCH_ESP32)
          (pins.activeLow ? INPUT_PULLUP : INPUT_PULLDOWN)
        #else
          (pins.activeLow ? INPUT_PULLUP : INPUT)
        #endif
          : INPUT);

        // FORWARD
        pinMode(pins.forwardPin,
        pins.useInternalPull ?
        #if defined(ARDUINO_ARCH_ESP32)
          (pins.activeLow ? INPUT_PULLUP : INPUT_PULLDOWN)
        #else
          (pins.activeLow ? INPUT_PULLUP : INPUT)
        #endif
          : INPUT);
        
        // SELECT
        pinMode(pins.selectPin,
        pins.useInternalPull ?
        #if defined(ARDUINO_ARCH_ESP32)
          (pins.activeLow ? INPUT_PULLUP : INPUT_PULLDOWN)
        #else
          (pins.activeLow ? INPUT_PULLUP : INPUT)
        #endif
          : INPUT);
    }

    // MODALITA' AZIONE
    struct Action {
        void (*onLeft)() = nullptr;
        void (*onRight)() = nullptr;
        void (*onSelect)() = nullptr;
        void (*onRender)() = nullptr;
    };  

    inline void createMenu (const MenuItem* root) {
        _depth = 0;
        _path[_depth] = root;
        _current = root;
        _cursor = 0;
    }

    inline bool select () {
        if (!_current || !_current->children || !_current->childCount) return false;
        const MenuItem* selected = &_current->children[_cursor];

        // 1. Esegue sempre la callback se c'è
        if (selected->onSelect) {
            selected->onSelect();

            // La callback potrebbe aver cambiato il menu corrente…
            if (!_current || !_current->children || !_current->childCount) return false;
            selected = &_current->children[_cursor];
        }

        // 2. Se ci sono figli, entra nel sotto-menu
        if (selected->children && selected->childCount > 0) {
            _path[++_depth] = selected;
            _current = selected;
            _cursor = 0;
            return true;
        }

        // 3. Nessun submenu -> solo callback
        return false;
    }

    inline void back () {
        if (!_current || !_current->children || !_current->childCount) return;
        if (_cursor == 0) {
            _cursor = _current->childCount - 1; 
        } else {
            _cursor--;
        }
    }

    inline void forward () {
        if (!_current || !_current->children || !_current->childCount) return;
        _cursor++;
        if (_cursor >= _current->childCount) _cursor = 0; // ricomincia da capo
    }

    inline void exit () {
        if (_depth > 0) {
            _current = _path[--_depth];
            _cursor = 0;
        }
    }

    inline void update () { scanButtons_(); }
    inline void attachDisplay (PCD8544* lcd) { _lcd = lcd; }
    void displayMenu ();

    void enterAction (Action& a);
    void exitAction ();
    inline bool isInAction () const { return _mode == Mode::ACTION; }

    // Ritorna la posizione del attuale cursore nel menu corrente
    inline uint8_t getCursor() const { return _cursor; }
    // Ritorna il sub-menu all'interno del quale ci si trova
    inline const MenuItem* getCurrent() const { return _current; }


private:
    const MenuInputPins _pins;
    const MenuItem* _current = nullptr; // elemento attuale
    const MenuItem* _path[MAX_DEPTH];   // percorso del menu corrente (root -> submenu)
    uint8_t _depth = 0;  // profondità del menu corrente (_current)
    uint8_t _cursor = 0; // Indice nel livello corrente
    PCD8544* _lcd = nullptr; // Puntatore all'istanza del display
    enum class Mode {MENU, ACTION};
    Mode _mode = Mode::MENU;
    Action _act;

    struct Btn {
        bool state;
        bool last;
        unsigned long t;
        uint8_t pin;

        Btn (uint8_t p) : pin(p) {
            state = false;
            last = false;
            t = 0;
        };
    };

    Btn _bBack, _bFwd, _bSel;

    void scanButtons_ ();
    inline void onPressBack_() { 
        if (_mode == Mode::MENU) { back(); displayMenu(); }
        else leftAction();
    }
    inline void onPressForward_() { 
        if (_mode == Mode::MENU) { forward(); displayMenu(); }
        else rightAction();
    }
    inline void onPressSelect_() { 
        if (_mode == Mode::MENU) {
            if(select()) displayMenu();
        } else selectAction();
    }

    inline void leftAction () {
        if (_act.onLeft) _act.onLeft();
        if (_act.onRender) _act.onRender();
    }
    inline void rightAction () {
        if (_act.onRight) _act.onRight();
        if (_act.onRender) _act.onRender();
    }
    inline void selectAction () {
        if (_act.onSelect) _act.onSelect();
    }
};