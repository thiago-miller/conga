#pragma once

#define EVENT_QUEUE_SEC  1000000
#define EVENT_QUEUE_SIZE 1024
#define EVENT_DELAY_MAX  EVENT_QUEUE_SEC
#define EVENT_DELAY_MIN  20000

typedef struct _EventQueue EventQueue;

typedef enum
{
	EVENT_NONE = 0,
	EVENT_QUIT,
	EVENT_KEY,
	EVENT_TIMER,
	EVENT_WINCH
} EventType;

typedef struct
{
	EventType type;
	int       key;
} Event;

EventQueue * event_queue_new             (int fps, int delay);
void         event_queue_wait_for_event  (EventQueue *queue, Event *event);
void         event_queue_free            (EventQueue *queue);
void         event_queue_pause           (EventQueue *queue, int paused);
int          event_queue_add_delay       (EventQueue *queue, int delay);
