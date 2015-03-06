#include <common/types.h>
#include <QtAlgorithms>

static struct QtMetatypesRegistrer
{
    QtMetatypesRegistrer()
    {
        qRegisterMetaType<Common::Cluster>("Common::Cluster");
        qRegisterMetaType<Common::Clusters>("Common::Clusters");
        qRegisterMetaType<Common::Result>("Common::Result");
    }
} theQtMetatypesRegistrer;

namespace Common {

Buffer::Buffer()
    : mPos(0)
{

}

Buffer::Buffer(const Buffer &other)
{
    *this = other;
}

Buffer &Buffer::operator=(const Buffer &other)
{
    mData = other.mData;
    mPos = other.mPos;
    return *this;
}

void Buffer::setData(const QByteArray &arr)
{
    mData = arr;
    mPos = 0;
}

bool Buffer::equalsTo(const QByteArray &arr) const
{
    return !mData.isNull() && mData.isSharedWith(arr);
}

QPair<const char *, int> Buffer::read(int len)
{
    if (isEmpty())
        return qMakePair((const char *)nullptr, 0);

    auto rv = qMakePair( mData.constData() + mPos, qMin( len, mData.size() - mPos));
    mPos += rv.second;

    return rv;
}

bool Buffer::isEmpty() const
{
    return mData.isEmpty() || mPos >= mData.size();
}

int Buffer::bytesLeft() const
{
    return isEmpty() ? 0 : mData.size() - mPos;
}

}
