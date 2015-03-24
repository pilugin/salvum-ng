#include <jpeg/sharedimage.h>
#include <QColor>

namespace Jpeg
{

ImagePart::ImagePart(SharedImage *image, int offset, int count)
: mImage(image)
, mOffset(offset)
, mCount(count)
, mBadOffset(0)
{
}

ConstBlocks ImagePart::getBlocks(int blocksOffset) const
{
    return mImage->getBlocks( (isBad() ? mBadOffset : mOffset) + blocksOffset, mCount);
}

Block ImagePart::addWritableBlock() 
{
    Block res = mImage->getWritableBlock();
    if (!res.isNull())
        ++mCount;
        
    return res;
}

bool ImagePart::setBad()
{
    mBadOffset = mImage->createBadPart( mOffset, mCount );
    return mBadOffset >= 0;
}

void ImagePart::setGoodAgain()
{
    if (isBad()) {
        mImage->moveBadPartBack(mBadOffset, mCount, mOffset);
        mBadOffset = 0;
    }
}

int ImagePart::blockWidth() const 
{
    return Block::numCols( mImage->getScanType() );
}

int ImagePart::blockHeight() const
{
    return Block::numRows( mImage->getScanType() );
}

bool ImagePart::addBlock(pjpeg_image_info_t *pjpegInfo)
{
    Block block = addWritableBlock();
    if (block.isNull())
        return false;
    
    copyPixels( block, 0, 8, 0, 8, pjpegInfo->m_pMCUBufR, pjpegInfo->m_pMCUBufG, pjpegInfo->m_pMCUBufB );    
    if ( mImage->getScanType() & H2 )
        copyPixels( block, 8, 8, 0, 8, pjpegInfo->m_pMCUBufR +64, pjpegInfo->m_pMCUBufG +64, pjpegInfo->m_pMCUBufB +64 );
    if ( mImage->getScanType() & V2 )
        copyPixels( block, 0, 8, 8, 8, pjpegInfo->m_pMCUBufR +128, pjpegInfo->m_pMCUBufG +128, pjpegInfo->m_pMCUBufB +128 );
    if ( mImage->getScanType() == H2V2 )
        copyPixels( block, 8, 8, 8, 8, pjpegInfo->m_pMCUBufR +192, pjpegInfo->m_pMCUBufG +192, pjpegInfo->m_pMCUBufB +192 );
    return true;
}

void ImagePart::copyPixels(Block &block, int x, int cx, int y, int cy, unsigned char *r, unsigned char *g, unsigned char *b)
{
    for (int iy=0; iy<cy; ++cy)
        for (int ix=0; ix<cx; ++ix)
            block.line(iy + y)[ix + x] = qRgb( *r++, *g++, *b++ );
}

/////////////////////////////////////////////////////

static int intCeil(int v, int divisor)
{
    if ((v % divisor) ==0 )
        return 0;
    return ( v/divisor + 1) * divisor;
}

SharedImage::SharedImage(int width, int height, JpegScanType scanType, int badSectorRatio)
: mScanType(scanType)
, mSize(width, height)
, mCurrentWritableBlock(0)
, mData(intCeil(width, Block::numCols(scanType)) * (( intCeil(height, Block::numRows(scanType)) * (100+badSectorRatio)) /100)  ) //< width * (height+10%)
{
    mCurrentBadBlock = totalBlockCount();
}

ImagePart SharedImage::createPart()
{
    return ImagePart(this, mCurrentWritableBlock);
}

ConstBlocks SharedImage::getBlocks(int offset, int count) const
{    
    ConstBlocks bs;

    QPoint c = getBlockCoordinates(offset);
    if (c.x() < 0)
        return bs;
    bs.linePtr = pointerByCoordinate(c, 0);
    bs.rowOffset = blocksInRow() * Block::numCols(mScanType);
    bs.span = qMin(count, blocksInRow() - (offset/Block::numCols(mScanType))%blocksInRow() );
    bs.ripped = count > bs.span;

    return bs;
}

Block SharedImage::getWritableBlock()
{
    Block b;
    if (atEnd())
        return b;
        
    QPoint c = getBlockCoordinates(mCurrentWritableBlock ++);
    if (c.x() < 0)
        return b;
    b.linePtr = pointerByCoordinate(c, 0);
    b.rowOffset = blocksInRow() * Block::numCols(mScanType);
    
    return b;
}

bool SharedImage::atEnd() const
{
    return mCurrentWritableBlock >= totalBlockCount();
}

int SharedImage::totalBlockCount() const
{
    return blocksInRow() * blocksInCol();
}

int SharedImage::blocksInRow() const
{
    return mSize.width() / Block::numCols(mScanType);
}

int SharedImage::blocksInCol() const
{
    return mSize.height() / Block::numRows(mScanType);
}

int SharedImage::getBadSectorSize() const
{
    return (mData.capacity() / (Block::numRows(mScanType) * Block::numCols(mScanType) )) - totalBlockCount();
}

int SharedImage::getBadSectorUsed() const
{
    return mCurrentBadBlock - totalBlockCount();
}

void SharedImage::copyBlocks(int srcOffset, int dstOffset, int count)
{
    int copied = 0;
    while ((count - copied) > 0) {
        QPoint srcC = getBlockCoordinates(srcOffset + copied);
        QPoint dstC = getBlockCoordinates(dstOffset + copied);
        int srcSpan = qMin(blocksInRow() - srcC.x(), (count - copied));
        int dstSpan = blocksInRow() - dstC.x();
        int span = qMin(srcSpan, dstSpan);

        for (int y=0; y<Block::numRows(mScanType); ++y) {
            memcpy( pointerByCoordinate(dstC, y), pointerByCoordinate(srcC, y), span * sizeof(unsigned int) * Block::numCols(mScanType) );
        }

        copied += span;
    }
}

int SharedImage::createBadPart(int offset, int count)
{
    if (count <= getBadSectorFree()) {
        copyBlocks( offset, mCurrentBadBlock, count);
        int rv = mCurrentBadBlock;
        mCurrentBadBlock += count;
        mCurrentWritableBlock = offset;
        return rv;

    } else {
        return -1;
    }
}

void SharedImage::dropBadParts()
{
    mCurrentBadBlock = totalBlockCount();
}

void SharedImage::moveBadPartBack(int badOffset, int count, int goodOffset)
{
    copyBlocks( badOffset, goodOffset, count );
    mCurrentWritableBlock = goodOffset + count;
}


QPoint SharedImage::getBlockCoordinates(int offset) const
{
    auto rv = QPoint( offset % (mSize.width()/Block::numRows(mScanType)), offset / (mSize.width()/Block::numRows(mScanType)) );
    if (rv.y() < mSize.height())
        return rv;
    else
        return QPoint(-1, -1);
}

unsigned int *SharedImage::pointerByCoordinate(const QPoint &c, int i)
{
    return mData.data() + (c.y() * Block::numRows(mScanType) + i)*mSize.width() + c.x() * Block::numCols(mScanType);
}

const unsigned int *SharedImage::pointerByCoordinate(const QPoint &c, int i) const
{
    return mData.data() + (c.y() * Block::numRows(mScanType) + i)*mSize.width() + c.x() * Block::numCols(mScanType);
}


#ifdef QT_GUI_LIB
QImage toQImage(const SharedImage &img)
{
    QImage res(img.getSize(), QImage::Format_ARGB32);
    for (int line=0; line<img.getSize().height(); ++line) {
        memcpy(res.scanLine(line), img.scanline(line), img.getSize().width() * sizeof(unsigned int));
    }
    return res;
}
#endif

Block::Block()
{
    linePtr = nullptr;
}

int Block::numRows(JpegScanType scanType)
{
    if ( scanType & V1 )
        return 8;
    else if ( scanType & V2 )
        return 16;
     
    return 0;
}

int Block::numCols(JpegScanType scanType)
{
    if ( scanType & H1 )
        return 8;
    else if ( scanType & H2 )
        return 16;
        
    return 0;        
}

}

