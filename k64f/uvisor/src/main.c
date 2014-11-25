#include <uvisor.h>
#include "vmpu.h"

#ifdef  NV_CONFIG_OFFSET
__attribute__ ((section(".nv_config")))
const NV_Type nv_config = {
    /* backdoor key */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* flash protection */
    0xFF, 0xFF, 0xFF, 0xFF,
    /* FSEC */
    0xFF,
    /* FOPT */
    0xFF,
    /* FEPROT */
    0xFF,
    /* FDPROT */
    0xFF
};
#endif/*NV_CONFIG_OFFSET*/

#define STACK_SIZE 256
__attribute__ ((aligned (32))) volatile uint32_t g_priv_stack[STACK_SIZE];

void main_init(void)
{
    int t;
    volatile int i;

    /* reset previous channel settings */
    ITM->LAR  = 0xC5ACCE55;
    ITM->TCR  = ITM->TER = 0x0;
    /* wait for debugger to connect */
    while(!((ITM->TCR & ITM_TCR_ITMENA_Msk) && (ITM->TER & (1<<CHANNEL_DEBUG))));

    /* init MPU */
    vmpu_init();

    t = 0;
    while(t < 10)
    {
        dprintf("Hello World %i!\n", t++);
        for(i = 0; i < 200000; i++);
    }
}

void main_entry(void)
{
    /* swap stack pointers*/
    __set_PSP(__get_MSP());
    __set_MSP((uint32_t)&g_priv_stack[STACK_SIZE-1]);

    main_init();

    /* switch to unprivileged mode.
     * this is possible as uvisor code is readable by unprivileged.
     * code and only the key-value database is protected from the.
     * unprivileged mode */
    __set_CONTROL(__get_CONTROL()|3);
    __ISB();
    __DSB();
}