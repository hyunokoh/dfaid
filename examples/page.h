#ifndef PAGE_H
#define PAGE_H

// define a page in S4
//#define PAGE_SIZE (1024*4)
#define PAGE_SIZE 32*4*2
typedef struct {
	char data[PAGE_SIZE];
} page_type;

#endif
