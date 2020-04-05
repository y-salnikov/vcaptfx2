#include <stddef.h>
#include <stdlib.h>
#include "fifo.h"

t_fifo* fifo_init(unsigned int lenght)
{
	t_fifo* fifo;
	fifo=malloc(sizeof(fifo));
	fifo->buffer=malloc(lenght);
	if(fifo->buffer==NULL) return NULL;
	fifo->head=0;
	fifo->tail=0;
	fifo->lenght=lenght;
	return fifo;
}

unsigned int fifo_free_bytes(t_fifo *fifo)
{
	if(fifo->head>=fifo->tail)
		return fifo->lenght-(fifo->head - fifo->tail);
	else
		return (fifo->tail - fifo->head)-1;
}

char fifo_is_empty(t_fifo *fifo)
{
	return fifo->head==fifo->tail;
}

char fifo_read(t_fifo *fifo)
{
	char c;
	c=fifo->buffer[fifo->tail];
	fifo->tail=(fifo->tail+1) & (fifo->lenght-1);
	return c;
}

void fifo_write_block(t_fifo *fifo,char *src, unsigned int lenght)
{
	unsigned int i;
	for(i=0;i<lenght;i++)
	{
		fifo->buffer[(fifo->head+i) & (fifo->lenght-1)]=src[i];
	}
	fifo->head=(fifo->head+lenght) & (fifo->lenght-1);
}
