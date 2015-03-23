#include <jpeg/pjdecodr.h>
#include <QtDebug>

namespace Jpeg {

void PjDecodr::doInit(PjFrame &frame)
{
    unsigned char rv = pjpeg_decode_init(&frame.pjpegImgInfo, &PjDecodr::fetchCallback, this, 0);
    if (rv != 0) {
        qDebug()<<"PjDecodr::doInit: " <<rv;
        frame.setDecodFailed();

    } else {
        frame.setInitialized();
        // TODO: create new SharedImage and connect to frame object
    }
}

void PjDecodr::doDecode(PjFrame &frame)
{
    static const int MAX_BLOCKS_PER_CLUSTER = 470;
    int blockCount = 0;

    unsigned char rv = 0;
    for (rv=pjpeg_decode_mcu(); rv==0; rv=pjpeg_decode_mcu()) {

        for (int i=0; i<2; ++i, ++blockCount) {
            //frame.addBlock(  frame.pjpegImgInfo.m_pMCUBuf{R,G,B} +i*64 );
        }

//        if (frame.image.atEnd() && FFD9) return;

        if (blockCount > MAX_BLOCKS_PER_CLUSTER) {
            frame.setDecodFailed();
            return;
        }

        if (frame.bufferBytesLeft() < PJPG_MAX_IN_BUF_SIZE) {
            //if (!check) frame.setCheckFailed();

            return;
        }
    }

    if (rv == PJPG_NO_MORE_BLOCKS) {
        // should never happen
        Q_ASSERT(false);

    } else if (rv > 0) {
        qDebug()<<"PjDecodr::doDecode: pjpg error-"<<rv;
        frame.setDecodFailed();

    }
}

unsigned char PjDecodr::fetchCallback(unsigned char *buf, unsigned char bufSize, unsigned char *bytesRead, void *param)
{
    return static_cast<PjDecodr *>(param) -> fetchCallback(buf, bufSize, bytesRead);
}

unsigned char PjDecodr::fetchCallback(unsigned char *buf, unsigned char bufSize, unsigned char *bytesRead)
{
    return PJPG_NO_MORE_BLOCKS;
}

////////////////////////////////////////////////////////////////////////////////

bool PjFrame::saveMore(const QString &destPath, QFile &descFile) const
{

}

bool PjFrame::loadMore(const QString &sourcePath, QFile &descFile)
{

}

}
