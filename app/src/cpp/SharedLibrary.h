#pragma once
namespace cs {
    class SharedLibrary {
        char* m_moduleName = nullptr;
        void* m_handle = nullptr;

        void* getSymbol_(const char* name) __nonull_args(2);
    public:
        SharedLibrary(const char* name, bool exitOnError = true) __nonull_args(2);
        ~SharedLibrary();

        template<typename T>
        __forceinline T getSymbol(const char* name) __nonull_args(2) {
            return reinterpret_cast<T>(getSymbol_(name));
        }

        template<typename T, typename...args>
        __forceinline T operator ()(const char* name, args... Args) __nonull_args(2) {
            return getSymbol<T (*)(args...)>(name)(Args...);
        };

        __forceinline operator bool() const {
            return bool(m_moduleName);
        }

        __forceinline bool operator !() const {
            return !static_cast<bool>(*this);
        }

        __forceinline const char* getModuleName() const {
            return m_moduleName;
        }


    };
}