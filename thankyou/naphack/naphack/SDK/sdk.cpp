#include "sdk.hpp"
#include "displacement.hpp"

const char* string_t::ToCStr() const
{
	return ( pszValue ) ? pszValue : "";
}

#pragma region impl_functions
void RandomSeed( unsigned int seed )
{
	using Fn = void ( * )( unsigned int );
	auto Procedure = ( Fn )( Engine::Displacement.Function.m_uRandomSeed );

	if( Procedure )
		Procedure( seed );
}

float RandomFloat( float min, float max )
{
	using Fn = float ( * )( float, float );
	auto Procedure = ( Fn )( Engine::Displacement.Function.m_uRandomFloat );

	return ( Procedure ? Procedure( min, max ) : 0.0f );
}

int RandomInt( int min, int max )
{
	using Fn = int ( * )( int, int );
	auto Procedure = ( Fn )( Engine::Displacement.Function.m_uRandomInt );

	return ( Procedure ? Procedure( min, max ) : 0 );
}

mstudioseqdesc_t &CStudioHdr::pSeqdesc( int iSequence ) {
	auto hdr_model = ( void * )( ( std::uintptr_t )this + 4 );
	if( hdr_model ) {
		static auto nigge = Memory::Scan( "client.dll", "55 8B EC 56 8B 75 ? 57 8B F9 85 F6 78 ? 8B 47" );

		return *reinterpret_cast< mstudioseqdesc_t * ( __thiscall * )( void *, int ) >( nigge )( this, iSequence );
	}

	auto studiohdr = *reinterpret_cast< studiohdr_t ** >( this );
	return *studiohdr->seq_desc( iSequence );
}

#pragma region impl_crc32
#define CRC32_INIT_VALUE 0xFFFFFFFFUL
#define CRC32_XOR_VALUE  0xFFFFFFFFUL

const CRC32_t pulCRCTable[256] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

void CRC32_Init( CRC32_t* pulCRC )
{
	*pulCRC = CRC32_INIT_VALUE;
}

void CRC32_ProcessBuffer( CRC32_t* pulCRC, const void* p, int len )
{
	CRC32_t ulCrc = *pulCRC;
	unsigned char *pb = (unsigned char *)p;
	unsigned int nFront;
	int nMain;

JustAfew:

	switch (len)
	{
	case 7:
		ulCrc  = pulCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 6:
		ulCrc  = pulCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 5:
		ulCrc  = pulCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 4:
		ulCrc ^= ( *(CRC32_t *)pb );
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		*pulCRC = ulCrc;
		return;

	case 3:
		ulCrc  = pulCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 2:
		ulCrc  = pulCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 1:
		ulCrc  = pulCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 0:
		*pulCRC = ulCrc;
		return;
	}

	nFront = ((unsigned int)pb) & 3;
	len -= nFront;
	switch (nFront)
	{
	case 3:
		ulCrc  = pulCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);
	case 2:
		ulCrc  = pulCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);
	case 1:
		ulCrc  = pulCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);
	}

	nMain = len >> 3;
	while (nMain--)
	{
		ulCrc ^= ( *(CRC32_t *)pb );
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc ^= ( *(CRC32_t *)(pb + 4) );
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc  = pulCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		pb += 8;
	}

	len &= 7;
	goto JustAfew;
}

void CRC32_Final( CRC32_t* pulCRC )
{
	*pulCRC ^= CRC32_XOR_VALUE;
}

CRC32_t CRC32_GetTableEntry( unsigned int slot )
{
	return pulCRCTable[ ( unsigned char )slot ];
}


#pragma endregion
#pragma endregion

#define big_short( val ) word_swap( val )
#define big_word( val ) word_swap( val )
#define big_long( val ) dword_swap( val )
#define big_dword( val ) dword_swap( val )
#define little_short( val ) ( val )
#define little_word( val ) ( val )
#define little_long( val ) ( val )
#define little_dword( val ) ( val )
#define swap_short( val ) big_short( val )
#define swap_word( val ) big_word( val )
#define swap_long( val ) big_long( val )
#define swap_dword( val ) big_dword( val )

#define crc32_init_value 0xFFFFFFFFUL
#define crc32_xor_value 0xFFFFFFFFUL

#define num_bytes 256

// The four core functions - F1 is optimized somewhat
// #define F1(x, y, z) (x & y | ~x & z)
#define F1( x, y, z ) ( z ^ ( x & ( y ^ z ) ) )
#define F2( x, y, z ) F1( z, x, y )
#define F3( x, y, z ) ( x ^ y ^ z )
#define F4( x, y, z ) ( y ^ ( x | ~z ) )

// This is the central step in the MD5 algorithm.
#define MD5STEP( f, w, x, y, z, data, s ) ( w += f( x, y, z ) + data, w = w << s | w >> ( 32 - s ), w += x )

//-----------------------------------------------------------------------------
// Purpose: The core of the MD5 algorithm, this alters an existing MD5 hash to
//  reflect the addition of 16 longwords of new data.  MD5Update blocks
//  the data and converts bytes into longwords for this routine.
// Input  : buf[4] -
//			in[16] -
// Output : static void
//-----------------------------------------------------------------------------
static void MD5Transform( unsigned int buf[ 4 ], unsigned int const in[ 16 ] ) {
	unsigned int a, b, c, d;

	a = buf[ 0 ];
	b = buf[ 1 ];
	c = buf[ 2 ];
	d = buf[ 3 ];

	MD5STEP( F1, a, b, c, d, in[ 0 ] + 0xd76aa478, 7 );
	MD5STEP( F1, d, a, b, c, in[ 1 ] + 0xe8c7b756, 12 );
	MD5STEP( F1, c, d, a, b, in[ 2 ] + 0x242070db, 17 );
	MD5STEP( F1, b, c, d, a, in[ 3 ] + 0xc1bdceee, 22 );
	MD5STEP( F1, a, b, c, d, in[ 4 ] + 0xf57c0faf, 7 );
	MD5STEP( F1, d, a, b, c, in[ 5 ] + 0x4787c62a, 12 );
	MD5STEP( F1, c, d, a, b, in[ 6 ] + 0xa8304613, 17 );
	MD5STEP( F1, b, c, d, a, in[ 7 ] + 0xfd469501, 22 );
	MD5STEP( F1, a, b, c, d, in[ 8 ] + 0x698098d8, 7 );
	MD5STEP( F1, d, a, b, c, in[ 9 ] + 0x8b44f7af, 12 );
	MD5STEP( F1, c, d, a, b, in[ 10 ] + 0xffff5bb1, 17 );
	MD5STEP( F1, b, c, d, a, in[ 11 ] + 0x895cd7be, 22 );
	MD5STEP( F1, a, b, c, d, in[ 12 ] + 0x6b901122, 7 );
	MD5STEP( F1, d, a, b, c, in[ 13 ] + 0xfd987193, 12 );
	MD5STEP( F1, c, d, a, b, in[ 14 ] + 0xa679438e, 17 );
	MD5STEP( F1, b, c, d, a, in[ 15 ] + 0x49b40821, 22 );

	MD5STEP( F2, a, b, c, d, in[ 1 ] + 0xf61e2562, 5 );
	MD5STEP( F2, d, a, b, c, in[ 6 ] + 0xc040b340, 9 );
	MD5STEP( F2, c, d, a, b, in[ 11 ] + 0x265e5a51, 14 );
	MD5STEP( F2, b, c, d, a, in[ 0 ] + 0xe9b6c7aa, 20 );
	MD5STEP( F2, a, b, c, d, in[ 5 ] + 0xd62f105d, 5 );
	MD5STEP( F2, d, a, b, c, in[ 10 ] + 0x02441453, 9 );
	MD5STEP( F2, c, d, a, b, in[ 15 ] + 0xd8a1e681, 14 );
	MD5STEP( F2, b, c, d, a, in[ 4 ] + 0xe7d3fbc8, 20 );
	MD5STEP( F2, a, b, c, d, in[ 9 ] + 0x21e1cde6, 5 );
	MD5STEP( F2, d, a, b, c, in[ 14 ] + 0xc33707d6, 9 );
	MD5STEP( F2, c, d, a, b, in[ 3 ] + 0xf4d50d87, 14 );
	MD5STEP( F2, b, c, d, a, in[ 8 ] + 0x455a14ed, 20 );
	MD5STEP( F2, a, b, c, d, in[ 13 ] + 0xa9e3e905, 5 );
	MD5STEP( F2, d, a, b, c, in[ 2 ] + 0xfcefa3f8, 9 );
	MD5STEP( F2, c, d, a, b, in[ 7 ] + 0x676f02d9, 14 );
	MD5STEP( F2, b, c, d, a, in[ 12 ] + 0x8d2a4c8a, 20 );

	MD5STEP( F3, a, b, c, d, in[ 5 ] + 0xfffa3942, 4 );
	MD5STEP( F3, d, a, b, c, in[ 8 ] + 0x8771f681, 11 );
	MD5STEP( F3, c, d, a, b, in[ 11 ] + 0x6d9d6122, 16 );
	MD5STEP( F3, b, c, d, a, in[ 14 ] + 0xfde5380c, 23 );
	MD5STEP( F3, a, b, c, d, in[ 1 ] + 0xa4beea44, 4 );
	MD5STEP( F3, d, a, b, c, in[ 4 ] + 0x4bdecfa9, 11 );
	MD5STEP( F3, c, d, a, b, in[ 7 ] + 0xf6bb4b60, 16 );
	MD5STEP( F3, b, c, d, a, in[ 10 ] + 0xbebfbc70, 23 );
	MD5STEP( F3, a, b, c, d, in[ 13 ] + 0x289b7ec6, 4 );
	MD5STEP( F3, d, a, b, c, in[ 0 ] + 0xeaa127fa, 11 );
	MD5STEP( F3, c, d, a, b, in[ 3 ] + 0xd4ef3085, 16 );
	MD5STEP( F3, b, c, d, a, in[ 6 ] + 0x04881d05, 23 );
	MD5STEP( F3, a, b, c, d, in[ 9 ] + 0xd9d4d039, 4 );
	MD5STEP( F3, d, a, b, c, in[ 12 ] + 0xe6db99e5, 11 );
	MD5STEP( F3, c, d, a, b, in[ 15 ] + 0x1fa27cf8, 16 );
	MD5STEP( F3, b, c, d, a, in[ 2 ] + 0xc4ac5665, 23 );

	MD5STEP( F4, a, b, c, d, in[ 0 ] + 0xf4292244, 6 );
	MD5STEP( F4, d, a, b, c, in[ 7 ] + 0x432aff97, 10 );
	MD5STEP( F4, c, d, a, b, in[ 14 ] + 0xab9423a7, 15 );
	MD5STEP( F4, b, c, d, a, in[ 5 ] + 0xfc93a039, 21 );
	MD5STEP( F4, a, b, c, d, in[ 12 ] + 0x655b59c3, 6 );
	MD5STEP( F4, d, a, b, c, in[ 3 ] + 0x8f0ccc92, 10 );
	MD5STEP( F4, c, d, a, b, in[ 10 ] + 0xffeff47d, 15 );
	MD5STEP( F4, b, c, d, a, in[ 1 ] + 0x85845dd1, 21 );
	MD5STEP( F4, a, b, c, d, in[ 8 ] + 0x6fa87e4f, 6 );
	MD5STEP( F4, d, a, b, c, in[ 15 ] + 0xfe2ce6e0, 10 );
	MD5STEP( F4, c, d, a, b, in[ 6 ] + 0xa3014314, 15 );
	MD5STEP( F4, b, c, d, a, in[ 13 ] + 0x4e0811a1, 21 );
	MD5STEP( F4, a, b, c, d, in[ 4 ] + 0xf7537e82, 6 );
	MD5STEP( F4, d, a, b, c, in[ 11 ] + 0xbd3af235, 10 );
	MD5STEP( F4, c, d, a, b, in[ 2 ] + 0x2ad7d2bb, 15 );
	MD5STEP( F4, b, c, d, a, in[ 9 ] + 0xeb86d391, 21 );

	buf[ 0 ] += a;
	buf[ 1 ] += b;
	buf[ 2 ] += c;
	buf[ 3 ] += d;
}

//-----------------------------------------------------------------------------
// Purpose: Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious initialization constants.

// Input  : *g_ctx -
//-----------------------------------------------------------------------------
void MD5Init( MD5Context_t *ctx ) {
	ctx->buf[ 0 ] = 0x67452301;
	ctx->buf[ 1 ] = 0xefcdab89;
	ctx->buf[ 2 ] = 0x98badcfe;
	ctx->buf[ 3 ] = 0x10325476;

	ctx->bits[ 0 ] = 0;
	ctx->bits[ 1 ] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Update context to reflect the concatenation of another buffer full of bytes.
// Input  : *g_ctx -
//			*buf -
//			len -
//-----------------------------------------------------------------------------
void MD5Update( MD5Context_t *ctx, unsigned char const *buf, unsigned int len ) {
	unsigned int t;

	/* Update bitcount */

	t = ctx->bits[ 0 ];
	if( ( ctx->bits[ 0 ] = t + ( ( unsigned int )len << 3 ) ) < t )
		ctx->bits[ 1 ]++; /* Carry from low to high */
	ctx->bits[ 1 ] += len >> 29;

	t = ( t >> 3 ) & 0x3f; /* Bytes already in shsInfo->data */

	/* Handle any leading odd-sized chunks */

	if( t ) {
		unsigned char *p = ( unsigned char * )ctx->in + t;

		t = 64 - t;
		if( len < t ) {
			std::memcpy( p, buf, len );
			return;
		}
		std::memcpy( p, buf, t );
		// byteReverse(g_ctx->in, 16);
		MD5Transform( ctx->buf, ( unsigned int * )ctx->in );
		buf += t;
		len -= t;
	}
	/* Process data in 64-byte chunks */

	while( len >= 64 ) {
		std::memcpy( ctx->in, buf, 64 );
		// byteReverse(g_ctx->in, 16);
		MD5Transform( ctx->buf, ( unsigned int * )ctx->in );
		buf += 64;
		len -= 64;
	}

	/* Handle any remaining bytes of data. */
	std::memcpy( ctx->in, buf, len );
}

//-----------------------------------------------------------------------------
// Purpose: Final wrapup - pad to 64-byte boundary with the bit pattern
// 1 0* (64-bit count of bits processed, MSB-first)
// Input  : digest[MD5_DIGEST_LENGTH] -
//			*g_ctx -
//-----------------------------------------------------------------------------
void MD5Final( unsigned char digest[ MD5_DIGEST_LENGTH ], MD5Context_t *ctx ) {
	unsigned count;
	unsigned char *p;

	/* Compute number of bytes mod 64 */
	count = ( ctx->bits[ 0 ] >> 3 ) & 0x3F;

	/* Set the first char of padding to 0x80.  This is safe since there is
	always at least one byte free */
	p = ctx->in + count;
	*p++ = 0x80;

	/* Bytes of padding needed to make 64 bytes */
	count = 64 - 1 - count;

	/* Pad out to 56 mod 64 */
	if( count < 8 ) {
		/* Two lots of padding:  Pad the first block to 64 bytes */
		std::memset( p, 0, count );
		// byteReverse(g_ctx->in, 16);
		MD5Transform( ctx->buf, ( unsigned int * )ctx->in );

		/* Now fill the next block with 56 bytes */
		std::memset( ctx->in, 0, 56 );
	}
	else {
		/* Pad block to 56 bytes */
		std::memset( p, 0, count - 8 );
	}
	// byteReverse(g_ctx->in, 14);

	/* Append length in bits and transform */
	( ( unsigned int * )ctx->in )[ 14 ] = ctx->bits[ 0 ];
	( ( unsigned int * )ctx->in )[ 15 ] = ctx->bits[ 1 ];

	MD5Transform( ctx->buf, ( unsigned int * )ctx->in );
	// byteReverse((unsigned char *) g_ctx->buf, 4);
	std::memcpy( digest, ctx->buf, MD5_DIGEST_LENGTH );
	std::memset( ctx, 0, sizeof( *ctx ) ); /* In case it's sensitive */
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *hash -
//			hashlen -
// Output : char
//-----------------------------------------------------------------------------
char *MD5_Print( unsigned char *hash, int hashlen ) {
	static char szReturn[ 64 ];

	return szReturn;
}

//-----------------------------------------------------------------------------
// Purpose: generate pseudo random number from a seed number
// Input  : seed number
// Output : pseudo random number
//-----------------------------------------------------------------------------
unsigned int MD5_PseudoRandom( unsigned int nSeed ) {
	MD5Context_t ctx;
	unsigned char digest[ MD5_DIGEST_LENGTH ]; // The MD5 Hash

	memset( &ctx, 0, sizeof( ctx ) );

	MD5Init( &ctx );
	MD5Update( &ctx, ( unsigned char * )&nSeed, sizeof( nSeed ) );
	MD5Final( digest, &ctx );

	return *( unsigned int * )( digest + 6 ); // use 4 middle bytes for random value
}

//-----------------------------------------------------------------------------
bool MD5_Compare( const void *&data, const void *&compare ) {
	// return V_memcmp(data.bits, compare.bits, MD5_DIGEST_LENGTH) == 0;
	return false;
}