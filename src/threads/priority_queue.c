#include <stdio.h>
#include <stdbool.h>
#include <list.h>
#include "threads/priority_queue.h"
#define MAXSIZE 64

/*Initialize runqueue.*/
void init_runqueue(struct runqueue *rq)
{
	rq = (struct runqueue *)malloc(sizeof(struct runqueue));
	init_prio_array(&(rq->arrays[0]));
	init_prio_array(&(rq->arrays[1]));
	rq->active = &(rq->arrays[0]);
	rq->expired = &(rq->arrays[1]);
}
/*Initialize priority array.*/
void init_prio_array(struct prio_array *array)
{
	int i;
	array = (struct prio_array *)malloc(sizeof(struct prio_array));
	for(i = 0; i < MAXSIZE; i++)
		list_init(array->queue[i]);
	
	array->nr_active = 0;
	
}
/*Add thread. Get the 2nd argument for list_elem.*/
void add_thread_e(struct runqueue *rq, struct list_elem *e)
{
	struct thread *t = list_entry(e, struct thread,	elem);
	int priority = t->priority;
	list_push_back(rq->active->queue[priority], e);
	(rq->active->nr_active)++;
	if(!(list_empty(rq->active->queue[priority])) && rq->active->bitmap[priority]==0)
		rq->active->bitmap[priority] = 1;
}
/*Add thread. Get the 2nd argument for thread.*/
void add_thread_t(struct runqueue *rq, struct thread *t)
{
	int priority = t->priority;
	list_push_back(rq->active->queue[priority], &t->elem);
	(rq->active->nr_active)++;
	if(!(list_empty(rq->active->queue[priority])) && rq->active->bitmap[priority]==0)
		rq->active->bitmap[priority] = 1;
}

/*Remove thread. Get the 2nd argument for list_elem.*/
void remove_thread_e(struct runqueue *rq, struct list_elem *e)
{
	struct thread *t = list_entry(e, struct thread, elem);
	int priority = t->priority;
	list_remove(e);
	(rq->active->nr_active)--;
	if(list_empty(rq->active->queue[priority]) && rq->active->bitmap[priority]==1)
		rq->active->bitmap[priority] = 0;	
}
/*Remove thread. Get the 2nd argument for thread.*/
void remove_thread_t(struct runqueue *rq, struct thread *t)
{
	int priority = t->priority;
	list_remove(&t->elem);
	(rq->active->nr_active)--;
	if(list_empty(rq->active->queue[priority]) && rq->active->bitmap[priority]==1)
		rq->active->bitmap[priority] = 0;	
}
/*Remove certain priority task from priority array*/
struct list_elem *remove_certain_priority(struct runqueue *rq, int priority)
{
	(rq->active->nr_active)--;
	if(list_empty(rq->active->queue[priority]) && rq->active->bitmap[priority]==1)
		rq->active->bitmap[priority] = 0;	
	return list_pop_front(rq->active->queue[priority]);
}

/*Find the highest-priority task.*/
struct list_elem *search_highest(struct runqueue *rq)
{
	int i;
	ASSERT (!prio_array_empty(rq->active));
	for(i = MAXSIZE-1; i > -1; i--){
		if(rq->active->bitmap[i] == 1)
			break;
	}
	return list_begin(rq->active->queue[i]);	
}
/*Find the highest-priority task and pop it from priority array.*/
struct list_elem *pop_highest(struct runqueue *rq)
{
	int i;
	ASSERT (!prio_array_empty(rq->active));
	for(i = MAXSIZE-1; i > -1; i--){
		if(rq->active->bitmap[i] == 1)
			break;
	}
	return list_pop_front(rq->active->queue[i]);
}
/*Check if priority array is empty.*/
bool prio_array_empty(struct prio_array *arr)
{
	return !(arr->nr_active);	
}
/*Swap active priority array with expired priority array when the active array is empty.*/
void swap_array(struct runqueue *rq)
{
	if(prio_array_empty(rq->active)){
		if(rq->active == &(rq->arrays[0])){
			rq->active = &(rq->arrays[1]);
			rq->expired = &(rq->arrays[0]);
		}
		else{
			rq->active = &(rq->arrays[0]);
			rq->expired = &(rq->arrays[1]);
		}
	}
}
