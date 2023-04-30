#include "tgclient.h"

#include <QStringList>
#include "tlschema.h"

qint64 TgClient::helpGetCountriesList(qint32 hash, QString langCode)
{
    TGOBJECT(TLType::HelpGetCountriesListMethod, method);

    if (langCode.isEmpty())
        langCode = QLocale::system().name().split("_")[0];
    method["lang_code"] = langCode;
    method["hash"] = hash;

    return sendObject<&writeTLMethodHelpGetCountriesList>(method);
}

