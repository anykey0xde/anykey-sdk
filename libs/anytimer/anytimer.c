
#include "anytimer.h"

anytimer_func ct16b0func = NULL;
anytimer_func ct16b1func = NULL;
anytimer_func ct32b0func = NULL;
anytimer_func ct32b1func = NULL;


void ct16b0_handler() {
	Timer_ClearInterruptMask(CT16B0, TIMER_MR0INT);
  if (ct16b0func != NULL) {
    ct16b0func();
  }
}
void ct16b1_handler() {
	Timer_ClearInterruptMask(CT16B1, TIMER_MR0INT);
  if (ct16b1func != NULL) {
    ct16b1func();
  }
}
void ct32b0_handler() {
	Timer_ClearInterruptMask(CT32B0, TIMER_MR0INT);
  if (ct32b0func != NULL) {
    ct32b0func();
  }
}
void ct32b1_handler() {
	Timer_ClearInterruptMask(CT16B1, TIMER_MR0INT);
  if (ct32b1func != NULL) {
    ct32b1func();
  }
}

static uint32_t _abs_diff(uint32_t a, uint32_t b) {
  if (a>b)
    return a-b;
  return b-a;
}
// timerfreq = clockIn / (prescale * comp)
// 1 / x ms = (72 000 / ms) / (prescale * comp)
// 1 = xms * (72 000 / ms) / (prescale * comp)
// prescale * comp = x * 72 000

static void _anytimer(TimerId timer, int32_t ms, anytimer_func f, TIMER_MATCH_BEHAVIOUR b) {
 // 72 000 000 / s
  // 72 000 / ms
  // 65535 = 2^16
  // 2147483647 = 2^32
  uint32_t prescale;
  uint32_t match;
  uint32_t actual;
  switch (timer) {
    case CT16B0:
			NVIC_EnableInterrupt(NVIC_CT16B0);
      ct16b0func = f;
      break;
    case CT16B1:
			NVIC_EnableInterrupt(NVIC_CT16B1);
      ct16b1func = f;
      break;
    case CT32B0:
			NVIC_EnableInterrupt(NVIC_CT32B0);
      ct32b0func = f;
    case CT32B1:
			NVIC_EnableInterrupt(NVIC_CT32B1);
      ct32b1func = f;
    default:
      return;
  };
  
  // 16 and 32 bit timers aren't differentiated at the moment...

  // scale in is 72000 1/ms * ms
  // and must be smaller than prescale * match
  // prescale * match / clk = 
  //  (2^16)+1 * 2^16 / 72000 = 59653
  if (ms > 59653) {
    return;
  }
  for (prescale = (ms * 72000) >> 16; prescale != 0x10001; ++prescale) {
    match  = (ms * 72000) / prescale;
    actual = (match * prescale) / 72000;
    if ( ((_abs_diff(ms, actual) *1000) / actual) <= 5 ) {
      // 0.5% precision ...
      break;
    } 
  }

  Timer_Enable(timer, true);
  Timer_SetPrescale(timer, prescale);
  Timer_SetMatchValue(timer, 0, match);
  Timer_SetMatchBehaviour(timer, 0, b);
  Timer_Start(timer); 

}

void anytimer_in(TimerId timer, int32_t ms, anytimer_func f) {
	_anytimer(timer, ms, f, TIMER_MATCH_INTERRUPT | TIMER_MATCH_STOP);
} 

void anytimer_every(TimerId timer, int32_t ms, anytimer_func f) {
	_anytimer(timer, ms, f, TIMER_MATCH_INTERRUPT | TIMER_MATCH_RESET);
} 




