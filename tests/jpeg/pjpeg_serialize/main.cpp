#include <picojpeg/picojpeg.h>
#include <QFile>
#include <QImage>
#include <QtDebug>
#include <stdio.h>
#include <unistd.h>

unsigned char read_cb(unsigned char *buf, unsigned char bufSize, unsigned char *bytesRead, void *param)
{
    QFile *f = static_cast<QFile *>(param);    
    if (f->pos() == f->size()) {
        qDebug("F.ATEND");
        return PJPG_NO_MORE_BLOCKS;        
    }
    
    *bytesRead = f->read( (char *)buf, bufSize);
    return 0;
}

int main(int argc, char **argv)
{    
    for (int i=0; i<argc; ++i)
        printf("%s ", argv[i]);
    printf("\n");        

    if (argc == 1) {
        printf("Usage: %s file.jpg\n", argv[0] );
        return 0;
        
    } 
    
    QFile src;
    pjpeg_image_info_t i;
    int decoded = 0;
    QImage img;
    
    if (argc == 2) {
        src.setFileName(argv[1]);
        if (!src.open(QFile::ReadOnly)) {
            qDebug()<<"Failed to open: "<<src.fileName();
            return -1;
        }
        
        unsigned char rv = pjpeg_decode_init(&i, &read_cb, &src, 0);
        if (rv != 0) {
            qDebug()<<"pjinit failed: " <<rv;
            return 0;
        }
        img = QImage( i.m_width, i.m_height, QImage::Format_ARGB32);

        printf("Image type=%d   mcu/col=%d  mcu/row=%d\n", (int)i.m_scanType, i.m_MCUSPerRow, i.m_MCUSPerCol);
        printf("MCU w/h =%d/%d;   w/h=%d/%d\n", i.m_MCUWidth, i.m_MCUHeight, i.m_width, i.m_height);
        
    } else if (argc == 6) {
        src.setFileName(argv[1]);
        if (!src.open(QFile::ReadOnly)) {
            qDebug()<<"Failed to open: "<<src.fileName();
            return -1;
        }
        
        bool ok;
        if (!src.seek( QByteArray(argv[2]).toInt(&ok) ))
            qDebug()<<"File seek failed";
        if (!ok) {
            qDebug()<<"Cannot parse int: "<<argv[2];
            return -2;
        }
        decoded = QByteArray(argv[4]).toInt(&ok);
        if (!ok) {
            qDebug()<<"Cannot parse int: "<<argv[4];
            return -6;
        }
        
        QFile seria(argv[3]);
        if (!seria.open(QFile::ReadOnly)) {
            qDebug()<<"Failed to open: "<<seria.fileName();
            return -3;
        }
        QByteArray seriaData = seria.readAll();
        if (seriaData.size() != pjpeg_serialize_buffer_size) {
            qDebug()<<"Serialized data has incorrect size: "<<seriaData.size();
            return -4;
        }
        
        if (!img.load(argv[5], "png")) {
            qDebug()<<"Failed to load png: "<<argv[5];
        }
        
        pjpeg_serialize_load(seriaData.data(), &i, 0, &read_cb, &src);
        
    } else {
        execl(argv[0], argv[0], NULL);
    }
    
    
    int expected = (i.m_width * i.m_height)/8/8;       
    
    printf("START. D=%d\n", decoded);
    for (int j=0; j<14000; ++j) {
    
        if (decoded >=expected) {
            qDebug("Finished");
            return 0;    
        }
    
        unsigned char rv=pjpeg_decode_mcu();        
                
        if (rv == PJPG_NO_MORE_BLOCKS) {
            qDebug()<<"No more blocks";
            return 0;            
        } 
        if (rv != 0) {
            qDebug()<<"Some pjpg error: "<<rv;
            return -5;
        }
        
        for (int b=0; b<1; ++b, ++decoded) {
            int bx = decoded % (i.m_width /8);
            int by = decoded / (i.m_width /8);
            int ax = bx*8;
            int ay = by*8;
            
//            printf("%d   x=%d y=%d\n", decoded, ax, ay);
        
            uchar *rptr = i.m_pMCUBufR +b*64;
            uchar *gptr = i.m_pMCUBufG +b*64;
            uchar *bptr = i.m_pMCUBufB +b*64;
            for (int y=0; y<8; ++y) {
                for (int x=0; x<8; ++x) {
                    if (  (img.size().width() <= (ax + x) )
                        ||(img.size().height() <= (ay+ y) ) ) {
                            qDebug()<<"out of image "<<(ax+x)<<(ay+y);
                            return -9;
                        }
                
                    img.setPixel( ax + x, ay + y, qRgb( *rptr++, *gptr++, *bptr++ ) );
                }
            }
        }

    }

    QByteArray imagePath = QByteArray().setNum(decoded) + ".png";
    img.save(imagePath, "png");
    
    QByteArray saveData;
    saveData.resize( pjpeg_serialize_buffer_size );
    pjpeg_serialize_save( &i, 0, saveData.data() );
    
    QFile seria("SERIA");
    if (!seria.open(QFile::Truncate | QFile::WriteOnly)) {
        qDebug()<<"Failed to open file: "<<seria.fileName();
        return -5;
    }
    seria.write(saveData);
    seria.close();
    return 0;
    execl(argv[0], argv[0], argv[1],    QByteArray().setNum(src.pos()).data(), 
                                        seria.fileName().toLatin1().data(), 
                                        QByteArray().setNum(decoded).data(),
                                        imagePath.data(),  NULL );

    return 0;
}