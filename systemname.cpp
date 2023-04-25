#include "systemname.h"

QString systemName()
{
#if defined(Q_OS_ANDROID)
    return "Android";
#elif defined(Q_OS_SYMBIAN)
    switch (QSysInfo::symbianVersion()) {
    case QSysInfo::SV_9_2:
        return "Symbian OS v9.2";
    case QSysInfo::SV_9_3:
        return "Symbian OS v9.3";
    case QSysInfo::SV_9_4:
        return "Symbian OS v9.4";
    case QSysInfo::SV_SF_2:
        return "Symbian^2";
    case QSysInfo::SV_SF_3:
        return "Symbian^3";
    case QSysInfo::SV_SF_4:
        return "Symbian^4";
    default:
        return "Symbian";
    }
#elif defined(Q_OS_BLACKBERRY)
    return "Blackberry OS";
#elif defined(Q_OS_IOS)
    return "iOS";
#elif defined(Q_OS_MACOS)
    //TODO: QSysInfo version
    return "macOS";
#elif defined(Q_OS_WIN)
    switch (QSysInfo::windowsVersion()) {
    case QSysInfo::WV_32s:
        return "Windows 3.1 with Win32s";
    case QSysInfo::WV_95:
        return "Windows 95";
    case QSysInfo::WV_98:
        return "Windows 98";
    case QSysInfo::WV_Me:
        return "Windows Me";
    case QSysInfo::WV_NT:
        return "Windows NT";
    case QSysInfo::WV_2000:
        return "Windows 2000";
    case QSysInfo::WV_XP:
        return "Windows XP";
    case QSysInfo::WV_2003:
        return "Windows 2003";
    case QSysInfo::WV_VISTA:
        return "Windows Vista";
    case QSysInfo::WV_WINDOWS7:
        return "Windows 7";
    case QSysInfo::WV_CE:
        return "Windows CE";
    case QSysInfo::WV_CENET:
        return "Windows CE .NET";
    case QSysInfo::WV_CE_5:
        return "Windows CE 5";
    case QSysInfo::WV_CE_6:
        return "Windows CE 6";
    default:
#if defined(Q_OS_WINCE)
        return "Windows CE";
#else
        return "Windows";
#endif
    }
#elif defined(Q_OS_LINUX)
    return "Linux";
#elif defined(Q_OS_UNIX)
    return "Unix";
#else
    //TODO: more systems macros
    return "Unknown";
#endif
}
