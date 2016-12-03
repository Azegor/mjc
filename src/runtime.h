#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdlib.h>
#include <unistd.h>

__attribute__((__visibility__("default"))) void print_int(int val);
__attribute__((__visibility__("default"))) void print_int_fast(int val);
__attribute__((__visibility__("default"))) void *allocate(size_t num, size_t size);

#endif // RUNTIME_H
