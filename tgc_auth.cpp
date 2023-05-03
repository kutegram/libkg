#include "tgclient.h"

#include "apisecrets.h"
#include "tlschema.h"

TgLong TgClient::authSendCode(QString phoneNumber)
{
    TGOBJECT(TLType::AuthSendCodeMethod, method);

    TGOBJECT(TLType::CodeSettings, codeSettings);
    method["settings"] = codeSettings;

    method["phone_number"] = phoneNumber;
    method["api_id"] = KUTEGRAM_API_ID;
    method["api_hash"] = KUTEGRAM_API_HASH;

    return sendObject<&writeTLMethodAuthSendCode>(method);
}

TgLong TgClient::authSignIn(QString phoneNumber, QString phoneCodeHash, QString phoneCode)
{
    TGOBJECT(TLType::AuthSignInMethod, method);

    method["phone_number"] = phoneNumber;
    method["phone_code_hash"] = phoneCodeHash;
    method["phone_code"] = phoneCode;

    return sendObject<&writeTLMethodAuthSignIn>(method);
}
