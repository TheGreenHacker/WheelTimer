#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "WheelTimer.h"

/* Comparator function to sort wheel timer's slots by the execution cycle of a wheel timer object. */
static int wt_element_comparator(void *wt_element_obj1, void *wt_element_obj2) {
    wheel_timer_element_t *wt_element1 = (wheel_timer_element_t *) wt_element_obj1;
    wheel_timer_element_t *wt_element2 = (wheel_timer_element_t *) wt_element_obj2;
    
    if (wt_element1->cycle > wt_element2->cycle) {
        return -1;
    }
    else if (wt_element1->cycle == wt_element2->cycle) {
        return 0;
    }
    else {
        return 1;
    }
}

/* Schedules an event in the wheel timer by computing a cycle and slot for it. */
static void assign_wt_element_to_slot(wheel_timer_t *wt, wheel_timer_element_t *wt_element) {
    unsigned int abs_slot = GET_WT_CURRENT_ABS_SLOT_NO(wt) + wt_element->interval;
    wt_element->cycle = abs_slot/wt->n_slots;
    wt_element->slot = abs_slot % wt->n_slots;
    singly_ll_add_ordered_data(wt->slots[wt_element->slot], wt_element);
}

/* Runs a timer in the background, waiting to process and schedule application tasks. */
static void *wheel_timer_fn(void *wheel_timer_ptr) {
    wheel_timer_t *wt = (wheel_timer_t *) wheel_timer_ptr;
    /* Allow wheel timer to be cancelled */
    int prevType;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &prevType);
    while (1) {
        sleep(wt->interval); // Waiting...
        
        /* For all events in the slot corresponding to this tick:
           If it is scheduled to run in the timer's current cycle
              Run it
              Remove it from the list for the current slot
              If it is periodic, set it to run later again appropriately by enqueuing it in (possibly) another slot's list
         */
        ll_t *slot_list = wt->slots[wt->curr_tic];
        singly_ll_node_t *slot_node, *prev;
        ITERATE_LIST_BEGIN2(slot_list, slot_node, prev) {
            wheel_timer_element_t *wt_element = (wheel_timer_element_t *)slot_node->data;
            if (wt_element->cycle == wt->curr_cycle) {
                wt_element->cb(wt_element->arg, wt_element->arg_size);
                singly_ll_delete_node_by_data_ptr(slot_list, wt_element);
                if (wt_element->is_periodic) {
                    assign_wt_element_to_slot(wt, wt_element);
                }
            }
            else {
                ITERATE_LIST_BREAK2(slot_list, slot_node, prev);
            }
        } ITERATE_LIST_END2(slot_list, slot_node, prev);
        
        /* Update timer progress */
        wt->curr_tic++;
        wt->curr_tic %= wt->n_slots;
        if (!wt->curr_tic) {
            wt->curr_cycle++;
        }
        
        printf("Wheel Timer Time = %u : \n", GET_WT_CURRENT_ABS_SLOT_NO(wt));
    }
    
    return NULL;
}

/* Creates an instance of a wheel timer */
wheel_timer_t*
init_wheel_timer(unsigned int interval, unsigned int n_slots) {
    wheel_timer_t *wt = calloc(1, sizeof(wheel_timer_t) + n_slots * sizeof(ll_t *));
    wt->interval = interval;
    wt->n_slots = n_slots;
    
    unsigned int i;
    for (i = 0; i < n_slots; i++) {
        ll_t *slot_list = init_singly_ll();
        singly_ll_set_comparison_fn(slot_list, wt_element_comparator);
        wt->slots[i] = slot_list;
    }
    
    return wt;
}

/* Creates a wheel timer element for an event the first time */
wheel_timer_element_t *
add_event(wheel_timer_t *wt, unsigned int interval, call_back cb, void *arg, unsigned int arg_size,  char is_periodic) {
    wheel_timer_element_t *wt_element = calloc(1, sizeof(wheel_timer_element_t));
    wt_element->interval = interval;
    wt_element->cb = cb;
    wt_element->arg = calloc(1, arg_size);
    memcpy(wt_element->arg, arg, arg_size);
    wt_element->arg_size = arg_size;
    wt_element->is_periodic = is_periodic;
    assign_wt_element_to_slot(wt, wt_element);
    
    return wt_element;
}

void
remove_event(wheel_timer_t *wt, wheel_timer_element_t *wt_element) {
    singly_ll_delete_node_by_data_ptr(wt->slots[wt_element->slot], wt_element);
}

void
wt_element_reschedule(wheel_timer_element_t *wt_element, unsigned int interval) {
    wt_element->interval = interval;
}

void
print_wheel_timer(wheel_timer_t *wt) {
    printf("Printing Wheel Timer\n");
    printf("wt->curr_tic = %u\n", wt->curr_tic);
    printf("wt->interval = %u\n", wt->interval);
    printf("wt->n_slots = %u\n", wt->n_slots);
    printf("wt->curr_cycle = %u\n", wt->curr_cycle);
    printf("wt->thread = %p\n", &wt->thread);
    printf("Printing the slots : \n");
    
    unsigned int i;
    for (i = 0; i < wt->n_slots; i++) {
        singly_ll_node_t *slot_node;
        ll_t *slot_list = wt->slots[i];
        printf("slot_list[%u] has %d nodes\n", i, GET_NODE_COUNT_SINGLY_LL(slot_list));
        
        ITERATE_LIST_BEGIN(slot_list, slot_node) {
            wheel_timer_element_t *wt_element = (wheel_timer_element_t *)slot_node->data;
            printf("wt_element->interval = %u\n", wt_element->interval);
            printf("wt_element->cycle = %d\n", wt_element->cycle);
            printf("wt_element->app_callback = %p\n", wt_element->cb);
            printf("wt_element->arg = %p\n", wt_element->arg);
            printf("wt_element->is_periodic = %d\n", wt_element->is_periodic);
        } ITERATE_LIST_END;
    }
}

void
start_wheel_timer(wheel_timer_t *wt) {
    if (pthread_create(&wt->thread, NULL, wheel_timer_fn, wt)) {
        fprintf(stderr, "Failed to start wheel timer thread!\n");
        exit(1);
    }
}

void
cancel_wheel_timer(wheel_timer_t *wt) {
    pthread_cancel(wt->thread);
}

void
reset_wheel_timer(wheel_timer_t *wt) {
    wt->curr_tic = 0;
    wt->curr_cycle = 0;
}
