#include <sharedimage.h>
#include <QColor>

namespace Jpeg
{

struct InvalidBlock : public Block {
    InvalidBlock() {
        for (uint i=0; i<sizeof(lines)/sizeof(lines[0]); ++i)
            lines[i] = nullptr;
    }
};

struct InvalidConstBlock : public ConstBlocks {
    InvalidConstBlock() {
        for (uint i=0; i<sizeof(lines)/sizeof(lines[0]); ++i)
            lines[i] = nullptr;
        ripped = false;
        span = 0;
    }
};

static const InvalidBlock invalidBlock;
static const InvalidConstBlock invalidConstBlocks;


Image::Image()
: mCurrentWritableBlock(0)
, mBadIdGen(INV_BAD_PART_ID)
{
}

ConstBlocks Image::getBlocks(int offset, int count, int badId) const
{
    if (badId == INV_BAD_PART_ID) {
        ConstBlocks bs;
        QPoint c = getBlockCoordinates(offset);
        for (int i=0; i<Block::NUM_ROWS; ++i)
            bs.lines[i] = pointerByCoordinate(c, i);

        bs.span = qMin(count, blocksPerRow() - c.x() );
        bs.ripped = count > bs.span;

        return bs;

    } else {
        return invalidConstBlocks;

    }
}

Block Image::getWritableBlock()
{
    if (atEnd())
        return invalidBlock;
    QPoint c = getBlockCoordinates(mCurrentWritableBlock ++);
    if (c.x() < 0)
        return invalidBlock;
    Block b;
    for (int i=0; i<Block::NUM_ROWS; ++i) {
        b.lines[i] = pointerByCoordinate(c, i);
    }
    return b;
}

bool Image::atEnd() const
{
    return mCurrentWritableBlock == mSize.width() * mSize.height() / Block::NUM_COLS / Block::NUM_ROWS;
}

int Image::blocksPerRow() const
{
    return mSize.width() / Block::NUM_ROWS;
}

int Image::blocksPerCol() const
{
    return mSize.height() / Block::NUM_COLS;
}

Image::BadPart::BadPart()
{
    scanlines.resize(Block::NUM_ROWS);
}

int Image::createBadPart(int offset, int count)
{
    int badId = ++mBadIdGen;

    BadPart badPart;
    // copy using getBlocks

    mBadParts.insert( badId, badPart );

    return badId;
}

void Image::dropBadPart(int badPartId)
{
    mBadParts.remove(badPartId);
}

void Image::moveBadPartBack(int badPartId)
{
//    auto itr = mBadParts.find(badPartId);
//    if (itr != mBadParts.end()) {
        // copy using
//    }
}

void Image::init(int width, int height)
{
    mSize = QSize(width, height);
    mData = QVector<unsigned int>( height*width, 0 );
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
QImage toQImage(const Image &img)
{
    QImage res(img.getSize(), QImage::Format_ARGB32);
    for (int line=0; line<img.getSize().height(); ++line) {
        memcpy(res.scanLine(line), img.scanline(line), img.getSize().width() * sizeof(unsigned int));

//        for (int pixel=0; pixel<img.getSize().width(); ++pixel) res.setPixel(pixel, line, img.scanline(line)[pixel]);

    }
    return res;
}
#endif

Block::Block(const Block &other)
{
    *this = other;
}

Block::Block()
{
    *this = invalidBlock;
}

Block &Block::operator=(const Block &other)
{
    for (int i=0; i<NUM_COLS; ++i)
        lines[i] = other.lines[i];
    return *this;
}

}
