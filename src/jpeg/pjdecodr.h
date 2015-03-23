#ifndef JPEG_PJDECODR_H
#define JPEG_PJDECODR_H

#include <core/decodr.h>
#include <picojpeg.h>
#include <util/singleton.h>

namespace Jpeg {

class PjFrame : public Core::BaseFrame
{
public:
    QByteArray pjpegCtxt;
    pjpeg_image_info_t pjpegImgInfo;
    
protected:
    bool saveMore(const QString &destPath, QFile &descFile) const;
    bool loadMore(const QString &sourcePath, QFile &descFile);
};

class PjDecodr : public Core::Decodr<PjFrame>, private Singleton<PjDecodr>
{
public:
protected:
    void doInit(PjFrame &frame);
    void doDecode(PjFrame &frame);

    static unsigned char fetchCallback(unsigned char *buf, unsigned char bufSize, unsigned char *bytesRead, void *param);
    unsigned char fetchCallback(unsigned char *buf, unsigned char bufSize, unsigned char *bytesRead);
};

}

#endif
