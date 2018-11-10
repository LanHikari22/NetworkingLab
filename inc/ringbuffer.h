#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

// should be a value that is divisible by 2 and 4. This makes the buffer able to contain
// uint16_t and uint32_t elements as well.
#define BUF_SIZE 1028

//Ring Buffer Structure
typedef struct
{
    unsigned int put;
    unsigned int get;
    char buffer[BUF_SIZE];
} RingBuffer;

//Function Declaration
void put(volatile RingBuffer* buffer, char element);
char get(volatile RingBuffer* buffer);
char peek(volatile RingBuffer* buffer);
int hasSpace(volatile RingBuffer*);
int hasElement(volatile RingBuffer*);
int numElements(volatile RingBuffer*);

#endif /* RINGBUFFER_H_ */
