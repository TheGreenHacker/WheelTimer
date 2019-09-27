#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "WheelTimer.h"
#include "LinkedList.h"

#define WHEEL_SIZE 10
#define WHEEL_TIMER_CLOCK_TIC_INTERVAL 1

/* Application routine to be (indirectly) invoked by Wheel timer.
 * It could be of any prototype*/
void
print_hello(char *S){ 
    printf("%s\n", S);
}

/* But Only routines (Events) which have prototype : void *(fn)(void *arg, int arg_size) 
 * could be registered with wheel timer. Therefore, we need to encapsulate
 * the actual routine print_hello() to be invoked inside the routine of 
 * void *(fn)(void *arg, int arg_size) prototype. It is the wrapper which will be registered 
 * with wheel timer and invoked by wheel timer. We will unwrap the argument and invoke the actual 
 * appln routine with correct arguments. This technique is called 'Masking of routines'*/

void wrapper_print_hello(void *arg, int arg_size){
    char *S = (char *)arg;
    print_hello(S);
}

int
main(){

    /*create a wheel timer object*/
    wheel_timer_t *wt = init_wheel_timer(WHEEL_TIMER_CLOCK_TIC_INTERVAL, WHEEL_SIZE);
    /*start the wheel timer thread*/
    start_wheel_timer(wt);

    /*Now Wheel timer has started running in a separte thread. 
     * Register the events to be triggered with Wheel timer now.*/

    wheel_timer_element_t *wt_element1 = add_event(wt, 5, wrapper_print_hello, "Jack",
                           strlen("Jack"),
                           1);

    wheel_timer_element_t *wt_element2 = add_event(wt, 3, wrapper_print_hello, "Jerry",
                           strlen("Jerry"),
                           1);
    
    wheel_timer_element_t *wt_element3 = add_event(wt, 15, wrapper_print_hello, "CNN",
    strlen("CNN"),
    1);
   
    /*
    sleep(30);
    remove_event(wt, wt_element1);
     */
    /*
    sleep(30);
    cancel_wheel_timer(wt);
    printf("cancelled wheel timer\n");
     */
    
    //printf("wt_element3->cycle : %u", wt_element3->cycle);
     
    /*stop the main program from gettin terminated, otherwise wheel timer
     * thread we created will also get terminated*/
    scanf("\n");
    return 0;
}
