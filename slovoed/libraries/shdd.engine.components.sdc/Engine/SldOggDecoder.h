#ifndef _SLD_OGG_DECODER_H_
#define _SLD_OGG_DECODER_H_

#include "SldPlatform.h"
#include "SldTypes.h"
#include "ISldLayerAccess.h"
#include "SldError.h"


/// Производит декодирование озвучки в формате mp3
ESldError OggDecode(ISldLayerAccess* aLayerAccess, FSoundBuilderMethodPtr aBuilderPtr, const UInt8* aDataPtr, UInt32 aDataSize, UInt8 aIsLast, UInt32* aStartPos);

#endif //_SLD_MP3_DECODER_H_
