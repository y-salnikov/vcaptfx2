typedef struct fifo_struct
{
	char* buffer;
	size_t lenght;  // must be 2^n
	unsigned int head;
	unsigned int tail;
}t_fifo;

t_fifo* fifo_init(unsigned int length);
