#ifndef DECLARE_DBUS_METATYPES_H
#define DECLARE_DBUS_METATYPES_H

#include <common/types.h>

#include <QDBusArgument>

const QDBusArgument &operator>>(const QDBusArgument &stream, Common::Result &result);

QDBusArgument &operator<<(QDBusArgument &stream, const Common::Result &result);

#endif
