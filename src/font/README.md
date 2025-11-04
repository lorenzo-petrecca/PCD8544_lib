# Documentazione Font

Ogni font prevede un file di metadati *(meta.h)*, dove sono inserite le informazioni necessarie per leggere il font, e un file di dati *(data)*, in cui è presente l'array di byte necessario per rappresentare i caratteri.
<br>

### meta.h
I metadati sono rappresentati da 8 byte con questa struttura.

|Offset (byte)|Lunghezza (byte)|Nome|Valori tipici|
---|---|---|---|
|0|1|FLAGS||
|1|2|FIRST_CODEPOINT| es. 32|
|3|2|LAST_CODEPOINT| es. 126|
|5|1|GLYPH_HEIGHT|0x01 - 0x30|
|6|1|GLYPH_WIDTH|0x01 - 0x54; 0x00 se variabile|
|7|1|GLYPH_SPACING|0x00 - 0x53|

<br>

##### FLAGS
Il campo **FLAGS** è un campo di lunghezza di un byte formato nel modo seguente:
|Bit 7|Bit 6|Bit 5|Bit 4|Bit 3|Bit 2|Bit 1| Bit 0|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|V3|V2|V1|V0|0|0|0|M|

* **Bit 7 - 4** - Numero di versione (0x01 - 0x0F)
* **Bit 3 - 1** - Riservati (0)
* **Bit 0** - Mono *(specifica il tipo di font)*
    ```
        0 - monospace
        1 - variabile
    ```
<br>

##### GLYPH_WIDTH
Il campo **GLYPH_WIDTH** è un campo di lunghezza di 1 byte che rappresenta la larghezza di ogni carattere *(glyph)* del font.
* **1 - 84** → dimensione fissa (monospace)
* **0** → variabile
<br>

### data.h
Il file dei dati è costituito da un unico array lineare in cui sono inseriti tutti i byte necessari per rappresentare ogni carattere, in modo continuativo, senza interruzioni nè array bidimensionali o multidimensionali. Il compito di estrarre i dati esatti per la rappresentazione di uno specifico carattere è lasciato al render presente nella libreria, che utilizzerà le informazioni fondamentali estratte dal file *meta.h*.

> **⚠️ Nota bene**
> Ogni byte rappresente una singola colonna, pertanto se il ***GLYPH_WIDTH*** del font è pari a ***n*** ci sarà bisogno di ***n byte*** per rappresentare un singolo **glyph**.
Inoltre bisogna evidenziare che ogni byte (colonna) è rappresentato in **LSB**, così come raffigurato nell'esempio in figura.

<br>


###### Fig. Rappresentazione del carattere '!' *(font 5x8 px)*
|n° bit|BYTE 0|BYTE 1|BYTE 2|BYTE 3|BYTE 4|
|:-:|:-:|:-:|:-:|:-:|:-:|
|7|0|0|1|0|0|
|6|0|0|1|0|0|
|5|0|0|1|0|0|
|4|0|0|1|0|0|
|3|0|0|1|0|0|
|2|0|0|0|0|0|
|1|0|0|1|0|0|
|0|0|0|0|0|0|

````
· BYTE 0 = 0x00 (0b00000000) 
· BYTE 1 = 0x00 (0b00000000) 
· BYTE 2 = 0x5F (0b01011111) 
· BYTE 3 = 0x00 (0b00000000) 
· BYTE 4 = 0x00 (0b00000000) 
````
