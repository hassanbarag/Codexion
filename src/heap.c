/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassamoh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 15:17:23 by hassamoh          #+#    #+#             */
/*   Updated: 2026/10/05 12:01:57 by hassamoh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Ordering rule of the queue: the request with the smaller key is
** served first. If two keys are equal (only possible under edf,
** since fifo keys are unique arrival numbers)
*/
static int	waiter_before(t_waiter *a, t_waiter *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	return (a->seq < b->seq);
}

/*
** Moves the element at index i up until its parent is not after it.
*/
static void	sift_up(t_heap *heap, int i)
{
	t_waiter	tmp;
	int			parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (waiter_before(&heap->data[i], &heap->data[parent]) == 0)
			break ;
		tmp = heap->data[i];
		heap->data[i] = heap->data[parent];
		heap->data[parent] = tmp;
		i = parent;
	}
}

/*
** Moves the element at index i down until both children are not
** before it.
*/
static void	sift_down(t_heap *heap, int i)
{
	t_waiter	tmp;
	int			child;

	while (2 * i + 1 < heap->size)
	{
		child = 2 * i + 1;
		if (child + 1 < heap->size
			&& waiter_before(&heap->data[child + 1], &heap->data[child]))
			child++;
		if (waiter_before(&heap->data[child], &heap->data[i]) == 0)
			break ;
		tmp = heap->data[i];
		heap->data[i] = heap->data[child];
		heap->data[child] = tmp;
		i = child;
	}
}

/*
** Inserts a request. Returns 1 on success, 0 if the heap is full.
*/
int	heap_push(t_heap *heap, t_waiter waiter)
{
	if (heap->size >= DONGLE_QUEUE_CAP)
		return (0);
	heap->data[heap->size] = waiter;
	sift_up(heap, heap->size);
	heap->size++;
	return (1);
}

/*
** Removes the request identified by "seq", wherever it sits in the
** heap (not necessarily at the top). Used when a waiter gives up
** (simulation stopping) before ever being granted the dongle.
** Does nothing if "seq" is not found.
*/
void	heap_remove(t_heap *heap, long seq)
{
	int	i;

	i = 0;
	while (i < heap->size && heap->data[i].seq != seq)
		i++;
	if (i == heap->size)
		return ;
	heap->size--;
	heap->data[i] = heap->data[heap->size];
	if (i < heap->size)
	{
		sift_up(heap, i);
		sift_down(heap, i);
	}
}
