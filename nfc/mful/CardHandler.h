#pragma once
#include "CardHandlerInterface.h"

#ifdef _MSC_VER
#define MIFARE_ULTRALIGHT_HANDLER
#include "win32/CardHandler.h"

namespace nfc
{
	namespace mful
	{
		class CardHandler : public win32::CardHandler {};
	}
}
#endif
