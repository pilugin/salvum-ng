#include <sharedimage.h>
#include <QColor>

#if 0

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

Block ImagePart::getWritableBlock() 
{
    Block res = mImage->getWritableBlock()
    if (!res.isNull())
        ++mCount;
        
    return res;
}

void ImagePart::setBad()
{
    mOffset = mImage->createBadPart( mOffset, mCount );
}

void ImagePart::setGoodAgain()
{
    if (isBad()) {
        mImage->moveBadPartBack(mBadOffset, mCount, mOffset);
        mBadOffset = 0;
    }
}

/////////////////////////////////////////////////////

SharedImage::SharedImage(int width, int height, int badSectorRatio)
: mSize(width, height)
, mCurrentWritableBlock(0)
, mData(width * ((height * (100+badSectorRatio)) /100)  ) //< width * (height+10%)
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
    bs.line = pointerByCoordinate(c, 0);
    bs.rowOffset = blocksInRow() * Block::NUM_COLS;
    bs.span = qMin(count, blocksInRow() - (offset/Block::NUM_COLS)%blocksInRow() );
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
    b.line = pointerByCoordinate(c, 0);
    b.rowOffset = blocksInRow() * Block::NUM_COLS;
    
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
    return mSize.width() / Block::NUM_COLS;
}

int SharedImage::blocksInCol() const
{
    return mSize.height() / Block::NUM_ROWS;
}

int SharedImage::getBadSectorSize() const
{
    return (mData.capacity() / (Block::NUM_ROWS * Block::NUM_COLS)) - totalBlockCount();
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

        for (int y=0; y<Block::NUM_ROWS; ++y) {
            memcpy( pointerByCoordinate(dstC, y), pointerByCoordinate(srcC, y), span * sizeof(unsigned int) * Block::NUM_COLS );
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


QPoint Image::getBlockCoordinates(int offset) const
{
    auto rv = QPoint( offset % (mSize.width()/Block::NUM_ROWS), offset / (mSize.width()/Block::NUM_ROWS) );
    if (rv.y() < mSize.height())
        return rv;
    else
        return QPoint(-1, -1);
}



#define POINTER_BY_COORDINATE(c, i) mData.data() + (c.y() * Block::NUM_ROWS + i)*mSize.width() + c.x() * Block::NUM_COLS

unsigned int *Image::pointerByCoordinate(const QPoint &c, int i)
{
    return POINTER_BY_COORDINATE(c, i);
}

const unsigned int *Image::pointerByCoordinate(const QPoint &c, int i) const
{
    return POINTER_BY_COORDINATE(c, i);
}


#ifdef QT_GUI_LIB
QImage toQImage(const SharedImage &img)
{
    QImage res(img.getSize(), QImage::Format_ARGB32);
    for (int line=0; line<img.getSize().height(); ++line) {
        memcpy(res.scanLine(line), img.scanline(line), img.getSize().width() * sizeof(unsigned int));

//        for (int pixel=0; pixel<img.getSize().width(); ++pixel) res.setPixel(pixel, line, img.scanline(line)[pixel]);

    }
    return res;
}
#endif

Block::Block()
{
    line = nullptr;
}

}

#endif