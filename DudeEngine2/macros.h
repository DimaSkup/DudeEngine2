/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   macros.h
    Desc:       just declarations of different macroses
    Created:    21.05.2026 by DimaSkup
\**********************************************************************************/
#ifndef MACROS_H
#define MACROS_H


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define CONCAT(a, b) a##b

#define STATIC_ASSERT(cond) \
    typedef char CONCAT(static_assertion_, __LINE__)[(cond) ? 1 : -1]

//---------------------------------------------------------
// convert 4 uint8 values (rgba) into a single uint32 value (abgr)
//---------------------------------------------------------
#define RGBATOU32(r, g, b, a) \
    (((a) << 24) | ((b) << 16) | ((g) << 8) | (r))

//---------------------------------------------------------
// Angles convertion
//---------------------------------------------------------
#define DEG_TO_RAD( angle )	  ( ( angle )  *PIOVER180 )
#define RAD_TO_DEG( radians ) ( ( radians )*PIUNDER180 )

// --------------------------------------------------------
// Desc:   check if input value is power of 2
// Args:   - value: a value to check
// Ret:    - true if is power of 2
// --------------------------------------------------------
#define IS_POW2(x) (((x) != 0) && !((x) & ((x) - 1)))

//---------------------------------------
// swapping values with XOR (NOTE: works only for INTEGER values)
//---------------------------------------
#define SWAP(a, b) ((a ^= b), (b ^= a), (a ^= b))

//---------------------------------------
// Desc:   helpers to get a square/cube of input value
//---------------------------------------
#define SQR(n)  ((n)*(n))
#define CUBE(n) ((n)*(n)*(n))


#endif
