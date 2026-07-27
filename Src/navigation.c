#include <stdint.h>
#include "stm32f103xb.h"
#include "navigation.h"
#include "direction.h"
#include "speed.h"
#include "ultrasonic.h"

#define ARRAY_SIZE(a)  (sizeof(a) / sizeof((a)[0]))
#define CPU_HZ         8000000UL

#define OBSTACLE_CM    20      // stop when closer than this
#define CLEAR_CM       40      // reverse until at least this far away
#define DRIVE_CCR      800
#define STOP_MS        250     // settle time after braking
#define BACKUP_MAX_MS  2000    // safety limit while reversing
#define TURN_MS        900

struct state_machine_data
{
    state_e  state;
    uint32_t deadline_ms;
};

struct state_transition
{
    state_e       from;
    state_event_e event;
    state_e       to;
};

static const struct state_transition state_transitions[] =
{
    { STATE_FORWARD,  STATE_EVENT_OBSTACLE, STATE_STOP     },
    { STATE_STOP,     STATE_EVENT_TIMEOUT,  STATE_BACKWORD },
    { STATE_BACKWORD, STATE_EVENT_CLEAR,    STATE_RIGHT    },  // RIGHT becomes LEFT on alternate runs
    { STATE_BACKWORD, STATE_EVENT_TIMEOUT,  STATE_RIGHT    },
    { STATE_RIGHT,    STATE_EVENT_TIMEOUT,  STATE_FORWARD  },
    { STATE_LEFT,     STATE_EVENT_TIMEOUT,  STATE_FORWARD  },
};

static volatile uint32_t ms_ticks = 0;
static uint8_t turn_left = 0;

void SysTick_Handler(void)
{
    ms_ticks++;
}

static void time_init(void)
{
    SysTick_Config(CPU_HZ / 1000UL);
}

static void state_enter(struct state_machine_data *data, state_e to)
{
    data->state = to;

    switch (to)
    {
    case STATE_FORWARD:
        distance_reset();              // drop the old close reading
        straight();
        forward();
        setspeed(DRIVE_CCR);
        break;

    case STATE_STOP:
        brake();                       // both inputs low, enable high = fast stop
        setspeed(DRIVE_CCR);
        data->deadline_ms = ms_ticks + STOP_MS;
        break;

    case STATE_BACKWORD:               // keep last_distance: it is still the close reading
        straight();
        backward();
        setspeed(DRIVE_CCR);
        data->deadline_ms = ms_ticks + BACKUP_MAX_MS;
        break;

    case STATE_RIGHT:
        right();
        forward();
        setspeed(DRIVE_CCR);
        data->deadline_ms = ms_ticks + TURN_MS;
        break;

    case STATE_LEFT:
        left();
        forward();
        setspeed(DRIVE_CCR);
        data->deadline_ms = ms_ticks + TURN_MS;
        break;
    }
}

static state_event_e process_input(const struct state_machine_data *data)
{
    int32_t late = (int32_t)(ms_ticks - data->deadline_ms);   // wrap-safe

    switch (data->state)
    {
    case STATE_FORWARD:
        if (get_distance() < OBSTACLE_CM)
            return STATE_EVENT_OBSTACLE;
        break;

    case STATE_BACKWORD:
        if (get_distance() >= CLEAR_CM)
            return STATE_EVENT_CLEAR;
        if (late >= 0)
            return STATE_EVENT_TIMEOUT;
        break;

    case STATE_STOP:
    case STATE_RIGHT:
    case STATE_LEFT:
        if (late >= 0)
            return STATE_EVENT_TIMEOUT;
        break;
    }
    return STATE_EVENT_NONE;
}

static void process_event(struct state_machine_data *data, state_event_e event)
{
    if (event == STATE_EVENT_NONE)
        return;

    for (uint32_t i = 0; i < ARRAY_SIZE(state_transitions); i++)
    {
        if (data->state == state_transitions[i].from &&
            event       == state_transitions[i].event)
        {
            state_e to = state_transitions[i].to;

            if (to == STATE_RIGHT)              // alternate turn direction
            {
                if (turn_left) to = STATE_LEFT;
                turn_left ^= 1;
            }
            state_enter(data, to);
            return;
        }
    }
}

void state_run(void)
{
    struct state_machine_data data = { .state = STATE_FORWARD, .deadline_ms = 0 };

    time_init();
    state_enter(&data, STATE_FORWARD);

    while (1)
        process_event(&data, process_input(&data));
}
