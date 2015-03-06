#ifndef CORE_HISTORY_H
#define CORE_HISTORY_H

#include <common/types.h>
#include <type_traits>

#include <QtCore/QList>

namespace Core {

class BaseFrame
{
public:
    BaseFrame();
    BaseFrame(const BaseFrame &other);

    BaseFrame &operator= (const BaseFrame &other);

    void setFail(bool fail =true)   { /*m_isFail = fail;*/ }
    bool isFail() const             { return false; /*m_isFail;*/ } // connect this to with decod/check

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

protected:

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
    void addFrame(const Frame &frame);
    bool addFailedFrame(const Frame &frame); //< adds only if previous frame wasn't failed; return true on add
    const Frame &getLastGoodFrame() const;

    int getFrameCount() const { return m_undoLine.size(); }
    const Frame &getFrame(int at) const { return m_undoLine[at]; }
    void selectFrame(int at);

    int blacklistedCluster() const { return m_blacklistedCluster; }

private:
    QList<Frame> m_undoLine;
    int m_blacklistedCluster;
};

//////////////////////////////////////////////

template <class Frame>
History<Frame>::History() : m_blacklistedCluster(Common::InvalidClusterNo)
{
}

template <class Frame>
void History<Frame>::addFrame(const Frame &frame)
{
    m_undoLine.push_back(frame);
    m_undoLine.back().setFail(false);
    m_blacklistedCluster = Common::InvalidClusterNo;
}

template <class Frame>
bool History<Frame>::addFailedFrame(const Frame &frame)
{
    if (m_undoLine.size()>0 && m_undoLine.back().isFail())
        return false;

    m_undoLine.push_back(frame);
    m_undoLine.back().setFail();
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
        itr->setFail(false);

    } else {
        auto nextitr = itr;
        ++nextitr;
        if (nextitr != m_undoLine.end() && !nextitr->isFail()) {
            m_blacklistedCluster = nextitr->cluster().first;
        }
    }

    m_undoLine.erase(++itr, m_undoLine.end());
}


}

#endif
