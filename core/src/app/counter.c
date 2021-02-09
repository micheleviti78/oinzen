//
//  counter.h
//  
//
//  Created by Moreno Marcellini on 08/02/21.
//

/*  Copyright (c) 2016 Tilen Majerle
 *
 *  Only rewritten to use the HAL framework and
 *  low level access
 *  DWT: Data Watch Point
 *  TRC: Trace ...
 *
 */
#include <math.h> /* for floor */
#include <stdlib.h> /* for calloc */
#include <stdio.h> /* for sprintf */

#include "main.h"
#include "diag.h"
#include "cmsis_os.h"
#include "counter.h"

uint32_t uDWTCounterEnable(void) {
    
    volatile uint32_t _c;
#if 1
    /* Enable TRC
     DEMCR:
     The 32-bit Debug Exception and Monitor Control Register:
     This provides Vector Catching and Debug Monitor Control. This register
     contains a bit named TRCENA which enable the use of a TRACE.
     */
    CoreDebug->DEMCR &= ~0x01000000;
    CoreDebug->DEMCR |=  0x01000000;
#endif
    
#if defined(STM32F7xx) // STM32F7xx is not defined
    /* Unclock DWT timer:
     Write 0xC5ACCE55 to unlock Write Access to the other ITM registers
     */
    DWT->LAR = 0xC5ACCE55;
#endif
    /* We know our MPU */
    DWT->LAR = 0xC5ACCE55;
    
    /* Enable counter */
    DWT->CTRL &= ~0x00000001;
    DWT->CTRL |=  0x00000001;
    
    /* Reset counter */
    DWT->CYCCNT = 0;
    
    /* Check if DWT has started */
    _c = DWT->CYCCNT;
    
    /* 2 dummys */
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");
    
    /* Return difference, if result is zero, DWT has not started */
    return (DWT->CYCCNT);
}

/*
 * Counting functions
 */

void begin_count(struct counter *usage){
    
    usage->begin = DWT->CYCCNT;
    
    return;
}

void end_count(struct counter *usage){
    
    usage->end = DWT->CYCCNT;
    usage->total_usage += usage->end - usage->begin;
    
    return;
}

void reset_count(struct counter *usage){
    
    DWT->CYCCNT = 0;
    usage->end = 0;
    usage->begin = 0;
    usage->total_usage = 0;
    
    return;
}

void init_container(void){
    
    enum task_counter _counters;
    
    _counters = LAST;

    *container = (struct counter*) calloc(sizeof(struct counter*), _counters);

    if (*container == NULL){
        RAW_DIAG("[ ERROR ] init_container");
        return;
    }
    
    /*
     name one container for each task:
     we have 2 tasks for the moment
     */
    container[STARTDEFAULTTASK] = &counter_StartDefaultTask;
    container[ETHERNETIFSETLINK] = &counter_ethernetif_set_link;
    
    return;
}

void q_init_container(struct counter *c){
    
    static uint32_t _last = 0;
    static uint32_t _minimum_size = 16;
    
    if (*container == NULL) {
        *container = (struct counter*) calloc(sizeof(struct counter*), _minimum_size);
        
        if (*container == NULL){
            RAW_DIAG("[ ERROR ] init_container");
            while (1) {};
            return;
        }
    }
    
    if (_last == (_minimum_size-1)) {
        _minimum_size = 2*_minimum_size;
        *container = (struct counter*) realloc(*container, sizeof(struct counter*) * _minimum_size);
    }
    /*
     name one container for each task:
     we have 2 tasks for the moment
     */
    container[_last] = c;
    _last++;
    container[_last] = NULL;
    
    return;
}


void init_counter(struct counter *c, char *name){
    
    uint32_t i = 0;
    
    reset_count(c);
    
    while (name[i] != '\0'){ // equivalent to strcpy
        c->task_name[i] = name[i];
        i++;
        if (i == 63) break;
    }
    c->task_name[i] = '\0';
    q_init_container(c);
    
    return;
}

/* Task: StartDefaultTask */
void init_counter_StartDefaultTask(void) {
    
    char* _name = "StartDefaultTask";
    uint32_t i = 0;
    reset_count(&counter_StartDefaultTask);
    
    while (_name[i] != '\0'){ // equivalent to strcpy
        counter_StartDefaultTask.task_name[i] = _name[i];
        i++;
        if (i == 63) break;
    }
    counter_StartDefaultTask.task_name[i] = '\0';
    
    return;
}

/* Task: ethernetif_set_link*/
void init_counter_ethernetif_set_link(void) {
    
    char* _name = "EthernetifSetLink";
    uint32_t i = 0;
    reset_count(&counter_ethernetif_set_link);
    
    while (_name[i] != '\0'){
        counter_ethernetif_set_link.task_name[i] = _name[i];
        i++;
        if (i == 63) break;
    }
    counter_ethernetif_set_link.task_name[i] = '\0';
    
    return;
}

#if 1
void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
     to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
     task. It is essential that code added to this hook function never attempts
     to block in any way (for example, call xQueueReceive() with a block time
     specified, or call vTaskDelay()). If the application makes use of the
     vTaskDelete() API function (as this demo application does) then it is also
     important that vApplicationIdleHook() is permitted to return to its calling
     function, because it is the responsibility of the idle task to clean up
     memory allocated by the kernel to any task that has since been deleted. */
    
    static uint32_t _tick=0, _last_tick=0, _this_tick=0;
    const float _freq = 16000000.0f; //(float) SystemCoreClock;
    enum task_counter _counters;
    const uint32_t _n_seconds = 60;
    const uint32_t _ticks_pause = 1000;
    const float _f_seconds = (float) _n_seconds;
    
    char log[32];
    
    sprintf(log, "[ Logging ] every %lds", _n_seconds);

    _this_tick = xTaskGetTickCount();
    _tick = _tick + ( _this_tick - _last_tick);
    _last_tick = _this_tick;
    
    if (_tick >= _n_seconds * _ticks_pause){
        RAW_DIAG(log);
#if 0   // original version
        for (uint32_t i=1; i < LAST; i++){
            char buf[128];
            float _total_usage = 100.0f * (float) container[i]->total_usage/(_freq * _f_seconds);
            uint32_t _integer = (uint32_t) floorf(_total_usage);
            uint32_t _dec = (uint32_t) ((_total_usage - (float) _integer) * 100.0f);
#if 0   // logging out
            sprintf(buf, "%s: %lu", container[i]->task_name,
                    container[i]->total_usage);
#else
            sprintf(buf, "%s: %lu.%lu", container[i]->task_name,
                    _integer, _dec);
#endif  // end logging out
            RAW_DIAG(buf);
            reset_count(container[i]);
        }
#else   // new version of initialization of counter
        uint32_t i = 0;
        while (container[i] != NULL){
            char buf[128];
            float _total_usage = 100.0f * (float) container[i]->total_usage/(_freq * _f_seconds);
            uint32_t _integer = (uint32_t) floorf(_total_usage);
            uint32_t _dec = (uint32_t) ((_total_usage - (float) _integer) * 100.0f);
            sprintf(buf, "%s: %lu.%lu", container[i]->task_name,
                    _integer, _dec);
            RAW_DIAG(buf);
            reset_count(container[i]);
            i++;
        }
#endif
        _tick = 0;
        return;
    }
    
    return;
}

#else

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
     to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
     task. It is essential that code added to this hook function never attempts
     to block in any way (for example, call xQueueReceive() with a block time
     specified, or call vTaskDelay()). If the application makes use of the
     vTaskDelete() API function (as this demo application does) then it is also
     important that vApplicationIdleHook() is permitted to return to its calling
     function, because it is the responsibility of the idle task to clean up
     memory allocated by the kernel to any task that has since been deleted. */
    
    static uint32_t _tick = 0, _last_tick = 0, _this_tick;
    
    _this_tick = xTaskGetTickCount();
    _tick = _tick + ( _this_tick - _last_tick);
    _last_tick = _this_tick;
    
    if (_tick >= 60000){ // each minute
            RAW_DIAG("[ IDLE ]");
            _tick = 0;
        return;
    }

    return;
}

#endif
