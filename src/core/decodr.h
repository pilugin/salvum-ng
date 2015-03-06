#ifndef JPEG_DECODR_H
#define JPEG_DECODR_H

#include <common/types.h>
#include <core/history.h>

namespace Core {

template <class Frame>
class Decodr
{
    static_assert(std::is_base_of<BaseFrame, Frame>::value, "Must be based on Core::BaseFrame");

public:
    void set(const Frame &frame);

    const Frame &decode(const Common::Cluster &cluster);

protected:
    virtual void doSet(Frame &frame)    { Q_UNUSED(frame) }
    virtual void doInit(Frame &frame)   { Q_UNUSED(frame) }
    virtual void doDecode(Frame &frame) { Q_UNUSED(frame) }
private:
    Frame mFrame;
};

//////////////////////////////////////

template <class Frame>
void Decodr<Frame>::set(const Frame &frame)
{
    mFrame = frame;
    doSet(mFrame);
}

template <class Frame>
const Frame &Decodr<Frame>::decode(const Common::Cluster &cluster)
{
    mFrame.setCluster(cluster);

    if (!mFrame.initialized()) {
        doInit(mFrame);
    }

    if (mFrame.initialized()) {
        doDecode(mFrame);
    }
    return mFrame;
}

}

#endif
