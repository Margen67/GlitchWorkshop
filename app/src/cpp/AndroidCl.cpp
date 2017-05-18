#include <CL/cl.h>
#include "compiler_features.hpp"
#include "GlobalError.h"
#include "SharedLibrary.h"

#include "AndroidCl.h"
#include "CL/cl.h"
using namespace cs;

#define CLFUNC(sym)     auto _##sym = clLib.getSymbol<typeof(&sym)>(#sym)

#define CL_SYMBOL(sym)  \
    CLFUNC(sym); \
    if(!_##sym) { \
        clUnavailable("Failed to obtain symbol " #sym "."); \
        return; \
    }
#define errCheck(reason)        if(err != CL_SUCCESS)   {\
        clUnavailable(reason); \
        return; \
    }
AndroidCl::AndroidCl() :clLib(*(new cs::SharedLibrary("/system/vendor/lib/libOpenCL.so"))){
    int err = CL_SUCCESS;
    auto clUnavailable = [this](const char* reason) {
        this->clAvailable = false;
        cs::globalLog("Failed to initialize OpenCL. Reason: %s", reason);
    };

    if(!clLib) {
        clUnavailable("Failed to load OpenCL library.");
        return;
    }
    cl_platform_id plat_id = nullptr;

    CL_SYMBOL(clGetPlatformIDs);
    err = _clGetPlatformIDs(1, &plat_id, nullptr);
    errCheck("Couldn't obtain platform id!");
    cl_device_id device_id = nullptr;

    CL_SYMBOL(clGetDeviceIDs);

    err = _clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_GPU, 1, &device_id, nullptr);

    errCheck("Couldn't obtain device id!");
    m_platform_id = plat_id;
    m_device_id = device_id;

}

AndroidCl::~AndroidCl() {
    if(clLib) {
        delete &clLib;
    }
}

AndroidCl::context_t AndroidCl::createContext() {
    int err = CL_SUCCESS;
    CLFUNC(clCreateContext);
    cl_context ctx = _clCreateContext(0, 1, (cl_device_id*)&m_device_id, NULL, NULL, &err);
    if(!ctx || err != CL_SUCCESS)
        globalError("Failed to create OpenCL context!");
    return ctx;
}
AndroidCl::cmdqueue_t AndroidCl::createCmdQueue(context_t ctx) {
    CLFUNC(clCreateCommandQueue);
    int err = CL_SUCCESS;
    cl_command_queue queue = _clCreateCommandQueue((cl_context)ctx, (cl_device_id)m_device_id, 0, &err);
    if(!queue || err != CL_SUCCESS)
        globalError("Failed to create OpenCL command queue!");
    return queue;
}
AndroidCl::program_t AndroidCl::createProgram(context_t ctx, const char *src) {
    int err;
    CLFUNC(clCreateProgramWithSource);
    cl_program prog = _clCreateProgramWithSource((cl_context)ctx, 1, &src, nullptr, &err);
    if(!prog || err != CL_SUCCESS)
        globalError("Failed to create OpenCL program!");
    return prog;
}

AndroidCl::mem_t AndroidCl::moveToMemory(context_t ctx, cmdqueue_t cmds, void *data, size_t sz) {
    CLFUNC(clCreateBuffer);
    cl_mem input = _clCreateBuffer((cl_context)ctx, CL_MEM_READ_ONLY, sz, NULL, NULL);
    CLFUNC(clEnqueueWriteBuffer);
    int err = _clEnqueueWriteBuffer((cl_command_queue)cmds, input, CL_TRUE, 0, sz, data, 0, NULL, NULL);
    if(err != CL_SUCCESS)
        globalError("OpenCL failed to move memory to GPU!");
    return input;
}