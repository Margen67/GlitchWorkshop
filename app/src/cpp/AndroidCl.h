#pragma once
namespace cs {
    class AndroidCl {
        cs::SharedLibrary& clLib;
        bool clAvailable = true;
        void* m_platform_id = nullptr;
        void* m_device_id = nullptr;
    public:
        using context_t = void*;
        using cmdqueue_t = void*;
        using program_t = void*;
        using mem_t = void*;
        AndroidCl();
        ~AndroidCl();
        context_t createContext();
        cmdqueue_t createCmdQueue(context_t ctx);
        program_t  createProgram(context_t ctx, const char* src);
        mem_t moveToMemory(context_t ctx, cmdqueue_t cmds, void* data, size_t sz);
    };
}