#include <jpeg/sharedimage.h>
#include <QColor>

namespace Jpeg
{

ImagePart::ImagePart(std::shared_ptr<SharedImage> image_, int offset_, int count_, int badOffset_)
: offset(offset_)
, count(count_)
, badOffset(badOffset_)
, image(image_)
{
}

ConstBlocks ImagePart::getBlocks(int blocksOffset) const
{
    return image.lock()->getBlocks( (isBad() ? badOffset : offset) + blocksOffset, count);
}

Block ImagePart::addWritableBlock() 
{
    Block res = image.lock()->getWritableBlock();
    if (!res.isNull())
        ++count;
        
    return res;
}

bool ImagePart::setBad()
{
    badOffset = image.lock()->createBadPart( offset, count );
    return badOffset >= 0;
}

void ImagePart::setGoodAgain()
{
    if (isBad()) {
        image.lock()->moveBadPartBack(badOffset, count, offset);
        badOffset = 0;
    }
}

int ImagePart::blockWidth() const 
{
    return Block::numCols( image.lock()->getScanType() );
}

int ImagePart::blockHeight() const
{
    return Block::numRows( image.lock()->getScanType() );
}

bool ImagePart::addBlock(pjpeg_image_info_t *pjpegInfo)
{    
    auto i = image.lock();
    Block block = addWritableBlock();
    if (block.isNull())
        return false;
    
    copyPixels( block, 0, 8, 0, 8, pjpegInfo->m_pMCUBufR, pjpegInfo->m_pMCUBufG, pjpegInfo->m_pMCUBufB );    
    if ( i->getScanType() & H2 )
        copyPixels( block, 8, 8, 0, 8, pjpegInfo->m_pMCUBufR +64, pjpegInfo->m_pMCUBufG +64, pjpegInfo->m_pMCUBufB +64 );
    if ( i->getScanType() & V2 )
        copyPixels( block, 0, 8, 8, 8, pjpegInfo->m_pMCUBufR +128, pjpegInfo->m_pMCUBufG +128, pjpegInfo->m_pMCUBufB +128 );
    if ( i->getScanType() == H2V2 )
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

int SharedImage::dataCapacity(JpegScanType scanType, int width, int height, int badSectorRatio)
{
    // width * (height+10%)
    return  intCeil(width, Block::numCols(scanType)) 
            * (  intCeil(height, Block::numRows(scanType)) 
                * (100+badSectorRatio)    )/100;
}

int SharedImage::calculateSizeof(JpegScanType scanType, int width, int height, int badSectorRatio)
{
    return sizeof(SharedImage) + dataCapacity(scanType, width, height, badSectorRatio)*sizeof(unsigned int);
}

SharedImage::SharedImage(JpegScanType scanType, int width, int height, int badSectorRatio)
: mScanType(scanType)
, mBadSectorRatio(badSectorRatio)
, mSize(width, height)
, mCurrentWritableBlock(0)
, mData( dataCapacity(scanType, width, height, badSectorRatio) ) 
{
    mCurrentBadBlock = totalBlockCount();
}

SharedImage::~SharedImage()
{
}

ImagePart SharedImage::createPart(std::shared_ptr<SharedImage> sharedPtr)
{
    assert(sharedPtr.get() == this);
    return ImagePart(sharedPtr, mCurrentWritableBlock);
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

////////////////////////////////////////////////////////

SharedImageAllocator::~SharedImageAllocator()
{
}

SharedImage *SharedImageAllocator::alloc(JpegScanType scanType, int width, int height, int badSectorPercentRatio)
{
    void *mem = malloc( SharedImage::calculateSizeof(scanType, width, height, badSectorPercentRatio) );
    if (mem) {
        return create(mem, scanType, width, height, badSectorPercentRatio);
    }
    return nullptr;
}

void SharedImageAllocator::free(SharedImage *image)
{
    if (image) {
        destroy(image);
        free(image);
    }
}

SharedImage *SharedImageAllocator::create(void *mem, JpegScanType scanType, int width, int height, int badSectorPercentRatio)
{
    return new (mem) SharedImage(scanType, width, height, badSectorPercentRatio);
}

void SharedImageAllocator::destroy(SharedImage *image)
{
    image->~SharedImage();
}


}

