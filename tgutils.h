#ifndef TGUTILS_H
#define TGUTILS_H

#include "tgstream.h"

TgObject emptyPeer();
TgObject toInputPeer(TgObject obj);
TgLong getPeerId(TgObject obj);
bool isUser(TgObject obj);
bool isChat(TgObject obj);
TgObject dialogsOffset(TgObject dialogs);

#endif // TGUTILS_H
