#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"

int getsw(void)
{
    return (PORTD >> 8) & 0x000F;
} 

int getbtns(void)
{
    return (PORTD >> 5) & 0x0007;
}

int getbtn1(void)
{
    return (PORTF >> 1) & 0x1;
}