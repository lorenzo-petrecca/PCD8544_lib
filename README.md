# PCD8544_lib

<br>

| English |
|---|

<br>

C++ library for the **Nokia 5110 display**, based on the **PCD8544** controller and utilizing **SPI** interface.  
Compatible with **Arduino IDE** and **PlatformIO**. Supports **Arduino AVR** and **ESP32**.
<br>

### 🎯 Library Purpose
This library is designed to use **minimal RAM**, making it ideal for microcontrollers with limited memory or those performing other tasks in parallel.

Most PCD8544 libraries rely on a **full RAM framebuffer** to represent the display content.  
This library, however, has been designed to work **without allocating a framebuffer in RAM**. Instead, data is sent to the display **in streaming mode, one column at a time**, avoiding unnecessary buffering or memory rewrites.

This approach allows:
- Display buffer RAM usage reduced to 0 bytes
- Minimized SPI data transfers
- More efficient drawing on MCUs with limited resources
<br>

##### ⚠️ Design Limitations
> The absence of a framebuffer introduces a trade-off:  
> - It’s not possible to read back the current display content
> - Overwrite operations do **not support blending (`OR`, `AND`, `XOR`) with existing content**
> - Drawing overlapping elements must be manually handled by the user

In summary, **the library favors RAM and speed optimization at the cost of advanced graphic overlap features.**


### 🧩 MCU / Board Compatibility

> *Current status based on real-world tests + technical assumptions. If you've tested a new board, feel free to open a PR to update the table.*

**Legend**  
🟢 Tested · 🟡 Expected to work (not tested) · ❓ To be verified

| MCU | Board | Status |
|-----|-------|:------:|
| AVR (ATmega328P, 16 MHz) | Arduino Nano | 🟢 |
| AVR (ATmega328P, 16 MHz) | Arduino Uno | 🟢 |
| AVR (ATmega328P, 16 MHz) | Arduino Uno R3 | 🟡 |
| AVR (ATmega2560, 16 MHz) | Arduino Mega 2560 | 🟡 |
| AVR (ATmega32U4, 16 MHz) | Leonardo / Micro | 🟡 |
| megaAVR (ATmega4809) | Arduino Nano Every | ❓ |
| ESP32 | Arduino Nano ESP32 | 🟢 |
| ESP32 (Xtensa dual-core) | ESP32 DevKit | 🟢 |
| ESP8266 | NodeMCU / Wemos D1 | 🟡 |
| RP2040 | Raspberry Pi Pico | 🟡 |
| STM32 (Arduino core) | “Blue Pill” / Nucleo | ❓ |

<br>

**Electrical / Wiring Notes**
- The **PCD8544 runs at 3.3V**. Most Nokia 5110 modules are 3.3V, some can accept 5V. If using **5V boards** (UNO/Nano/Mega), use **level shifting** (logic or VCC) or voltage dividers.
- The PCD8544 backlight is typically driven by an LED that requires a current-limiting resistor (~470 Ω) if driven directly from an MCU pin.
For enhanced protection, driving it through an NPN transistor (with base resistor) is recommended.

***How to report compatibility?***  
Have you tried this library on a board not listed here?  
Report it via [issue](../../issues) or send a [Pull Request](../../pulls) with your test details.
<br>

### 📋 Requirements

- **Arduino IDE** (recommended version: 1.8.x or later)
- Or **PlatformIO** (for streamlined integration)
- Dependencies:
  - `Arduino.h` (included by default in all Arduino cores)
  - `SPI.h` (bundled with the core; needed for communicating with the display)
- **Nokia 5110 (PCD8544)** display connected via SPI

> 💡 **Important note:** This library DOES NOT support software SPI. Use your MCU’s hardware SPI peripheral.


<br>

### 🔧 Installation

##### Arduino IDE (manual, via ZIP)
1. Download the latest release from [GitHub Releases](../../releases)
2. Open Arduino IDE
3. Go to **Sketch → Include Library → Add .ZIP Library**
4. Select the `.zip` file you just downloaded

The library will show up under **File → Examples → PCD8544_lib**

<br>

##### PlatformIO

- Add the following line to your `platformio.ini` under `lib_deps`:

    ```ini
    lib_deps =
        https://github.com/lorenzo-petrecca/PCD8544_lib.git
    ```
<br>

- You can also pin a specific version/tag:

    ```ini
    lib_deps =
        https://github.com/lorenzo-petrecca/PCD8544_lib.git#v0.2.0
    ```
<br>

- To clone the library directly (not recommended):

    ```bash
    git clone https://github.com/lorenzo-petrecca/PCD8544_lib.git lib/PCD8544_lib
    ```

<br>

### 📝 License
Released under the BSD 3-Clause License (see [LICENSE](./LICENSE)).  
Attribution and copyright notices must be preserved in redistributed copies.

<br>

---

<br><br>

| Italiano |
|---|

<br>

Libreria C++ per il display **Nokia 5110**, basato sul controller **PCD8544**, che utilizza interfaccia **SPI**.  
Compatibile con **Arduino IDE** e **PlatformIO**. Supporta **Arduino AVR** ed **ESP32**.
<br>

### 🎯 Obiettivo della libreria
Questa libreria è progettata per occupare **il minimo spazio in RAM**, rendendola ideale per microcontrollori con memoria limitata o che devono eseguire altre operazioni in parallelo.
La maggior parte delle librerie per display PCD8544 si basa su un **framebuffer RAM completo** per rappresentare lo stato del display.  
Questa libreria, invece, è stata progettata per funzionare **senza un buffer preallocato in memoria**; i dati vengono inviati al display **in streaming, una colonna alla volta**, evitando buffering o riscritture non necessarie. 
Questo approccio consente di:
- Ridurre l’utilizzo della RAM a 0 byte per il buffer display
- Minimizzare i trasferimenti di dati SPI
- Eseguire operazioni grafiche in modo più efficiente su MCU con risorse limitate
<br>

##### ⚠️ Limitazioni progettuali
> L’assenza del framebuffer introduce un compromesso:  
> - Non è possibile leggere il contenuto corrente del display dal driver
> - Operazioni di sovrascrittura **non supportano la fusione (`OR`, `AND`, `XOR`) del contenuto precedente**
> - Disegnare elementi sovrapposti richiede una gestione manuale a livello utente

In sintesi, **la libreria ottimizza RAM e velocità a scapito della sovrapposizione grafica avanzata.**


### 🧩 Compatibilità MCU / board

> *Stato attuale basato su test reali + valutazione tecnica. Se provi una board, apri una PR per aggiornare la tabella.*

**Legenda**
🟢 Testato  ·  🟡 Atteso ok (non testato)  ·  ❓ Da verificare

|MCU|Board|Stato|
|---|---|:-:|
|AVR (ATmega328P, 16 MHz)|Arduino Nano|🟢|
|AVR (ATmega328P, 16 MHz)|Arduino Uno|🟢|
|AVR (ATmega328P, 16 MHz)|Arduino Uno R3|🟡|
|AVR (ATmega2560, 16 MHz) |Arduino Mega 2560|🟡|
|AVR (ATmega32U4, 16 MHz)|Leonardo / Micro|🟡|
|megaAVR (ATmega4809)|Arduino Nano Every|❓|
|ESP32|Arduino nano ESP32|🟢|
|ESP32 (Xtensa dual-core)|ESP32 DevKit|🟢|
|ESP8266|NodeMCU / Wemos D1|🟡|
|RP2040|Raspberry Pi Pico|🟡|
|STM32 (Arduino core)|“Blue Pill”/Nucleo|❓|

<br>

**Note elettriche / cablaggio**
- Il **PCD8544 è 3.3 V**, spesso moduli display 5110 sono sempre 3,3V o qualche volta anche 5V; su board **5 V** (UNO/Nano/Mega) usa **livello 3.3 V** per VCC e logiche (level shifter o partitori/serie).
- La retroilluminazione del PCD8544 è collegata solitamente a un LED interno che richiede un limitatore di corrente (es. resistenza da ~470 Ω) se comandata direttamente dal pin MCU.
Per protezione superiore, si consiglia l’uso di un transistor NPN con resistenza alla base.

***Come segnalare compatibilità?***
Hai provato questa libreria con una board non in tabella?  
Segnalalo tramite [issue](../../issues) o invia una [Pull Request](../../pulls) con i dettagli del test.
<br>

### 📋 Requisiti

- **Arduino IDE** (versione consigliata: 1.8.x o superiore)
- Oppure **PlatformIO** (per integrazione semplificata)
- Dipendenze:
  - `Arduino.h` (inclusa di default in tutte le board Arduino)
  - `SPI.h` (inclusa nel core, necessaria per comunicare con il display)
- Display **Nokia 5110 (PCD8544)** collegato via SPI

> 💡 **Nota importante:** questa libreria NON supporta SPI software. Usa la periferica SPI hardware della tua MCU.


<br>

### 🔧 Installazione

##### Arduino IDE (manuale via ZIP)
1. Scarica l'ultima release della libreria da [GitHub Releases](../../releases)
2. Apri Arduino IDE
3. Vai su **Sketch → Include Library → Add .ZIP Library**
4. Seleziona il file `.zip` appena scaricato

La libreria sarà disponibile sotto **File → Examples → PCD8544_lib**

<br>

##### PlatformIO

- Aggiungi questa riga nel tuo `platformio.ini` sotto `lib_deps`:

    ```ini
    lib_deps =
        https://github.com/lorenzo-petrecca/PCD8544_lib.git
    ```
<br>

- Puoi anche specificare una versione/tag stabile:

    ```ini
    lib_deps =
        https://github.com/lorenzo-petrecca/PCD8544_lib.git#v0.2.0
    ```
<br>

- Per clonare, invece, la libreria direttamente nella tua struttura PlatformIO (sconsigliato):

    ```bash
    git clone https://github.com/lorenzo-petrecca/PCD8544_lib.git lib/PCD8544_lib

    ```
<br>

### 📝 Licenza
Rilasciata con licenza BSD 3-Clause (vedi [LICENSE](./LICENSE)).
È richiesto di mantenere il copyright e la citazione dell’autore nelle redistribuzioni.