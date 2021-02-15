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

#if 0
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

#else

void init_container(struct counter *c){
    
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

#endif

void init_counter(struct counter *c, char *name){
    
    uint32_t i = 0;
    
    reset_count(c);
    
    while (name[i] != '\0'){ // equivalent to strcpy
        c->task_name[i] = name[i];
        i++;
        if (i == 63) break;
    }
    c->task_name[i] = '\0';
    init_container(c);
    
    return;
}
/* NOT NEEDED ANYMORE */
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
    const uint32_t _n_seconds = 60;
    const uint32_t _ticks_pause = 1000;
    static const float _freq = 160000000; //(float) SystemCoreClock;
    static const float _f_seconds = (float) _n_seconds;
    static const float _ratio = 100.0f/(_freq * _f_seconds);
    char buf[128];
    
    sprintf(buf, "[ Logging ] every %lds", _n_seconds);

    _this_tick = xTaskGetTickCount();
    _tick = _tick + (_this_tick - _last_tick);
    _last_tick = _this_tick;
    
    if (_tick >= _n_seconds * _ticks_pause){
        RAW_DIAG(log);
        uint32_t i = 0;
        while (container[i] != NULL){
            
#if 0
            float _total_usage = 100.0f * (float) container[i]->total_usage/(_freq * _f_seconds);
#else
            float _total_usage = (float) container[i]->total_usage * _ratio;
#endif
            uint32_t _integer = (uint32_t) floorf(_total_usage);
            uint32_t _dec = (uint32_t) ((_total_usage - (float) _integer) * 100.0f);
            sprintf(buf, "%s: %lu.%lu", container[i]->task_name,
                    _integer, _dec);
            RAW_DIAG(buf);
            reset_count(container[i]);
            i++;
        }
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
