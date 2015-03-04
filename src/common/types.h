#ifndef DBUS_TYPES_H
#define DBUS_TYPES_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QMetaType>

namespace Common {

enum { InvalidClusterNo =-1 };

typedef QPair<int, QByteArray> Cluster;

typedef QList<Cluster> Clusters;

struct Result
{
    operator bool() const { return errorCode == 0; }
    
    int errorCode; //< 0 - success
    QString error;
};

}

Q_DECLARE_METATYPE(Common::Cluster)
Q_DECLARE_METATYPE(Common::Clusters)
Q_DECLARE_METATYPE(Common::Result)

#endif
