#define DEBUG

#include "util.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define POOL_SIZE 67108864 /* 64 MB */

static uint8_t pool[POOL_SIZE] = {0};

struct block {
	struct block *prev, *next;
	size_t data_size;
};

static_assert(sizeof(struct block) == 24, "24 byte block size");

size_t
block_size(struct block *b) {
	return sizeof(*b) + b->data_size;
}

ptrdiff_t
block_position(struct block *b) {
	return (uint8_t *)b - pool;
}

bool
block_in_pool(struct block *b) {
	ptrdiff_t pos = block_position(b);
	return pos >= 0 && pos < POOL_SIZE;
}

struct block *
next_block(struct block *b) {
	return (struct block *)((char *)b + block_size(b));
}

struct block *
block_before_space(size_t size) {
	struct block *b = (struct block *)pool;
	for (; b->next; b = b->next) {
		if (!block_in_pool(b->next))
			die("Corrupted address %p", (void *)b->next);
		if ((struct block *)((char *)next_block(b) + size) < b->next)
			return b;
	}
	return b;
}

void
join_blocks(struct block *a, struct block *b) {
	a->next = b;
	b->prev = a;
}

void
insert_block(struct block *x, struct block *a, struct block *b) {
	x->next = b;
	join_blocks(a, x);
}

void
mempool_init(void) {
	size_t s = 128;
	struct block genesis = {
		.prev = NULL,
		.next = NULL,
		.data_size = s - sizeof(struct block),
	};
	*(struct block *)pool = genesis;
}

void *
mempool_alloc(size_t size) {
	struct block *b = block_before_space(size);
	struct block *new = next_block(b);
	new->data_size = size;
	insert_block(new, b, b->next);
	DEBUG_PRINT("Allocated %zu bytes at %zu\n",
	            size,
	            block_position(new) + sizeof(struct block));
	return new + 1;
}

void
mempool_free(void *ptr) {
	struct block *b = (struct block *)ptr - 1;
	join_blocks(b->prev, b->next);
	DEBUG_PRINT("Freed %zu bytes at %zu\n",
	            b->data_size,
	            block_position(b) + sizeof(struct block));
}
