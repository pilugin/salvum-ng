#include <core/history.h>

using namespace Core;

void F()
{
    History<BaseFrame> h;

    h.addFrame(BaseFrame());
    h.addFrame(BaseFrame());
}
