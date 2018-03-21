/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Roundtrip tests for the NEON-accelerated shuffle/unshuffle.

  Creation date: 2017-07-28
  Author: Lucian Marc <ruben.lucian@gmail.com>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/

#include "test_common.h"
#include "../blosc/shuffle.h"
#include "../blosc/bitshuffle-generic.h"


/* Include NEON-accelerated shuffle implementation if supported by this compiler.
   TODO: Need to also do run-time CPU feature support here. */
#if defined(SHUFFLE_NEON_ENABLED)
  #include "../blosc/bitshuffle-neon.h"
#else
  #if defined(_MSC_VER)
    #pragma message("NEON shuffle tests not enabled.")
  #else
    #warning NEON shuffle tests not enabled.
  #endif
#endif  /* defined(SHUFFLE_NEON_ENABLED) */


/** Roundtrip tests for the NEON-accelerated shuffle/unshuffle. */
static int test_bitshuffle_roundtrip_neon(size_t type_size, size_t num_elements,
                                       size_t buffer_alignment, int test_type) {

fprintf(stdout, "type_size=%d num_elements=%d buffer_alignment=%d test_type=%d\n", type_size, num_elements, buffer_alignment, test_type);
#if defined(SHUFFLE_NEON_ENABLED)
  size_t buffer_size = type_size * num_elements;

  /* Allocate memory for the test. */
  void* original = blosc_test_malloc(buffer_alignment, buffer_size);
  void* shuffled = blosc_test_malloc(buffer_alignment, buffer_size);
  void* unshuffled = blosc_test_malloc(buffer_alignment, buffer_size);

  void* tmp_buf = blosc_test_malloc(buffer_alignment, buffer_size);

  /* Fill the input data buffer with random values. */
  blosc_test_fill_random(original, buffer_size);

  /* Shuffle/unshuffle, selecting the implementations based on the test type. */
  blosc_timestamp_t start;
  blosc_set_timestamp(&start);

  switch (test_type) {
    case 0:
      /* neon/neon */
      bitshuffle_neon(original, shuffled, num_elements, type_size, tmp_buf);
      bitunshuffle_neon(shuffled, unshuffled, num_elements, type_size, tmp_buf);
      break;
    case 1:
      /* generic/neon */
      bshuf_trans_bit_elem_scal(original, shuffled, num_elements, type_size, tmp_buf);
      bitunshuffle_neon(shuffled, unshuffled,  num_elements, type_size, tmp_buf);
      break;
    case 2:
      /* neon/generic */
      bitshuffle_neon(original, shuffled, num_elements, type_size, tmp_buf);
      bshuf_untrans_bit_elem_scal(shuffled, unshuffled, num_elements, type_size, tmp_buf);
      break;
    case 3:
      /* generic/generic */
      bshuf_trans_bit_elem_scal(original, shuffled, num_elements, type_size, tmp_buf);
      bshuf_untrans_bit_elem_scal(shuffled, unshuffled, num_elements, type_size, tmp_buf);
      break;
    default:
      fprintf(stderr, "Invalid test type specified (%d).", test_type);
      return EXIT_FAILURE;
  }
  blosc_timestamp_t end;
  blosc_set_timestamp(&end);

  double elapsed = blosc_elapsed_secs(start, end);
  fprintf(stdout, "elapsed = %f\n", elapsed);

  /* The round-tripped data matches the original data when the
     result of memcmp is 0. */
  int exit_code = memcmp(original, unshuffled, buffer_size) ?
                  EXIT_FAILURE : EXIT_SUCCESS;

  /* Free allocated memory. */
  blosc_test_free(original);
  blosc_test_free(shuffled);
  blosc_test_free(unshuffled);

  return exit_code;
#else
  return EXIT_SUCCESS;
#endif /* defined(SHUFFLE_NEON_ENABLED) */
}


/** Required number of arguments to this test, including the executable name. */
#define TEST_ARG_COUNT  5

int main(int argc, char** argv) {
  /*  argv[1]: sizeof(element type)
      argv[2]: number of elements
      argv[3]: buffer alignment
      argv[4]: test type
  */

  /*  Verify the correct number of command-line args have been specified. */
  if (TEST_ARG_COUNT != argc) {
    blosc_test_print_bad_argcount_msg(TEST_ARG_COUNT, argc);
    return EXIT_FAILURE;
  }

  /* Parse arguments */
  uint32_t type_size;
  if (!blosc_test_parse_uint32_t(argv[1], &type_size) || (type_size < 1)) {
    blosc_test_print_bad_arg_msg(1);
    return EXIT_FAILURE;
  }

  uint32_t num_elements;
  if (!blosc_test_parse_uint32_t(argv[2], &num_elements) || (num_elements < 1)) {
    blosc_test_print_bad_arg_msg(2);
    return EXIT_FAILURE;
  }

  uint32_t buffer_align_size;
  if (!blosc_test_parse_uint32_t(argv[3], &buffer_align_size)
      || (buffer_align_size & (buffer_align_size - 1))
      || (buffer_align_size < sizeof(void*))) {
    blosc_test_print_bad_arg_msg(3);
    return EXIT_FAILURE;
  }

  uint32_t test_type;
  if (!blosc_test_parse_uint32_t(argv[4], &test_type) || (test_type > 3)) {
    blosc_test_print_bad_arg_msg(4);
    return EXIT_FAILURE;
  }

  /* Run the test. */
  return test_bitshuffle_roundtrip_neon(type_size, num_elements, buffer_align_size, test_type);
}
