#include "anykey/anykey.h"
#include "cnctypes.h"
#include "config.h"
#include "state.h"
#include "upstream.h"
#include "downstream.h"

#define SYSTICK_INTERVAL ((72000000/HEARTBEAT_HZ)-1)

uint32_t counter;

void main(void) {
	State_Init();
	CQ_Init();
	Downstream_Init();
	Upstream_Init();

	SYSCON_StartSystick(SYSTICK_INTERVAL);

	Upstream_Start();
}


void systick() {
	Downstream_Tick();
	Upstream_Tick();
}
