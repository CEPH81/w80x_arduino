/*
 * inttypes.h - la libreria C del compilador (minilibc) no la trae.
 * Macros de formato para printf/scanf (int32_t es "int" en este compilador).
 */
#ifndef _W80X_INTTYPES_H_
#define _W80X_INTTYPES_H_

#include <stdint.h>

#define PRId8  "d"
#define PRIi8  "i"
#define PRIu8  "u"
#define PRIx8  "x"
#define PRIX8  "X"
#define PRId16 "d"
#define PRIi16 "i"
#define PRIu16 "u"
#define PRIx16 "x"
#define PRIX16 "X"
#define PRId32 "d"
#define PRIi32 "i"
#define PRIu32 "u"
#define PRIx32 "x"
#define PRIX32 "X"
#define PRIo32 "o"
#define PRId64 "lld"
#define PRIi64 "lli"
#define PRIu64 "llu"
#define PRIx64 "llx"
#define PRIX64 "llX"
#define PRIdPTR "d"
#define PRIuPTR "u"
#define PRIxPTR "x"

#define SCNd8  "hhd"
#define SCNu8  "hhu"
#define SCNd16 "hd"
#define SCNu16 "hu"
#define SCNx16 "hx"
#define SCNd32 "d"
#define SCNu32 "u"
#define SCNx32 "x"


#endif
