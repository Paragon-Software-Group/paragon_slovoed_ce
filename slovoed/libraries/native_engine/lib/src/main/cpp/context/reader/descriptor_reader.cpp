#include "descriptor_reader.h"

#include <unistd.h>
#include <sys/stat.h>

Int8 DescriptorReader::IsOpened() const
{
  if ( -1 == m_fd )
  {
    return 0;
  }
  return 1;
}

UInt32 DescriptorReader::Read( void* aDestPtr, UInt32 aSize, UInt32 aOffset )
{
  return static_cast<UInt32>(::pread( m_fd, aDestPtr, size_t( aSize ), off_t( m_offset ) + off_t( aOffset ) ));
}

UInt32 DescriptorReader::GetSize() const
{
  jlong res = m_size;
  if ( res < 0L )
  {
    res = 0L;
    struct stat st = {0};
    if ( 0 == ::fstat( m_fd, &st ) )
    {
      res = static_cast<jlong>(st.st_size) - m_offset;
    }
  }
  return static_cast<UInt32>(res);
}
