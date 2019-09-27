#ifndef WHEEL_TIMER_H
#define WHEEL_TIMER_H

#include <pthread.h>
#include "LinkedList.h"

/* Data structures and API declarations for implementing wheel timer to schedule system tasks */
typedef void (*call_back)(void *arg, int sizeof_arg);

/* Represents an event to be scheduled in timer */
typedef struct _wheel_timer_element_t{
    unsigned int interval;      // time interval after which call back (the event) needs to be triggered
    unsigned int cycle;         // cycle in which event needs to be triggered
    unsigned int slot;          // slot in which event resides in
    call_back cb;               // the app function to be executed
    void *arg;                  // argument for app function
    unsigned int arg_size;      // size of the argument
    char is_periodic;           // indicates if this event should be triggered periodically or only once
} wheel_timer_element_t;

/* Represents the timer */
typedef struct _wheel_timer_t {
    unsigned int curr_tic;      // current slot pointed by the clock tic, increments by 1 per interval
    unsigned int interval;      // time interval by which clock tic moves
    unsigned int n_slots;       // number of slots in timer
    unsigned int curr_cycle;    // number of cycles clock tick has completed
    pthread_t thread;           // thread of wheel timer to execute app callback
    ll_t *slots[0];             // linked list objects attached to slots of timer, dynamically sized based on wheel_size
} wheel_timer_t;

wheel_timer_t*
init_wheel_timer(unsigned int interval, unsigned int n_slots);

/*Gives the absolute slot no since the time WT has started*/
#define GET_WT_CURRENT_ABS_SLOT_NO(wt)    ((wt->curr_cycle * wt->n_slots) + wt->curr_tic)

wheel_timer_element_t *
add_event(wheel_timer_t *wt,
                   unsigned int interval,
                   call_back cb,
                   void *arg,
                   unsigned int arg_size,
                   char is_periodic);


void
remove_event(wheel_timer_t *wt, wheel_timer_element_t *wt_element);

void
wt_element_reschedule(wheel_timer_element_t *wt_element, unsigned int interval);

void
print_wheel_timer(wheel_timer_t *wt);

void
start_wheel_timer(wheel_timer_t *wt);

void
cancel_wheel_timer(wheel_timer_t *wt);

void
reset_wheel_timer(wheel_timer_t *wt);

#endif
