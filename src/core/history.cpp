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

bool BaseFrame::save(const QString &destPath) const
{
    QDir dest(destPath);
    if (!dest.exists() &&  !QDir().mkpath( dest.absolutePath() ) ) {
        qDebug()<<"Failed to create dir:"<<dest.absolutePath();
        return false;
    }

    QFile descF( destPath + "/desc.txt");
    QFile buffF( destPath + "/buffer");
    if (!descF.open(QFile::WriteOnly | QFile::Truncate | QFile::Text) ) {
        qDebug()<<"Failed to create/open file:"<<descF.fileName();
        return false;
    }
    if (!buffF.open(QFile::WriteOnly | QFile::Truncate) ) {
        qDebug()<<"Failed to create/open file:"<<buffF.fileName();
        return false;
    }

    descF.write(QString().sprintf("%s%s%s\n",   mInitOk ? "i" : "",
                                                mDecodOk ? "d" : "",
                                                mCheckOk ? "c" : ""
                                  ).toLatin1());
    buffF.write(mBuffer.data(), mBuffer.bytesLeft());

    return saveMore(destPath, descF);
}

bool BaseFrame::load(const QString &sourcePath, const Common::Cluster &cluster)
{
    QFile descF( sourcePath + "/desc.txt");
    QFile buffF( sourcePath + "/buffer");

    if (!descF.open(QFile::ReadOnly | QFile::Text)) {
        qDebug()<<"Failed to open file: "<<descF.fileName();
        return false;
    }
    if (!buffF.open(QFile::ReadOnly)) {
        qDebug()<<"Failed to open file: "<<buffF.fileName();
        return false;
    }

    QByteArray desc = descF.readLine();
    mInitOk = desc.contains('i');
    mDecodOk = desc.contains('d');
    mCheckOk = desc.contains('c');

    mBuffer.setData(buffF.readAll());

    return loadMore(sourcePath, descF);
}

bool BaseFrame::saveMore(const QString &destPath, QFile &descFile) const
{
    return true;
}

bool BaseFrame::loadMore(const QString &sourcePath, QFile &descFile)
{
    return true;
}

}
