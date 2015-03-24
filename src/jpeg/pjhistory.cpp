#include <jpeg/pjhistory.h>

namespace Jpeg {

PjHistory::PjHistory( BaseSharedImageAllocator &alloc )
: mImageAlloc( alloc )
{
}

bool PjHistory::loadMore( const QDir &source )
{
    return true;
}

}
