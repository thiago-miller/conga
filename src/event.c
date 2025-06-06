#include "event.h"

#include <ncurses.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include "wrapper.h"

#define EVENT_QUEUE_SEC  1000000
#define EVENT_QUEUE_SIZE 1024

struct _EventQueue
{
	Event      events[EVENT_QUEUE_SIZE];

	int        head;
	int        tail;

	useconds_t tick;
	useconds_t delay;
	useconds_t acm;

	int        paused;
};

static volatile sig_atomic_t quit   = 0;
static volatile sig_atomic_t redraw = 0;

static void
event_set_quit (int signum)
{
	quit = 1;
}

static void
event_set_redraw (int signum)
{
	redraw = 1;
}

static inline int
event_index_next (int i)
{
	return (i + 1) % EVENT_QUEUE_SIZE;
}

static inline int
event_queue_is_empty (const EventQueue *queue)
{
	return queue->head == -1;
}

static inline int
event_queue_is_full (const EventQueue *queue)
{
	return event_index_next (queue->tail) == queue->head;
}

static inline Event *
event_queue_shift (EventQueue *queue)
{
	if (event_queue_is_empty (queue))
		return NULL;

	Event *event = &queue->events[queue->head];

	if (queue->head == queue->tail)
		{
			queue->head = -1;
			queue->tail = -1;
		}
	else
		queue->head = event_index_next (queue->head);

	return event;
}

static inline void
event_queue_push (EventQueue *queue, EventType type, int key)
{
	assert (!event_queue_is_full (queue));

	if (event_queue_is_empty (queue))
		queue->head = 0;

	queue->tail = event_index_next (queue->tail);

	queue->events[queue->tail].type = type;
	queue->events[queue->tail].key  = key;
}

static inline void
event_init (void)
{
	signal (SIGINT,   event_set_quit);
	signal (SIGQUIT,  event_set_quit);
	signal (SIGTERM,  event_set_quit);
	signal (SIGWINCH, event_set_redraw);
}

EventQueue *
event_queue_new (int fps, int delay)
{
	assert (delay > EVENT_QUEUE_SEC / fps);

	event_init ();

	EventQueue *queue = xmalloc (sizeof (EventQueue));

	*queue  = (EventQueue) {
		.events = {},
		.head   = -1,
		.tail   = -1,
		.tick   = EVENT_QUEUE_SEC / fps,
		.delay  = delay,
		.acm    = 0
	};

	return queue;
}

void
event_queue_free (EventQueue *queue)
{
	xfree (queue);
}

void
event_queue_wait_for_event (EventQueue *queue, Event *event)
{
	assert (queue != NULL);
	assert (event != NULL);

	while (event_queue_is_empty (queue))
		{
			usleep (queue->tick);

			if (quit)
				event_queue_push (queue, EVENT_QUIT, -1);

			if (redraw)
				{
					event_queue_push (queue, EVENT_WINCH, -1);
					redraw = 0;
				}

			int ch = getch ();
			if (ch != ERR)
				event_queue_push (queue, EVENT_KEY, ch);

			if (!queue->paused)
				{
					queue->acm += queue->tick;

					if (queue->acm >= queue->delay)
						{
							queue->acm %= queue->delay;
							event_queue_push (queue, EVENT_TIMER, -1);
						}
				}
		}

	*event = * (event_queue_shift (queue));
}

void
event_queue_pause (EventQueue *queue, int paused)
{
	assert (queue != NULL);
	queue->paused = paused;
}
