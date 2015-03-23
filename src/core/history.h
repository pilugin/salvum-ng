#ifndef CORE_HISTORY_H
#define CORE_HISTORY_H

#include <common/types.h>
#include <type_traits>

#include <QtCore/QList>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtDebug>

namespace Core {

class BaseFrame
{
public:
    BaseFrame();
    BaseFrame(const BaseFrame &other);
    virtual ~BaseFrame();

    BaseFrame &operator= (const BaseFrame &other);

    bool isFail() const;
    void restoreCheckOk();

    bool initOk() const     { return mInitOk; }
    bool decodOk() const    { return mDecodOk; }
    bool checkOk() const    { return mCheckOk; }

    void setInitialized()   { mInitOk = true; }
    void setDecodFailed()   { mDecodOk = false; }
    void setCheckFailed()   { mCheckOk = false; }

    Common::Cluster cluster() const { return mCluster; }
    void setCluster(const Common::Cluster &cluster) { mCluster = cluster; }

    bool bufferEmpty() const;
    QPair<const char *, int> bufferRead(int len);
    int bufferBytesLeft() const;

    virtual bool save(const QString &destPath) const;
    virtual bool load(const QString &sourcePath, const Common::Cluster &cluster);
protected:
    virtual bool saveMore(const QString &destPath, QFile &descFile) const;
    virtual bool loadMore(const QString &sourcePath, QFile &descFile);

private:
    bool mInitOk;
    bool mDecodOk;
    bool mCheckOk;

    Common::Cluster mCluster;
    Common::Buffer mBuffer;
};

template <class Frame>
class History
{
    static_assert(std::is_base_of<BaseFrame, Frame>::value, "Must be based on Core::BaseFrame");

public:
    History();
    bool addFrame(const Frame &frame);
    const Frame &getLastGoodFrame() const;

    int getFrameCount() const { return m_undoLine.size(); }
    const Frame &getFrame(int at) const { return m_undoLine[at]; }
    void selectFrame(int at);

    int blacklistedCluster() const { return m_blacklistedCluster; }

    bool save(const QDir &dest) const;    
    bool load(const QDir &source);
private:
    QList<Frame> m_undoLine;
    int m_blacklistedCluster;
};

//////////////////////////////////////////////
/// History<Frame> implementation
//////////////////////////////////////////////

template <class Frame>
History<Frame>::History() : m_blacklistedCluster(Common::InvalidClusterNo)
{
}

template <class Frame>
bool History<Frame>::addFrame(const Frame &frame)
{
    if (frame.isFail()) {
        if (m_undoLine.size()>0 && m_undoLine.back().isFail())
            return false;

        m_undoLine.push_back(frame);

    } else {
        m_undoLine.push_back(frame);
        m_blacklistedCluster = Common::InvalidClusterNo;

    }
    return true;

}

template <class Frame>
const Frame &History<Frame>::getLastGoodFrame() const
{
    Q_ASSERT(m_undoLine.size() >0);

    if (!m_undoLine.back().isFail())
        return m_undoLine.back();

    Q_ASSERT(m_undoLine.size() >1);
    return m_undoLine[ m_undoLine.size() -2 ];
}

template <class Frame>
void History<Frame>::selectFrame(int at)
{
    Q_ASSERT(at < m_undoLine.size() && at>0);

    int i=0;
    auto itr = m_undoLine.begin();
    while (i<at) {
        if (itr->isFail()) {
            itr = m_undoLine.erase(itr);
            --at;
        } else {
            ++i;
            ++itr;
        }
    }
    if (itr->isFail()) {
        itr->restoreCheckOk();

    } else {
        auto nextitr = itr;
        ++nextitr;
        if (nextitr != m_undoLine.end() && !nextitr->isFail()) {
            m_blacklistedCluster = nextitr->cluster().first;
        }
    }

    m_undoLine.erase(++itr, m_undoLine.end());
}

template <class Frame>
bool History<Frame>::save(const QDir &dest) const
{
    if (!dest.exists() &&  !QDir().mkpath( dest.absolutePath() ) ) {
        qDebug()<<"Failed to create dir:"<<dest.absolutePath();
        return false;
    }

    QFile clustersF(    dest.path() + "/clusters");
    QFile clusterNumsF( dest.path() + "/clusters.txt");
    QFile infoF(        dest.path() + "/info.txt");

    if (!clustersF.open(QFile::WriteOnly | QFile::Truncate) ) {
        qDebug()<<"Failed to create/open file:"<<clustersF.fileName();
        return false;
    }
    if (!clusterNumsF.open(QFile::WriteOnly | QFile::Truncate | QFile::Text) ) {
        qDebug()<<"Failed to create/open file:"<<clusterNumsF.fileName();
        return false;
    }
    if (!infoF.open(QFile::WriteOnly | QFile::Truncate | QFile::Text) ) {
        qDebug()<<"Failed to create/open file:"<<infoF.fileName();
        return false;
    }
    QDir framesD(dest.path() + "/frames");
    if (!framesD.exists() && !QDir::mkpath( framesD.absolutePath() )) {
        qDebug()<<"Failed to create dir:"<<framesD.absolutePath();
        return false;
    }

    for (const Frame &fr : m_undoLine) {
        QString num;
        num.sprintf("%08X", fr.cluster().first);
        if (!fr.save(framesD.path() + "/" + num)) {
            qDebug()<<"Failed to save frame:"<<(framesD.path() + "/" + num);
            return false;
        }
        clustersF.write(fr.cluster().second);
        clusterNumsF.write(num.toLatin1());
        if (fr.isFail())
            clusterNumsF.write("-");
        clusterNumsF.write("\n");
    }
    infoF.write(QString().sprintf("blacklisted:%08X\n", m_blacklistedCluster).toLatin1());

    return true;
}

template <class Frame>
bool History<Frame>::load(const QDir &source)
{
    QFile clustersF(    source.path() + "/clusters");
    QFile clusterNumsF( source.path() + "/clusters.txt");
    QFile infoF(        source.path() + "/info.txt");

    if (!clustersF.open(QFile::ReadOnly) ) {
        qDebug()<<"Failed to open file:"<<clustersF.fileName();
        return false;
    }
    if (!clusterNumsF.open(QFile::ReadOnly | QFile::Text) ) {
        qDebug()<<"Failed to open file:"<<clusterNumsF.fileName();
        return false;
    }
    if (!infoF.open(QFile::ReadOnly | QFile::Text) ) {
        qDebug()<<"Failed to open file:"<<infoF.fileName();
        return false;
    }

    const char *blacklisted = "blacklisted:";
    while (!infoF.atEnd()) {
        QByteArray line = infoF.readLine();
        if (line.startsWith(blacklisted)) {
            QByteArray num = line.constData() + strlen(blacklisted);
            bool ok;
            m_blacklistedCluster = num.toInt(&ok, 16);
            if (!ok) {
                qDebug()<<"Failed to parse line: "<<line;
                return false;
            }
        }
    }

    QDir framesD(source.path() + "/frames");
    while (!clusterNumsF.atEnd()) {
        QByteArray line = clusterNumsF.readLine();
        if (line.endsWith('\n'))
            line.resize(line.size()-1);
        if (line.size() == 0)
            continue;
        bool isFail = false;
        if (line.endsWith('-')) {
            isFail = true;
            line.resize(line.size()-1);
        }
        bool ok;
        Common::Cluster cluster;
        cluster.first = line.toInt(&ok, 16);
        if (!ok) {
            qDebug()<<"Failed to parse clusterNo: "<<line;
            return false;
        }
        cluster.second = clustersF.read(Common::ClusterSize);
        if (cluster.second.size() != Common::ClusterSize) {
            qDebug()<<"Failed to read full cluster: (no,size)="<<cluster.first<<cluster.second.size();
            return false;
        }

        m_undoLine.push_back(Frame());
        QString numFrame;
        numFrame.sprintf("/%08X", cluster.first);
        if (!m_undoLine.back().load(framesD.path() + numFrame, cluster)) {
            qDebug()<<"Failed to load frame: "<<(framesD.path() + numFrame);
            return false;
        }
    }

    return true;
}

}

#endif
