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

class Buffer
{
public:
    Buffer();
    Buffer(const Buffer &other);
    Buffer &operator=(const Buffer &other);

    void setData(const QByteArray &arr);
    bool equalsTo(const QByteArray &arr) const;

    QPair<const char *, int> read(int len);
    bool isEmpty() const;
    int bytesLeft() const;

private:
    QByteArray mData;
    int mPos;
};

}

Q_DECLARE_METATYPE(Common::Cluster)
Q_DECLARE_METATYPE(Common::Clusters)
Q_DECLARE_METATYPE(Common::Result)

#endif
