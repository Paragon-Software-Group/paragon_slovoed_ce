//
// Created by borbat on 01.10.2018.
//

#include <string>
#include <fcntl.h>
#include <zconf.h>
#include <java/java_objects.h>
#include <sys/stat.h>
#include <dirent.h>
#include "DataAccess.h"
#include <utime.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>


DataAccess::DataAccess( const CSldDictionary* pDictionary, const JavaObjects::string& _cacheDir )
{
  const CSldStyleInfo* styleInfo;
  sld2::IntFormatter fmt;
  sld2::html::StyleSerializer serializer;
  dictionary = const_cast<CSldDictionary *>(pDictionary);
  cacheDir = _cacheDir;
  for ( UInt32 idx = 0 ; idx < pDictionary->GetNumberOfStyles() ; idx++ )
  {
    ESldError error = pDictionary->GetStyleInfo( idx, &styleInfo );
    if ( error == eOK )
    {
      SldU16String styleString;
      styleString.assign( SldU16StringRefLiteral( ".S" ) );
      styleString.append( fmt.format( idx ) );
      styleString.append( SldU16StringRefLiteral( " { " ) );
      serializer.toString( styleString, styleInfo );
      styleString.append( SldU16StringRefLiteral( " }" ) );
      m_htmlStyles.emplace( idx, std::move( styleString ) );
    }
  }
  mHidePhonetics = false;
  mHideExamples = false;
  mHideImages = false;
  mHideIdioms = false;
  mHidePhrase = false;
}

void DataAccess::clear()
{
  usedStyles.clear();
}

SldU16StringRef DataAccess::GetStyleString( UInt32 styleId ) const
{
  auto&& it = m_htmlStyles.find( styleId );
  return it != m_htmlStyles.end() ? it->second : SldU16StringRef();
}

SldU16StringRef DataAccess::GetCSSStyleString( UInt32 styleId ) const
{
  auto&& it = m_cssStyles.find( styleId );
  return it != m_cssStyles.end() ? it->second : SldU16StringRef();
}

SldU16String DataAccess::ResolveImageUrl( UInt32 aIndex )
{

    TImageElement imageElement;
    ESldError error = dictionary->GetWordPicture(aIndex, 0xffff, &imageElement);
    if (error != eOK)
        return SldU16String();


    UInt32 imageType;
    TSizeValue width, height;
    error = dictionary->GetPictureInfo(&imageElement, &imageType, &width, &height);
    if (error != eOK)
        return SldU16String();

    JavaObjects::string path = createImageFile(&imageElement, imageType);
    if (path.empty())
        return SldU16String();

    SldU16String styleString( SldU16StringRefLiteral("file://") );
    styleString.append(path.data());

  return styleString;
}

SldU16String DataAccess::ResolveImageUrl( SldU16StringRef aDictId, UInt32 aListId, SldU16StringRef aKey )
{
  // Not load picture data right here to speed up translation loading (because full pictures could be pretty big).
  // Only pass parameters needed to read picture from ext base. These parameters should be handled just before opening a picture.
  sld2::IntFormatter fmt;
  SldU16String styleString;
  styleString.append(aDictId);
  styleString.append( SldU16StringRefLiteral( ":" ) );
  styleString.append( fmt.format(aListId) );
  styleString.append( SldU16StringRefLiteral( ":" ) );
  styleString.append( aKey );
  return styleString;
}

static void AppendStringHelper( JavaObjects::string& buffer, const char* toAdd )
{
    while ( *toAdd )
    {
        buffer.append( 1u, (UInt16) (unsigned char) *toAdd );
        toAdd++;
    }
}

// std::to_string doesn't supported
template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

template< typename T >
std::string int_to_hex( T i )
{
    std::stringstream stream;
    stream << "0x"
           << std::setfill ('0') << std::setw(sizeof(T)*2)
           << std::hex << i;
    return stream.str();
}

static void clearCache(const std::string &cacheDir, long long newFileSize) {
    long long maxCacheSize = 10 * 1024 * 1024;

    DIR *dir;
    struct dirent *ent;
    long long cacheDirSize = 0;
    struct stat fileStat;
    std::string oldestModifiedFile;
    long oldestTime = LONG_MAX;
    std::string fullFilePath;

    if ((dir = opendir(cacheDir.data())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name,  "..") == 0)
                continue;
            fullFilePath = cacheDir;
            fullFilePath += ("/");
            fullFilePath += ent->d_name;
            stat(fullFilePath.data(), &fileStat);
            cacheDirSize += fileStat.st_size;
            if (fileStat.st_mtim.tv_sec < oldestTime) {
                oldestTime = fileStat.st_mtim.tv_sec;
                oldestModifiedFile = fullFilePath;
            }
        }
        closedir(dir);
    } else {
        /* could not open directory */
        return;
    }

    if (cacheDirSize + newFileSize > maxCacheSize) {
        remove(oldestModifiedFile.data());
        clearCache(cacheDir, newFileSize);
    } else {
        return;
    }
}



JavaObjects::string DataAccess::createImageFile(const TImageElement * aImageElement, UInt32 aExtension)
{
    if (!aImageElement || !aImageElement->ImageIndex)
        return JavaObjects::string();

    JavaObjects::string path;
    path = cacheDir;
    std::string cache_s(path.begin(), path.end());

    AppendStringHelper(path, "/IMG-");
    UInt32 data=0;

    dictionary->GetDictionaryID(&data);
    AppendStringHelper(path, int_to_hex(data).data());
    AppendStringHelper(path, "-");
    data=0;
    dictionary->GetDictionaryMajorVersion(&data);
    AppendStringHelper(path, to_string(data).data());
    AppendStringHelper(path, "-");
    data=0;
    dictionary->GetDictionaryMinorVersion(&data);
    AppendStringHelper(path, to_string(data).data());
    AppendStringHelper(path, "-");
    AppendStringHelper(path,  to_string(aImageElement->ImageIndex).data());

    AppendStringHelper(path, dictionary->IsInApp()!=0?"demo":"full");

        switch(aExtension){
        case EPictureFormatType::ePictureFormatType_PNG:
            AppendStringHelper(path, ".png");
            break;
        case EPictureFormatType::ePictureFormatType_JPG:
            AppendStringHelper(path, ".jpg");
            break;
        case EPictureFormatType::ePictureFormatType_SVG:
            AppendStringHelper(path, ".svg");
            break;
        default:break;
    }

    int pfd;
    std::string path_s(path.begin(), path.end());
    if ((pfd = open(path_s.data(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
    {
        utime(path_s.data(), nullptr );
        return path;
    }
    clearCache(cache_s, aImageElement->Size);
    write (pfd, (char*)aImageElement->Data,  aImageElement->Size);
    close(pfd);
    return path;
}

SldU16StringRef DataAccess::StyleName( UInt32 aStyleId )
{
  sld2::IntFormatter fmt;
  m_scratchString.assign( SldU16StringRefLiteral( "S" ) );
  m_scratchString.append( fmt.format( aStyleId ) );
  return m_scratchString;
}

void DataAccess::InjectCSSStyle( SldU16StringRef aStyleString )
{
  UInt32 index = static_cast<UInt32>(m_htmlStyles.size());
  m_htmlStyles.emplace( index, to_string( aStyleString ) );
  usedStyles.insert( index );
}

SldU16StringRef DataAccess::CSSClassName( const CSldMetadataProxyBase& aData )
{
  const UInt32 cssBlockIndex = aData.cssStyleId();

  sld2::IntFormatter fmt;
  m_scratchString.assign( SldU16StringRefLiteral( "sld-css" ) );
  m_scratchString.append( fmt.format( cssBlockIndex ) );

  if ( m_cssStyles.find( cssBlockIndex ) == m_cssStyles.end() )
  {
    SldU16String temp;
    ESldError error = aData.cssStyle( temp );
    if ( error == eOK )
    {
      SldU16String styleString( SldU16StringRefLiteral( "." ) );
      styleString.append( m_scratchString );
      styleString.append( SldU16StringRefLiteral( " { " ) );

      styleString.append( temp );
      styleString.append( SldU16StringRefLiteral( " }" ) );

      m_cssStyles.emplace( cssBlockIndex, std::move( styleString ) );
    }
  }

  return m_scratchString;
}

SldU16StringRef DataAccess::CSSClassName( const CSldMetadataProxy< eMetaMediaContainer >& aData )
{
  static UInt32 count = 0;

  sld2::IntFormatter fmt;
  m_scratchString.assign( SldU16StringRefLiteral( "sld-mc" ) );
  m_scratchString.append( fmt.format( count++ ) );

  SldU16String styleString( SldU16StringRefLiteral( "." ) );
  styleString.append( m_scratchString );
  styleString.append( SldU16StringRefLiteral( " { " ) );

  sld2::html::CreateCSSStyle( styleString, aData, *this );

  styleString.append( SldU16StringRefLiteral( " }" ) );

  UInt32 index = static_cast<UInt32>(m_htmlStyles.size());
  m_htmlStyles.emplace( index, std::move( styleString ) );
  usedStyles.insert( index );

  return m_scratchString;
}

SldU16StringRef DataAccess::CSSClassName( const CSldMetadataProxy< eMetaBackgroundImage >& aData )
{
  sld2::IntFormatter fmt;

  auto it = m_bgImageStyles.find( aData->PictureIndex );
  if ( it == m_bgImageStyles.end() && aData->PictureIndex != SLD_INDEX_PICTURE_NO )
  {
    // TODO: механизм обработки различных версий картинок (разных размеров) пока отсутствует
    SldU16String file_name = ResolveImageUrl( aData->PictureIndex );
    if ( file_name.empty() )
    {
      return SldU16StringRef();
    }

    // т.к. одному тегу метаданных может соответствовать несколько стилей (из-за разных картинок в теге), привязку делаем не по номеру стиля,
    // а создаем виртуальные номера по одному на каждую уникальную картинку
    UInt32 currentStyleIndex = m_htmlStyles.size();

    SldU16String styleString( SldU16StringRefLiteral( ".S" ) );
    styleString.append( fmt.format( currentStyleIndex ) );
    styleString.append( SldU16StringRefLiteral( " { " ) );

    // предполагаем, что высота регулируется зашитыми в базу картинками - padding-top/background-position не регулируем
    // высота картинки пока не настраивается - стабильно 6px
    styleString.append( SldU16StringRefLiteral( "background-size: 100% 6px; background: url(" ) );
    styleString.append( file_name );
    styleString.append( SldU16StringRefLiteral( ") no-repeat scroll center transparent; }" ) );

    m_htmlStyles.emplace( currentStyleIndex, std::move( styleString ) );
    it = m_bgImageStyles.emplace( aData->PictureIndex, currentStyleIndex ).first;
  }

  if ( it == m_bgImageStyles.end() )
  {
    return nullptr;
  }

  usedStyles.insert( it->second );
  return StyleName( it->second );
}

UInt32 DataAccess::getDefaultSwitchState( const sld2::html::StateTracker& state, ESldMetaSwitchThematicTypeEnum thematic )
{
  if ( !state.isInside(eMetaSwitchControl) && !state.switchBlocks.empty() )
  {
    // apply 'My View' settings
    if ( (thematic == eSwitchThematicExamples && mHideExamples)
        || (thematic == eSwitchThematicPhrase && mHidePhrase)
        || (thematic == eSwitchThematicIdioms && mHideIdioms)
        || (thematic == eSwitchThematicPhonetics && mHidePhonetics)
        || (thematic == eSwitchThematicImages && mHideImages) )
    {
      return 1;
    }
  }
  return 0;
}

void DataAccess::setMyViewSettings(
    bool hidePhonetics,
    bool hideExamples,
    bool hideImages,
    bool hideIdioms,
    bool hidePhrase
)
{
  mHidePhonetics = hidePhonetics;
  mHideExamples = hideExamples;
  mHideImages = hideImages;
  mHideIdioms = hideIdioms;
  mHidePhrase = hidePhrase;
}
