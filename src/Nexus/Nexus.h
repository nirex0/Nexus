#ifndef _NEXUS_h_
#define _NEXUS_h_
#ifdef _MSC_VER 
// MS Visual Studio gives warnings when using 
// fopen. But fopen_s is not going to work well 
// with most compilers, and fopen_s uses different 
// syntax than fopen. (i.e., a macro won't work) 
// So, we'lll use this:
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <iostream>
#include <cmath>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>

#ifndef _Nexus_
#define _Nexus_

#ifdef __BCPLUSPLUS__ 
// The Borland compiler must use this because something
// is wrong with their cstdio file. 
#include <stdio.h>
#else 
#include <cstdio>
#endif

#ifdef __GNUC__
// If g++ specific code is ever required, this is 
// where it goes. 
#endif

#ifdef __INTEL_COMPILER
// If Intel specific code is ever required, this is 
// where it goes. 
#endif

#ifndef _DefaultXPelsPerMeter_
#define _DefaultXPelsPerMeter_
#define DefaultXPelsPerMeter 3780
// set to a default of 96 dpi 
#endif

#ifndef _DefaultYPelsPerMeter_
#define _DefaultYPelsPerMeter_
#define DefaultYPelsPerMeter 3780
// set to a default of 96 dpi
#endif

#include "Nexus_DataStructures.h"
#include "Nexus_Bitmap.h"
#include "Nexus_BitmapUtils.h"
#include "Nexus_EInjectionState.h"
#include "Nexus_Injector.h"
#include "Nexus_Entropy.h"
#include "Nexus_StringUtils.h"
#include "Nexus_Converter.h"
#include "Nexus_PNG.h"

#ifndef _Nexus_Version_
#define _Nexus_Version_ 1.2.2
#define _Nexus_Version_Integer_ 122
#define _Nexus_Version_String_ "1.2.2"
#endif

#ifndef _Nexuswarnings_
#define _Nexuswarnings_
#endif

void SetNexuswarningsOff( void );
void SetNexuswarningsOn( void );
bool GetNexuswarningState( void );

#endif
#endif // !_Nexus_h_
