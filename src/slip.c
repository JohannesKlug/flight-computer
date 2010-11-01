/** defines for SLIP protocol **/
#define END     0300 /* begin/end marker */
#define ESC     0333 /* start escape sequence */
#define ESC_END 0334 /* to designate an escaped 0300 value */
#define ESC_ESC 0335 /* to designate an escaped 0333 value */

int convertToSLIP(unsigned char *dst, unsigned char *src, int src_len)
{
	int i, dst_len = 0;
	unsigned char *p, *q;
	
	p = src;
	q = dst;
	
	*q = END; q++;
	for ( i = 0; i < src_len; i++ )
	{
		switch ( *p )
		{
		case END:
			*q = ESC; q++; *q = ESC_END;
			break;
		case ESC:
			*q = ESC; q++; *q = ESC_ESC;
			break;
		default:
			*q = *p;
		}
		p++;
		q++;
	}
	*q = END; *q++;
	dst_len = (q - dst);
	
	return dst_len;
} /* convertToSLIP */
