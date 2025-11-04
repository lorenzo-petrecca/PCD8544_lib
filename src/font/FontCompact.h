#if defined(ARDUINO_ARCH_AVR)
  #include <avr/pgmspace.h>
  #define FONT_PROGMEM PROGMEM
  #define FONT_READ_U8(p)  pgm_read_byte(p)
  #define FONT_READ_U16(p) pgm_read_word(p)
#else
  #define FONT_PROGMEM
  #define FONT_READ_U8(p)  (*(const uint8_t*)(p))
  #define FONT_READ_U16(p) (*(const uint16_t*)(p))
#endif