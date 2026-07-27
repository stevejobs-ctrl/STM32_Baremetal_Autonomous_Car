#include "stm32f103xb.h"
#include "ultrasonic.h"
#include <stdint.h>

#define TIM_ARR 60000UL

#define DIST_FAR_CM  400

static uint32_t last_distance = DIST_FAR_CM;
volatile uint32_t rising = 0;
volatile uint32_t falling = 0;
volatile uint8_t  capture_done = 0;

void pwm2(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    // PA6 = TIM3_CH1 (echo input), floating input (CNF=01, MODE=00)
    GPIOA->CRL &= ~(0xFUL << (6 * 4));
    GPIOA->CRL |=  (0x4UL << (6 * 4));

    // PB0 = TIM3_CH3 (trigger output), AF push-pull 2 MHz
    GPIOB->CRL &= ~(0xFUL << (0));
    GPIOB->CRL |=  (0xAUL << (0));

    // Time base: 1 MHz tick, 60 ms period
    TIM3->SMCR &= ~TIM_SMCR_SMS_Msk;
    TIM3->PSC  = 7UL;
    TIM3->ARR  = TIM_ARR;
    TIM3->CCR3 = 10U;                  // 10 us trigger pulse
    TIM3->CR1 |= TIM_CR1_ARPE;
    TIM3->CR1 &= ~TIM_CR1_DIR;

    // CH3: PWM mode 1, preload, active high
    TIM3->CCMR2 &= ~(TIM_CCMR2_CC3S_Msk | TIM_CCMR2_OC3M_Msk);
    TIM3->CCMR2 |= (TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE);
    TIM3->CCER  &= ~TIM_CCER_CC3P;
    TIM3->CCER  |= TIM_CCER_CC3E;

    // CH1: capture on TI1, rising edge
    TIM3->CCMR1 &= ~(TIM_CCMR1_CC1S_Msk | TIM_CCMR1_IC1F_Msk | TIM_CCMR1_IC1PSC_Msk);
    TIM3->CCMR1 |= (1UL << TIM_CCMR1_CC1S_Pos) | (0xFUL << TIM_CCMR1_IC1F_Pos);
    TIM3->CCER  &= ~TIM_CCER_CC1P;

    // CH2: capture on TI1 as well, falling edge
    TIM3->CCMR1 &= ~(TIM_CCMR1_CC2S_Msk | TIM_CCMR1_IC2F_Msk | TIM_CCMR1_IC2PSC_Msk);
    TIM3->CCMR1 |= (2UL << TIM_CCMR1_CC2S_Pos) | (0xFUL << TIM_CCMR1_IC2F_Pos);
    TIM3->CCER  |= TIM_CCER_CC2P;

    TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
    TIM3->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE;

    TIM3->EGR |= TIM_EGR_UG;
    TIM3->SR = 0;
    TIM3->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM3_IRQn);
}

void 	TIM3_IRQHandler (void)
{
    if (TIM3->SR & TIM_SR_CC1IF) {
        rising = TIM3->CCR1;           // reading CCR clears the flag too
        TIM3->SR = ~TIM_SR_CC1IF;
    }
    if (TIM3->SR & TIM_SR_CC2IF) {
        falling = TIM3->CCR2;
        TIM3->SR = ~TIM_SR_CC2IF;
        capture_done = 1;
    }
}

static volatile uint8_t skip_next = 0;
//Returns the latest valid
uint32_t get_distance(void)
{
    if (capture_done)
    {
        capture_done = 0;

        if (skip_next)
        {
            skip_next = 0;
            return last_distance;
        }

        uint32_t r = rising, f = falling;
        uint32_t width = (f >= r) ? (f - r) : (f + (TIM_ARR + 1) - r);
        uint32_t cm = width / 58;

        if (cm >= DIST_FAR_CM)      last_distance = DIST_FAR_CM;
        else if (cm > 0)            last_distance = cm;
    }
    return last_distance;
}

void distance_reset(void)
{
    last_distance = DIST_FAR_CM;
    capture_done = 0;
    skip_next = 1;
}
