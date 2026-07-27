#include "speed.h"
#include "stm32f103xb.h"

#define PWM_ARR 999UL

void timerpwm(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    // PA0 = TIM2_CH1: AF push-pull, 10 MHz
    GPIOA->CRL &= ~(0xFUL << (0 * 4));
    GPIOA->CRL |=  (0x9UL << (0 * 4));

    // 8 MHz timer clock, PSC=0, ARR=999 -> 8 kHz PWM
    TIM2->SMCR &= ~TIM_SMCR_SMS_Msk;
    TIM2->PSC  = 0UL;
    TIM2->ARR  = PWM_ARR;
    TIM2->CCR1 = 0UL;                  // start stopped, safer than 50%
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->CR1 &= ~TIM_CR1_DIR;

    // CH1: PWM mode 1, preload, active high
    TIM2->CCMR1 &= ~(TIM_CCMR1_CC1S_Msk | TIM_CCMR1_OC1M_Msk);
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE;
    TIM2->CCER  &= ~TIM_CCER_CC1P;
    TIM2->CCER  |= TIM_CCER_CC1E;

    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void setspeed(uint32_t ccr)
{
    if (ccr > PWM_ARR) ccr = PWM_ARR;
    TIM2->CCR1 = ccr;
}

void stop(void)
{
    TIM2->CCR1 = 0;
}
