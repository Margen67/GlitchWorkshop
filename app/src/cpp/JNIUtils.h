//
// Created by chris on 1/20/17.
//

#ifndef IMAGEBENDER_JNIUTILS_H
#define IMAGEBENDER_JNIUTILS_H

namespace cs {
    namespace JNIUtils {

        template<typename T, typename... ts>
        static __forceinline T callNonStaticMethod(JNIEnv* env, jobject obj, jmethodID methodID, ts... args) {
            using namespace cloture::util::generic;

            constexpr bool isObject = type_is_pointer(T);
            static_if(type_is_void(T)) {
                return env->CallVoidMethod(obj, methodID, args...);
            }
            static_else(type_is_void(T))
            {
                static_if(isObject)
                {
                    return env->CallObjectMethod(obj, methodID, args...);
                }
                static_else(isObject)
                {
                    static_if(type_is_boolean(T))
                    {
                        return env->CallBooleanMethod(obj, methodID, args...);
                    }
                    static_else(type_is_boolean(T))
                    {
                        static_if(type_is_char(T))
                        {
                            return env->CallCharMethod(obj, methodID, args...);
                        }
                        static_else(type_is_char(T))
                        {
                            static_if(sizeof(T) == 2)
                            {
                                return env->CallShortMethod(obj, methodID, args...);
                            }
                            static_if(sizeof(T) == 4)
                            {
                                return env->CallIntMethod(obj, methodID, args...);
                            }
                            static_if(sizeof(T) == 8)
                            {
                                return env->CallLongMethod(obj, methodID, args...);
                            }
                        }
                    }
                }
            }
        }
        template<typename T, typename... ts>
        static __forceinline T callStaticMethod(JNIEnv* env, jclass obj, jmethodID methodID, ts... args) {
            using namespace cloture::util::generic;

            constexpr bool isObject = type_is_pointer(T);
            static_if(type_is_void(T)) {
                return env->CallStaticVoidMethod(obj, methodID, args...);
            }
            static_else(type_is_void(T))
            {
                static_if(isObject)
                {
                    return env->CallStaticObjectMethod(obj, methodID, args...);
                }
                static_else(isObject)
                {
                    static_if(type_is_boolean(T))
                    {
                        return env->CallStaticBooleanMethod(obj, methodID, args...);
                    }
                    static_else(type_is_boolean(T))
                    {
                        static_if(type_is_char(T))
                        {
                            return env->CallStaticCharMethod(obj, methodID, args...);
                        }
                        static_else(type_is_char(T))
                        {
                            static_if(sizeof(T) == 2)
                            {
                                return env->CallStaticShortMethod(obj, methodID, args...);
                            }
                            static_if(sizeof(T) == 4)
                            {
                                return env->CallStaticIntMethod(obj, methodID, args...);
                            }
                            static_if(sizeof(T) == 8)
                            {
                                return env->CallStaticLongMethod(obj, methodID, args...);
                            }
                        }
                    }
                }
            }
        }
        template<typename T, typename T2, typename... ts>
        static __forceinline T callMethod(JNIEnv* env, T2 obj, jmethodID methodID, ts... args) {
            using namespace cloture::util::generic;
            constexpr bool isJavaClass = typesIdentical<T2, JavaClass*>();
            static_if(!isJavaClass) {
                return callNonStaticMethod<T>(env, obj, methodID, args...);
            }
            static_if(isJavaClass) {
                return callStaticMethod<T>(env, obj->getClass(), methodID, args...);
            }
        }


    }

}

#endif //IMAGEBENDER_JNIUTILS_H
