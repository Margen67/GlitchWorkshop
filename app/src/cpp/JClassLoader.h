//
// Created by chris on 1/9/17.
//

#ifndef IMAGEBENDER_JCLASSLOADER_H
#define IMAGEBENDER_JCLASSLOADER_H
class CSApplication;

class JClassLoader {
    JavaClass* classLoaderClass; //what a name, eh?
    jobject systemClassloader;
    jmethodID loadClassMethod;
public:
    JClassLoader(CSApplication& env);
    bool loadClass(JNIEnv* env, const char* name);
};


#endif //IMAGEBENDER_JCLASSLOADER_H
