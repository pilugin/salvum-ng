#ifndef SHARED_IMAGE_HPP
#define SHARED_IMAGE_HPP

#include <memory>
#include <QVector>
#include <QMap>
#include <QSize>
#include <QPoint>

namespace Jpeg {

struct Block
{
    enum { NUM_COLS = 8, NUM_ROWS = 8 };
    unsigned int *lines[NUM_ROWS];

    bool isNull() const { return lines[0] == nullptr; }

    Block(const Block &other);
    Block();
    Block &operator= (const Block &other);
};

struct ConstBlock
{
    const unsigned int *lines[Block::NUM_ROWS];
    bool isNull() const { return lines[0] == nullptr; }
};

struct ConstBlocks : public ConstBlock
{
    int span;
    bool ripped;
};

class ImagePart;

class Image
{
public:
    enum { INV_BAD_PART_ID = 0 };

    Image();

    std::shared_ptr<ImagePart> createPart();

    ConstBlocks getBlocks(int offset, int count, int badId = INV_BAD_PART_ID) const;
    Block getWritableBlock();

    bool atEnd() const;
    int blocksPerRow() const;
    int blocksPerCol() const;

    int createBadPart(int offset, int count);
    void dropBadPart(int badPartId);
    void moveBadPartBack(int badPartId);

    void init(int width, int height);

    QPoint getBlockCoordinates(int offset) const;
    QSize getSize() const { return mSize; }

    const unsigned int *scanline(int num) const { return mData.data() + num * mSize.width(); }
private:
    QSize mSize;
    QVector< unsigned int > mData;
    int mCurrentWritableBlock;

    struct BadPart
    {
        BadPart();
        int id;
        QVector< unsigned int > scanlines;
    };
    QVector< BadPart > mBadParts;
    int mBadIdGen;


    unsigned int *pointerByCoordinate(const QPoint &coordinate, int blockRow);
    const unsigned int *pointerByCoordinate(const QPoint &coordinate, int blockRow) const;
};

class ImagePart
{
public:
    bool isBad() const { return mBadPartId != Image::INV_BAD_PART_ID; }

    ConstBlocks getBlocks(int portion =0) const;

    Block addWritableBlock();

    void setBad();
    void setGoodAgain();

private:
    std::shared_ptr< Image > mImage;
    int mOffset;
    int mCount;

    int mBadPartId;
};

}

#ifdef QT_GUI_LIB
#include <QtGui/QImage>
namespace Jpeg {
    QImage toQImage(const Image &img);
}
#endif

#endif // IMAGE_HPP
