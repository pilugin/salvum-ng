#include <jpeg/pjdecodr.h>
#include <QtDebug>
#include <QFile>

namespace Jpeg {


PjDecodr::PjDecodr(BaseSharedImageAllocator &alloc)
: mImageAlloc(alloc)
, mImage(nullptr)
{
}

JpegScanType pjpegScanType2SalvumScanType( int pjpegScanType )
{
    switch (pjpegScanType) {
    case PJPG_YH1V1: return H1V1;
    case PJPG_YH2V2: return H2V2;
    case PJPG_YH1V2: return H1V2;
    case PJPG_YH2V1: return H2V1;
    default:         return BAD_SCAN_TYPE;
    }
}

void PjDecodr::doInit(PjFrame &frame)
{
    unsigned char rv = pjpeg_decode_init(&frame.pjpegImgInfo, &PjDecodr::fetchCallback, this, 0);
    if (rv != 0) {
        qDebug()<<"PjDecodr::doInit: " <<rv;
        frame.setDecodFailed();

    } else {
        frame.setInitialized();
        mImage = mImageAlloc.alloc( pjpegScanType2SalvumScanType(frame.pjpegImgInfo.m_scanType), 
                    frame.pjpegImgInfo.m_width, frame.pjpegImgInfo.m_height );
    }
}

void PjDecodr::doDecode(PjFrame &frame)
{
    frame.imagePart = mImage->createPart();

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

PjFrame::PjFrame()
: pjpegCtxt( pjpeg_ctxt_buffer_size, '\0' )
, imagePart( nullptr )
{
}

bool PjFrame::saveMore(const QString &destPath, QFile &descFile) const
{
    QFile pjpegF( destPath + "/pjpeg.bin" );
    if (! pjpegF.open(QFile::WriteOnly | QFile::Truncate )) {
        qDebug()<<"Failed to open file: "<<pjpegF.fileName();
        return false;
    }
    
    QByteArray pjpegData( pjpeg_serialize_buffer_size, '\0' );
    
    pjpeg_serialize_save( &pjpegImgInfo, pjpegCtxt.data(), pjpegData.data() );
    pjpegF.write(pjpegData);
    
    if ( imagePart.offset == 0 && imagePart.count > 0 ) { // first one. save the whole image
        const SharedImage *i = imagePart.image;
        descFile.write( QString().sprintf("image=%d/%dx%d/%d/%d/%d/%d\n",
                                    (int)i->getScanType(), i->getSize().width(), i->getSize().height(),
                                    i->getCurrentWritableBlock(), i->getCurrentBadBlock(), i->getBadSectorRatio(),
                                    i->getDataSize()    ).toLatin1() );
    
        QFile imageF( destPath + "/image.data" );
        if (! imageF.open(QFile::WriteOnly | QFile::Truncate )) {
            qDebug()<<"Failed to open file: "<<imageF.fileName();
            return false;
        }
        imageF.write( i->getData(), i->getDataSize() * sizeof(unsigned int) );
        imageF.close();
        
        if (!QFile::link( QDir(destPath).absolutePath(), destPath + "../headframe" )) {
            qDebug()<<"Failed to create symlink: "<<QDir(destPath).absolutePath();
        }
    }
    if (!imagePart.isNull())
        descFile.write( QString().sprintf("imagePart=%d/%d/%d\n", imagePart.offset, imagePart.count, imagePart.badOffset).toLatin1() );
    
    return true;
}

bool PjFrame::loadMore(const QString &sourcePath, QFile &descFile)
{
    QFile pjpegF( sourcePath + "/pjpeg.bin" );
    if (! pjpegF.open(QFile::ReadOnly)) {
        qDebug()<<"Failed to open file: "<<pjpegF.fileName();
        return false;
    }

    QByteArray pjpegData = pjpegF.readAll();
    pjpeg_serialize_load( pjpegData.data(), &pjpegImgInfo, pjpegCtxt.data(), &PjDecodr::fetchCallback, PjDecodr::instance() );

    while (!descFile.atEnd()) {
        QByteArray line = descFile.readLine();
        if (line.startsWith("imagePart=")) {
            if (line.endsWith('\n'))
                line.resize(line.size() -1);
            QList<QByteArray> digits = line.split('=');
            if (digits.size() != 2) {
                qDebug()<<"Failed to parse desc: "<<line;
                return false;
            }
            digits = digits[1].split('/');
            if (digits.size() != 3) {
                qDebug()<<"Failed to parse desc[2]: "<<line;
                return false;
            }
            
            bool ok;
            imagePart.offset = digits[0].toInt(&ok);
            if (!ok) {
                qDebug()<<"Failed to parse imagePart::offset: "<<line;
                return false;
            }
            imagePart.count = digits[1].toInt(&ok);
            if (!ok) {
                qDebug()<<"Failed to parse imagePart::count: "<<line;
                return false;
            }
            imagePart.badOffset = digits[2].toInt(&ok);
            if (!ok) {
                qDebug()<<"Failed to parse imagePart::badOffset: "<<line;
                return false;
            }
        }
    }
    
    return !imagePart.isNull();
}

}
