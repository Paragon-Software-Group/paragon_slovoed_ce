#include "open_sdc_via_java_reader.h"
#include "open_sdc_via_descriptor.h"
#include "context/reader/java_reader.h"

jobject
OpenSdcViaJavaReader::native(
    JNIEnv* env,
    jbyteArray array,
    jobject isOpenedCallback,
    jobject readCallback,
    jobject getSizeCallback,
    jobject cacheDir
) const noexcept
{
  EnvAwareSdcReader* inputStreamReader = new JavaReader(
      env, isOpenedCallback, readCallback, getSizeCallback
  );
  return OpenSdcViaDescriptor::OpenSdcHelper( env, inputStreamReader, array, cacheDir );
}
