/*
Copyright (c) 2007 by Juliusz Chroboczek

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

int seqno_compare(unsigned char s1, unsigned char s2)
    ATTRIBUTE ((const));
int seqno_minus(unsigned char s1, unsigned char s2)
    ATTRIBUTE ((const));
void timeval_minus(struct timeval *d,
                   const struct timeval *s1, const struct timeval *s2);
int timeval_minus_msec(const struct timeval *s1, const struct timeval *s2)
    ATTRIBUTE ((pure));
int timeval_compare(const struct timeval *s1, const struct timeval *s2)
    ATTRIBUTE ((pure));
void timeval_min(struct timeval *d, const struct timeval *s);
void timeval_min_sec(struct timeval *d, int secs);
void do_debugf(const char *format, ...) ATTRIBUTE ((format (printf, 1, 2)));
const char *format_address(const unsigned char *address);
int parse_address(const char *address, unsigned char *addr_r);
int parse_net(const char *net, unsigned char *prefix_r, unsigned short *plen_r);

/* If debugging is disabled, we want to avoid calling format_address
   for every omitted debugging message.  So debug is a macro.  But
   vararg macros are not portable... */
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#define debugf(...) \
    do { \
        if(debug >= 2) do_debugf(__VA_ARGS__); \
    } while(0)
#elif defined(__GNUC__)
#define debugf(_args...) \
    do { \
        if(debug >= 2) do_debugf(_args); \
    } while(0)
#else
#define debugf do_debugf
#endif
