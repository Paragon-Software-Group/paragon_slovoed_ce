#ifndef OPEN_SDC_VIA_DESCRIPTOR_H
#define OPEN_SDC_VIA_DESCRIPTOR_H

#include <java/java_objects.h>
#include "../byte_array_function.h"

#include "../../context/env_aware.h"

class OpenSdcViaDescriptor : public CByteArrayFunction
{
  public:
    static jobject OpenSdcHelper( JNIEnv* env, EnvAwareSdcReader* pFileReader, jbyteArray dictionaryContextPointer,  jobject _cacheDir  );

  protected:
    jobject
    native( JNIEnv* env, jbyteArray array, jobject descriptor, jobject offset, jobject size, jobject _cacheDir) const noexcept override;
};

#endif
