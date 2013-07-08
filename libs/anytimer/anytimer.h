
#ifndef ANYTIMER_H
#define ANYTIMER_T

#include "anykey/anykey.h"

typedef void(*anytimer_func)(void);

void anytimer_every(TimerId timer, int32_t ms, anytimer_func f);
void anytimer_in(TimerId timer, int32_t ms, anytimer_func f);

#endif



