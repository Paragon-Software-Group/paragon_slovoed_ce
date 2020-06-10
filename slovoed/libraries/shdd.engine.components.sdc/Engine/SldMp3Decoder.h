#ifndef _SLD_MP3_DECODER_H_
#define _SLD_MP3_DECODER_H_

#include "SldPlatform.h"
#include "SldTypes.h"
#include "ISldLayerAccess.h"
#include "SldError.h"


/// Производит декодирование озвучки в формате mp3
ESldError Mp3Decode(ISldLayerAccess* aLayerAccess, FSoundBuilderMethodPtr aBuilderPtr, const UInt8* aDataPtr, UInt32 aDataSize, UInt8 aIsLast, UInt32* aStartPos);

#endif //_SLD_MP3_DECODER_H_
