#ifndef __DESDUMMY_H_
#define __DESDUMMY_H_

#define DES_ENCRYPT	1
#define DES_DECRYPT	0
#define DES_LONG unsigned int
typedef unsigned char DES_cblock[8];
typedef /* const */ unsigned char const_DES_cblock[8];
typedef struct DES_ks
    {
    union
	{
	DES_cblock cblock;
	/* make sure things are correct size on machines with
	 * 8 byte longs */
	DES_LONG deslong[2];
	} ks[16];
    } DES_key_schedule;

void DES_ecb_encrypt(const_DES_cblock *input,DES_cblock *output, DES_key_schedule *ks,int enc);
int DES_set_key(const_DES_cblock *key,DES_key_schedule *schedule);

#endif