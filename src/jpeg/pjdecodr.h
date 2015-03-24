#ifndef JPEG_PJDECODR_H
#define JPEG_PJDECODR_H

#include <picojpeg.h>
#include <core/decodr.h>
#include <util/singleton.h>
#include <jpeg/sharedimage.h>

namespace Jpeg {

class PjFrame : public Core::BaseFrame
{
public:
    PjFrame();

    QByteArray pjpegCtxt;
    pjpeg_image_info_t pjpegImgInfo;
    ImagePart imagePart;
    
protected:
    bool saveMore(const QString &destPath, QFile &descFile) const;
    bool loadMore(const QString &sourcePath, QFile &descFile);
};

class PjDecodr : public Core::Decodr<PjFrame>, public Singleton<PjDecodr> //< singleton is for picojpeg (it uses global vars)
{
public:
    PjDecodr(BaseSharedImageAllocator &alloc);
    
    static unsigned char fetchCallback(unsigned char *buf, unsigned char bufSize, unsigned char *bytesRead, void *param);
protected:
    void doInit(PjFrame &frame);
    void doDecode(PjFrame &frame);

    unsigned char fetchCallback(unsigned char *buf, unsigned char bufSize, unsigned char *bytesRead);
        
private:
    BaseSharedImageAllocator &mImageAlloc;
    SharedImage *mImage;
};

}

#endif
