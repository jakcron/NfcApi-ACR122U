#pragma once
#include "CardHandlerInterface.h"

#ifdef _MSC_VER
#define MIFARE_CLASSIC_HANDLER
#include "win32/CardHandler.h"

namespace nfc
{
	namespace mfc
	{
		class CardHandler : public win32::CardHandler {};
	}
}
#endif
