#define WOLFSSL_ESPIDF_COMPONENT_VERSION 0x01

/* Examples such as test and benchmark are known to cause watchdog timeouts.
 * Note this is often set in project Makefile:
 * CFLAGS += -DWOLFSSL_ESP_NO_WATCHDOG=1 */
#define WOLFSSL_ESP_NO_WATCHDOG 1

/* The Espressif project config file. See also sdkconfig.defaults */
#include "sdkconfig.h"

/* The Espressif sdkconfig will have chipset info.
**
** Some possible values:
**
**   CONFIG_IDF_TARGET_ESP32
**   CONFIG_IDF_TARGET_ESP32S2
**   CONFIG_IDF_TARGET_ESP32S3
**   CONFIG_IDF_TARGET_ESP32C3
**   CONFIG_IDF_TARGET_ESP32C6
*/

#undef WOLFSSL_ESPIDF
#define WOLFSSL_ESPIDF

#define WOLFSSL_MAX_ERROR_SZ 500

/* Enable AES for all examples */
#ifdef NO_AES
#error "Found NO_AES, wolfSSL AES Cannot be enabled. Check config."
#else
#define WOLFSSL_AES
#define WOLFSSL_AES_COUNTER

/* Typically only needed for wolfssl_test, see docs. */
// #define WOLFSSL_AES_DIRECT
#endif

/* The Espressif sdkconfig will have chipset info.
**
** Some possible values:
**
**   CONFIG_IDF_TARGET_ESP32
**   CONFIG_IDF_TARGET_ESP32S2
**   CONFIG_IDF_TARGET_ESP32S3
**   CONFIG_IDF_TARGET_ESP32C3
**   CONFIG_IDF_TARGET_ESP32C6
*/

#define FP_MAX_BITS (4096 * 2)

#define HAVE_ALPN

/* Not yet using WiFi lib, so don't compile in the esp-sdk-lib WiFi helpers: */
/* #define USE_WOLFSSL_ESP_SDK_WIFI */

/*
 * ONE of these Espressif chip families will be detected from sdkconfig:
 *
 * WOLFSSL_ESP32
 * WOLFSSL_ESPWROOM32SE
 * WOLFSSL_ESP8266
 *
 * following ifdef detection only for syntax highlighting:
 */
#ifdef WOLFSSL_ESPWROOM32SE
#undef WOLFSSL_ESPWROOM32SE
#endif
#ifdef WOLFSSL_ESP8266
#undef WOLFSSL_ESP8266
#endif
#ifdef WOLFSSL_ESP32
#undef WOLFSSL_ESP32
#endif
/* See below for chipset detection from sdkconfig.h */

/* when you want to use SINGLE THREAD. Note Default ESP-IDF is FreeRTOS */
#define SINGLE_THREADED

/* Small session cache saves a lot of RAM for ClientCache and SessionCache.
 * Memory requirement is about 5KB, otherwise 20K is needed when not specified.
 * If extra small footprint is needed, try MICRO_SESSION_CACHE (< 1K)
 * When really desperate or no TLS used, try NO_SESSION_CACHE.  */
#define NO_SESSION_CACHE
#define NO_CLIENT_CACHE

/* Small Stack uses more heap. */
#define WOLFSSL_SMALL_STACK

/* Full debugging turned off, but show malloc failure detail */
// #define DEBUG_WOLFSSL_MALLOC

/* RSA_LOW_MEM: Half as much memory but twice as slow. */
#define RSA_LOW_MEM

/* optionally turn off SHA512/224 SHA512/256 */
/* #define WOLFSSL_NOSHA512_224 */
/* #define WOLFSSL_NOSHA512_256 */

/* when you want to use SINGLE THREAD. Note Default ESP-IDF is FreeRTOS */
/* #define SINGLE_THREADED */

/* When you don't want to use the old SHA */
/* #define NO_SHA */
/* #define NO_OLD_TLS */

// #define BENCH_EMBEDDED

/* TLS 1.3                                 */
#ifdef CONFIG_WOLFSSL_ALLOW_TLS13
#define WOLFSSL_TLS13
#define HAVE_TLS_EXTENSIONS
#define HAVE_HKDF

/* Server Name Indication - needed for any server that hosts multiple
 * sites on one IP (e.g. CDNs like Cloudflare); not tied to esp-tls. */
#define HAVE_SNI

/* May be required */
#ifndef HAVE_AEAD
#endif

/* Required for ECC */
#define HAVE_SUPPORTED_CURVES

/* Required for RSA */
#define WC_RSA_PSS

/* TLS 1.3 normally requires HAVE_FFDHE */
#if defined(HAVE_FFDHE_2048) || defined(HAVE_FFDHE_3072) ||     \
	defined(HAVE_FFDHE_4096) || defined(HAVE_FFDHE_6144) || \
	defined(HAVE_FFDHE_8192)
#else
#define HAVE_FFDHE_2048
/* #error "TLS 1.3 requires HAVE_FFDHE_[nnnn]" */
#endif
#endif

#if defined(CONFIG_IDF_TARGET_ESP32C2) || defined(CONFIG_IDF_TARGET_ESP8684)
/* Optionally set smaller size here */
#ifdef HAVE_FFDHE_4096
/* this size may be problematic on the C2 */
#endif
#define HAVE_FFDHE_2048
#else
#define HAVE_FFDHE_4096
#endif

#define NO_FILESYSTEM

#define NO_OLD_TLS

#define HAVE_AESGCM

/* Optional RIPEMD: RACE Integrity Primitives Evaluation Message Digest */
/* #define WOLFSSL_RIPEMD */

/* when you want to use SHA384 */
#define WOLFSSL_SHA384

/* when you want to use SHA512 */
#define WOLFSSL_SHA512

/* when you want to use SHA3 */
/* #define WOLFSSL_SHA3 */

/* ED25519 requires SHA512 */
#define HAVE_ED25519

#define MY_USE_ECC 1
#define MY_USE_RSA 0

/* We can use either or both ECC and RSA, but must use at least one. */
#if MY_USE_ECC || MY_USE_RSA
#if MY_USE_ECC
/* ---- ECDSA / ECC ---- */
#define HAVE_ECC
#define HAVE_CURVE25519
#define HAVE_ED25519
#define WOLFSSL_SHA512
/*
#define HAVE_ECC384
#define CURVE25519_SMALL
*/
#else
#define WOLFSSH_NO_ECC
/* WOLFSSH_NO_ECDSA is typically defined automatically,
 * here for clarity: */
#define WOLFSSH_NO_ECDSA
#endif

#if MY_USE_RSA
/* ---- RSA ----- */
/* #define RSA_LOW_MEM */

/* DH disabled by default, needed if ECDSA/ECC also turned off */
#define HAVE_DH
#else
#define WOLFSSH_NO_RSA
#endif
#else
#error "Either RSA or ECC must be enabled"
#endif

/* Optional OpenSSL compatibility */
/* #define OPENSSL_EXTRA */

/* #Optional HAVE_PKCS7 */
/* #define HAVE_PKCS7 */

#if defined(HAVE_PKCS7)
/* HAVE_PKCS7 may enable HAVE_PBKDF2 see settings.h */
#define NO_PBKDF2

#define HAVE_AES_KEYWRAP
#define HAVE_X963_KDF
#define WOLFSSL_AES_DIRECT
#endif

/* esp32-wroom-32se specific definition */
#if defined(WOLFSSL_ESPWROOM32SE)
#define WOLFSSL_ATECC508A
#define HAVE_PK_CALLBACKS
/* when you want to use a custom slot allocation for ATECC608A */
/* unless your configuration is unusual, you can use default   */
/* implementation.                                             */
/* #define CUSTOM_SLOT_ALLOCATION                              */
#endif

/* Adjust wait-timeout count if you see timeout in RSA HW acceleration.
 * Set to very large number and enable WOLFSSL_HW_METRICS to determine max. */
#ifndef ESP_RSA_TIMEOUT_CNT
#define ESP_RSA_TIMEOUT_CNT 0xFF0000
#endif

/* USE_FAST_MATH is default */
#define USE_FAST_MATH

/*****      Use SP_MATH      *****/
/* #undef  USE_FAST_MATH         */
/* #define SP_MATH               */
/* #define WOLFSSL_SP_MATH_ALL   */
/* #define WOLFSSL_SP_RISCV32    */

/***** Use Integer Heap Math *****/
/* #undef USE_FAST_MATH          */
/* #define USE_INTEGER_HEAP_MATH */

/* Just syntax highlighting to check math libraries: */
#if defined(SP_MATH) || defined(USE_INTEGER_HEAP_MATH) ||           \
	defined(USE_INTEGER_HEAP_MATH) || defined(USE_FAST_MATH) || \
	defined(WOLFSSL_SP_MATH_ALL) || defined(WOLFSSL_SP_RISCV32)
#endif

#define HAVE_VERSION_EXTENDED_INFO
/* #define HAVE_WC_INTROSPECTION */

#ifndef NO_SESSION_CACHE
#error "ERROR NO_SESSION_CACHE is not set"
#endif

#define WOLFSSL_ASN_TEMPLATE

/* Chipset detection from sdkconfig.h
 * Default is HW enabled unless turned off.
 * Uncomment lines to force SW instead of HW acceleration */
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(WOLFSSL_ESPWROOM32SE)
#define WOLFSSL_ESP32
/*  Alternatively, if there's an ECC Secure Element present: */
/* #define WOLFSSL_ESPWROOM32SE */

/* wolfSSL HW Acceleration supported on ESP32. Uncomment to disable: */
/*  #define NO_ESP32_CRYPT                 */
/*  #define NO_WOLFSSL_ESP32_CRYPT_HASH    */
/*  #define NO_WOLFSSL_ESP32_CRYPT_AES     */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MP_MUL  */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MULMOD  */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_EXPTMOD */

/*  These are defined automatically in esp32-crypt.h, here for clarity:  */
#define NO_WOLFSSL_ESP32_CRYPT_HASH_SHA224 /* no SHA224 HW on ESP32  */

#undef ESP_RSA_MULM_BITS
#define ESP_RSA_MULM_BITS 16 /* TODO add compile-time warning */
/***** END CONFIG_IDF_TARGET_ESP32 *****/

#elif defined(CONFIG_IDF_TARGET_ESP32S2)
#define WOLFSSL_ESP32
/* wolfSSL HW Acceleration supported on ESP32-S2. Uncomment to disable: */
/*  #define NO_ESP32_CRYPT                 */
/*  #define NO_WOLFSSL_ESP32_CRYPT_HASH    */
/* Note: There's no AES192 HW on the ESP32-S2; falls back to SW */
/*  #define NO_WOLFSSL_ESP32_CRYPT_AES     */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MP_MUL  */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MULMOD  */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_EXPTMOD */
/***** END CONFIG_IDF_TARGET_ESP32S2 *****/

#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define WOLFSSL_ESP32
/* wolfSSL HW Acceleration supported on ESP32-S3. Uncomment to disable: */
/*  #define NO_ESP32_CRYPT                         */
/*  #define NO_WOLFSSL_ESP32_CRYPT_HASH            */
/* Note: There's no AES192 HW on the ESP32-S3; falls back to SW */
/*  #define NO_WOLFSSL_ESP32_CRYPT_AES             */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI         */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MP_MUL  */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MULMOD  */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_EXPTMOD */
/***** END CONFIG_IDF_TARGET_ESP32S3 *****/

#elif defined(CONFIG_IDF_TARGET_ESP32C2) || defined(CONFIG_IDF_TARGET_ESP8684)
#define WOLFSSL_ESP32
/* ESP8684 is essentially ESP32-C2 chip + flash embedded together in a
 * single QFN 4x4 mm package. Out of released documentation, Technical
 * Reference Manual as well as ESP-IDF Programming Guide is applicable
 * to both ESP32-C2 and ESP8684.
 *
 * See:
 * https://www.esp32.com/viewtopic.php?f=5&t=27926#:~:text=ESP8684%20is%20essentially%20ESP32%2DC2,both%20ESP32%2DC2%20and%20ESP8684.
 */

/* wolfSSL HW Acceleration supported on ESP32-C2. Uncomment to disable: */
/*  #define NO_ESP32_CRYPT                 */
/*  #define NO_WOLFSSL_ESP32_CRYPT_HASH    */ /* to disable all SHA HW   */

/* These are defined automatically in esp32-crypt.h, here for clarity    */
#define NO_WOLFSSL_ESP32_CRYPT_HASH_SHA384 /* no SHA384 HW on C2  */
#define NO_WOLFSSL_ESP32_CRYPT_HASH_SHA512 /* no SHA512 HW on C2  */

/* There's no AES or RSA/Math accelerator on the ESP32-C2
 * Auto defined with NO_WOLFSSL_ESP32_CRYPT_RSA_PRI, for clarity: */
#define NO_WOLFSSL_ESP32_CRYPT_AES
#define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI
#define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MP_MUL
#define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MULMOD
#define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_EXPTMOD
/***** END CONFIG_IDF_TARGET_ESP32C2 *****/

#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#define WOLFSSL_ESP32
/* wolfSSL HW Acceleration supported on ESP32-C3. Uncomment to disable: */

/*  #define NO_ESP32_CRYPT                 */
/*  #define NO_WOLFSSL_ESP32_CRYPT_HASH    */ /* to disable all SHA HW   */

/* These are defined automatically in esp32-crypt.h, here for clarity:  */
#define NO_WOLFSSL_ESP32_CRYPT_HASH_SHA384 /* no SHA384 HW on C6  */
#define NO_WOLFSSL_ESP32_CRYPT_HASH_SHA512 /* no SHA512 HW on C6  */

/*  #define NO_WOLFSSL_ESP32_CRYPT_AES             */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI         */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MP_MUL  */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MULMOD  */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_EXPTMOD */
/***** END CONFIG_IDF_TARGET_ESP32C3 *****/

#elif defined(CONFIG_IDF_TARGET_ESP32C6)
#define WOLFSSL_ESP32
/* wolfSSL HW Acceleration supported on ESP32-C6. Uncomment to disable: */

/*  #define NO_ESP32_CRYPT                 */
/*  #define NO_WOLFSSL_ESP32_CRYPT_HASH    */
/*  These are defined automatically in esp32-crypt.h, here for clarity:  */
#define NO_WOLFSSL_ESP32_CRYPT_HASH_SHA384 /* no SHA384 HW on C6  */
#define NO_WOLFSSL_ESP32_CRYPT_HASH_SHA512 /* no SHA512 HW on C6  */

/*  #define NO_WOLFSSL_ESP32_CRYPT_AES             */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI         */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MP_MUL  */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_MULMOD  */
/*  #define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI_EXPTMOD */
/***** END CONFIG_IDF_TARGET_ESP32C6 *****/

#elif defined(CONFIG_IDF_TARGET_ESP32H2)
#define WOLFSSL_ESP32
/*  wolfSSL Hardware Acceleration not yet implemented */
#define NO_ESP32_CRYPT
#define NO_WOLFSSL_ESP32_CRYPT_HASH
#define NO_WOLFSSL_ESP32_CRYPT_AES
#define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI
/***** END CONFIG_IDF_TARGET_ESP32H2 *****/

#elif defined(CONFIG_IDF_TARGET_ESP32P4)
#define WOLFSSL_ESP32
/*  wolfSSL Hardware Acceleration not yet implemented */
#define NO_ESP32_CRYPT
#define NO_WOLFSSL_ESP32_CRYPT_HASH
#define NO_WOLFSSL_ESP32_CRYPT_AES
#define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI
/***** END CONFIG_IDF_TARGET_ESP32P4 *****/

#elif defined(CONFIG_IDF_TARGET_ESP8266)
#define WOLFSSL_ESP8266

/* There's no hardware encryption on the ESP8266 */
/* Consider using the ESP32-C2/C3/C6             */
#define NO_ESP32_CRYPT
#define NO_WOLFSSL_ESP32_CRYPT_HASH
#define NO_WOLFSSL_ESP32_CRYPT_AES
#define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI
#ifndef FP_MAX_BITS
/* FP_MAX_BITS matters in wolfssl_test, not just TLS setting.   */
/* MIN_FFDHE_FP_MAX_BITS = (MIN_FFDHE_BITS * 2); see settings.h */
#define FP_MAX_BITS MIN_FFDHE_FP_MAX_BITS
#endif
/***** END CONFIG_IDF_TARGET_ESP266 *****/

#elif defined(CONFIG_IDF_TARGET_ESP8684)
/*  There's no Hardware Acceleration available on ESP8684 */
#define NO_ESP32_CRYPT
#define NO_WOLFSSL_ESP32_CRYPT_HASH
#define NO_WOLFSSL_ESP32_CRYPT_AES
#define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI
/***** END CONFIG_IDF_TARGET_ESP8684 *****/

#else
/* Anything else encountered, disable HW acceleration */
#warning "Unexpected CONFIG_IDF_TARGET_NN value"
#define NO_ESP32_CRYPT
#define NO_WOLFSSL_ESP32_CRYPT_HASH
#define NO_WOLFSSL_ESP32_CRYPT_AES
#define NO_WOLFSSL_ESP32_CRYPT_RSA_PRI
#endif /* CONFIG_IDF_TARGET Check */

/* RSA primitive specific definition, listed AFTER the Chipset detection */
#if defined(WOLFSSL_ESP32) || defined(WOLFSSL_ESPWROOM32SE)
/* Consider USE_FAST_MATH and SMALL_STACK                        */

#ifndef NO_RSA
#define ESP32_USE_RSA_PRIMITIVE

#ifdef CONFIG_ESP_MAIN_TASK_STACK_SIZE
/* See idf.py menuconfig for stack warning settings */
#if !defined(CONFIG_ESP_WOLFSSL_NO_STACK_SIZE_BUILD_WARNING)
#if CONFIG_ESP_MAIN_TASK_STACK_SIZE < 10500
#warning "RSA may be difficult with less than 10KB Stack"
#endif
#else
/* Implement your own stack warning here */
#endif
#endif

#if defined(CONFIG_IDF_TARGET_ESP32)
/* NOTE HW unreliable for small values! */
/* threshold for performance adjustment for HW primitive use   */
/* X bits of G^X mod P greater than                            */
#undef ESP_RSA_EXPT_XBITS
#define ESP_RSA_EXPT_XBITS 32

/* X and Y of X * Y mod P greater than                         */
#undef ESP_RSA_MULM_BITS
#define ESP_RSA_MULM_BITS 16
#endif
#endif
#endif

/* Debug options:
See wolfssl/wolfcrypt/port/Espressif/esp32-crypt.h for details on debug options

optionally increase error message size for very long paths.
#define WOLFSSL_MAX_ERROR_SZ 500

Turn wolfSSL debugging on/off:
    wolfSSL_Debugging_ON();
    wolfSSL_Debugging_OFF();
*/
// #define ESP_VERIFY_MEMBLOCK
// #define DEBUG_WOLFSSL
// #define DEBUG_WOLFSSL_VERBOSE
// #define DEBUG_WOLFSSL_SHA_MUTEX
// #define WOLFSSL_DEBUG_IGNORE_ASN_TIME
// #define WOLFSSL_DEBUG_CERT_BUNDLE
// #define WOLFSSL_DEBUG_CERT_BUNDLE_NAME
// #define WOLFSSL_ESP32_CRYPT_DEBUG
// #define WOLFSSL_ESP32_CRYPT_HASH_SHA224_DEBUG
// #define NO_RECOVER_SOFTWARE_CALC
// #define WOLFSSL_TEST_STRAY 1
// #define USE_ESP_DPORT_ACCESS_READ_BUFFER
// #define WOLFSSL_ESP32_HW_LOCK_DEBUG
// #define WOLFSSL_DEBUG_MUTEX
// #define WOLFSSL_DEBUG_ESP_RSA_MULM_BITS
// #define WOLFSSL_DEBUG_ESP_HW_MOD_RSAMAX_BITS
// #define WOLFSSL_DEBUG_ESP_HW_MULTI_RSAMAX_BITS
// #define ESP_DISABLE_HW_TASK_LOCK
// #define ESP_MONITOR_HW_TASK_LOCK
// #define USE_ESP_DPORT_ACCESS_READ_BUFFER
/*
See wolfcrypt/benchmark/benchmark.c for debug and other settings:

Turn on benchmark timing debugging (CPU Cycles, RTOS ticks, etc)
#define DEBUG_WOLFSSL_BENCHMARK_TIMING

Turn on timer debugging (used when CPU cycles not available)
#define WOLFSSL_BENCHMARK_TIMER_DEBUG
*/

/* Pause in a loop rather than exit. */
/* #define WOLFSSL_ESPIDF_ERROR_PAUSE */
/* #define WOLFSSL_ESP32_HW_LOCK_DEBUG */

#define WOLFSSL_HW_METRICS

// #define USE_FAST_MATH
// #define FP_MAX_BITS (4096 * 2) /* RSA-4096 CA */
// #define ALT_ECC_SIZE           /* ECC keeps small bignums */

/******************************************************************************
** Sanity Checks
******************************************************************************/
#if defined(CONFIG_ESP_MAIN_TASK_STACK_SIZE)
#if defined(WOLFCRYPT_HAVE_SRP)
#if defined(FP_MAX_BITS)
#if FP_MAX_BITS < (8192 * 2)
#define ESP_SRP_MINIMUM_STACK_8K (24 * 1024)
#else
#define ESP_SRP_MINIMUM_STACK_8K (28 * 1024)
#endif
#else
#error "Please define FP_MAX_BITS when using WOLFCRYPT_HAVE_SRP."
#endif

#if (CONFIG_ESP_MAIN_TASK_STACK_SIZE < ESP_SRP_MINIMUM_STACK)
#warning "WOLFCRYPT_HAVE_SRP enabled with small stack size"
#endif
#endif
#else
#warning "CONFIG_ESP_MAIN_TASK_STACK_SIZE not defined!"
#endif

/* esp_mp_mulmod declares 6 x MATH_INT_T on stack; with DEBUG_WOLFSSL
 * it is ~7.5 kB in one frame. Default 10500 is not enough. */
#if defined(DEBUG_WOLFSSL) && defined(CONFIG_ESP_MAIN_TASK_STACK_SIZE)
#if CONFIG_ESP_MAIN_TASK_STACK_SIZE < 24576
#error "DEBUG_WOLFSSL needs CONFIG_ESP_MAIN_TASK_STACK_SIZE >= 24576"
#endif
#endif

/* See settings.h for some of the possible hardening options:
 *
 *  #define NO_ESPIDF_DEFAULT
 *  #define WC_NO_CACHE_RESISTANT
 *  #define WC_AES_BITSLICED
 *  #define HAVE_AES_ECB
 *  #define HAVE_AES_DIRECT
 */

#define HAVE_DILITHIUM /* ML-DSA / FIPS 204 */
#define WOLFSSL_WC_DILITHIUM /* wolfCrypt impl, not liboqs */
#define WOLFSSL_DILITHIUM_VERIFY_ONLY /* client only verifies — big saving */

/* SHAKE256 is a hard dependency */

#define WOLFSSL_HAVE_MLKEM
#define WOLFSSL_SHAKE128
#define WOLFSSL_SHA3
#define WOLFSSL_SHAKE256

/* memory tuning, matters a lot on ESP32 */
#define WOLFSSL_DILITHIUM_SMALL_MEM
#define WOLFSSL_DILITHIUM_VERIFY_SMALL_MEM
#define WOLFSSL_DILITHIUM_DYNAMIC_KEYS /* right-sizes key buffers */

#define WOLFSSL_EXPERIMENTAL_SETTINGS
#define HAVE_FALCON

#define WOLFSSL_HAVE_SLHDSA
#define WOLFSSL_SLHDSA_SHA2

#ifdef HAVE_SESSION_TICKET
#error "Defined session tickets"
#endif

#ifdef SESSION_CERTS
#error "Defined session certs"
#endif

#ifdef KEEP_PEER_CERT
#error "Defined keep peer cert"
#endif
//#define DEBUG_WOLFSSL
