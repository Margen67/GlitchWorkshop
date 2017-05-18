#include "compiler_features.hpp"

#include "SharedLibrary.h"
#include "GlobalError.h"
#include <dlfcn.h>
#include <cstdlib>

using namespace cs;

SharedLibrary::SharedLibrary(const char *name, bool exitOnError) {
    m_moduleName = __builtin_strdup(name);
    m_handle = dlopen(name, RTLD_LAZY);
    if(!m_handle && exitOnError) {
        const char* errstr = dlerror();
        if(!errstr) {
            globalError("Failed to open dynamic library %s, but no error message is available!", m_moduleName);
        }
        globalError("Failed to open dynamic library %s! Error: (%s).", m_moduleName, errstr);
    }
}

SharedLibrary::~SharedLibrary() {
    if(m_handle)
        dlclose(m_handle);
    if(m_moduleName)
        free(m_moduleName);
}

void* SharedLibrary::getSymbol_(const char *name) {
    if(!m_handle)
        globalError("Couldn't obtain symbol %s in module %s! Module handle was null!", name, m_moduleName);

    return dlsym(m_handle, name);
}