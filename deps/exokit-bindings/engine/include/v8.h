#if __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
// iOS
#include "../chakra/v8.h"
#else
// Other kinds of Mac OS
#error bad platform
#include "../v8/v8.h"
#endif
#else
// Non-apple platform
#error bad platform
#include "../v8/v8.h"
#endif
