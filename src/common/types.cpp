#include <common/types.h>

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

}
