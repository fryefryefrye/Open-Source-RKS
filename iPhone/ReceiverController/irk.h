#define BOOLEAN bool
#define UINT8 uint8_t
#define uint_8t uint8_t
#define UINT16 uint16_t
#define uint_32t uint32_t
#define TRUE true
#define FALSE false
#define osi_free free
#define osi_malloc malloc
#define BT_OCTET16_LEN    16
#define SMP_ENCRYT_DATA_SIZE               16
#define SMP_ENCRYT_KEY_SIZE                16
#define HCI_SUCCESS                                     0x00
#define HCI_GRP_BLE_CMDS                (0x08 << 10)
#define HCI_BLE_ENCRYPT                 (0x0017 | HCI_GRP_BLE_CMDS)

/////////////////////////////////////////
//aes define
#define N_ROW                   4
#define N_COL                   4
#define N_BLOCK   (N_ROW * N_COL)

#define WPOLY   0x011b
#define BPOLY     0x1b
#define DPOLY   0x008d

#define f1(x)   (x)
#define f2(x)   ((x << 1) ^ (((x >> 7) & 1) * WPOLY))
#define f4(x)   ((x << 2) ^ (((x >> 6) & 1) * WPOLY) ^ (((x >> 6) & 2) * WPOLY))
#define f8(x)   ((x << 3) ^ (((x >> 5) & 1) * WPOLY) ^ (((x >> 5) & 2) * WPOLY) \
	^ (((x >> 5) & 4) * WPOLY))
#define d2(x)   (((x) >> 1) ^ ((x) & 1 ? DPOLY : 0))

#define f3(x)   (f2(x) ^ x)
#define f9(x)   (f8(x) ^ x)
#define fb(x)   (f8(x) ^ f2(x) ^ x)
#define fd(x)   (f8(x) ^ f4(x) ^ x)
#define fe(x)   (f8(x) ^ f4(x) ^ f2(x))

#define sb_data(w) {    /* S Box data values */                            \
	w(0x63), w(0x7c), w(0x77), w(0x7b), w(0xf2), w(0x6b), w(0x6f), w(0xc5),\
	w(0x30), w(0x01), w(0x67), w(0x2b), w(0xfe), w(0xd7), w(0xab), w(0x76),\
	w(0xca), w(0x82), w(0xc9), w(0x7d), w(0xfa), w(0x59), w(0x47), w(0xf0),\
	w(0xad), w(0xd4), w(0xa2), w(0xaf), w(0x9c), w(0xa4), w(0x72), w(0xc0),\
	w(0xb7), w(0xfd), w(0x93), w(0x26), w(0x36), w(0x3f), w(0xf7), w(0xcc),\
	w(0x34), w(0xa5), w(0xe5), w(0xf1), w(0x71), w(0xd8), w(0x31), w(0x15),\
	w(0x04), w(0xc7), w(0x23), w(0xc3), w(0x18), w(0x96), w(0x05), w(0x9a),\
	w(0x07), w(0x12), w(0x80), w(0xe2), w(0xeb), w(0x27), w(0xb2), w(0x75),\
	w(0x09), w(0x83), w(0x2c), w(0x1a), w(0x1b), w(0x6e), w(0x5a), w(0xa0),\
	w(0x52), w(0x3b), w(0xd6), w(0xb3), w(0x29), w(0xe3), w(0x2f), w(0x84),\
	w(0x53), w(0xd1), w(0x00), w(0xed), w(0x20), w(0xfc), w(0xb1), w(0x5b),\
	w(0x6a), w(0xcb), w(0xbe), w(0x39), w(0x4a), w(0x4c), w(0x58), w(0xcf),\
	w(0xd0), w(0xef), w(0xaa), w(0xfb), w(0x43), w(0x4d), w(0x33), w(0x85),\
	w(0x45), w(0xf9), w(0x02), w(0x7f), w(0x50), w(0x3c), w(0x9f), w(0xa8),\
	w(0x51), w(0xa3), w(0x40), w(0x8f), w(0x92), w(0x9d), w(0x38), w(0xf5),\
	w(0xbc), w(0xb6), w(0xda), w(0x21), w(0x10), w(0xff), w(0xf3), w(0xd2),\
	w(0xcd), w(0x0c), w(0x13), w(0xec), w(0x5f), w(0x97), w(0x44), w(0x17),\
	w(0xc4), w(0xa7), w(0x7e), w(0x3d), w(0x64), w(0x5d), w(0x19), w(0x73),\
	w(0x60), w(0x81), w(0x4f), w(0xdc), w(0x22), w(0x2a), w(0x90), w(0x88),\
	w(0x46), w(0xee), w(0xb8), w(0x14), w(0xde), w(0x5e), w(0x0b), w(0xdb),\
	w(0xe0), w(0x32), w(0x3a), w(0x0a), w(0x49), w(0x06), w(0x24), w(0x5c),\
	w(0xc2), w(0xd3), w(0xac), w(0x62), w(0x91), w(0x95), w(0xe4), w(0x79),\
	w(0xe7), w(0xc8), w(0x37), w(0x6d), w(0x8d), w(0xd5), w(0x4e), w(0xa9),\
	w(0x6c), w(0x56), w(0xf4), w(0xea), w(0x65), w(0x7a), w(0xae), w(0x08),\
	w(0xba), w(0x78), w(0x25), w(0x2e), w(0x1c), w(0xa6), w(0xb4), w(0xc6),\
	w(0xe8), w(0xdd), w(0x74), w(0x1f), w(0x4b), w(0xbd), w(0x8b), w(0x8a),\
	w(0x70), w(0x3e), w(0xb5), w(0x66), w(0x48), w(0x03), w(0xf6), w(0x0e),\
	w(0x61), w(0x35), w(0x57), w(0xb9), w(0x86), w(0xc1), w(0x1d), w(0x9e),\
	w(0xe1), w(0xf8), w(0x98), w(0x11), w(0x69), w(0xd9), w(0x8e), w(0x94),\
	w(0x9b), w(0x1e), w(0x87), w(0xe9), w(0xce), w(0x55), w(0x28), w(0xdf),\
	w(0x8c), w(0xa1), w(0x89), w(0x0d), w(0xbf), w(0xe6), w(0x42), w(0x68),\
	w(0x41), w(0x99), w(0x2d), w(0x0f), w(0xb0), w(0x54), w(0xbb), w(0x16) }


#define sb_data(w) {    /* S Box data values */                            \
	w(0x63), w(0x7c), w(0x77), w(0x7b), w(0xf2), w(0x6b), w(0x6f), w(0xc5),\
	w(0x30), w(0x01), w(0x67), w(0x2b), w(0xfe), w(0xd7), w(0xab), w(0x76),\
	w(0xca), w(0x82), w(0xc9), w(0x7d), w(0xfa), w(0x59), w(0x47), w(0xf0),\
	w(0xad), w(0xd4), w(0xa2), w(0xaf), w(0x9c), w(0xa4), w(0x72), w(0xc0),\
	w(0xb7), w(0xfd), w(0x93), w(0x26), w(0x36), w(0x3f), w(0xf7), w(0xcc),\
	w(0x34), w(0xa5), w(0xe5), w(0xf1), w(0x71), w(0xd8), w(0x31), w(0x15),\
	w(0x04), w(0xc7), w(0x23), w(0xc3), w(0x18), w(0x96), w(0x05), w(0x9a),\
	w(0x07), w(0x12), w(0x80), w(0xe2), w(0xeb), w(0x27), w(0xb2), w(0x75),\
	w(0x09), w(0x83), w(0x2c), w(0x1a), w(0x1b), w(0x6e), w(0x5a), w(0xa0),\
	w(0x52), w(0x3b), w(0xd6), w(0xb3), w(0x29), w(0xe3), w(0x2f), w(0x84),\
	w(0x53), w(0xd1), w(0x00), w(0xed), w(0x20), w(0xfc), w(0xb1), w(0x5b),\
	w(0x6a), w(0xcb), w(0xbe), w(0x39), w(0x4a), w(0x4c), w(0x58), w(0xcf),\
	w(0xd0), w(0xef), w(0xaa), w(0xfb), w(0x43), w(0x4d), w(0x33), w(0x85),\
	w(0x45), w(0xf9), w(0x02), w(0x7f), w(0x50), w(0x3c), w(0x9f), w(0xa8),\
	w(0x51), w(0xa3), w(0x40), w(0x8f), w(0x92), w(0x9d), w(0x38), w(0xf5),\
	w(0xbc), w(0xb6), w(0xda), w(0x21), w(0x10), w(0xff), w(0xf3), w(0xd2),\
	w(0xcd), w(0x0c), w(0x13), w(0xec), w(0x5f), w(0x97), w(0x44), w(0x17),\
	w(0xc4), w(0xa7), w(0x7e), w(0x3d), w(0x64), w(0x5d), w(0x19), w(0x73),\
	w(0x60), w(0x81), w(0x4f), w(0xdc), w(0x22), w(0x2a), w(0x90), w(0x88),\
	w(0x46), w(0xee), w(0xb8), w(0x14), w(0xde), w(0x5e), w(0x0b), w(0xdb),\
	w(0xe0), w(0x32), w(0x3a), w(0x0a), w(0x49), w(0x06), w(0x24), w(0x5c),\
	w(0xc2), w(0xd3), w(0xac), w(0x62), w(0x91), w(0x95), w(0xe4), w(0x79),\
	w(0xe7), w(0xc8), w(0x37), w(0x6d), w(0x8d), w(0xd5), w(0x4e), w(0xa9),\
	w(0x6c), w(0x56), w(0xf4), w(0xea), w(0x65), w(0x7a), w(0xae), w(0x08),\
	w(0xba), w(0x78), w(0x25), w(0x2e), w(0x1c), w(0xa6), w(0xb4), w(0xc6),\
	w(0xe8), w(0xdd), w(0x74), w(0x1f), w(0x4b), w(0xbd), w(0x8b), w(0x8a),\
	w(0x70), w(0x3e), w(0xb5), w(0x66), w(0x48), w(0x03), w(0xf6), w(0x0e),\
	w(0x61), w(0x35), w(0x57), w(0xb9), w(0x86), w(0xc1), w(0x1d), w(0x9e),\
	w(0xe1), w(0xf8), w(0x98), w(0x11), w(0x69), w(0xd9), w(0x8e), w(0x94),\
	w(0x9b), w(0x1e), w(0x87), w(0xe9), w(0xce), w(0x55), w(0x28), w(0xdf),\
	w(0x8c), w(0xa1), w(0x89), w(0x0d), w(0xbf), w(0xe6), w(0x42), w(0x68),\
	w(0x41), w(0x99), w(0x2d), w(0x0f), w(0xb0), w(0x54), w(0xbb), w(0x16) }

static const uint_8t gfm2_sbox[256] = sb_data(f2);
#define gfm2_sb(x)   gfm2_sbox[(x)]

static const uint_8t gfm3_sbox[256] = sb_data(f3);
#define gfm3_sb(x)   gfm3_sbox[(x)]

static const uint_8t sbox[256]  =  sb_data(f1);
#define s_box(x)     sbox[(x)]


static void copy_and_key( void *d, const void *s, const void *k )
{
	((uint_8t *)d)[ 0] = ((uint_8t *)s)[ 0] ^ ((uint_8t *)k)[ 0];
	((uint_8t *)d)[ 1] = ((uint_8t *)s)[ 1] ^ ((uint_8t *)k)[ 1];
	((uint_8t *)d)[ 2] = ((uint_8t *)s)[ 2] ^ ((uint_8t *)k)[ 2];
	((uint_8t *)d)[ 3] = ((uint_8t *)s)[ 3] ^ ((uint_8t *)k)[ 3];
	((uint_8t *)d)[ 4] = ((uint_8t *)s)[ 4] ^ ((uint_8t *)k)[ 4];
	((uint_8t *)d)[ 5] = ((uint_8t *)s)[ 5] ^ ((uint_8t *)k)[ 5];
	((uint_8t *)d)[ 6] = ((uint_8t *)s)[ 6] ^ ((uint_8t *)k)[ 6];
	((uint_8t *)d)[ 7] = ((uint_8t *)s)[ 7] ^ ((uint_8t *)k)[ 7];
	((uint_8t *)d)[ 8] = ((uint_8t *)s)[ 8] ^ ((uint_8t *)k)[ 8];
	((uint_8t *)d)[ 9] = ((uint_8t *)s)[ 9] ^ ((uint_8t *)k)[ 9];
	((uint_8t *)d)[10] = ((uint_8t *)s)[10] ^ ((uint_8t *)k)[10];
	((uint_8t *)d)[11] = ((uint_8t *)s)[11] ^ ((uint_8t *)k)[11];
	((uint_8t *)d)[12] = ((uint_8t *)s)[12] ^ ((uint_8t *)k)[12];
	((uint_8t *)d)[13] = ((uint_8t *)s)[13] ^ ((uint_8t *)k)[13];
	((uint_8t *)d)[14] = ((uint_8t *)s)[14] ^ ((uint_8t *)k)[14];
	((uint_8t *)d)[15] = ((uint_8t *)s)[15] ^ ((uint_8t *)k)[15];
}

static void mix_sub_columns( uint_8t dt[N_BLOCK], uint_8t st[N_BLOCK] )
{
	dt[ 0] = gfm2_sb(st[0]) ^ gfm3_sb(st[5]) ^ s_box(st[10]) ^ s_box(st[15]);
	dt[ 1] = s_box(st[0]) ^ gfm2_sb(st[5]) ^ gfm3_sb(st[10]) ^ s_box(st[15]);
	dt[ 2] = s_box(st[0]) ^ s_box(st[5]) ^ gfm2_sb(st[10]) ^ gfm3_sb(st[15]);
	dt[ 3] = gfm3_sb(st[0]) ^ s_box(st[5]) ^ s_box(st[10]) ^ gfm2_sb(st[15]);

	dt[ 4] = gfm2_sb(st[4]) ^ gfm3_sb(st[9]) ^ s_box(st[14]) ^ s_box(st[3]);
	dt[ 5] = s_box(st[4]) ^ gfm2_sb(st[9]) ^ gfm3_sb(st[14]) ^ s_box(st[3]);
	dt[ 6] = s_box(st[4]) ^ s_box(st[9]) ^ gfm2_sb(st[14]) ^ gfm3_sb(st[3]);
	dt[ 7] = gfm3_sb(st[4]) ^ s_box(st[9]) ^ s_box(st[14]) ^ gfm2_sb(st[3]);

	dt[ 8] = gfm2_sb(st[8]) ^ gfm3_sb(st[13]) ^ s_box(st[2]) ^ s_box(st[7]);
	dt[ 9] = s_box(st[8]) ^ gfm2_sb(st[13]) ^ gfm3_sb(st[2]) ^ s_box(st[7]);
	dt[10] = s_box(st[8]) ^ s_box(st[13]) ^ gfm2_sb(st[2]) ^ gfm3_sb(st[7]);
	dt[11] = gfm3_sb(st[8]) ^ s_box(st[13]) ^ s_box(st[2]) ^ gfm2_sb(st[7]);

	dt[12] = gfm2_sb(st[12]) ^ gfm3_sb(st[1]) ^ s_box(st[6]) ^ s_box(st[11]);
	dt[13] = s_box(st[12]) ^ gfm2_sb(st[1]) ^ gfm3_sb(st[6]) ^ s_box(st[11]);
	dt[14] = s_box(st[12]) ^ s_box(st[1]) ^ gfm2_sb(st[6]) ^ gfm3_sb(st[11]);
	dt[15] = gfm3_sb(st[12]) ^ s_box(st[1]) ^ s_box(st[6]) ^ gfm2_sb(st[11]);
}


static void shift_sub_rows( uint_8t st[N_BLOCK] )
{
	uint_8t tt;

	st[ 0] = s_box(st[ 0]); st[ 4] = s_box(st[ 4]);
	st[ 8] = s_box(st[ 8]); st[12] = s_box(st[12]);

	tt = st[1]; st[ 1] = s_box(st[ 5]); st[ 5] = s_box(st[ 9]);
	st[ 9] = s_box(st[13]); st[13] = s_box( tt );

	tt = st[2]; st[ 2] = s_box(st[10]); st[10] = s_box( tt );
	tt = st[6]; st[ 6] = s_box(st[14]); st[14] = s_box( tt );

	tt = st[15]; st[15] = s_box(st[11]); st[11] = s_box(st[ 7]);
	st[ 7] = s_box(st[ 3]); st[ 3] = s_box( tt );
}

typedef uint_8t return_type;
typedef uint_8t length_type;

#  define block_copy_nn(d, s, l)    memcpy(d, s, l)
#  define block_copy(d, s)          memcpy(d, s, N_BLOCK)

#define N_ROW                   4
#define N_COL                   4
#define N_BLOCK   (N_ROW * N_COL)
#define N_MAX_ROUNDS           14
typedef struct {
	uint_8t ksch[(N_MAX_ROUNDS + 1) * N_BLOCK];
	uint_8t rnd;
} aes_context;

/* AES Encryption output */
typedef struct {
	UINT8   status;
	UINT8   param_len;
	UINT16  opcode;
	UINT8   param_buf[BT_OCTET16_LEN];
} tSMP_ENC;


#define ARRAY_TO_STREAM(p, a, len) {int ijk; for (ijk = 0; ijk < len;        ijk++) *(p)++ = (uint8_t) a[ijk];}
#define REVERSE_ARRAY_TO_STREAM(p, a, len)  {register int ijk; for (ijk = 0; ijk < len; ijk++) *(p)++ = (UINT8) a[len - 1 - ijk];}


return_type aes_set_key( const unsigned char key[], length_type keylen, aes_context ctx[1] )
{
	uint_8t cc, rc, hi;

	switch ( keylen ) {
	case 16:
	case 128:           /* length in bits (128 = 8*16) */
		keylen = 16;
		break;
	case 24:
	case 192:           /* length in bits (192 = 8*24) */
		keylen = 24;
		break;
	case 32:
		/*    case 256:           length in bits (256 = 8*32) */
		keylen = 32;
		break;
	default:
		ctx->rnd = 0;
		return (return_type) - 1;
	}
	block_copy_nn(ctx->ksch, key, keylen);
	hi = (keylen + 28) << 2;
	ctx->rnd = (hi >> 4) - 1;
	for ( cc = keylen, rc = 1; cc < hi; cc += 4 ) {
		uint_8t tt, t0, t1, t2, t3;

		t0 = ctx->ksch[cc - 4];
		t1 = ctx->ksch[cc - 3];
		t2 = ctx->ksch[cc - 2];
		t3 = ctx->ksch[cc - 1];
		if ( cc % keylen == 0 ) {
			tt = t0;
			t0 = s_box(t1) ^ rc;
			t1 = s_box(t2);
			t2 = s_box(t3);
			t3 = s_box(tt);
			rc = f2(rc);
		} else if ( keylen > 24 && cc % keylen == 16 ) {
			t0 = s_box(t0);
			t1 = s_box(t1);
			t2 = s_box(t2);
			t3 = s_box(t3);
		}
		tt = cc - keylen;
		ctx->ksch[cc + 0] = ctx->ksch[tt + 0] ^ t0;
		ctx->ksch[cc + 1] = ctx->ksch[tt + 1] ^ t1;
		ctx->ksch[cc + 2] = ctx->ksch[tt + 2] ^ t2;
		ctx->ksch[cc + 3] = ctx->ksch[tt + 3] ^ t3;
	}
	return 0;
}

/* @breif change the name by snake for avoid the conflict with libcrypto */
return_type bluedroid_aes_encrypt( const unsigned char in[N_BLOCK], unsigned char  out[N_BLOCK], const aes_context ctx[1] )
{
	if ( ctx->rnd ) {
		uint_8t s1[N_BLOCK], r;
		copy_and_key( s1, in, ctx->ksch );
		for ( r = 1 ; r < ctx->rnd ; ++r )

		{
			uint_8t s2[N_BLOCK];
			mix_sub_columns( s2, s1 );
			copy_and_key( s1, s2, ctx->ksch + r * N_BLOCK);
		}
		shift_sub_rows( s1 );
		copy_and_key( out, s1, ctx->ksch + r * N_BLOCK );
	} else {
		return (return_type) - 1;
	}
	return 0;
}
//*******************************************************************************
//**
//** Function         smp_encrypt_data
//**
//** Description      This function is called to encrypt data.
//**                  It uses AES-128 encryption algorithm.
//**                  Plain_text is encrypted using key, the result is at p_out.
//**
//** Returns          void
//**
//*******************************************************************************/
BOOLEAN smp_encrypt_data (UINT8 *key, UINT8 key_len,
						  UINT8 *plain_text, UINT8 pt_len,
						  tSMP_ENC *p_out)
{
	aes_context ctx;
	UINT8 *p_start = NULL;
	UINT8 *p = NULL;
	UINT8 *p_rev_data = NULL;    /* input data in big endilan format */
	UINT8 *p_rev_key = NULL;     /* input key in big endilan format */
	UINT8 *p_rev_output = NULL;  /* encrypted output in big endilan format */

	//printf("%s\n", __func__);
	if ( (p_out == NULL ) || (key_len != SMP_ENCRYT_KEY_SIZE) ) {
		printf("%s failed\n", __func__);
		return FALSE;
	}

	if ((p_start = (UINT8 *)osi_malloc((SMP_ENCRYT_DATA_SIZE * 4))) == NULL) {
		printf("%s failed unable to allocate buffer\n", __func__);
		return FALSE;
	}

	if (pt_len > SMP_ENCRYT_DATA_SIZE) {
		pt_len = SMP_ENCRYT_DATA_SIZE;
	}

	memset(p_start, 0, SMP_ENCRYT_DATA_SIZE * 4);
	p = p_start;
	ARRAY_TO_STREAM (p, plain_text, pt_len); /* byte 0 to byte 15 */
	p_rev_data = p = p_start + SMP_ENCRYT_DATA_SIZE; /* start at byte 16 */
	REVERSE_ARRAY_TO_STREAM (p, p_start, SMP_ENCRYT_DATA_SIZE);  /* byte 16 to byte 31 */
	p_rev_key = p; /* start at byte 32 */
	REVERSE_ARRAY_TO_STREAM (p, key, SMP_ENCRYT_KEY_SIZE); /* byte 32 to byte 47 */

//#if SMP_DEBUG == TRUE && SMP_DEBUG_VERBOSE == TRUE
//	smp_debug_print_nbyte_little_endian(key, (const UINT8 *)"Key", SMP_ENCRYT_KEY_SIZE);
//	smp_debug_print_nbyte_little_endian(p_start, (const UINT8 *)"Plain text", SMP_ENCRYT_DATA_SIZE);
//#endif
	p_rev_output = p;
	aes_set_key(p_rev_key, SMP_ENCRYT_KEY_SIZE, &ctx);
	bluedroid_aes_encrypt(p_rev_data, p, &ctx);  /* outputs in byte 48 to byte 63 */

	p = p_out->param_buf;
	REVERSE_ARRAY_TO_STREAM (p, p_rev_output, SMP_ENCRYT_DATA_SIZE);
//#if SMP_DEBUG == TRUE && SMP_DEBUG_VERBOSE == TRUE
//	smp_debug_print_nbyte_little_endian(p_out->param_buf, (const UINT8 *)"Encrypted text", SMP_ENCRYT_KEY_SIZE);
//#endif

	p_out->param_len = SMP_ENCRYT_KEY_SIZE;
	p_out->status = HCI_SUCCESS;
	p_out->opcode =  HCI_BLE_ENCRYPT;

	osi_free(p_start);

	return TRUE;
}

BOOLEAN SMP_Encrypt (UINT8 *key, UINT8 key_len,
					 UINT8 *plain_text, UINT8 pt_len,
					 tSMP_ENC *p_out)

{
	BOOLEAN status = FALSE;
	status = smp_encrypt_data(key, key_len, plain_text, pt_len, p_out);
	return status;
}

/*******************************************************************************
**
** Function         btm_ble_addr_resolvable
**
** Description      This function checks if a RPA is resolvable by the device key.
**
** Returns          TRUE is resolvable; FALSE otherwise.
**
*******************************************************************************/
#define BD_ADDR_LEN 6
typedef UINT8 BD_ADDR[BD_ADDR_LEN];         /* Device address */

/*Define the bt octet 16 bit size*/
#define ESP_BT_OCTET16_LEN    16
typedef uint8_t esp_bt_octet16_t[ESP_BT_OCTET16_LEN];   /* octet array: size 16 */

BOOLEAN btm_ble_addr_resolvable(BD_ADDR rpa, esp_bt_octet16_t irk)
{
	BOOLEAN rt = FALSE;

	//if (!BTM_BLE_IS_RESOLVE_BDA(rpa)) {
	//	return rt;
	//}

	UINT8 rand[3];
	tSMP_ENC output;
	//if ((p_dev_rec->device_type & BT_DEVICE_TYPE_BLE) &&
	//	(p_dev_rec->ble.key_type & BTM_LE_KEY_PID)) {



			//printf("%s try to resolve\r\n", __func__);
			/* use the 3 MSB of bd address as prand */
			rand[0] = rpa[2];
			rand[1] = rpa[1];
			rand[2] = rpa[0];

			/* generate X = E irk(R0, R1, R2) and R is random address 3 LSO */
			SMP_Encrypt(irk, BT_OCTET16_LEN,
				&rand[0], 3, &output);

			rand[0] = rpa[5];
			rand[1] = rpa[4];
			rand[2] = rpa[3];

			if (!memcmp(output.param_buf, &rand[0], 3)) {
				//btm_ble_init_pseudo_addr (p_dev_rec, rpa);
				rt = TRUE;
			}
	//}

	return rt;
}