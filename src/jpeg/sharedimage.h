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
    BAD_SCAN_TYPE = 0,

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

struct ImagePart
{
    ImagePart(SharedImage *image =nullptr, int offset =0, int count =0, int badOffset =0);

    bool isNull() const { return image == nullptr; }
    bool isBad() const { return badOffset > 0; }

    ConstBlocks getBlocks(int blocksOffset =0) const;

    Block addWritableBlock();
    bool addBlock(pjpeg_image_info_t *pjpegInfo);

    bool setBad(); //< return false if SharedImage is lack of free space
    void setGoodAgain();
    
    int blockWidth() const;
    int blockHeight() const;

    int offset;
    int count;
    int badOffset;
    SharedImage *image;
private:    
    void copyPixels(Block &block, int x, int cx, int y, int cy, unsigned char *r, unsigned char *g, unsigned char *b);

};

class SharedImage
{
public:
    SharedImage(JpegScanType scanType, int width, int height, int badSectorPercentRatio =10);
    static int calculateSizeof(JpegScanType scanType, int width, int height, int badSectorPercentRatio =10);

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
    int getBadSectorSize() const; //< these 3 functions return size in Blocks 
    int getBadSectorUsed() const;
    int getBadSectorFree() const { return getBadSectorSize() - getBadSectorUsed(); }

    QPoint getBlockCoordinates(int offset) const;
    const unsigned int *scanline(int num) const { return mData.data() + num * mSize.width(); }

    // these used for save/load    
    JpegScanType getScanType() const    { return mScanType; }
    QSize getSize() const               { return mSize; }
    int getCurrentWritableBlock() const { return mCurrentWritableBlock; }
    int getCurrentBadBlock() const      { return mCurrentBadBlock; }
    int getBadSectorRatio() const       { return mBadSectorRatio; }    
    const char *getData() const         { return reinterpret_cast<const char *>(mData.data()); }
    int         getDataSize() const     { return mData.capacity() * sizeof(unsigned int); }
    bool load(const char *data, int size, int writableBlock, int badBlock);
    
private:
    static int dataCapacity(JpegScanType scanType, int width, int height, int badSectorRatio);
    void copyBlocks(int srcOffset, int dstOffset, int count);
    unsigned int *pointerByCoordinate(const QPoint &coordinate, int blockRow);
    const unsigned int *pointerByCoordinate(const QPoint &coordinate, int blockRow) const;

    JpegScanType mScanType;
    int mBadSectorRatio;
    QSize mSize;
    int mCurrentWritableBlock;
    int mCurrentBadBlock;
    DynArray<unsigned int> mData;
};

class BaseSharedImageAllocator
{
public:
    virtual ~BaseSharedImageAllocator();
    virtual SharedImage *alloc(JpegScanType scanType, int width, int height, int badSectorPercentRatio =10) =0;
    virtual void free(SharedImage *image) =0;
};

}

#ifdef QT_GUI_LIB
#include <QtGui/QImage>
namespace Jpeg {
    QImage toQImage(const SharedImage &img);
}
#endif

#endif // IMAGE_HPP
