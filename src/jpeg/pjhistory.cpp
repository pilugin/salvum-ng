#include <jpeg/pjhistory.h>

namespace Jpeg {

PjHistory::PjHistory(SharedImageAllocator *alloc )
: mImageAlloc( alloc )
{
}

bool PjHistory::loadMore( const QDir &source )
{
    return true;
}

}
