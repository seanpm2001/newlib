/*
FUNCTION
        <<mempcpy>>---copy memory regions and return end pointer

SYNOPSIS
        #include <string.h>
        void* mempcpy(void *<[out]>, const void *<[in]>, size_t <[n]>);

DESCRIPTION
        This function copies <[n]> bytes from the memory region
        pointed to by <[in]> to the memory region pointed to by
        <[out]>.

        If the regions overlap, the behavior is undefined.

RETURNS
        <<mempcpy>> returns a pointer to the byte following the
        last byte copied to the <[out]> region.

PORTABILITY
<<mempcpy>> is a GNU extension.

<<mempcpy>> requires no supporting OS subroutines.

	*/

#include <_ansi.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>

#ifdef __CHERI__
typedef __intcap_t BLOCK_TYPE;
#else
typedef long BLOCK_TYPE;
#endif

/* Nonzero if either X or Y is not aligned on a "BLOCK_TYPE" boundary.  */
#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (BLOCK_TYPE) - 1)) | ((long)Y & (sizeof (BLOCK_TYPE) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof (BLOCK_TYPE) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof (BLOCK_TYPE))

/* Threshhold for punting to the byte copier.  */
#ifdef __CHERI__
#define TOO_SMALL(LEN)  ((LEN) < LITTLEBLOCKSIZE)
#else
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)
#endif

void *
mempcpy (void *dst0,
	const void *src0,
	size_t len0)
{
#if (defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)) && !defined(__CHERI__)
  char *dst = (char *) dst0;
  char *src = (char *) src0;

  while (len0--)
    {
      *dst++ = *src++;
    }

  return dst;
#else
  char *dst = dst0;
  const char *src = src0;
  BLOCK_TYPE *aligned_dst;
  const BLOCK_TYPE *aligned_src;

  /* If the size is small, or either SRC or DST is unaligned,
     then punt into the byte copy loop.  This should be rare.  */
  if (!TOO_SMALL(len0) && !UNALIGNED (src, dst))
    {
      aligned_dst = (BLOCK_TYPE*)dst;
      aligned_src = (BLOCK_TYPE*)src;

      /* Copy 4X BLOCK_TYPE words at a time if possible.  */
      while (len0 >= BIGBLOCKSIZE)
        {
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          len0 -= BIGBLOCKSIZE;
        }

      /* Copy one BLOCK_TYPE word at a time if possible.  */
      while (len0 >= LITTLEBLOCKSIZE)
        {
          *aligned_dst++ = *aligned_src++;
          len0 -= LITTLEBLOCKSIZE;
        }

       /* Pick up any residual with a byte copier.  */
      dst = (char*)aligned_dst;
      src = (char*)aligned_src;
    }

  while (len0--)
    *dst++ = *src++;

  return dst;
#endif /* not PREFER_SIZE_OVER_SPEED */
}
