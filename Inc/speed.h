
#include <stdint.h>
#ifndef SPEED_H_
#define SPEED_H_

void timerpwm (void);
void setspeed(uint32_t ccr);
void stop(void);

#endif
