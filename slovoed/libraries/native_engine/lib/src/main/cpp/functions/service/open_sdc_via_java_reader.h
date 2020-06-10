#ifndef OPEN_SDC_VIA_JAVA_READER_H
#define OPEN_SDC_VIA_JAVA_READER_H

#include "../byte_array_function.h"

class OpenSdcViaJavaReader : public CByteArrayFunction
{
  protected:
    jobject
    native( JNIEnv* env, jbyteArray array, jobject isOpenedCallback, jobject readCallback, jobject getSizeCallback , jobject cacheDir) const noexcept override;
};

#endif
