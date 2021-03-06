/******************************************************************************/
/*                                                                            */
/*  TIME.C:  Time Functions for 1000Hz Clock Tick                             */
/*                                                                            */
/******************************************************************************/
/*  ported to arm-elf-gcc / WinARM by Martin Thomas, KL, .de                  */
/*  <eversmith@heizung-thomas.de>                                             */
/*  modifications Copyright Martin Thomas 2005                                */
/*                                                                            */
/*  Based on file that has been a part of the uVision/ARM development         */
/*  tools, Copyright KEIL ELEKTRONIK GmbH 2002-2004                           */
/******************************************************************************/

/*
  - mt: modified interrupt ISR handling
*/

#include <AT91SAM7S64.h>                    /* AT91SAMT7S64 definitions */
#include "Board.h"
#include "interrupt_utils.h"

#ifdef ERAM  /* Fast IRQ functions Run in RAM  - see board.h */
#define ATTR RAMFUNC
#else
#define ATTR
#endif

#define TCK  1000                           /* Timer Clock  */

#define PIV  ((MCK/TCK/16)-1)               /* Periodic Interval Value */


volatile unsigned long timeval;             /* Current Time Tick */

static unsigned long previousTimeval = 0;
void wait(unsigned long time)
{
    // the units of "time" are PIT periods, which are set in Time.c.
    unsigned long when = previousTimeval + time;
    previousTimeval = when;
    while (timeval < when);
}

void  NACKEDFUNC ATTR system_int (void) {        /* System Interrupt Handler */
    volatile AT91S_PITC * pPIT = AT91C_BASE_PITC;

    ISR_ENTRY();

    if (pPIT->PITC_PISR & AT91C_PITC_PITS) {  /* Check PIT Interrupt */
        *AT91C_AIC_EOICR = pPIT->PITC_PIVR;     /* Ack & End of Interrupt */
        timeval++;                              /* Increment Time Tick */
    } else {
        *AT91C_AIC_EOICR = 0;                   /* End of Interrupt */
    }

    ISR_EXIT();
}


void init_timer(void) {                    /* Setup PIT with Interrupt */
    volatile AT91S_AIC * pAIC = AT91C_BASE_AIC;
    volatile AT91S_PITC * pPIT = AT91C_BASE_PITC;

    pPIT->PITC_PIMR = AT91C_PITC_PITIEN |    /* PIT Interrupt Enable */
        AT91C_PITC_PITEN  |    /* PIT Enable */
        (PIV & 0xfffff);    /* Periodic Interval Value */

    /* Setup System Interrupt Mode and Vector with Priority 7 and Enable it */
    pAIC->AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE | 7;

    pAIC->AIC_SVR[AT91C_ID_SYS] = (unsigned long) system_int;
    pAIC->AIC_IECR = (1 << AT91C_ID_SYS);
}
