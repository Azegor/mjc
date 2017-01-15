#include "runtime.h"

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

void print_int(int val) {
  printf("%d\n", val);
}


// from http://bufferedbyteswriter.blogspot.de/2013/03/fast-inputoutput-in-c.html
#define pc(x) putchar_unlocked(x);
void print_int_fast(int val) {
  int N = val, rev, count = 0;
  rev = N;
  if (N == 0) { pc('0'); pc('\n'); return ;}
  while ((rev % 10) == 0) { count++; rev /= 10;} //obtain the count of the number of 0s
  rev = 0;
  while (N != 0) { rev = (rev<<3) + (rev<<1) + N % 10; N /= 10;}  //store reverse of N in rev
  while (rev != 0) { pc(rev % 10 + '0'); rev /= 10;}
  while (count--) pc('0');
  pc('\n');
}


void *allocate(size_t num, size_t size) {
  void* res = calloc(num, size);
  assert(res);
  return res;
}

int read_int() {
  int c = fgetc(stdin);
  if (c == EOF)
    return -1;
  return c;
}

void write_int(int32_t val) {
  const int octet = (int) (((unsigned) val) & 0xffU );
  if (fputc (octet , stdout ) < 0) {
    fprintf(stderr , "error: write: %s\n", strerror(errno));
    abort();
  }
}

void flush_int() {
 fflush(stdout);
}
