#pragma once

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
