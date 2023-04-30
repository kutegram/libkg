#include "tgclient.h"

#include "apisecrets.h"
#include "tlschema.h"

qint64 TgClient::authSendCode(QString phoneNumber)
{
    TGOBJECT(TLType::AuthSendCodeMethod, method);

    TGOBJECT(TLType::CodeSettings, codeSettings);
    method["settings"] = codeSettings;

    method["phone_number"] = phoneNumber;
    method["api_id"] = KUTEGRAM_API_ID;
    method["api_hash"] = KUTEGRAM_API_HASH;

    return sendObject<&writeTLMethodAuthSendCode>(method);
}
