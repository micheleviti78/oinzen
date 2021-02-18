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


/* Variable definition */
/* Counter structure */
struct counter {
    uint32_t begin;
    uint32_t end;
    uint32_t total_usage;
    char task_name[64];
};

uint32_t number_of_counters;

/* Aliasing */
typedef struct counter counter;

/* StartDefaultTask */
struct counter counter_StartDefaultTask;
/* ethernetif_set_link */
struct counter counter_ethernetif_set_link;
/* Stupid */
struct counter counter_Stupid;
/* tinyd */
struct counter counter_tinyd;
/* counter for thread EthIf */
struct counter counter_EthIf;
/* counter for thread tcpip_thread */
struct counter counter_tcpip_thread;

/* Container for all the struct counter */
struct counter **container;
/* Parallel container to be used and shown on the web page*/
#define LENGTHOFWEBCONTAINER 64
char **web_container;

/* Functions */
extern uint32_t uDWTCounterEnable(void);
extern uint32_t uDWTCounterReset(void);
extern void begin_count(struct counter *u);
extern void end_count(struct counter *u);
extern void reset_count(struct counter *u);
extern void init_counter(struct counter *c, char *name);

#if 0
extern void init_counter_StartDefaultTask(void);
extern void init_counter_ethernetif_set_link(void);
#endif
