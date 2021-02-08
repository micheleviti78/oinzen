//
//  counter.h
//  
//
//  Created by Moreno Marcellini on 04/02/21.
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

uint32_t uDWTCounterEnable(void) {
    
    volatile uint32_t _c;
#if 0
    /* Enable TRC
     DEMCR:
     The 32-bit Debug Exception and Monitor Control Register:
     This provides Vector Catching and Debug Monitor Control. This register
     contains a bit named TRCENA which enable the use of a TRACE.
     */
    CoreDebug->DEMCR &= ~0x01000000;
    CoreDebug->DEMCR |=  0x01000000;
#endif
    
#if defined(STM32F7xx)
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

/* Counter structure */
struct counter {
    uint32_t begin;
    uint32_t end;
    uint32_t total_usage;
    char task_name[64];
};

/* Aliasing */
typedef struct counter counter;

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

/* Counter structures for each task */
enum task_counter {BEGIN=0, STARTDEFAULTTASK,
    ETHERNETIFSETLINK,
    LAST};

/* ethernetif_set_link */
/* StartDefaultTask */
struct counter counter_StartDefaultTask;
struct counter counter_ethernetif_set_link;

/* Container for all the struct counter */
struct counter **container;

static void init_container(void){
    
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

/* Task: StartDefaultTask */
static void init_counter_StartDefaultTask(void) {
    
    char* _name = "StartDefaultTask";
    uint32_t i = 0;
    reset_count(&counter_StartDefaultTask);
    
    while (_name[i] != '\0'){
        counter_StartDefaultTask.task_name[i] = _name[i];
        i++;
        if (i == 63) break;
    }
    counter_StartDefaultTask.task_name[i] = '\0';
    
    return;
}

/* Task: ethernetif_set_link*/
static void init_counter_ethernetif_set_link(void) {
    
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

    _this_tick = xTaskGetTickCount();
    _tick = _tick + ( _this_tick - _last_tick);
    _last_tick = _this_tick;
    
    if (_tick >= 10000){
        for (uint32_t i=1; i < LAST; i++){
            char buf[128];
            float _total_usage = 100.0f * (float) container[i]->total_usage/_freq;
            uint32_t _integer = (uint32_t) floorf(_total_usage);
            uint32_t _dec = (uint32_t) ((_total_usage - (float) _integer) * 100.0f);
#if 0
            sprintf(buf, "%s: %lu", container[i]->task_name,
                    container[i]->total_usage);
#else
            sprintf(buf, "%s: %lu.%lu", container[i]->task_name,
                    _integer, _dec);
#endif
            RAW_DIAG(buf);
            reset_count(container[i]);
            _tick = 0;
        }
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
