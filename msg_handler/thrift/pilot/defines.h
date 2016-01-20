#ifndef __DEFINES_H__
#define __DEFINES_H__
#include <sys/types.h>
#include <stdint.h>

//ms
#define CONN_TIMEOUT	500
#define	SEND_TIMEOUT	1000
#define	RECV_TIMEOUT	1000
#define	ZK_RECV_TIMEOUT	1000
#define UPDATE_TASK_POOL_SIZE	5
#define NUM_CLIENTS_PER_REPLICA	16

#define DEFAULT_NAMESPACE	"/jdns"

/**
* Protocol that a service uses.
*/
#define TBINARY_PROTOCOL	"TBinaryProtocol"
#define TCOMPACT_PROTOCOL	"TCompactProtocol"
#define TJSON_PROTOCOL		"TJSONProtocol"
#define TDENSE_PROTOCOL		"TDenseProtocol"
#define TDEBUG_PROTOCOL		"TDebugProtocol"

/**
* Used by TZException to identify one error.
*/
const int32_t TZE_ZKC_OPENFAIL			=1;
const int32_t TZE_ZKC_DUPNODE			=2;
const int32_t TZE_ZKC_CREATENODEFAIL	=3;
const int32_t TZE_THB_OPENFAIL			=10;
const int32_t TZE_REG_INVALIDARGS		=20;
const int32_t TZE_NOVALID_CLIENT		=30;
const int32_t TZE_GETADDRINFO_FAIL		=998;
const int32_t TZE_FATAL					=999;
const int32_t TZE_PROXY_INVOKE_FAILED	=1000;

#define MAX_ERROR_CNT_THRESHOLD	10

/**
* vvp = tvp + uvp
*/
#define timeval_add(tvp, uvp, vvp) \
do { \
	(vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec; \
	(vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec; \
	if ((vvp)->tv_usec >= 1000000) { \
		(vvp)->tv_sec++; \
		(vvp)->tv_usec -= 1000000; \
	} \
} while (0)

/**
* vvp = tvp - uvp
*/
#define timeval_sub(tvp, uvp, vvp) \
do { \
	(vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec; \
	(vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec; \
	if ((vvp)->tv_usec < 0) { \
		(vvp)->tv_sec--; \
		(vvp)->tv_usec += 1000000; \
	} \
} while (0)

/**
* init vvp
*/
#define timeval_init(vvp) \
do { \
	(vvp)->tv_sec = 0; \
	(vvp)->tv_usec = 0; \
} while (0) 

#endif
