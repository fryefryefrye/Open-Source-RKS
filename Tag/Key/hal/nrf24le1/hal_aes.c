/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic
 * Semiconductor ASA.Terms and conditions of usage are described in detail
 * in NORDIC SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRENTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * $LastChangedRevision: 2368 $
 */

/** @file
* @brief Implementation of hal_aes
 *
 * @details This file contains functions for using the AES crytographic in software
 * utilizing the hardware support provided in nRF24LE1. This version only
 * supports ECB encryption mode of operation.
 */

//lint -e438

#include "hal_aes.h"
#include "nrf24le1.h"

/*----------------------- AES Parameters -------------------------------------*/
static uint8_t xdata cipher_key[16];    //AES key

//These should be placed in fast RAM
static uint8_t pdata aes_round_key[16]; //AES Round Key

/*----------------------- AES Encryption Functions ---------------------------*/
static void mix_columns_hw(uint8_t data * aes_state);
static void add_sub_shift(uint8_t data * aes_state);
static void add_key(uint8_t data * aes_state);
static void aes_set_key(const uint8_t * key_bytes);
static void key_upgrade(uint8_t round);

/*----------------------- AES STATIC values ----------------------------------*/
static uint8_t const code s_box[256] = {
 99, 124, 119, 123, 242, 107, 111, 197,  48,   1, 103,  43, 254, 215, 171, 118,
202, 130, 201, 125, 250,  89,  71, 240, 173, 212, 162, 175, 156, 164, 114, 192,
183, 253, 147,  38,  54,  63, 247, 204,  52, 165, 229, 241, 113, 216,  49,  21,
  4, 199,  35, 195,  24, 150,   5, 154,   7,  18, 128, 226, 235,  39, 178, 117,
  9, 131,  44,  26,  27, 110,  90, 160,  82,  59, 214, 179,  41, 227,  47, 132,
 83, 209,   0, 237,  32, 252, 177,  91, 106, 203, 190,  57,  74,  76,  88, 207,
208, 239, 170, 251,  67,  77,  51, 133,  69, 249,   2, 127,  80,  60, 159, 168,
 81, 163,  64, 143, 146, 157,  56, 245, 188, 182, 218,  33,  16, 255, 243, 210,
205,  12,  19, 236,  95, 151,  68,  23, 196, 167, 126,  61, 100,  93,  25, 115,
 96, 129,  79, 220,  34,  42, 144, 136,  70, 238, 184,  20, 222,  94,  11, 219,
224,  50,  58,  10,  73,   6,  36,  92, 194, 211, 172,  98, 145, 149, 228, 121,
231, 200,  55, 109, 141, 213,  78, 169, 108,  86, 244, 234, 101, 122, 174,   8,
186, 120,  37,  46,  28, 166, 180, 198, 232, 221, 116,  31,  75, 189, 139, 138,
112,  62, 181, 102,  72,   3, 246,  14,  97,  53,  87, 185, 134, 193,  29, 158,
225, 248, 152,  17, 105, 217, 142, 148, 155,  30, 135, 233, 206,  85,  40, 223,
140, 161, 137,  13, 191, 230,  66, 104,  65, 153,  45,  15, 176,  84, 187,  22,
};

static uint8_t const code rcon[]={
0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

/*----------------------- AES API --------------------------------------------*/
void hal_aes_setup(bool decrypt, aes_modes_t mode, const uint8_t *keyin, const uint8_t *ivin)
{
	aes_set_key(keyin);

  //lint -save -e438
	// Not used in LE1, included to prevent compile-warnings------ |
	decrypt = decrypt; 	                                        // |
	mode = mode;                                                // |
	ivin = ivin;                                                // |
	//-------------------------------------------------------------|
  //lint -restore
}

#ifdef __ICC8051__
 static uint8_t data aes_state[16];      //AES State
#endif  

void hal_aes_crypt(uint8_t * dest_buf, const uint8_t * src_buf)
{
#ifdef __C51__
  uint8_t data aes_state[16];      //AES State
#endif

  uint8_t a;
	for(a=0;a<16;a++)
	{
		aes_state[a] = src_buf[a];
	}

  //Start
  CCPDATIB = 0x02;  //Set co-prosessor the GF(2^8)*2 (used in mix-colums)
  for (a=0;a<9;a++)
  {
    add_sub_shift(aes_state);
    mix_columns_hw(aes_state);
    key_upgrade(a);
  }

  //FINAL	round
  add_sub_shift(aes_state);
  key_upgrade(9);
  add_key(aes_state);

  //Clean up
	for(a=0;a<16;a++)
	{
		aes_round_key[a]=cipher_key[a]; //Write back cipher-key
		dest_buf[a] = aes_state[a];     //Write out encrypted result
	}
}
/*----------------------- Private AES Encryption Functions -------------------*/
static void aes_set_key(const uint8_t * key_bytes)
{
  uint8_t k;
  for (k=0;k<16;k++)
  {
    cipher_key[k]=aes_round_key[k]=key_bytes[k];
  }
}

static void add_key(uint8_t data * aes_state)
{
  aes_state[0]^=aes_round_key[0];
  aes_state[1]^=aes_round_key[1];
  aes_state[2]^=aes_round_key[2];
  aes_state[3]^=aes_round_key[3];
  aes_state[4]^=aes_round_key[4];
  aes_state[5]^=aes_round_key[5];
  aes_state[6]^=aes_round_key[6];
  aes_state[7]^=aes_round_key[7];
  aes_state[8]^=aes_round_key[8];
  aes_state[9]^=aes_round_key[9];
  aes_state[10]^=aes_round_key[10];
  aes_state[11]^=aes_round_key[11];
  aes_state[12]^=aes_round_key[12];
  aes_state[13]^=aes_round_key[13];
  aes_state[14]^=aes_round_key[14];
  aes_state[15]^=aes_round_key[15];
}

static void key_upgrade(uint8_t round)
{
  aes_round_key[0]=s_box[aes_round_key[13]]^aes_round_key[0]^rcon[round+1];
  aes_round_key[1]=s_box[aes_round_key[14]]^aes_round_key[1];
  aes_round_key[2]=s_box[aes_round_key[15]]^aes_round_key[2];
  aes_round_key[3]=s_box[aes_round_key[12]]^aes_round_key[3];

  aes_round_key[4]=aes_round_key[0]^aes_round_key[4];
  aes_round_key[5]=aes_round_key[1]^aes_round_key[5];
  aes_round_key[6]=aes_round_key[2]^aes_round_key[6];
  aes_round_key[7]=aes_round_key[3]^aes_round_key[7];

  aes_round_key[8]=aes_round_key[4]^aes_round_key[8];
  aes_round_key[9]=aes_round_key[5]^aes_round_key[9];
  aes_round_key[10]=aes_round_key[6]^aes_round_key[10];
  aes_round_key[11]=aes_round_key[7]^aes_round_key[11];

  aes_round_key[12]=aes_round_key[8]^aes_round_key[12];
  aes_round_key[13]=aes_round_key[9]^aes_round_key[13];
  aes_round_key[14]=aes_round_key[10]^aes_round_key[14];
  aes_round_key[15]=aes_round_key[11]^aes_round_key[15];
}

static void add_sub_shift(uint8_t data * aes_state)
{
  uint8_t row[2];

  aes_state[0]=s_box[aes_state[0]^aes_round_key[0]];
  aes_state[4]=s_box[aes_state[4]^aes_round_key[4]];
  aes_state[8]=s_box[aes_state[8]^aes_round_key[8]];
  aes_state[12]=s_box[aes_state[12]^aes_round_key[12]];

  row[0]=s_box[aes_state[1]^aes_round_key[1]];
  aes_state[1]=s_box[aes_state[5]^aes_round_key[5]];
  aes_state[5]=s_box[aes_state[9]^aes_round_key[9]];
  aes_state[9]=s_box[aes_state[13]^aes_round_key[13]];
  aes_state[13]=row[0];

  row[0]=s_box[aes_state[2]^aes_round_key[2]];
  row[1]=s_box[aes_state[6]^aes_round_key[6]];
  aes_state[2]=s_box[aes_state[10]^aes_round_key[10]];
  aes_state[6]=s_box[aes_state[14]^aes_round_key[14]];
  aes_state[10]=row[0];
  aes_state[14]=row[1];

  row[0]=s_box[aes_state[15]^aes_round_key[15]];
  aes_state[15]=s_box[aes_state[11]^aes_round_key[11]];
  aes_state[11]=s_box[aes_state[7]^aes_round_key[7]];
  aes_state[7]=s_box[aes_state[3]^aes_round_key[3]];
  aes_state[3]=row[0];
}

static void mix_columns_hw(uint8_t data * aes_state)
{
  uint8_t col,r,tmp;
  /*
  This function operates on the columns of the state. Each column is subject to the
  following transform (in vector and matrix notation):
  (b0)   (2 3 1 1)   (a0)
  (b1) = (1 2 3 1) * (a1)
  (b2)   (1 1 2 3)   (a2)
  (b3)   (3 1 1 2)   (a3)
  Here the vector a is a column of the state before the mix columns operation, and
  b is the same column after the operation. We use hardware to perform the
  multiplication in GF(2^8).
  */

  tmp=aes_state[3+0]^aes_state[2+0]^aes_state[1+0]^aes_state[0];
  //tmp = a0 + a1 + a2 + a3 (in modular aritmetic)
  col=aes_state[0];

  CCPDATIA = aes_state[0]^aes_state[1+0];
  r=CCPDATO;
  //r = 2*(a0 + a1)
  aes_state[0]= r^tmp^(aes_state[0]);
  //b0 = 3a0 + 3a1 + a2 +a3 - a0 = 2a0 +3a1 + a2 + a3

  CCPDATIA = aes_state[1+0]^aes_state[2+0];
  r=CCPDATO;
  //r = 2*(a1 + a2)
  aes_state[1+0]= r^tmp^(aes_state[1+0]);
  //b1 = a0 + 3a1 + 3a2 +a3 - a1 = a0 +2a1 + 3a2 + a3

  CCPDATIA = aes_state[2+0]^aes_state[3+0];
  r=CCPDATO;
  //r = 2*(a2 + a3)
  aes_state[2+0]= r^tmp^(aes_state[2+0]);
  //b2 = a0 + a1 + 3a2 +3a3 - a2 = a0 +a1 + 2a2 + 3a3

  CCPDATIA = aes_state[3+0]^col;
  r=CCPDATO;
  //r = 2*(a3 + a0)
  aes_state[3+0]= r^tmp^(aes_state[3+0]);
  //b3 = 3a0 + a1 + a2 +3a3 - a3 = 3a0 +a1 + a2 + 2a3


  tmp=aes_state[3+4]^aes_state[2+4]^aes_state[1+4]^aes_state[4];
  col=aes_state[4];

  CCPDATIA = aes_state[4]^aes_state[1+4];
  r=CCPDATO;
  aes_state[4]= r^tmp^(aes_state[4]);

  CCPDATIA = aes_state[1+4]^aes_state[2+4];
  r=CCPDATO;
  aes_state[1+4]= r^tmp^(aes_state[1+4]);

  CCPDATIA = aes_state[2+4]^aes_state[3+4];
  r=CCPDATO;
  aes_state[2+4]= r^tmp^(aes_state[2+4]);

  CCPDATIA = aes_state[3+4]^col;
  r=CCPDATO;
  aes_state[3+4]= r^tmp^(aes_state[3+4]);


  tmp=aes_state[3+8]^aes_state[2+8]^aes_state[1+8]^aes_state[8];
  col=aes_state[8];

  CCPDATIA = aes_state[8]^aes_state[1+8];
  r=CCPDATO;
  aes_state[8]= r^tmp^(aes_state[8]);

  CCPDATIA = aes_state[1+8]^aes_state[2+8];
  r=CCPDATO;
  aes_state[1+8]= r^tmp^(aes_state[1+8]);

  CCPDATIA = aes_state[2+8]^aes_state[3+8];
  r=CCPDATO;
  aes_state[2+8]= r^tmp^(aes_state[2+8]);

  CCPDATIA = aes_state[3+8]^col;
  r=CCPDATO;
  aes_state[3+8]= r^tmp^(aes_state[3+8]);


  tmp=aes_state[3+12]^aes_state[2+12]^aes_state[1+12]^aes_state[12];
  col=aes_state[12];

  CCPDATIA =aes_state[12]^aes_state[1+12];
  r=CCPDATO;
  aes_state[12]= r^tmp^(aes_state[12]);

  CCPDATIA = aes_state[1+12]^aes_state[2+12];
  r=CCPDATO;
  aes_state[1+12]= r^tmp^(aes_state[1+12]);

  CCPDATIA =aes_state[2+12]^aes_state[3+12];
  r=CCPDATO;
  aes_state[2+12]= r^tmp^(aes_state[2+12]);

  CCPDATIA = aes_state[3+12]^col;
  r=CCPDATO;
  aes_state[3+12]= r^tmp^(aes_state[3+12]);
}

