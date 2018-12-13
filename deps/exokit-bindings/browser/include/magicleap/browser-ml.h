#ifndef _BROWSER_ML_H_
#define _BROWSER_ML_H_

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <list>
#include <functional>

#include <browser-common.h>
#include <magicleap/Servo2D.h>

using namespace std;
using namespace v8;
using namespace node;

namespace browser {
 

extern std::list<EmbeddedBrowser> browsers;

}

#endif
