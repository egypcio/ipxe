/*
 * Copyright (C) 2012 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * You can also choose to distribute this program under the terms of
 * the Unmodified Binary Distribution Licence (as given in the file
 * COPYING.UBDL), provided that you have satisfied its requirements.
 */

FILE_LICENCE ( GPL2_OR_LATER_OR_UBDL );

/** @file
 *
 * RSA self-tests
 *
 * These test vectors are generated using openssl's genrsa, rsa,
 * rsautl, and dgst tools.
 */

/* Forcibly enable assertions */
#undef NDEBUG

#include <string.h>
#include <ipxe/crypto.h>
#include <ipxe/rsa.h>
#include <ipxe/md5.h>
#include <ipxe/sha1.h>
#include <ipxe/sha256.h>
#include <ipxe/test.h>
#include "pubkey_test.h"

/** Define inline private key data */
#define PRIVATE(...) { __VA_ARGS__ }

/** Define inline public key data */
#define PUBLIC(...) { __VA_ARGS__ }

/** Define inline plaintext data */
#define PLAINTEXT(...) { __VA_ARGS__ }

/** Define inline ciphertext data */
#define CIPHERTEXT(...) { __VA_ARGS__ }

/** Define inline signature data */
#define SIGNATURE(...) { __VA_ARGS__ }

/** An RSA encryption and decryption self-test */
struct rsa_encrypt_decrypt_test {
	/** Private key */
	const void *private;
	/** Private key length */
	size_t private_len;
	/** Public key */
	const void *public;
	/** Public key length */
	size_t public_len;
	/** Plaintext */
	const void *plaintext;
	/** Plaintext length */
	size_t plaintext_len;
	/** Ciphertext
	 *
	 * Note that the encryption process includes some random
	 * padding, so a given plaintext will encrypt to multiple
	 * different ciphertexts.
	 */
	const void *ciphertext;
	/** Ciphertext length */
	size_t ciphertext_len;
};

/**
 * Define an RSA encryption and decryption test
 *
 * @v name		Test name
 * @v PRIVATE		Private key
 * @v PUBLIC		Public key
 * @v PLAINTEXT		Plaintext
 * @v CIPHERTEXT	Ciphertext
 * @ret test		Encryption and decryption test
 */
#define RSA_ENCRYPT_DECRYPT_TEST( name, PRIVATE, PUBLIC, PLAINTEXT,	\
				  CIPHERTEXT )				\
	static const uint8_t name ## _private[] = PRIVATE;		\
	static const uint8_t name ## _public[] = PUBLIC;		\
	static const uint8_t name ## _plaintext[] = PLAINTEXT;		\
	static const uint8_t name ## _ciphertext[] = CIPHERTEXT;	\
	static struct rsa_encrypt_decrypt_test name = {			\
		.private = name ## _private,				\
		.private_len = sizeof ( name ## _private ),		\
		.public = name ## _public,				\
		.public_len = sizeof ( name ## _public ),		\
		.plaintext = name ## _plaintext,			\
		.plaintext_len = sizeof ( name ## _plaintext ),		\
		.ciphertext = name ## _ciphertext,			\
		.ciphertext_len = sizeof ( name ## _ciphertext ),	\
	}

/** An RSA signature self-test */
struct rsa_signature_test {
	/** Private key */
	const void *private;
	/** Private key length */
	size_t private_len;
	/** Public key */
	const void *public;
	/** Public key length */
	size_t public_len;
	/** Plaintext */
	const void *plaintext;
	/** Plaintext length */
	size_t plaintext_len;
	/** Signature algorithm */
	struct asn1_algorithm *algorithm;
	/** Signature */
	const void *signature;
	/** Signature length */
	size_t signature_len;
};

/**
 * Define an RSA signature test
 *
 * @v name		Test name
 * @v PRIVATE		Private key
 * @v PUBLIC		Public key
 * @v PLAINTEXT		Plaintext
 * @v ALGORITHM		Signature algorithm
 * @v SIGNATURE		Signature
 * @ret test		Signature test
 */
#define RSA_SIGNATURE_TEST( name, PRIVATE, PUBLIC, PLAINTEXT,		\
			    ALGORITHM, SIGNATURE )			\
	static const uint8_t name ## _private[] = PRIVATE;		\
	static const uint8_t name ## _public[] = PUBLIC;		\
	static const uint8_t name ## _plaintext[] = PLAINTEXT;		\
	static const uint8_t name ## _signature[] = SIGNATURE;		\
	static struct rsa_signature_test name = {			\
		.private = name ## _private,				\
		.private_len = sizeof ( name ## _private ),		\
		.public = name ## _public,				\
		.public_len = sizeof ( name ## _public ),		\
		.plaintext = name ## _plaintext,			\
		.plaintext_len = sizeof ( name ## _plaintext ),		\
		.algorithm = ALGORITHM,					\
		.signature = name ## _signature,			\
		.signature_len = sizeof ( name ## _signature ),		\
	}

/**
 * Report RSA encryption and decryption test result
 *
 * @v test		RSA encryption and decryption test
 */
#define rsa_encrypt_decrypt_ok( test ) do {				\
	pubkey_decrypt_ok ( &rsa_algorithm, (test)->private,		\
			    (test)->private_len, (test)->ciphertext,	\
			    (test)->ciphertext_len, (test)->plaintext,	\
			    (test)->plaintext_len );			\
	pubkey_encrypt_ok ( &rsa_algorithm, (test)->private,		\
			    (test)->private_len, (test)->public,	\
			    (test)->public_len, (test)->plaintext,	\
			    (test)->plaintext_len );			\
	pubkey_encrypt_ok ( &rsa_algorithm, (test)->public,		\
			    (test)->public_len, (test)->private,	\
			    (test)->private_len, (test)->plaintext,	\
			    (test)->plaintext_len );			\
	} while ( 0 )


/**
 * Report RSA signature test result
 *
 * @v test		RSA signature test
 */
#define rsa_signature_ok( test ) do {					\
	struct digest_algorithm *digest = (test)->algorithm->digest;	\
	uint8_t bad_signature[ (test)->signature_len ];			\
	pubkey_sign_ok ( &rsa_algorithm, (test)->private,		\
			 (test)->private_len, digest,			\
			 (test)->plaintext, (test)->plaintext_len,	\
			 (test)->signature, (test)->signature_len );	\
	pubkey_verify_ok ( &rsa_algorithm, (test)->public,		\
			   (test)->public_len, digest,			\
			   (test)->plaintext, (test)->plaintext_len,	\
			   (test)->signature, (test)->signature_len );	\
	memset ( bad_signature, 0, sizeof ( bad_signature ) );		\
	pubkey_verify_fail_ok ( &rsa_algorithm, (test)->public,		\
				(test)->public_len, digest,		\
				(test)->plaintext,			\
				(test)->plaintext_len, bad_signature,	\
				sizeof ( bad_signature ) );		\
	} while ( 0 )

/** "Hello world" encryption and decryption test (traditional PKCS#1 key) */
RSA_ENCRYPT_DECRYPT_TEST ( hw_test,
	PRIVATE ( 0x30, 0x82, 0x01, 0x3b, 0x02, 0x01, 0x00, 0x02, 0x41, 0x00,
		  0xd2, 0xf1, 0x04, 0x67, 0xf6, 0x2c, 0x96, 0x07, 0xa6, 0xbd,
		  0x85, 0xac, 0xc1, 0x17, 0x5d, 0xe8, 0xf0, 0x93, 0x94, 0x0c,
		  0x45, 0x67, 0x26, 0x67, 0xde, 0x7e, 0xfb, 0xa8, 0xda, 0xbd,
		  0x07, 0xdf, 0xcf, 0x45, 0x04, 0x6d, 0xbd, 0x69, 0x8b, 0xfb,
		  0xc1, 0x72, 0xc0, 0xfc, 0x03, 0x04, 0xf2, 0x82, 0xc4, 0x7b,
		  0x6a, 0x3e, 0xec, 0x53, 0x7a, 0xe3, 0x4e, 0xa8, 0xc9, 0xf9,
		  0x1f, 0x2a, 0x13, 0x0d, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02,
		  0x40, 0x49, 0xb8, 0x61, 0xc9, 0xd3, 0x87, 0x11, 0x87, 0xeb,
		  0x06, 0x21, 0x49, 0x96, 0xd2, 0x0b, 0xc7, 0xf5, 0x0c, 0x1e,
		  0x99, 0x8b, 0x47, 0xd9, 0x6c, 0x43, 0x9e, 0x2d, 0x65, 0x7d,
		  0xcc, 0xc2, 0x8b, 0x1a, 0x6f, 0x2b, 0x55, 0xbe, 0xb3, 0x9f,
		  0xd1, 0xe2, 0x9a, 0xde, 0x1d, 0xac, 0xec, 0x67, 0xec, 0xa5,
		  0xbf, 0x9c, 0x30, 0xd6, 0xf9, 0x0a, 0x1a, 0x48, 0xf3, 0xc2,
		  0x93, 0x3a, 0x17, 0x27, 0x21, 0x02, 0x21, 0x00, 0xfc, 0x8d,
		  0xfb, 0xee, 0x8a, 0xaa, 0x45, 0x19, 0x4b, 0xf0, 0x68, 0xb0,
		  0x02, 0x38, 0x3e, 0x03, 0x6b, 0x24, 0x77, 0x20, 0xbd, 0x5e,
		  0x6c, 0x76, 0xdb, 0xc9, 0xe1, 0x43, 0xa3, 0x40, 0x62, 0x6f,
		  0x02, 0x21, 0x00, 0xd5, 0xd1, 0xb4, 0x4d, 0x03, 0x40, 0x69,
		  0x3f, 0x9a, 0xa7, 0x44, 0x15, 0x28, 0x1e, 0xa5, 0x5f, 0xcf,
		  0x97, 0x21, 0x12, 0xb3, 0xe6, 0x1c, 0x9a, 0x8d, 0xb7, 0xb4,
		  0x80, 0x3a, 0x9c, 0xb0, 0x43, 0x02, 0x20, 0x71, 0xf0, 0xa0,
		  0xab, 0x82, 0xf5, 0xc4, 0x8c, 0xe0, 0x1c, 0xcb, 0x2e, 0x35,
		  0x22, 0x28, 0xa0, 0x24, 0x33, 0x64, 0x67, 0x69, 0xe7, 0xf2,
		  0xa9, 0x41, 0x09, 0x78, 0x4e, 0xaa, 0x95, 0x3e, 0x93, 0x02,
		  0x21, 0x00, 0x85, 0xcc, 0x4d, 0xd9, 0x0b, 0x39, 0xd9, 0x22,
		  0x75, 0xf2, 0x49, 0x46, 0x3b, 0xee, 0xc1, 0x69, 0x6d, 0x0b,
		  0x93, 0x24, 0x92, 0xf2, 0x61, 0xdf, 0xcc, 0xe2, 0xb1, 0xce,
		  0xb3, 0xde, 0xac, 0xe5, 0x02, 0x21, 0x00, 0x9c, 0x23, 0x6a,
		  0x95, 0xa6, 0xfe, 0x1e, 0xd8, 0x0c, 0x3f, 0x6e, 0xe6, 0x0a,
		  0xeb, 0x97, 0xd6, 0x36, 0x1c, 0x80, 0xc1, 0x02, 0x87, 0x0d,
		  0x4d, 0xfe, 0x28, 0x02, 0x1e, 0xde, 0xe1, 0xcc, 0x72 ),
	PUBLIC ( 0x30, 0x5c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
		 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x4b, 0x00,
		 0x30, 0x48, 0x02, 0x41, 0x00, 0xd2, 0xf1, 0x04, 0x67, 0xf6,
		 0x2c, 0x96, 0x07, 0xa6, 0xbd, 0x85, 0xac, 0xc1, 0x17, 0x5d,
		 0xe8, 0xf0, 0x93, 0x94, 0x0c, 0x45, 0x67, 0x26, 0x67, 0xde,
		 0x7e, 0xfb, 0xa8, 0xda, 0xbd, 0x07, 0xdf, 0xcf, 0x45, 0x04,
		 0x6d, 0xbd, 0x69, 0x8b, 0xfb, 0xc1, 0x72, 0xc0, 0xfc, 0x03,
		 0x04, 0xf2, 0x82, 0xc4, 0x7b, 0x6a, 0x3e, 0xec, 0x53, 0x7a,
		 0xe3, 0x4e, 0xa8, 0xc9, 0xf9, 0x1f, 0x2a, 0x13, 0x0d, 0x02,
		 0x03, 0x01, 0x00, 0x01 ),
	PLAINTEXT ( 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c,
		    0x64, 0x0a ),
	CIPHERTEXT ( 0x39, 0xff, 0x5c, 0x54, 0x65, 0x3e, 0x6a, 0xab, 0xc0, 0x62,
		     0x91, 0xb2, 0xbf, 0x1d, 0x73, 0x5b, 0xd5, 0x4c, 0xbd, 0x16,
		     0x0f, 0x24, 0xc9, 0xf5, 0xa7, 0xdd, 0x94, 0xd6, 0xf8, 0xae,
		     0xd3, 0xa0, 0x9f, 0x4d, 0xff, 0x8d, 0x81, 0x34, 0x47, 0xff,
		     0x2a, 0x87, 0x96, 0xd3, 0x17, 0x5d, 0x93, 0x4d, 0x7b, 0x27,
		     0x88, 0x4f, 0xec, 0x43, 0x9c, 0xed, 0xb3, 0xf2, 0x19, 0x89,
		     0x38, 0x43, 0xf9, 0x41 ) );

/** "Hello world" encryption and decryption test (PKCS#8 key) */
RSA_ENCRYPT_DECRYPT_TEST ( hw_test_pkcs8,
	PRIVATE ( 0x30, 0x82, 0x01, 0x55, 0x02, 0x01, 0x00, 0x30, 0x0d, 0x06,
		  0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
		  0x05, 0x00, 0x04, 0x82, 0x01, 0x3f, 0x30, 0x82, 0x01, 0x3b,
		  0x02, 0x01, 0x00, 0x02, 0x41, 0x00, 0xd2, 0xf1, 0x04, 0x67,
		  0xf6, 0x2c, 0x96, 0x07, 0xa6, 0xbd, 0x85, 0xac, 0xc1, 0x17,
		  0x5d, 0xe8, 0xf0, 0x93, 0x94, 0x0c, 0x45, 0x67, 0x26, 0x67,
		  0xde, 0x7e, 0xfb, 0xa8, 0xda, 0xbd, 0x07, 0xdf, 0xcf, 0x45,
		  0x04, 0x6d, 0xbd, 0x69, 0x8b, 0xfb, 0xc1, 0x72, 0xc0, 0xfc,
		  0x03, 0x04, 0xf2, 0x82, 0xc4, 0x7b, 0x6a, 0x3e, 0xec, 0x53,
		  0x7a, 0xe3, 0x4e, 0xa8, 0xc9, 0xf9, 0x1f, 0x2a, 0x13, 0x0d,
		  0x02, 0x03, 0x01, 0x00, 0x01, 0x02, 0x40, 0x49, 0xb8, 0x61,
		  0xc9, 0xd3, 0x87, 0x11, 0x87, 0xeb, 0x06, 0x21, 0x49, 0x96,
		  0xd2, 0x0b, 0xc7, 0xf5, 0x0c, 0x1e, 0x99, 0x8b, 0x47, 0xd9,
		  0x6c, 0x43, 0x9e, 0x2d, 0x65, 0x7d, 0xcc, 0xc2, 0x8b, 0x1a,
		  0x6f, 0x2b, 0x55, 0xbe, 0xb3, 0x9f, 0xd1, 0xe2, 0x9a, 0xde,
		  0x1d, 0xac, 0xec, 0x67, 0xec, 0xa5, 0xbf, 0x9c, 0x30, 0xd6,
		  0xf9, 0x0a, 0x1a, 0x48, 0xf3, 0xc2, 0x93, 0x3a, 0x17, 0x27,
		  0x21, 0x02, 0x21, 0x00, 0xfc, 0x8d, 0xfb, 0xee, 0x8a, 0xaa,
		  0x45, 0x19, 0x4b, 0xf0, 0x68, 0xb0, 0x02, 0x38, 0x3e, 0x03,
		  0x6b, 0x24, 0x77, 0x20, 0xbd, 0x5e, 0x6c, 0x76, 0xdb, 0xc9,
		  0xe1, 0x43, 0xa3, 0x40, 0x62, 0x6f, 0x02, 0x21, 0x00, 0xd5,
		  0xd1, 0xb4, 0x4d, 0x03, 0x40, 0x69, 0x3f, 0x9a, 0xa7, 0x44,
		  0x15, 0x28, 0x1e, 0xa5, 0x5f, 0xcf, 0x97, 0x21, 0x12, 0xb3,
		  0xe6, 0x1c, 0x9a, 0x8d, 0xb7, 0xb4, 0x80, 0x3a, 0x9c, 0xb0,
		  0x43, 0x02, 0x20, 0x71, 0xf0, 0xa0, 0xab, 0x82, 0xf5, 0xc4,
		  0x8c, 0xe0, 0x1c, 0xcb, 0x2e, 0x35, 0x22, 0x28, 0xa0, 0x24,
		  0x33, 0x64, 0x67, 0x69, 0xe7, 0xf2, 0xa9, 0x41, 0x09, 0x78,
		  0x4e, 0xaa, 0x95, 0x3e, 0x93, 0x02, 0x21, 0x00, 0x85, 0xcc,
		  0x4d, 0xd9, 0x0b, 0x39, 0xd9, 0x22, 0x75, 0xf2, 0x49, 0x46,
		  0x3b, 0xee, 0xc1, 0x69, 0x6d, 0x0b, 0x93, 0x24, 0x92, 0xf2,
		  0x61, 0xdf, 0xcc, 0xe2, 0xb1, 0xce, 0xb3, 0xde, 0xac, 0xe5,
		  0x02, 0x21, 0x00, 0x9c, 0x23, 0x6a, 0x95, 0xa6, 0xfe, 0x1e,
		  0xd8, 0x0c, 0x3f, 0x6e, 0xe6, 0x0a, 0xeb, 0x97, 0xd6, 0x36,
		  0x1c, 0x80, 0xc1, 0x02, 0x87, 0x0d, 0x4d, 0xfe, 0x28, 0x02,
		  0x1e, 0xde, 0xe1, 0xcc, 0x72 ),
	PUBLIC ( 0x30, 0x5c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
		 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x4b, 0x00,
		 0x30, 0x48, 0x02, 0x41, 0x00, 0xd2, 0xf1, 0x04, 0x67, 0xf6,
		 0x2c, 0x96, 0x07, 0xa6, 0xbd, 0x85, 0xac, 0xc1, 0x17, 0x5d,
		 0xe8, 0xf0, 0x93, 0x94, 0x0c, 0x45, 0x67, 0x26, 0x67, 0xde,
		 0x7e, 0xfb, 0xa8, 0xda, 0xbd, 0x07, 0xdf, 0xcf, 0x45, 0x04,
		 0x6d, 0xbd, 0x69, 0x8b, 0xfb, 0xc1, 0x72, 0xc0, 0xfc, 0x03,
		 0x04, 0xf2, 0x82, 0xc4, 0x7b, 0x6a, 0x3e, 0xec, 0x53, 0x7a,
		 0xe3, 0x4e, 0xa8, 0xc9, 0xf9, 0x1f, 0x2a, 0x13, 0x0d, 0x02,
		 0x03, 0x01, 0x00, 0x01 ),
	PLAINTEXT ( 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c,
		    0x64, 0x0a ),
	CIPHERTEXT ( 0x39, 0xff, 0x5c, 0x54, 0x65, 0x3e, 0x6a, 0xab, 0xc0, 0x62,
		     0x91, 0xb2, 0xbf, 0x1d, 0x73, 0x5b, 0xd5, 0x4c, 0xbd, 0x16,
		     0x0f, 0x24, 0xc9, 0xf5, 0xa7, 0xdd, 0x94, 0xd6, 0xf8, 0xae,
		     0xd3, 0xa0, 0x9f, 0x4d, 0xff, 0x8d, 0x81, 0x34, 0x47, 0xff,
		     0x2a, 0x87, 0x96, 0xd3, 0x17, 0x5d, 0x93, 0x4d, 0x7b, 0x27,
		     0x88, 0x4f, 0xec, 0x43, 0x9c, 0xed, 0xb3, 0xf2, 0x19, 0x89,
		     0x38, 0x43, 0xf9, 0x41 ) );

/** Random message MD5 signature test */
RSA_SIGNATURE_TEST ( md5_test,
	PRIVATE ( 0x30, 0x82, 0x01, 0x3b, 0x02, 0x01, 0x00, 0x02, 0x41, 0x00,
		  0xf9, 0x3f, 0x78, 0x44, 0xe2, 0x0e, 0x25, 0xf1, 0x0e, 0x94,
		  0xcd, 0xca, 0x6f, 0x9e, 0xea, 0x6d, 0xdf, 0xcd, 0xa0, 0x7c,
		  0xe2, 0x21, 0xeb, 0xde, 0xa6, 0x01, 0x4b, 0xb0, 0x76, 0x4b,
		  0xd8, 0x8b, 0x19, 0x83, 0xb4, 0xbe, 0x45, 0xde, 0x3d, 0x46,
		  0x61, 0x0f, 0x11, 0xe2, 0x2c, 0xf5, 0xb0, 0x63, 0xa0, 0x84,
		  0xc0, 0xaf, 0x4e, 0xbe, 0x6a, 0xd3, 0x84, 0x3f, 0xec, 0x42,
		  0x17, 0xe9, 0x25, 0xe1, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02,
		  0x40, 0x62, 0x7d, 0x93, 0x1f, 0xdd, 0x17, 0xec, 0x24, 0x42,
		  0x37, 0xc8, 0xce, 0x0a, 0xa7, 0x88, 0x49, 0x5c, 0x9b, 0x9b,
		  0xa4, 0x5d, 0x93, 0x3b, 0xea, 0x62, 0x3c, 0xb6, 0xd5, 0x07,
		  0x19, 0xd7, 0x79, 0xf0, 0x3b, 0xab, 0xa3, 0xa5, 0x43, 0x35,
		  0x8d, 0x58, 0x40, 0xa0, 0x95, 0xc5, 0x63, 0x28, 0x28, 0xda,
		  0x13, 0x28, 0xdf, 0xc9, 0x05, 0xdc, 0x69, 0x46, 0xff, 0x2a,
		  0xfb, 0xe4, 0xd1, 0x23, 0xa5, 0x02, 0x21, 0x00, 0xfc, 0xef,
		  0x3b, 0x9d, 0x9d, 0x69, 0xf3, 0x66, 0x0a, 0x2b, 0x52, 0xd6,
		  0x61, 0x14, 0x90, 0x6e, 0x7d, 0x3c, 0x08, 0x4b, 0x98, 0x44,
		  0x00, 0xf2, 0xa4, 0x16, 0x2d, 0xd1, 0xf9, 0xa0, 0x1e, 0x37,
		  0x02, 0x21, 0x00, 0xfc, 0x44, 0xcc, 0x7c, 0xc0, 0x26, 0x9a,
		  0x0a, 0x6e, 0xda, 0x17, 0x05, 0x7d, 0x66, 0x8d, 0x29, 0x1a,
		  0x44, 0xbf, 0x33, 0x76, 0xae, 0x8d, 0xe8, 0xb5, 0xed, 0xb8,
		  0x6f, 0xdc, 0xfe, 0x10, 0xa7, 0x02, 0x20, 0x76, 0x48, 0x8a,
		  0x60, 0x93, 0x14, 0xd1, 0x36, 0x8e, 0xda, 0xe3, 0xca, 0x4d,
		  0x6c, 0x08, 0x7f, 0x23, 0x21, 0xc7, 0xdf, 0x52, 0x3d, 0xbb,
		  0x13, 0xbd, 0x98, 0x81, 0xa5, 0x08, 0x4f, 0xd0, 0xd1, 0x02,
		  0x21, 0x00, 0xd9, 0xa3, 0x11, 0x37, 0xdf, 0x1e, 0x6e, 0x6e,
		  0xe9, 0xcb, 0xc5, 0x68, 0xbb, 0x13, 0x2a, 0x5d, 0x77, 0x88,
		  0x2f, 0xdc, 0x5a, 0x5b, 0xa5, 0x9a, 0x4a, 0xba, 0x58, 0x10,
		  0x49, 0xfb, 0xf6, 0xa9, 0x02, 0x21, 0x00, 0x89, 0xe8, 0x47,
		  0x5b, 0x20, 0x04, 0x3b, 0x0f, 0xb9, 0xe0, 0x1d, 0xab, 0xcf,
		  0xe8, 0x72, 0xfd, 0x7d, 0x17, 0x85, 0xc8, 0xd8, 0xbd, 0x1a,
		  0x92, 0xe0, 0xbc, 0x7a, 0xc7, 0x31, 0xbe, 0xef, 0xf4 ),
	PUBLIC ( 0x30, 0x5c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
		 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x4b, 0x00,
		 0x30, 0x48, 0x02, 0x41, 0x00, 0xf9, 0x3f, 0x78, 0x44, 0xe2,
		 0x0e, 0x25, 0xf1, 0x0e, 0x94, 0xcd, 0xca, 0x6f, 0x9e, 0xea,
		 0x6d, 0xdf, 0xcd, 0xa0, 0x7c, 0xe2, 0x21, 0xeb, 0xde, 0xa6,
		 0x01, 0x4b, 0xb0, 0x76, 0x4b, 0xd8, 0x8b, 0x19, 0x83, 0xb4,
		 0xbe, 0x45, 0xde, 0x3d, 0x46, 0x61, 0x0f, 0x11, 0xe2, 0x2c,
		 0xf5, 0xb0, 0x63, 0xa0, 0x84, 0xc0, 0xaf, 0x4e, 0xbe, 0x6a,
		 0xd3, 0x84, 0x3f, 0xec, 0x42, 0x17, 0xe9, 0x25, 0xe1, 0x02,
		 0x03, 0x01, 0x00, 0x01 ),
	PLAINTEXT ( 0x9d, 0x5b, 0x46, 0x42, 0x27, 0xc0, 0xf1, 0x4b, 0xe5, 0x9e,
		    0xd3, 0x10, 0xa1, 0xeb, 0x16, 0xc3, 0xc6, 0x8f, 0x1a, 0x18,
		    0x86, 0xc3, 0x92, 0x15, 0x2d, 0x65, 0xa0, 0x40, 0xe1, 0x3e,
		    0x29, 0x79, 0x7c, 0xd4, 0x08, 0xef, 0x53, 0xeb, 0x08, 0x07,
		    0x39, 0x21, 0xb3, 0x40, 0xff, 0x4b, 0xc7, 0x76, 0xb9, 0x12,
		    0x32, 0x41, 0xcc, 0x5a, 0x86, 0x5c, 0x2e, 0x0b, 0x05, 0xd8,
		    0x56, 0xd4, 0xdf, 0x6f, 0x2c, 0xf0, 0xbf, 0x4b, 0x6f, 0x68,
		    0xde, 0x39, 0x4a, 0x3e, 0xae, 0x44, 0xb9, 0xc6, 0x24, 0xb3,
		    0x83, 0x2e, 0x9f, 0xf5, 0x6d, 0x61, 0xc3, 0x8e, 0xe8, 0x8f,
		    0xa6, 0x87, 0x58, 0x3f, 0x36, 0x13, 0xf4, 0x7e, 0xf0, 0x20,
		    0x47, 0x87, 0x3f, 0x21, 0x6e, 0x51, 0x3c, 0xf1, 0xef, 0xca,
		    0x9f, 0x77, 0x9c, 0x91, 0x4f, 0xd4, 0x56, 0xc0, 0x39, 0x11,
		    0xab, 0x15, 0x2c, 0x5e, 0xad, 0x40, 0x09, 0xe6, 0xde, 0xe5,
		    0x77, 0x60, 0x19, 0xd4, 0x0d, 0x77, 0x76, 0x24, 0x8b, 0xe6,
		    0xdd, 0xa5, 0x8d, 0x4a, 0x55, 0x3a, 0xdf, 0xf8, 0x29, 0xfb,
		    0x47, 0x8a, 0xfe, 0x98, 0x34, 0xf6, 0x30, 0x7f, 0x09, 0x03,
		    0x26, 0x05, 0xd5, 0x46, 0x18, 0x96, 0xca, 0x96, 0x5b, 0x66,
		    0xf2, 0x8d, 0xfc, 0xfc, 0x37, 0xf7, 0xc7, 0x6d, 0x6c, 0xd8,
		    0x24, 0x0c, 0x6a, 0xec, 0x82, 0x5c, 0x72, 0xf1, 0xfc, 0x05,
		    0xed, 0x8e, 0xe8, 0xd9, 0x8b, 0x8b, 0x67, 0x02, 0x95 ),
	&md5_with_rsa_encryption_algorithm,
	SIGNATURE ( 0xdb, 0x56, 0x3d, 0xea, 0xae, 0x81, 0x4b, 0x3b, 0x2e, 0x8e,
		    0xb8, 0xee, 0x13, 0x61, 0xc6, 0xe7, 0xd7, 0x50, 0xcd, 0x0d,
		    0x34, 0x3a, 0xfe, 0x9a, 0x8d, 0xf8, 0xfb, 0xd6, 0x7e, 0xbd,
		    0xdd, 0xb3, 0xf9, 0xfb, 0xe0, 0xf8, 0xe7, 0x71, 0x03, 0xe6,
		    0x55, 0xd5, 0xf4, 0x02, 0x3c, 0xb5, 0xbc, 0x95, 0x2b, 0x66,
		    0x56, 0xec, 0x2f, 0x8e, 0xa7, 0xae, 0xd9, 0x80, 0xb3, 0xaa,
		    0xac, 0x45, 0x00, 0xa8 ) );

/** Random message SHA-1 signature test */
RSA_SIGNATURE_TEST ( sha1_test,
	PRIVATE ( 0x30, 0x82, 0x01, 0x3b, 0x02, 0x01, 0x00, 0x02, 0x41, 0x00,
		  0xe0, 0x3a, 0x8d, 0x35, 0xe1, 0x92, 0x2f, 0xea, 0x0d, 0x82,
		  0x60, 0x2e, 0xb6, 0x0b, 0x02, 0xd3, 0xf4, 0x39, 0xfb, 0x06,
		  0x43, 0x8e, 0xa1, 0x7c, 0xc5, 0xae, 0x0d, 0xc7, 0xee, 0x83,
		  0xb3, 0x63, 0x20, 0x92, 0x34, 0xe2, 0x94, 0x3d, 0xdd, 0xbb,
		  0x6c, 0x64, 0x69, 0x68, 0x25, 0x24, 0x81, 0x4b, 0x4d, 0x48,
		  0x5a, 0xd2, 0x29, 0x14, 0xeb, 0x38, 0xdd, 0x3e, 0xb5, 0x57,
		  0x45, 0x9b, 0xed, 0x33, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02,
		  0x40, 0x3d, 0xa9, 0x1c, 0x47, 0xe2, 0xdd, 0xf6, 0x7b, 0x20,
		  0x77, 0xe7, 0xc7, 0x30, 0x9c, 0x5a, 0x8c, 0xba, 0xae, 0x6f,
		  0x0f, 0x4b, 0xe8, 0x9f, 0x13, 0xd6, 0xb0, 0x84, 0x6d, 0xa4,
		  0x73, 0x67, 0x12, 0xa9, 0x7c, 0x75, 0xaf, 0x62, 0x92, 0x7b,
		  0x80, 0xaf, 0x39, 0x7d, 0x01, 0xb3, 0x43, 0xc8, 0x0d, 0x17,
		  0x7f, 0x82, 0x59, 0x46, 0xb8, 0xe5, 0x4e, 0xba, 0x5e, 0x71,
		  0x5c, 0xba, 0x62, 0x06, 0x91, 0x02, 0x21, 0x00, 0xf7, 0xaa,
		  0xb6, 0x9c, 0xc8, 0xad, 0x68, 0xa8, 0xd7, 0x25, 0xb1, 0xb5,
		  0x91, 0xd4, 0xc7, 0xd6, 0x69, 0x51, 0x5d, 0x04, 0xed, 0xd8,
		  0xc6, 0xea, 0x69, 0xd2, 0x24, 0xbe, 0x5e, 0x7c, 0x89, 0xa5,
		  0x02, 0x21, 0x00, 0xe7, 0xc5, 0xf4, 0x01, 0x35, 0xe0, 0x16,
		  0xb5, 0x13, 0x86, 0x14, 0x5a, 0x6a, 0x8d, 0x03, 0x90, 0xae,
		  0x7d, 0x3a, 0xc1, 0xfe, 0x8c, 0xa0, 0x4a, 0xb4, 0x94, 0x50,
		  0x58, 0xa4, 0xc6, 0x73, 0xf7, 0x02, 0x21, 0x00, 0xe2, 0xda,
		  0x16, 0x6c, 0x63, 0x90, 0x1a, 0xc6, 0x54, 0x53, 0x2d, 0x84,
		  0x8f, 0x70, 0x24, 0x1f, 0x6b, 0xd6, 0x5f, 0xea, 0x8c, 0xe5,
		  0xbb, 0xc5, 0xa9, 0x6a, 0x17, 0xc7, 0xdb, 0x8a, 0x1d, 0x15,
		  0x02, 0x21, 0x00, 0xe4, 0x2a, 0x7e, 0xe4, 0x76, 0x2a, 0x2d,
		  0x90, 0x83, 0x30, 0xda, 0x76, 0x8c, 0x30, 0x58, 0x13, 0x25,
		  0x83, 0x88, 0xc5, 0x93, 0x96, 0xd2, 0xf1, 0xd8, 0x45, 0xad,
		  0xb7, 0x26, 0x37, 0x6b, 0xcf, 0x02, 0x20, 0x73, 0x58, 0x1f,
		  0x0a, 0xcd, 0x0c, 0x83, 0x27, 0xcc, 0x15, 0xa2, 0x1e, 0x07,
		  0x32, 0x1b, 0xa3, 0xc6, 0xa6, 0xb8, 0x83, 0x97, 0x48, 0x45,
		  0x50, 0x6c, 0x37, 0x45, 0xa5, 0x54, 0x2a, 0x59, 0x3c ),
	PUBLIC ( 0x30, 0x5c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
		 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x4b, 0x00,
		 0x30, 0x48, 0x02, 0x41, 0x00, 0xe0, 0x3a, 0x8d, 0x35, 0xe1,
		 0x92, 0x2f, 0xea, 0x0d, 0x82, 0x60, 0x2e, 0xb6, 0x0b, 0x02,
		 0xd3, 0xf4, 0x39, 0xfb, 0x06, 0x43, 0x8e, 0xa1, 0x7c, 0xc5,
		 0xae, 0x0d, 0xc7, 0xee, 0x83, 0xb3, 0x63, 0x20, 0x92, 0x34,
		 0xe2, 0x94, 0x3d, 0xdd, 0xbb, 0x6c, 0x64, 0x69, 0x68, 0x25,
		 0x24, 0x81, 0x4b, 0x4d, 0x48, 0x5a, 0xd2, 0x29, 0x14, 0xeb,
		 0x38, 0xdd, 0x3e, 0xb5, 0x57, 0x45, 0x9b, 0xed, 0x33, 0x02,
		 0x03, 0x01, 0x00, 0x01 ),
	PLAINTEXT ( 0xf7, 0x42, 0x01, 0x57, 0x6b, 0x70, 0xcc, 0x4a, 0xdc, 0xed,
		    0x12, 0x83, 0x3f, 0xef, 0x27, 0xc1, 0x3c, 0x85, 0xdd, 0x5e,
		    0x0a, 0x34, 0x98, 0xf9, 0x21, 0xd3, 0x24, 0x2a, 0x5a, 0xb2,
		    0xdf, 0x60, 0x21, 0x28, 0x7c, 0x5b, 0x7a, 0xbe, 0xcb, 0xea,
		    0xbc, 0xd6, 0x0e, 0xae, 0x94, 0x64, 0x21, 0xda, 0x28, 0x66,
		    0x2f, 0x71, 0x48, 0xe5, 0xea, 0x59, 0x38, 0x28, 0x3e, 0xed,
		    0x3b, 0x95, 0x4f, 0x3d, 0x72, 0x2a, 0x00, 0xf3, 0x95, 0x4d,
		    0xf0, 0x02, 0x71, 0x63, 0x5a, 0xbc, 0x84, 0xd1, 0x81, 0x3f,
		    0x16, 0xcd, 0x28, 0x3d, 0x47, 0xa2, 0xee, 0xa1, 0x2f, 0x84,
		    0x8a, 0x22, 0x02, 0x88, 0xd7, 0x83, 0x06, 0x4a, 0x9f, 0xea,
		    0x0f, 0x15, 0x48, 0x43, 0x58, 0x6d, 0x39, 0x78, 0x5a, 0x43,
		    0x3f, 0xed, 0x6f, 0x68, 0xde, 0x9c, 0xfe, 0xd3, 0x67, 0x74,
		    0x08, 0x46, 0x7d, 0x20, 0x22, 0x60, 0x8c, 0x37, 0x35, 0x46,
		    0x56, 0x19, 0x3c, 0xfa, 0xa5, 0x40, 0xac, 0x44, 0x90, 0x8a,
		    0xa5, 0x80, 0xb2, 0x32, 0xbc, 0xb4, 0x3f, 0x3e, 0x5e, 0xd4,
		    0x51, 0xa9, 0x2e, 0xd9, 0x7f, 0x5e, 0x32, 0xb1, 0x24, 0x35,
		    0x88, 0x71, 0x3a, 0x01, 0x86, 0x5c, 0xa2, 0xe2, 0x2d, 0x02,
		    0x30, 0x91, 0x1c, 0xaa, 0x6c, 0x24, 0x42, 0x1b, 0x1a, 0xba,
		    0x30, 0x40, 0x49, 0x83, 0xd9, 0xd7, 0x66, 0x7e, 0x5c, 0x1a,
		    0x4b, 0x7f, 0xa6, 0x8e, 0x8a, 0xd6, 0x0c, 0x65, 0x75 ),
	&sha1_with_rsa_encryption_algorithm,
	SIGNATURE ( 0xa5, 0x5a, 0x8a, 0x67, 0x81, 0x76, 0x7e, 0xad, 0x99, 0x22,
		    0xf1, 0x47, 0x64, 0xd2, 0xfb, 0x81, 0x45, 0xeb, 0x85, 0x56,
		    0xf8, 0x7d, 0xb8, 0xec, 0x41, 0x17, 0x84, 0xf7, 0x2b, 0xbb,
		    0x2b, 0x8f, 0xb6, 0xb8, 0x8f, 0xc6, 0xab, 0x39, 0xbc, 0xa3,
		    0x72, 0xb3, 0x63, 0x45, 0x5a, 0xe0, 0xac, 0xf8, 0x1c, 0x83,
		    0x48, 0x84, 0x89, 0x8a, 0x6b, 0xdf, 0x93, 0xa0, 0xc3, 0x0b,
		    0x0e, 0x3d, 0x80, 0x80 ) );

/** Random message SHA-256 signature test */
RSA_SIGNATURE_TEST ( sha256_test,
	PRIVATE ( 0x30, 0x82, 0x01, 0x3a, 0x02, 0x01, 0x00, 0x02, 0x41, 0x00,
		  0xa5, 0xe9, 0xdb, 0xa9, 0x1a, 0x6e, 0xd6, 0x4c, 0x25, 0x50,
		  0xfe, 0x61, 0x77, 0x08, 0x7a, 0x80, 0x36, 0xcb, 0x88, 0x49,
		  0x5c, 0xe8, 0xaa, 0x15, 0xf8, 0xb3, 0xd6, 0x78, 0x51, 0x46,
		  0x86, 0x3a, 0x5f, 0xd5, 0x9f, 0xab, 0xfe, 0x74, 0x8c, 0x53,
		  0x0d, 0xb5, 0x3c, 0x7d, 0x2c, 0x35, 0x88, 0x3f, 0xde, 0xa2,
		  0xce, 0x46, 0x94, 0x30, 0xa9, 0x76, 0xee, 0x25, 0xc5, 0x5d,
		  0xa6, 0xa6, 0x3a, 0xa5, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02,
		  0x40, 0x14, 0x4b, 0xbc, 0x4c, 0x3e, 0x68, 0x8a, 0x9c, 0x7c,
		  0x00, 0x21, 0x6e, 0x28, 0xd2, 0x87, 0xb1, 0xc1, 0x82, 0x3a,
		  0x64, 0xc7, 0x11, 0xcb, 0x24, 0xae, 0xec, 0xc8, 0xf2, 0xa4,
		  0xf6, 0x9c, 0x9a, 0xbb, 0x05, 0x94, 0x80, 0x9b, 0xc1, 0x21,
		  0x83, 0x36, 0x23, 0xba, 0x04, 0x20, 0x23, 0x06, 0x48, 0xa7,
		  0xa4, 0xe6, 0x31, 0x8e, 0xa1, 0x73, 0xe5, 0x6b, 0x83, 0x4c,
		  0x3a, 0xb8, 0xd8, 0x22, 0x61, 0x02, 0x21, 0x00, 0xd4, 0xdf,
		  0xcb, 0x21, 0x4a, 0x9a, 0x35, 0x52, 0x02, 0x99, 0xcc, 0x40,
		  0x83, 0x65, 0x30, 0x1f, 0x9d, 0x13, 0xd6, 0xd1, 0x79, 0x10,
		  0xce, 0x5b, 0xeb, 0x25, 0xa2, 0x39, 0x4e, 0xdf, 0x1c, 0x29,
		  0x02, 0x21, 0x00, 0xc7, 0x86, 0x8f, 0xd9, 0x88, 0xe9, 0x98,
		  0x4b, 0x5c, 0x50, 0x06, 0x94, 0x05, 0x59, 0x31, 0x25, 0xa7,
		  0xa8, 0xe6, 0x95, 0x2b, 0xe3, 0x74, 0x93, 0x51, 0xa8, 0x8e,
		  0x3d, 0xe2, 0xe0, 0xfa, 0x1d, 0x02, 0x20, 0x6e, 0xe3, 0x81,
		  0x31, 0xff, 0x65, 0xa3, 0x1e, 0xec, 0x61, 0xe7, 0x67, 0x37,
		  0xcb, 0x0f, 0x2d, 0x78, 0xaa, 0xab, 0xfd, 0x84, 0x5e, 0x3f,
		  0xd0, 0xdc, 0x06, 0x47, 0xa2, 0x28, 0xb6, 0xca, 0x39, 0x02,
		  0x20, 0x13, 0x7d, 0x9f, 0x9b, 0xbe, 0x76, 0x23, 0x3c, 0x69,
		  0x5e, 0x1f, 0xe6, 0x61, 0xc7, 0x5e, 0xb7, 0xb0, 0xf3, 0x1c,
		  0xe3, 0x41, 0x90, 0x4c, 0x98, 0xff, 0x87, 0x19, 0xae, 0x0d,
		  0xf5, 0xb0, 0x39, 0x02, 0x21, 0x00, 0xb7, 0xeb, 0xcd, 0x01,
		  0x2e, 0x23, 0x42, 0x4f, 0x0c, 0x6f, 0xde, 0xc8, 0x4f, 0xa7,
		  0x69, 0x09, 0x12, 0x34, 0xb6, 0x95, 0x4d, 0xb8, 0x7f, 0x16,
		  0xd0, 0x48, 0x17, 0x4a, 0x9e, 0x6e, 0x5e, 0xe2 ),
	PUBLIC ( 0x30, 0x5c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
		 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x4b, 0x00,
		 0x30, 0x48, 0x02, 0x41, 0x00, 0xa5, 0xe9, 0xdb, 0xa9, 0x1a,
		 0x6e, 0xd6, 0x4c, 0x25, 0x50, 0xfe, 0x61, 0x77, 0x08, 0x7a,
		 0x80, 0x36, 0xcb, 0x88, 0x49, 0x5c, 0xe8, 0xaa, 0x15, 0xf8,
		 0xb3, 0xd6, 0x78, 0x51, 0x46, 0x86, 0x3a, 0x5f, 0xd5, 0x9f,
		 0xab, 0xfe, 0x74, 0x8c, 0x53, 0x0d, 0xb5, 0x3c, 0x7d, 0x2c,
		 0x35, 0x88, 0x3f, 0xde, 0xa2, 0xce, 0x46, 0x94, 0x30, 0xa9,
		 0x76, 0xee, 0x25, 0xc5, 0x5d, 0xa6, 0xa6, 0x3a, 0xa5, 0x02,
		 0x03, 0x01, 0x00, 0x01 ),
	PLAINTEXT ( 0x60, 0xe7, 0xba, 0x9d, 0x5a, 0xe3, 0x2d, 0xfa, 0x5f, 0x47,
		    0xdb, 0x93, 0x24, 0x2c, 0xc4, 0xe2, 0x61, 0xf3, 0x89, 0x4d,
		    0x67, 0xad, 0xc8, 0xae, 0xf8, 0xe2, 0xfb, 0x52, 0x0f, 0x8d,
		    0x18, 0x7e, 0x30, 0xd8, 0x8d, 0x94, 0x07, 0x92, 0x70, 0x91,
		    0xaf, 0x3b, 0x92, 0xa6, 0x0f, 0x7a, 0x9b, 0x46, 0x85, 0x8c,
		    0x2a, 0x5a, 0x78, 0x5d, 0x1e, 0x13, 0xbf, 0xe6, 0x12, 0xbd,
		    0xb1, 0xbb, 0x92, 0x6d, 0x11, 0xed, 0xe1, 0xe4, 0x6e, 0x88,
		    0x4d, 0x0b, 0x51, 0xd6, 0xfd, 0x6a, 0xb2, 0x9b, 0xd3, 0xfd,
		    0x56, 0xec, 0xd9, 0xd6, 0xb8, 0xc5, 0xfd, 0x0c, 0xf7, 0x55,
		    0x5f, 0xc5, 0x6f, 0xbc, 0xbb, 0x78, 0x2f, 0x50, 0x08, 0x65,
		    0x0f, 0x12, 0xca, 0x5a, 0xea, 0x52, 0xd0, 0x94, 0x76, 0x17,
		    0xe4, 0xba, 0x97, 0xba, 0x11, 0xbf, 0x05, 0x7e, 0xa1, 0xfd,
		    0x7d, 0xb5, 0xf1, 0x3a, 0x7e, 0x6f, 0xa1, 0xaa, 0x97, 0x66,
		    0x5d, 0x72, 0x76, 0x45, 0x40, 0xb5, 0x22, 0x71, 0x43, 0xe8,
		    0x77, 0x76, 0xc8, 0x1b, 0xd2, 0xd1, 0x33, 0x05, 0x64, 0xa9,
		    0xc2, 0xa8, 0x40, 0x40, 0x21, 0xdd, 0xcf, 0x07, 0x7e, 0xf2,
		    0x4b, 0x80, 0x3d, 0x0f, 0x67, 0xf6, 0xbd, 0xc2, 0xc7, 0xe3,
		    0x91, 0x71, 0xd6, 0x2d, 0xa1, 0xae, 0x81, 0x0c, 0xed, 0x54,
		    0x48, 0x79, 0x8a, 0x78, 0x05, 0x74, 0x4d, 0x4f, 0xf0, 0xe0,
		    0x3c, 0x41, 0x5c, 0x04, 0x0b, 0x68, 0x57, 0xc5, 0xd6 ),
	&sha256_with_rsa_encryption_algorithm,
	SIGNATURE ( 0x02, 0x2e, 0xc5, 0x2a, 0x2b, 0x7f, 0xb4, 0x80, 0xca, 0x9d,
		    0x96, 0x5b, 0xaf, 0x1f, 0x72, 0x5b, 0x6e, 0xf1, 0x69, 0x7f,
		    0x4d, 0x41, 0xd5, 0x9f, 0x00, 0xdc, 0x47, 0xf4, 0x68, 0x8f,
		    0xda, 0xfc, 0xd1, 0x23, 0x96, 0x11, 0x1d, 0xc0, 0x1b, 0x1d,
		    0x36, 0x66, 0x2a, 0xf9, 0x21, 0x51, 0xcb, 0xb9, 0x7d, 0x24,
		    0x7d, 0x38, 0x37, 0xc4, 0xea, 0xdd, 0x3a, 0x6f, 0xa8, 0x65,
		    0x60, 0x73, 0x77, 0x3c ) );

/**
 * Perform RSA self-tests
 *
 */
static void rsa_test_exec ( void ) {

	rsa_encrypt_decrypt_ok ( &hw_test );
	rsa_encrypt_decrypt_ok ( &hw_test_pkcs8 );
	rsa_signature_ok ( &md5_test );
	rsa_signature_ok ( &sha1_test );
	rsa_signature_ok ( &sha256_test );
}

/** RSA self-test */
struct self_test rsa_test __self_test = {
	.name = "rsa",
	.exec = rsa_test_exec,
};
