/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Unit tests for BLOSC_NOLOCK environment variable in Blosc.

  Creation date: 2016-04-25
  Author: Francesc Alted <francesc@blosc.org>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/

#include <unistd.h>
#include "test_common.h"

#define BUFFER_ALIGN_SIZE   32
#define NCHILDREN 4
#define NTHREADS 4

int tests_run = 0;

/* Global vars */
void *src, *srccpy, *dest, *dest2;
int nbytes, cbytes;
int clevel = 1;
int doshuffle = 1;
size_t typesize = 4;
size_t size = sizeof(int32_t) * 1000 * 1000;


/* Check just compressing */
static char *test_compress() {

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + 16);
  mu_assert("ERROR: cbytes is not correct", cbytes < size);

  return 0;
}


/* Check compressing + decompressing */
static char *test_compress_decompress() {

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + 16);
  mu_assert("ERROR: cbytes is not correct", cbytes < size);

  /* Decompress the buffer */
  nbytes = blosc_decompress(dest, dest2, size);
  mu_assert("ERROR: nbytes incorrect(1)", nbytes == (int)size);

  return 0;
}


static char *all_tests() {
  mu_run_test(test_compress);
  mu_run_test(test_compress_decompress);

  return 0;
}


int main(int argc, char **argv) {
  int32_t *_src;
  char *result;

  printf("STARTING TESTS for %s\n", argv[0]);

  /* Activate the BLOSC_NOLOCK variable */
  setenv("BLOSC_NOLOCK", "TRUE", 0);

  /* Launch several subprocesses */
  for (int i = 1; i <= NCHILDREN; i++) {
    int pid = fork();
  }

  blosc_init();
  blosc_set_nthreads(NTHREADS);

  /* Initialize buffers */
  src = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  srccpy = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  dest = blosc_test_malloc(BUFFER_ALIGN_SIZE, size + 16);
  dest2 = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  _src = (int32_t *)src;
  for (int i = 0; i < (size / sizeof(int32_t)); i++) {
    _src[i] = (int32_t)i;
  }
  memcpy(srccpy, src, size);

  /* Run all the suite */
  result = all_tests();
  if (result != 0) {
    printf(" (%s)\n", result);
  }
  else {
    printf(" ALL TESTS PASSED\n");
  }
  printf("\tTests run: %d\n", tests_run);

  blosc_test_free(src);
  blosc_test_free(srccpy);
  blosc_test_free(dest);
  blosc_test_free(dest2);

  blosc_destroy();

  return result != 0;
}
