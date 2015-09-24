#pragma once

// disable the STL warnings
#pragma warning( disable:4503 )
#pragma warning( disable:4530 )
#pragma warning( disable:4786 )
#pragma warning( disable:4290 )

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <iostream>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


#include <atlbase.h>
#include <atlcom.h>
#include <atlcomtime.h>
#include <atlcomcli.h>
#include <comdef.h>


#include <iostream>
#include <ios>
#include <iomanip>
#include <locale>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <utility>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <functional>
#include <deque>

#include <BloombergApi/triplet.h>

#pragma warning (disable: 4091)
#define DISABLE_Py_DEBUG
#include <Python.h>
#pragma warning (default: 4091)
