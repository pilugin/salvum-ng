#ifndef JPEG_PJHISTORY_H
#define JPEG_PJHISTORY_H

#include <core/history.h>
#include <jpeg/pjdecodr.h>

namespace Jpeg {

class SharedImageAllocator;

class PjHistory : public Core::History<PjFrame>
{
public:
    PjHistory(SharedImageAllocator *alloc);
    
protected:
    bool loadMore(const QDir &source);
private:
    SharedImageAllocator *mImageAlloc;

    std::shared_ptr< SharedImage > mImage;
};

}

#endif
