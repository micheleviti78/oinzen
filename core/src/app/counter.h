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

/* Aliasing */
typedef struct counter counter;

/* Counter structures for each task */
enum task_counter {BEGIN=0,
    STARTDEFAULTTASK,
    ETHERNETIFSETLINK,
    LAST};

/* StartDefaultTask */
struct counter counter_StartDefaultTask;
/* ethernetif_set_link */
struct counter counter_ethernetif_set_link;
/* Stupid */
struct counter counter_Stupid;

/* Container for all the struct counter */
struct counter **container;

/* Functions */
extern uint32_t uDWTCounterEnable(void);
extern void begin_count(struct counter *u);
extern void end_count(struct counter *u);
extern void reset_count(struct counter *u);
extern void init_container(void);
extern void q_init_container(struct counter *u);
extern void init_counter(struct counter *c, char *name);
extern void init_counter_StartDefaultTask(void);
extern void init_counter_ethernetif_set_link(void);
