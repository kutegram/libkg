#include "tgclient.h"

#include "apisecrets.h"
#include "tlschema.h"

TgLong TgClient::authSendCode(QString phoneNumber)
{
    TGOBJECT(TLType::AuthSendCodeMethod, method);

    TGOBJECT(TLType::CodeSettings, codeSettings);
    method["settings"] = codeSettings;

    method["phone_number"] = phoneNumber;

#if defined(KUTEGRAM_API_ID)
    method["api_id"] = KUTEGRAM_API_ID;
#else
    #error "Please, specify an API id."
#endif

#if defined(KUTEGRAM_API_HASH)
    method["api_hash"] = KUTEGRAM_API_HASH;
#else
    #error "Please, specify an API hash."
#endif

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

TgLong TgClient::authSignUp(QString phoneNumber, QString phoneCodeHash, QString firstName, QString lastName)
{
    TGOBJECT(TLType::AuthSignUpMethod, method);

    method["phone_number"] = phoneNumber;
    method["phone_code_hash"] = phoneCodeHash;
    method["first_name"] = firstName;
    method["last_name"] = lastName;

    return sendObject<&writeTLMethodAuthSignUp>(method);
}
