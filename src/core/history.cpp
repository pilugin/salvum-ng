#include <core/history.h>

namespace Core {

BaseFrame::BaseFrame()
    : mInitOk(false)
    , mDecodOk(true)
    , mCheckOk(true)
{

}

BaseFrame::BaseFrame(const BaseFrame &other)
{
    *this = other;
}

BaseFrame::~BaseFrame()
{

}

BaseFrame &BaseFrame::operator =(const BaseFrame &other)
{
    mInitOk = other.mInitOk;
    mDecodOk = other.mDecodOk;
    mCheckOk = other.mCheckOk;
    mCluster = other.mCluster;
    mBuffer = other.mBuffer;

    return *this;
}

void BaseFrame::restoreCheckOk()
{
    mCheckOk = mDecodOk = true;
}

bool BaseFrame::isFail() const
{
    return mDecodOk && mCheckOk;
}

bool BaseFrame::bufferEmpty() const
{
    // Buffer is empty and ALREADY copied from Cluster
    return mBuffer.isEmpty() && mBuffer.equalsTo(mCluster.second);
}

QPair<const char *, int> BaseFrame::bufferRead(int len)
{
    if (mBuffer.isEmpty() && !mBuffer.equalsTo(mCluster.second))
        mBuffer.setData(mCluster.second);

    return mBuffer.read(len);
}

int BaseFrame::bufferBytesLeft() const
{
    int b = mBuffer.bytesLeft();
    if (!mBuffer.equalsTo(mCluster.second))
        b += mCluster.second.size();

    return b;
}

}
