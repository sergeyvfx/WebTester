/*
 *
 * ================================================================================
 *  md5.c
 * ================================================================================
 *
 *  MD5 password encryption stuff
 *
 *  Based on PHP MD5 stuff source
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "md5.h"
#include <memory.h>

unsigned char PADDING[64] =
  {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };

unsigned char itoa64[] =  // 0 ... 63 => ascii - 64
	"@#0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void
md5_to64                           (char *__s, unsigned int __v, int __n)
{
  while (--__n>=0)
    {
      *__s++=itoa64[__v&0x3f];
      __v  >>= 6;
    }
}

unsigned
md5_strlcpy                        (char *__dst, const char *__src, unsigned __siz)
{
  register char *d=__dst;
  register const char *s=__src;
  register size_t n=__siz;

  // Copy as many bytes as will fit
  if (n!=0 && --n!=0)
    {
      do
        {
          if (!(*d++ = *s++))
            break;
        } while (--n!=0);
    }

  // Not enough room in dst, add NUL and traverse rest of src
  if (!n)
    {
      if (__siz != 0)
        *d = '\0';	// NUL-terminate dst
      while (*s++);
    }

  return (unsigned)(s-__src-1); // count does not include NUL
}

void
md5_init                           (md5_ctx_t *__context)
{
	__context->count[0]=__context->count[1] = 0;
  // Load magic initialization constants
  __context->state[0] = 0x67452301;
  __context->state[1] = 0xefcdab89;
  __context->state[2] = 0x98badcfe;
  __context->state[3] = 0x10325476;
}

void
md5_decode                         (unsigned int *__output, const unsigned char *__input, unsigned int __len)
{
  unsigned int i, j;
  for (i=0, j=0; j<__len; i++, j+=4)
    __output[i] = ((unsigned int) __input[j]) | (((unsigned int) __input[j+1])<<8) |
      (((unsigned int) __input[j+2])<<16) | (((unsigned int) __input[j+3])<<24);
}

void
md5_transform                      (unsigned int __state[4], const unsigned char __block[64])
{
  unsigned int a=__state[0], b=__state[1], c=__state[2], d=__state[3], x[16];
  md5_decode (x, __block, 64);
  // Round 1
  FF (a, b, c, d, x[0], S11, 0xd76aa478);  // 1
  FF (d, a, b, c, x[1], S12, 0xe8c7b756);  // 2
  FF (c, d, a, b, x[2], S13, 0x242070db);  // 3
  FF (b, c, d, a, x[3], S14, 0xc1bdceee);  // 4
  FF (a, b, c, d, x[4], S11, 0xf57c0faf);  // 5
  FF (d, a, b, c, x[5], S12, 0x4787c62a);  // 6
  FF (c, d, a, b, x[6], S13, 0xa8304613);  // 7
  FF (b, c, d, a, x[7], S14, 0xfd469501);  // 8
  FF (a, b, c, d, x[8], S11, 0x698098d8);  // 9
  FF (d, a, b, c, x[9], S12, 0x8b44f7af);  // 10
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); // 11
  FF (b, c, d, a, x[11], S14, 0x895cd7be); // 12
  FF (a, b, c, d, x[12], S11, 0x6b901122); // 13
  FF (d, a, b, c, x[13], S12, 0xfd987193); // 14
  FF (c, d, a, b, x[14], S13, 0xa679438e); // 15
  FF (b, c, d, a, x[15], S14, 0x49b40821); // 16

  // Round 2
  GG (a, b, c, d, x[1], S21, 0xf61e2562);  // 17
  GG (d, a, b, c, x[6], S22, 0xc040b340);  // 18
  GG (c, d, a, b, x[11], S23, 0x265e5a51); // 19
  GG (b, c, d, a, x[0], S24, 0xe9b6c7aa);  // 20
  GG (a, b, c, d, x[5], S21, 0xd62f105d);  // 21
  GG (d, a, b, c, x[10], S22, 0x2441453);  // 22
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); // 23
  GG (b, c, d, a, x[4], S24, 0xe7d3fbc8);  // 24
  GG (a, b, c, d, x[9], S21, 0x21e1cde6);  // 25
  GG (d, a, b, c, x[14], S22, 0xc33707d6); // 26
  GG (c, d, a, b, x[3], S23, 0xf4d50d87);  // 27
  GG (b, c, d, a, x[8], S24, 0x455a14ed);  // 28
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); // 29
  GG (d, a, b, c, x[2], S22, 0xfcefa3f8);  // 30
  GG (c, d, a, b, x[7], S23, 0x676f02d9);  // 31
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); // 32

  // Round 3
  HH (a, b, c, d, x[5], S31, 0xfffa3942); // 33
  HH (d, a, b, c, x[8], S32, 0x8771f681); // 34
  HH (c, d, a, b, x[11], S33, 0x6d9d6122);// 35
  HH (b, c, d, a, x[14], S34, 0xfde5380c);// 36
  HH (a, b, c, d, x[1], S31, 0xa4beea44); // 37
  HH (d, a, b, c, x[4], S32, 0x4bdecfa9); // 38
  HH (c, d, a, b, x[7], S33, 0xf6bb4b60); // 39
  HH (b, c, d, a, x[10], S34, 0xbebfbc70);// 40
  HH (a, b, c, d, x[13], S31, 0x289b7ec6);// 41
  HH (d, a, b, c, x[0], S32, 0xeaa127fa); // 42
  HH (c, d, a, b, x[3], S33, 0xd4ef3085); // 43 
  HH (b, c, d, a, x[6], S34, 0x4881d05);  // 44
  HH (a, b, c, d, x[9], S31, 0xd9d4d039); // 45
  HH (d, a, b, c, x[12], S32, 0xe6db99e5);// 46
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8);// 47
  HH (b, c, d, a, x[2], S34, 0xc4ac5665); // 48

  // Round 4
  II (a, b, c, d, x[0], S41, 0xf4292244); // 49
  II (d, a, b, c, x[7], S42, 0x432aff97); // 50
  II (c, d, a, b, x[14], S43, 0xab9423a7);// 51
  II (b, c, d, a, x[5], S44, 0xfc93a039); // 52
  II (a, b, c, d, x[12], S41, 0x655b59c3);// 53
  II (d, a, b, c, x[3], S42, 0x8f0ccc92); // 54
  II (c, d, a, b, x[10], S43, 0xffeff47d);// 55
  II (b, c, d, a, x[1], S44, 0x85845dd1); // 56
  II (a, b, c, d, x[8], S41, 0x6fa87e4f); // 57
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0);// 58
  II (c, d, a, b, x[6], S43, 0xa3014314); // 59
  II (b, c, d, a, x[13], S44, 0x4e0811a1);// 60
  II (a, b, c, d, x[4], S41, 0xf7537e82); // 61
  II (d, a, b, c, x[11], S42, 0xbd3af235);// 62
  II (c, d, a, b, x[2], S43, 0x2ad7d2bb); // 63
  II (b, c, d, a, x[9], S44, 0xeb86d391); // 64

  __state[0] += a;
  __state[1] += b;
  __state[2] += c;
  __state[3] += d;

  // Zeroize sensitive information
  memset ((unsigned char*) x, 0, sizeof(x));
}

void
md5_update                       (
                                   md5_ctx_t *__context,
                                   const unsigned char *__input,
                                   unsigned int __input_len
                                 )
{
  unsigned int i, index, part_len;
  // Compute number of bytes mod 64
  index=(unsigned int)((__context->count[0]>>3)&0x3F);
  // Update number of bits
  if ((__context->count[0]+=((unsigned int)__input_len<<3)) < ((unsigned int)__input_len<<3))
    __context->count[1]++;
  __context->count[1]+=((unsigned int)__input_len>>29);
  part_len=64-index;
  // Transform as many times as possible.
  if (__input_len>=part_len)
    {
      memcpy  ((unsigned char*)&__context->buffer[index], (unsigned char*)__input, part_len);
      md5_transform (__context->state, __context->buffer);
      for (i=part_len; i+63<__input_len; i+=64)
        md5_transform (__context->state, &__input[i]);
        index = 0;
    } else i=0;
  // Buffer remaining input
  memcpy ((unsigned char*)&__context->buffer[index], (unsigned char*)&__input[i], __input_len-i);
}

void
md5_encode                         (unsigned char *__output, unsigned int *__input, unsigned int __len)
{
  unsigned int i, j;
  for (i=0, j=0; j<__len; i++,j+=4)
    {
      __output[j]     = (unsigned char) ( __input[i]      & 0xff);
      __output[j + 1] = (unsigned char) ((__input[i]>>8)  & 0xff);
      __output[j + 2] = (unsigned char) ((__input[i]>>16) & 0xff);
      __output[j + 3] = (unsigned char) ((__input[i]>>24) & 0xff);
    }
}

void
md5_final                          (unsigned char __digest[16], md5_ctx_t *__context)
{
  unsigned char bits[8];
  unsigned int index, pad_len;
  // Save number of bits 
  md5_encode (bits, __context->count, 8);
  // Pad out to 56 mod 64
  index=(unsigned int)((__context->count[0]>>3)&0x3f);
  pad_len=(index<56)?(56-index):(120-index);
  md5_update (__context, PADDING, pad_len);
  // Append length (before padding)
  md5_update (__context, bits, 8);
  // Store state in digest
  md5_encode (__digest, __context->state, 16);
  // Zeroize sensitive information
  memset ((unsigned char*)__context, 0, sizeof(*__context));
}

// MD5 password encryption.
void
md5_crypt                          (const char *__pw, const char *__salt, char *__out)
{
  char passwd[120], *p;
  const char *sp, *ep;
  unsigned char final[16];
  unsigned int i, sl, pwl;
  md5_ctx_t ctx, ctx1;
  unsigned int l;
  int pl;
	pwl=(unsigned int)strlen (__pw);
  // Refine the salt first
  sp=__salt;
  // If it starts with the magic string, then skip that
  if (!strncmp (sp, MD5_MAGIC, MD5_MAGIC_LEN))
    sp+=MD5_MAGIC_LEN;
  //* It stops at the first '$', max 8 chars
  for (ep=sp; *ep!='\0' && *ep!='$' && ep<(sp+8); ep++)
		continue;
  // get the length of the true salt
  sl=(unsigned int)(ep-sp);
  md5_init (&ctx);
  // The password first, since that is what is most unknown
  md5_update (&ctx, (const unsigned char*)__pw, pwl);
  // Then our magic string
  md5_update (&ctx, (const unsigned char*)MD5_MAGIC, MD5_MAGIC_LEN);
  // Then the raw salt
  md5_update (&ctx, (const unsigned char*)sp, sl);
  // Then just as many characters of the MD5(pw,salt,pw)
  md5_init (&ctx1);
  md5_update (&ctx1, (const unsigned char*)__pw, pwl);
  md5_update (&ctx1, (const unsigned char*)sp, sl);
  md5_update (&ctx1, (const unsigned char*)__pw, pwl);
  md5_final (final, &ctx1);
  for (pl=pwl; pl>0; pl-=16)
    md5_update (&ctx, final, (unsigned int)(pl>16?16:pl));
  // Don't leave anything around in vm they could use
  memset (final, 0, sizeof (final));
  // Then something really weird...
  for (i=pwl; i!=0; i>>=1)
    if ((i&1) != 0)
      md5_update (&ctx, final, 1); else
      md5_update (&ctx, (const unsigned char*)__pw, 1);
  // Now make the output string
	memcpy (passwd, MD5_MAGIC, MD5_MAGIC_LEN);
  md5_strlcpy (passwd+MD5_MAGIC_LEN, sp, sl+1);
  strcat (passwd, "y");
  md5_final (final, &ctx);
  /*
   * And now, just to make sure things don't run too fast. On a 60 MHz
   * Pentium this takes 34 msec, so you would need 30 seconds to build
   * a 1000 entry dictionary...
     */
  for (i=0; i<1000; i++)
    {
      md5_init (&ctx1);
      if (i&1)
        md5_update (&ctx1, (const unsigned char*)__pw, pwl); else
        md5_update (&ctx1, final, 16);
      if (i%3) md5_update (&ctx1, (const unsigned char*)sp, sl);
      if (i%7) md5_update (&ctx1, (const unsigned char*)__pw, pwl);
      if (i&1)
        md5_update (&ctx1, final, 16); else
        md5_update (&ctx1, (const unsigned char*)__pw, pwl);
      md5_final (final, &ctx1);
    }
  p=passwd+sl+MD5_MAGIC_LEN+1;
  l=(final[ 0]<<16) | (final[ 6]<<8) | final[12]; md5_to64(p,l,4); p += 4;
  l=(final[ 1]<<16) | (final[ 7]<<8) | final[13]; md5_to64(p,l,4); p += 4;
  l=(final[ 2]<<16) | (final[ 8]<<8) | final[14]; md5_to64(p,l,4); p += 4;
  l=(final[ 3]<<16) | (final[ 9]<<8) | final[15]; md5_to64(p,l,4); p += 4;
  l=(final[ 4]<<16) | (final[10]<<8) | final[ 5]; md5_to64(p,l,4); p += 4;
  l=final[11]; md5_to64(p,l,2); p += 2;
  *p = '\0';
  
  if (__out)
    {
      int i, len=0;
      for (i=MD5_MAGIC_LEN; i<(int)strlen (passwd); i++)
        *(__out+len++)=passwd[i];
      *(__out+len)=0;
    }

  // Don't leave anything around in vm they could use
  memset (final, 0, sizeof(final));
  memset (passwd, 0, sizeof (passwd));
}
