#ifndef _ASCII_TABLE_H_
#define _ASCII_TABLE_H_

#include "SldPlatform.h"
#include "SldTypes.h"
#include "SldError.h"

#define LETTER  0x00
#define SPACE   0x01
#define OTHER   0x02

ESldError InitASCIItable(TSoundLieralType * array);

#endif

