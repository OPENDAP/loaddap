

union dnan {
    struct {
#if defined(LITTLE_ENDIAN) 
	unsigned fraction_low: 32;
	unsigned bits     :19;
	unsigned qnan_bit : 1;
	unsigned exponent :11;
	unsigned sign     : 1;
#else
	unsigned sign     : 1;
	unsigned exponent :11;
	unsigned qnan_bit : 1;
	unsigned bits     :19;
	unsigned fraction_low: 32;
#endif
    } nan;
    double d;
}; 

