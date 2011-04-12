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
	struct list *queue[MAXSIZE];
};

struct runqueue
{
	struct prio_array *active;
	struct prio_array *expired;
	struct prio_array arrays[2];
};
void init_runqueue(struct runqueue **rq);
void init_prio_array(struct prio_array **array);
void add_thread_a(struct runqueue *rq, struct list_elem *e);
void add_thread_e(struct runqueue *rq, struct list_elem *e);
void remove_thread_a(struct runqueue *rq, struct list_elem *e);
void remove_thread_e(struct runqueue *rq, struct list_elem *e);
struct list_elem *remove_certain_priority(struct runqueue *rq, int priority);
struct list_elem *search_highest(struct runqueue *rq);
struct list_elem *pop_highest(struct runqueue *rq);
bool prio_array_empty(struct prio_array *arr);
void swap_array(struct runqueue *rq);
