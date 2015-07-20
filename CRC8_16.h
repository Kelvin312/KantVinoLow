#include <mega8.h>

#ifndef _CRC8_16_INCLUDED_
#define _CRC8_16_INCLUDED_

#pragma used+

inline void CRC16_Clear();
inline void CRC16_Add(char value);
inline char CRC16_Check();
void CRC16_Get(char *data);

unsigned int CRC16_All(char *p, unsigned char n);

inline void CRC8_Clear();
inline void CRC8_Add(char value);
inline char CRC8_Check(); 

#pragma used-
#endif
