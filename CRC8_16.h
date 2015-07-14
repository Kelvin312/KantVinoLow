#include <mega8.h>

#ifndef _CRC8_16_INCLUDED_
#define _CRC8_16_INCLUDED_

#pragma used+

inline void CRC16_clear();
inline void CRC16_add(char value);
inline char CRC16_check();

unsigned int CRC16_all(char *p, unsigned char n);

void CRC8_clear();
void CRC8_add(char value);
char CRC8_check(); 

#pragma used-
#endif
