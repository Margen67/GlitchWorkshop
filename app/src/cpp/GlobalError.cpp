#include "util/stl.hpp"
#include <jni.h>
#include <glitch-common.h>
#include "JavaClass.h"

#include "CSApplication.h"
#include "GlobalError.h"
#include <cstdarg>
#include "android/log.h"


void cs::globalError(const char* fmt, ...) {
    char* s = new char[65536];
    memset(s, 0, 65536);
    va_list vlist;
    va_start(vlist, fmt);
    __builtin_vsprintf(s, fmt, vlist);
    getCSAPP()->fatalError(s);
}

void cs::globalLog(const char *fmt, ...) {
    va_list vlist;
    va_start(vlist, fmt);
    __android_log_vprint(ANDROID_LOG_ERROR, getCSAPP()->getTag(), fmt, vlist);
    va_end(vlist);

}