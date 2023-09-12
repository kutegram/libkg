#include "tgclient.h"

#include "tlschema.h"

TgLongVariant TgClient::usersGetUsers(TgVector users)
{
    TGOBJECT(TLType::UsersGetUsersMethod, method);
    if (users.isEmpty()) {
        TGOBJECT(TLType::InputUserSelf, self);
        users.append(self);
    }
    method["id"] = users;

    return sendObject<&writeTLMethodUsersGetUsers>(method);
}
