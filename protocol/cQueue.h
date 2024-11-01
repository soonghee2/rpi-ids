#ifndef CQUEUE_H
#define CQUEUE_H

#include <inttypes.h>
#include <cstdbool>
#include <cstdint>
#include <cstring>
#include <cstdlib>


#define QUEUE_INITIALIZED 0x5AA5
#define q_init_def(q, sz) q_init(q, sz, 20, FIFO, false)

#define q_pull q_pop
#define q_nbRecs q_getCount
#define q_clean q_flush

typedef enum enumQueueType {
	FIFO = 0,
	LIFO = 1
} QueueType;

typedef struct Queue_t {
	QueueType impl;
	bool      ovw = false;
	uint16_t  rec_nb = 0;
	uint16_t  rec_sz = 0;
	uint32_t  queue_sz = 0;
	uint8_t* queue = 0;

	uint16_t  in = 0;
	uint16_t  out = 0;
	uint16_t  cnt = 0;
	uint16_t  init = 0;
} Queue_t;

void q_flush(Queue_t* const q);

void* q_init(Queue_t* const q, const uint16_t size_rec, const uint16_t nb_recs, const QueueType type, const bool overwrite);
void q_kill(Queue_t* const q);

bool q_push(Queue_t* const q, const void* const record);
bool q_pop(Queue_t* const q, void* const record);
bool q_peek(const Queue_t* const q, void* const record);
bool q_drop(Queue_t* const q);
bool q_isEmpty(const Queue_t* const q);
bool q_isFull(const Queue_t* const q);
bool q_peekIdx(const Queue_t* const q, void* const record, const uint16_t idx);

uint16_t q_getSize(const Queue_t* const q);

#endif
