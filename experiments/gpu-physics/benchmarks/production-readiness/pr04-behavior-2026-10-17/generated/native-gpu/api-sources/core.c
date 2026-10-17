// SPDX-FileCopyrightText: 2025 Erin Catto
// SPDX-License-Identifier: MIT

#if defined( B3_COMPILER_MSVC )
// CRTDBG requires these to be included first
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#else
#include <stdlib.h>
#endif

#include "core.h"
#include "rapidhash.h"

#include "box3d/constants.h"
#include "box3d/math_functions.h"

#include <stdarg.h>
#include <string.h>

#ifdef BOX2D_PROFILE

#include <tracy/TracyC.h>
#define b3TracyCAlloc( ptr, size ) TracyCAlloc( ptr, size )
#define b3TracyCFree( ptr ) TracyCFree( ptr )

#else

#define b3TracyCAlloc( ptr, size )
#define b3TracyCFree( ptr )

#endif

#include "platform.h"

#include <stdio.h>

// This allows the user to change the length units at runtime
static float b3_lengthUnitsPerMeter = 1.0f;












static float b3_stallThreshold = FLT_MAX;












static int b3DefaultAssertFcn( const char* condition, const char* fileName, int lineNumber )
{
	printf( "BOX3D ASSERTION: %s, %s, line %d\n", condition, fileName, lineNumber );

	// return non-zero to break to debugger
	return 1;
}

b3AssertFcn* b3AssertHandler = b3DefaultAssertFcn;







#if !defined( NDEBUG ) || defined( B3_ENABLE_ASSERT )









#endif

static void b3DefaultLogFcn( const char* message )
{
	printf( "Box3D: %s\n", message );
}

b3LogFcn* b3LogHandler = b3DefaultLogFcn;































static b3AllocFcn* b3_allocFcn = NULL;
static b3FreeFcn* b3_freeFcn = NULL;

b3AtomicInt b3_byteCount;







































































































// Not used. Keeping around in case I need this.





























