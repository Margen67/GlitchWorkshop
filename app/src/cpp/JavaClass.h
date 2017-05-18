//
// Created by chris on 1/8/17.
//

#ifndef IMAGEBENDER_JAVACLASS_H
#define IMAGEBENDER_JAVACLASS_H

#include "CSApplication.h"

class JavaClass {
#ifdef String_t
#undef String_t
#endif


    cloture::util::string::String_t classpath;
    jclass classz;
public:
    JavaClass(class CSApplication& env, cloture::util::string::String_t& classpath);
    JavaClass(class CSApplication& env, cloture::util::string::String_t&& classpath);
    JavaClass(class CSApplication& env, const char* classpath);
    JavaClass(JavaClass& other);
    ~JavaClass();
    FORCEINLINE jclass getClass() {
        return classz;
    }
    FORCEINLINE cloture::util::string::String_t getClasspath() {
        return classpath;
    }
    FORCEINLINE cloture::util::string::String_t getSignature() {
        return (cloture::util::string::String_t("L") + classpath).append(";");
    }
    template<typename T, typename... ts>
    FORCEINLINE jmethodID getMethod(JNIEnv* env, const char* methodName, T returnType, ts... parms);

    template<typename T, typename... ts>
    FORCEINLINE jmethodID getStaticMethod(JNIEnv* env, const char* methodName, T returnType, ts... parms);
};


template<typename T>
__forceinline inline cloture::util::string::String_t parmString(T parm, ...) {
    //static_assert(false, "Should not be instantiated.");
    return cloture::util::string::String_t("");
}
template<>
__forceinline inline cloture::util::string::String_t parmString<cloture::util::string::String_t>(cloture::util::string::String_t parm, ...) {
    return parm;
}
template<>
__forceinline inline cloture::util::string::String_t parmString<JavaClass*>(JavaClass* parm, ...) {
    return parm->getSignature();
}

template<>
__forceinline inline cloture::util::string::String_t parmString<char>(char parm, ...) {
    //char str[] = {(char)parm, 0};

    return cloture::util::string::String_t(parm);
}



template<size_t sz, typename T1, typename... ts>
struct parmListBuilder {
    __forceinline static cloture::util::string::String_t buildParameterList(T1 t1, ts... rest) {
        static_if(sizeof...(ts) > 2)
        {
            return parmString(t1) + /*parmString(rest...) +*/ parmListBuilder<sizeof...(ts), ts...>::buildParameterList(rest...);
        }
        static_else(sizeof...(ts) > 2)
        {
            static_if(sizeof...(ts) > 1)
            {
                return parmString(t1) + parmListBuilder<sizeof...(ts), ts...>::buildParameterList(rest...);//parmString(rest...);
            }
            static_else(sizeof...(ts) > 1)
            {
                static_if(sizeof...(ts) > 0)
                {
                    return parmString(t1) + parmString(rest...);
                }
                static_else(sizeof...(ts) > 0)
                {
                    return parmString(t1);
                }
            }
        }
    }
};

template<typename T1>
struct parmListBuilder<1, T1> {
    __forceinline static cloture::util::string::String_t buildParameterList(T1 t1) {
        return parmString(t1);
    }
};

template<size_t args, typename... ts>
struct parmListBuilder_ {
    __forceinline static cloture::util::string::String_t buildParameterList_(ts... Ts) {
        return parmListBuilder<args, ts...>::buildParameterList(Ts...);
    }
};

template<>
struct parmListBuilder_<0> {
    __forceinline static cloture::util::string::String_t buildParameterList_() {
        return cloture::util::string::String_t("");
    }
};


extern char* sigBuffer;

template<typename T, typename... ts>
inline const char* buildJavaMethodSignature(T returnType, ts... params) {
    using cloture::util::string::String_t;

    if(sigBuffer) {
        //delete[] sigBuffer;
        free(sigBuffer);
        sigBuffer = nullptr;
    }

    String_t args = String_t("(") + parmListBuilder_<sizeof...(ts), ts...>::buildParameterList_(params...).append(")");
    String_t result = args + parmString(returnType);
    char* s = strdup(result.data);
    sigBuffer = s;

    return s;
}

template<typename T, typename... ts>
FORCEINLINE jmethodID JavaClass::getMethod(JNIEnv* env, const char* methodName, T returnType, ts... parms) {
    return env->GetMethodID(classz, methodName, buildJavaMethodSignature(returnType, parms...));
}

template<typename T, typename... ts>
FORCEINLINE jmethodID JavaClass::getStaticMethod(JNIEnv* env, const char* methodName, T returnType, ts... parms) {
    return env->GetStaticMethodID(classz, methodName, buildJavaMethodSignature(returnType, parms...));
}



#endif //IMAGEBENDER_JAVACLASS_H
