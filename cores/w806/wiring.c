/*
 * wiring.c - tiempos y funciones varias para W80x (FreeRTOS a 1000 Hz).
 *
 * millis(): contador de ticks de FreeRTOS (1 ms).
 * micros(): ticks * 1000 + lo que lleva contado el temporizador del nucleo
 *           (CORET), con lo que la resolucion es de 1 us.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"

#include "csi_core.h"
#include "wm_irq.h"
#include "wm_heap.h"
#include "wm_utils.h"
#include "wm_drv_rng.h"
#include "wm_drv_rcc.h"

void w80x_interrupts(void)
{
    __enable_irq();
}

void w80x_no_interrupts(void)
{
    __disable_irq();
}

static inline bool in_isr(void)
{
    /* PSR.VEC (bits 16..23) distinto de 0 dentro de una interrupcion */
    return ((__get_PSR() >> 16) & 0xFF) != 0;
}

uint32_t millis(void)
{
    return (uint32_t)xTaskGetTickCountFromISR();
}

uint32_t micros(void)
{
    uint32_t flags = csi_irq_save();
    uint32_t ticks = (uint32_t)xTaskGetTickCountFromISR();
    uint32_t load  = CORET->LOAD + 1;
    uint32_t val   = CORET->VAL;
    /* el contador ya dio la vuelta pero la interrupcion del tick no ha corrido */
    if (csi_vic_get_pending_irq(WM_IRQ_SYS_TICK) && val > load / 2) {
        ticks++;
    }
    csi_irq_restore(flags);
    uint32_t elapsed = load - 1 - val; /* cuenta hacia abajo */
    return ticks * 1000UL + (uint32_t)(((uint64_t)elapsed * 1000UL) / load);
}

void delayMicroseconds(uint32_t us)
{
    if (us == 0) {
        return;
    }
    uint32_t start = micros();
    while ((uint32_t)(micros() - start) < us) {
    }
}

void delay(uint32_t ms)
{
    if (in_isr() || xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {
        while (ms--) {
            delayMicroseconds(1000);
        }
        return;
    }
    if (ms == 0) {
        taskYIELD();
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void yield(void)
{
    if (!in_isr() && xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        taskYIELD();
    }
}

/* frecuencia real configurada en el reloj (la del menu "Frecuencia de CPU") */
uint32_t getCpuFrequencyMhz(void)
{
    wm_device_t *rcc = wm_dt_get_device_by_name("rcc");
    int mhz          = rcc ? wm_drv_rcc_get_config_clock(rcc, WM_RCC_TYPE_CPU) : 0;
    return mhz > 0 ? (uint32_t)mhz : F_CPU / 1000000UL;
}

uint32_t getFreeHeap(void)
{
    return wm_heap_get_free_size();
}

void w80x_restart(void)
{
    wm_system_reboot();
}

uint32_t w80x_random(void)
{
    static wm_device_t *rng;
    uint32_t v = 0;
    if (rng == NULL) {
        rng = wm_drv_rng_init("rng");
        if (rng == NULL) {
            rng = wm_dt_get_device_by_name("rng");
        }
    }
    if (rng != NULL) {
        v = wm_drv_rng_read(rng);
    } else {
        v = (uint32_t)rand() ^ (micros() << 16);
    }
    return v;
}
