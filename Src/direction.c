#include "stm32f103xb.h"
#include "direction.h"

// Motor 1 (drive):    PB6, PB7
// Motor 2 (steering): PA5, PA7

void gpioinit(void)
{
    RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN);

    // PB6, PB7: general purpose output push-pull, 10 MHz
    GPIOB->CRL &= ~(0xFFUL << 24);
    GPIOB->CRL |=  (0x11UL << 24);

    // PA5: output push-pull, 10 MHz
    GPIOA->CRL &= ~(0xFUL << 20);
    GPIOA->CRL |=  (0x1UL << 20);

    // PA7: output push-pull, 10 MHz
    GPIOA->CRL &= ~(0xFUL << 28);
    GPIOA->CRL |=  (0x1UL << 28);

    // Start with everything off
    GPIOB->BSRR = GPIO_BSRR_BR6 | GPIO_BSRR_BR7;
    GPIOA->BSRR = GPIO_BSRR_BR5 | GPIO_BSRR_BR7;
}

void forward(void)
{
    GPIOB->BSRR = GPIO_BSRR_BS6 | GPIO_BSRR_BR7;
}

void backward(void)
{
    GPIOB->BSRR = GPIO_BSRR_BR6 | GPIO_BSRR_BS7;
}

void left(void)
{
    GPIOA->BSRR = GPIO_BSRR_BS5 | GPIO_BSRR_BR7;
}

void right(void)
{
    GPIOA->BSRR = GPIO_BSRR_BR5 | GPIO_BSRR_BS7;
}

void straight(void)
{
    // PA5 and PA7 both low: steering motor off
    GPIOA->BSRR = GPIO_BSRR_BR5 | GPIO_BSRR_BR7;
}

void brake(void)
{
    // Both inputs equal while enable is high = fast stop on the L293D
    GPIOB->BSRR = GPIO_BSRR_BR6 | GPIO_BSRR_BR7;
}
