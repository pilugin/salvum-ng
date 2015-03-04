#include <dbus/declareDBusMetatypes.h>
#include <QtDBus>
#include <QtDebug>

static struct DBusMetatypesRegistrer
{
    DBusMetatypesRegistrer()
    {
        qRegisterMetaType<Common::Result>("Common::Result");

        qDBusRegisterMetaType<Common::Result>();
    }
} theDBusMetatypesRegistrer;

///////////////// << & >>

const QDBusArgument &operator>>(const QDBusArgument &stream, Common::Result &result)
{
    stream.beginStructure();
    stream >> result.errorCode >> result.error;
    stream.endStructure();
    return stream;
}

QDBusArgument &operator<<(QDBusArgument &stream, const Common::Result &result)
{  
    stream.beginStructure();
    stream << result.errorCode << result.error;
    stream.endStructure();
    return stream;
}

