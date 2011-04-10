#include <stdio.h>
#include <stdbool.h>
#include <list.h>
#include <bitmap.h>
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#define MAXSIZE 64

struct prio_array
{
	int nr_active;	//the number of tasks
	unsigned long bitmap[MAXSIZE];
	struct list (*queue)[MAXSIZE];
};

struct runqueue
{
	struct prio_array *active;
	struct prio_array *expired;
	struct prio_array arrays[2];
};
/*Initialize runqueue.*/
void init_runqueue(struct runqueue *rq);
/*Initialize priority array.*/
void init_prio_array(struct prio_array *array);
/*Add thread. Get the 2nd argument for list_elem.*/
void add_thread_e(struct runqueue *rq, struct list_elem *e);
/*Add thread. Get the 2nd argument for thread.*/
void add_thread_t(struct runqueue *rq, struct thread *t);
/*Remove thread. Get the 2nd argument for list_elem.*/
void remove_thread_e(struct runqueue *rq, struct list_elem *e);
/*Remove thread. Get the 2nd argument for thread.*/
void remove_thread_t(struct runqueue *rq, struct thread *t);
/*Remove certain priority task from priority array*/
struct list_elem *remove_certain_priority(struct runqueue *rq, int priority);
/*Find the highest-priority task.*/
struct list_elem *search_highest(struct runqueue *rq);
/*Find the highest-priority task and pop it from priority array.*/
struct list_elem *pop_highest(struct runqueue *rq);
/*Check if priority array is empty.*/
bool prio_array_empty(struct prio_array *arr);
/*Swap active priority array with expired priority array when the active array is empty.*/
void swap_array(struct runqueue *rq);
