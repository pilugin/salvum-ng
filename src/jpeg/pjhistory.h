#ifndef JPEG_PJHISTORY_H
#define JPEG_PJHISTORY_H

#include <core/history.h>
#include <jpeg/pjdecodr.h>

namespace Jpeg {

class BaseSharedImageAllocator;

class PjHistory : public Core::History<PjFrame>
{
public:
    PjHistory(BaseSharedImageAllocator &alloc);
    
protected:
    bool loadMore(const QDir &source);
private:
    BaseSharedImageAllocator &mImageAlloc;
};

}

#endif