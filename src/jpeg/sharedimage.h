#ifndef SHARED_IMAGE_HPP
#define SHARED_IMAGE_HPP

#include <memory>
#include <util/array.h>
#include <QMap>
#include <QSize>
#include <QPoint>

namespace Jpeg {

struct Block
{
    enum Nums { NUM_COLS = 8, NUM_ROWS = 8 };
    unsigned int *line;
    int rowOffset;

    bool isNull() const { return line == nullptr; }    
    unsigned int *operator[](int row) { return line + NUM_COLS; }

    Block();
};

struct ConstBlock
{
    typedef Block::Nums Nums;

    const unsigned int *line;
    unsigned int rowOffset;
    
    bool isNull() const { return line == nullptr; }
    const unsigned int *operator[](int row) const { return line + rowOffset*row; }
};

struct ConstBlocks : public ConstBlock
{
    int span;
    bool ripped;
    int nextBlockOffset; //< block offset for next blocks portion (used if ripped=true), counts from beginning of ImagePart
};

class SharedImage;

class ImagePart
{
public:
    ImagePart(SharedImage *image, int offset, int count =0);

    bool isBad() const { return mBadOffset > 0; }

    ConstBlocks getBlocks(int blocksOffset =0) const;

    Block addWritableBlock();

    bool setBad(); //< return false if SharedImage is lack of free space
    void setGoodAgain();

private:
    SharedImage *mImage;
    int mOffset;
    int mCount;
    int mBadOffset;
};

class SharedImage
{
public:
    SharedImage(int width, int height, int badSectorPercentRatio =10);

    ImagePart createPart();

    ConstBlocks getBlocks(int offset, int count) const;
    Block getWritableBlock();

    int totalBlockCount() const;
    bool atEnd() const;
    int blocksInRow() const;
    int blocksInCol() const;

    int createBadPart(int goodOffset, int count); //< returns offset
    void moveBadPartBack(int badOffset, int count, int goodOffset);
    void removeBadParts();

    QPoint getBlockCoordinates(int offset) const;
    QSize getSize() const { return mSize; }
    
    int getBadSectorSize() const; //< these 3 functions return size in Blocks 
    int getBadSectorUsed() const;
    int getBadSectorFree() const { return getBadSectorSize() - getBadSectorUsed(); }

    const unsigned int *scanline(int num) const { return mData.data() + num * mSize.width(); }
private:
    unsigned int *pointerByCoordinate(const QPoint &coordinate, int blockRow);
    const unsigned int *pointerByCoordinate(const QPoint &coordinate, int blockRow) const;

    QSize mSize;
    int mCurrentWritableBlock;
    int mCurrentBadBlock;
    DynArray<unsigned int> mData;
};

}

#ifdef QT_GUI_LIB
#include <QtGui/QImage>
namespace Jpeg {
    QImage toQImage(const SharedImage &img);
}
#endif

#endif // IMAGE_HPP
