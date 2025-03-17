/* This software is Copyright (c) 2012 Lukas Odzioba <ukasz@openwall.net>
 * and it is hereby released to the general public under the following terms:
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * Based on hmac-sha512 by magnum
 *
 * Minor fixes, format unification and OMP support done by Dhiru Kholia
 * <dhiru@openwall.com>
 *
 * Fixed for supporting $ml$ "dave" format as well as GRUB native format by
 * magnum 2013. Note: We support a binary size of >512 bits (64 bytes / 128
 * chars of hex) but we currently do not calculate it even in cmp_exact(). The
 * chance for a 512-bit hash collision should be pretty dang slim.
 */

#if FMT_EXTERNS_H
extern struct fmt_main fmt_pbkdf2_hmac_sha512;
#elif FMT_REGISTERS_H
john_register_one(&fmt_pbkdf2_hmac_sha512);
#else

#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "misc.h"
#include "arch.h"
#include "common.h"
#include "formats.h"
#include "sha2.h"
#include "johnswap.h"
#include "stdint.h"
#include "pbkdf2_hmac_common.h"
#include "pbkdf2_hmac_sha512.h"

#define FORMAT_LABEL            "PBKDF2-HMAC-SHA512"
#undef FORMAT_NAME
#define FORMAT_NAME             "GRUB2 / OS X 10.8+"

#ifdef SIMD_COEF_64
#define ALGORITHM_NAME		"PBKDF2-SHA512 " SHA512_ALGORITHM_NAME
#else
#if ARCH_BITS >= 64
#define ALGORITHM_NAME          "PBKDF2-SHA512 64/" ARCH_BITS_STR " " SHA2_LIB
#else
#define ALGORITHM_NAME          "PBKDF2-SHA512 32/" ARCH_BITS_STR " " SHA2_LIB
#endif
#endif

#define SALT_SIZE               sizeof(struct custom_salt)
#ifdef SIMD_COEF_64
#define MIN_KEYS_PER_CRYPT	SSE_GROUP_SZ_SHA512
#define MAX_KEYS_PER_CRYPT	SSE_GROUP_SZ_SHA512
#else
#define MIN_KEYS_PER_CRYPT      1
#define MAX_KEYS_PER_CRYPT      1
#endif
#ifdef _OPENMP
static int omp_t = 1;
#include <omp.h>
#ifndef OMP_SCALE
#define OMP_SCALE               1
#endif
#endif

#include "memdbg.h"

#define PAD_SIZE                128
#define PLAINTEXT_LENGTH        125

static struct custom_salt {
	uint8_t length;
	uint8_t salt[PBKDF2_64_MAX_SALT_SIZE];
	uint32_t rounds;
} *cur_salt;

static char (*saved_key)[PLAINTEXT_LENGTH + 1];
static ARCH_WORD_32 (*crypt_out)[PBKDF2_SHA512_BINARY_SIZE / sizeof(ARCH_WORD_32)];

static void init(struct fmt_main *self)
{
#ifdef _OPENMP
	omp_t = omp_get_max_threads();
	self->params.min_keys_per_crypt *= omp_t;
	omp_t *= OMP_SCALE;
	self->params.max_keys_per_crypt *= omp_t;
#endif
	saved_key = mem_calloc(sizeof(*saved_key), self->params.max_keys_per_crypt);
	crypt_out = mem_calloc(sizeof(*crypt_out), self->params.max_keys_per_crypt);
}

static void done(void)
{
	MEM_FREE(crypt_out);
	MEM_FREE(saved_key);
}

static void *get_salt(char *ciphertext)
{
	static struct custom_salt cs;
	char *p;
	int saltlen;
	char delim;

	memset(&cs, 0, sizeof(cs));
	ciphertext += PBKDF2_SHA512_TAG_LEN;
	cs.rounds = atou(ciphertext);
	delim = strchr(ciphertext, '.') ? '.' : '$';
	ciphertext = strchr(ciphertext, delim) + 1;
	p = strchr(ciphertext, delim);
	saltlen = 0;
	while (ciphertext < p) {        /** extract salt **/
		cs.salt[saltlen++] =
			atoi16[ARCH_INDEX(ciphertext[0])] * 16 +
			atoi16[ARCH_INDEX(ciphertext[1])];
		ciphertext += 2;
	}
	cs.length = saltlen;

	return (void *)&cs;
}

static void set_salt(void *salt)
{
	cur_salt = (struct custom_salt *)salt;
}

static int get_hash_0(int index) { return crypt_out[index][0] & PH_MASK_0; }
static int get_hash_1(int index) { return crypt_out[index][0] & PH_MASK_1; }
static int get_hash_2(int index) { return crypt_out[index][0] & PH_MASK_2; }
static int get_hash_3(int index) { return crypt_out[index][0] & PH_MASK_3; }
static int get_hash_4(int index) { return crypt_out[index][0] & PH_MASK_4; }
static int get_hash_5(int index) { return crypt_out[index][0] & PH_MASK_5; }
static int get_hash_6(int index) { return crypt_out[index][0] & PH_MASK_6; }

static int crypt_all(int *pcount, struct db_salt *salt)
{
	const int count = *pcount;
	int index = 0;

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (index = 0; index < count; index += MAX_KEYS_PER_CRYPT)
	{
#ifdef SSE_GROUP_SZ_SHA512
		int lens[SSE_GROUP_SZ_SHA512], i;
		unsigned char *pin[SSE_GROUP_SZ_SHA512];
		union {
			ARCH_WORD_32 *pout[SSE_GROUP_SZ_SHA512];
			unsigned char *poutc;
		} x;
		for (i = 0; i < SSE_GROUP_SZ_SHA512; ++i) {
			lens[i] = strlen(saved_key[index+i]);
			pin[i] = (unsigned char*)saved_key[index+i];
			x.pout[i] = crypt_out[index+i];
		}
		pbkdf2_sha512_sse((const unsigned char **)pin, lens, cur_salt->salt, cur_salt->length, cur_salt->rounds, &(x.poutc), PBKDF2_SHA512_BINARY_SIZE, 0);
#else
		pbkdf2_sha512((const unsigned char*)(saved_key[index]), strlen(saved_key[index]),
			cur_salt->salt, cur_salt->length,
			cur_salt->rounds, (unsigned char*)crypt_out[index], PBKDF2_SHA512_BINARY_SIZE, 0);
#endif
	}
	return count;
}

static int cmp_all(void *binary, int count)
{
	int index = 0;
	for (; index < count; index++)
		if (!memcmp(binary, crypt_out[index], ARCH_SIZE))
			return 1;
	return 0;
}

static int cmp_one(void *binary, int index)
{
	return !memcmp(binary, crypt_out[index], PBKDF2_SHA512_BINARY_SIZE);
}

static void set_key(char *key, int index)
{
	int saved_len = strlen(key);
	if (saved_len > PLAINTEXT_LENGTH)
		saved_len = PLAINTEXT_LENGTH;
	memcpy(saved_key[index], key, saved_len);
	saved_key[index][saved_len] = 0;
}

static char *get_key(int index)
{
	return saved_key[index];
}

static int cmp_exact(char *source, int index)
{
	return pbkdf2_hmac_sha512_cmp_exact(get_key(index), source, cur_salt->salt, cur_salt->length, cur_salt->rounds);
}

static unsigned int iteration_count(void *salt)
{
	struct custom_salt *my_salt;

	my_salt = salt;
	return (unsigned int) my_salt->rounds;
}

struct fmt_main fmt_pbkdf2_hmac_sha512 = {
	{
		FORMAT_LABEL,
		FORMAT_NAME,
		ALGORITHM_NAME,
		BENCHMARK_COMMENT,
		BENCHMARK_LENGTH,
		0,
		PLAINTEXT_LENGTH,
		PBKDF2_SHA512_BINARY_SIZE,
		sizeof(ARCH_WORD_32),
		SALT_SIZE,
		sizeof(ARCH_WORD),
		MIN_KEYS_PER_CRYPT,
		MAX_KEYS_PER_CRYPT,
		FMT_CASE | FMT_8_BIT | FMT_OMP | FMT_SPLIT_UNIFIES_CASE,
		{
			"iteration count",
		},
		{
			PBKDF2_SHA512_FORMAT_TAG,
			FORMAT_TAG_ML,
			FORMAT_TAG_GRUB
		},
		pbkdf2_hmac_sha512_common_tests
	}, {
		init,
		done,
		fmt_default_reset,
		pbkdf2_hmac_sha512_prepare,
		pbkdf2_hmac_sha512_valid,
		pbkdf2_hmac_sha512_split,
		pbkdf2_hmac_sha512_binary,
		get_salt,
		{
			iteration_count,
		},
		fmt_default_source,
		{
			fmt_default_binary_hash_0,
			fmt_default_binary_hash_1,
			fmt_default_binary_hash_2,
			fmt_default_binary_hash_3,
			fmt_default_binary_hash_4,
			fmt_default_binary_hash_5,
			fmt_default_binary_hash_6
		},
		fmt_default_salt_hash,
		NULL,
		set_salt,
		set_key,
		get_key,
		fmt_default_clear_keys,
		crypt_all,
		{
			get_hash_0,
			get_hash_1,
			get_hash_2,
			get_hash_3,
			get_hash_4,
			get_hash_5,
			get_hash_6
		},
		cmp_all,
		cmp_one,
		cmp_exact
	}
};

#endif /* plugin stanza */
