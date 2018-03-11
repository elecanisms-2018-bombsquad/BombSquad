#ifndef _STRM_H_
#define _STRM_H_

#include <stdint.h>

void stringcpy(char* str1, char* str2);
void CamelCase(char* str, uint8_t a, uint8_t b);
void Caesar(char* str, uint8_t freq, uint8_t shift);
void Brnrd(char* str, uint8_t a, uint8_t b);
void Leet(char* str,uint8_t a, uint8_t b);
void Pig(char* str,uint8_t a, uint8_t b);
void Missing(char* str, uint8_t freq, uint8_t a);
void Reverse(char* str,uint8_t a, uint8_t b);
void Scramble(char* str,uint8_t a, uint8_t b);
void Nada(char* str,uint8_t a, uint8_t b);

#endif