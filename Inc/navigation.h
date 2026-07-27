#ifndef NAVIGATION_H_
#define NAVIGATION_H_

#include <stdint.h>

// NAVIGATION STATES
typedef enum
{
    STATE_FORWARD = 0,
    STATE_LEFT,
    STATE_RIGHT,
    STATE_BACKWORD,

    STATE_STOP,
} state_e;

// TRIGGERS TO THE ACTIONS
typedef enum
{
    STATE_EVENT_NONE = 0,      // added: nothing happened this pass
    STATE_EVENT_OBSTACLE,
    STATE_EVENT_COMMAND,
	 STATE_EVENT_CLEAR,
    STATE_EVENT_TIMEOUT,       // added: a timed state (back up / turn) finished
} state_event_e;

void state_run(void);          // added: main.c calls this

#endif /* NAVIGATION_H_ */
