#ifndef DEBUG_H
#define DEBUG_H

#include <QDebug>

#ifndef KG_NO_INFO
#define kgInfo() qDebug() << "[INFO]"
#else
#define kgInfo() QNoDebug()
#endif

#ifndef KG_NO_DEBUG
#define kgDebug() qDebug() << "[DEBG]"
#else
#define kgDebug() QNoDebug()
#endif

#ifndef KG_NO_WARNING
#define kgWarning() qWarning() << "[WARN]"
#else
#define kgWarning() QNoDebug()
#endif

#ifndef KG_NO_CRITICAL
#define kgCritical() qCritical() << "[CRIT]"
#else
#define kgCritical() QNoDebug()
#endif

#ifndef KG_NO_FATAL
#define kgFatal() qFatal() << "[FATL]"
#else
#define kgFatal() QNoDebug()
#endif

#endif // DEBUG_H
