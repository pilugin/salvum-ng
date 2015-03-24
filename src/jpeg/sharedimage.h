#ifndef SHARED_IMAGE_HPP
#define SHARED_IMAGE_HPP

#include <picojpeg.h>
#include <util/array.h>
#include <memory>
#include <QMap>
#include <QSize>
#include <QPoint>

namespace Jpeg {

enum JpegScanType
{
    H1   = 0x01,
    H2   = 0x02,
    V1   = 0x10,
    V2   = 0x20,
    
    H1V1 = H1 | V1,
    H2V1 = H2 | V1,
    H1V2 = H1 | V2,
    H2V2 = H2 | V2
};

struct Block
{
    static int numCols(JpegScanType scanType);
    static int numRows(JpegScanType scanType);
    
    unsigned int *linePtr;
    int rowOffset;

    bool isNull() const { return linePtr == nullptr; }    
    unsigned int *line(int row) { return linePtr + row*rowOffset; }

    Block();
};

struct ConstBlock
{
    const unsigned int *linePtr;
    unsigned int rowOffset;
    
    bool isNull() const { return linePtr == nullptr; }
    const unsigned int *line(int row) const { return linePtr + rowOffset*row; }
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
    bool addBlock(pjpeg_image_info_t *pjpegInfo);

    bool setBad(); //< return false if SharedImage is lack of free space
    void setGoodAgain();
    
    int blockWidth() const;
    int blockHeight() const;

private:    
    void copyPixels(Block &block, int x, int cx, int y, int cy, unsigned char *r, unsigned char *g, unsigned char *b);

    SharedImage *mImage;
    int mOffset;
    int mCount;
    int mBadOffset;
};

class SharedImage
{
public:
    SharedImage(int width, int height, JpegScanType scanType, int badSectorPercentRatio =10);

    ImagePart createPart();

    ConstBlocks getBlocks(int offset, int count) const;
    Block getWritableBlock();

    int totalBlockCount() const;
    bool atEnd() const;
    int blocksInRow() const;
    int blocksInCol() const;

    int createBadPart(int goodOffset, int count); //< returns offset
    void moveBadPartBack(int badOffset, int count, int goodOffset);
    void dropBadParts();

    QPoint getBlockCoordinates(int offset) const;
    QSize getSize() const { return mSize; }
    
    int getBadSectorSize() const; //< these 3 functions return size in Blocks 
    int getBadSectorUsed() const;
    int getBadSectorFree() const { return getBadSectorSize() - getBadSectorUsed(); }

    const unsigned int *scanline(int num) const { return mData.data() + num * mSize.width(); }
    JpegScanType getScanType() const { return mScanType; }
private:
    void copyBlocks(int srcOffset, int dstOffset, int count);
    unsigned int *pointerByCoordinate(const QPoint &coordinate, int blockRow);
    const unsigned int *pointerByCoordinate(const QPoint &coordinate, int blockRow) const;

    JpegScanType mScanType;
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
