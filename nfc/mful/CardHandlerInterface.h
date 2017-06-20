#pragma once
#include "mful.h"

namespace nfc
{
	namespace mful
	{
		class CardHandlerInterface
		{
		public:
			enum NfcResultCode
			{
				NFC_ERROR_OK = 0x9000,
				NFC_ERROR_OPERATION_FAILED = 0x6300,
				NFC_ERROR_FUNCTION_NOT_SUPPORT = 0x6A81
			};

			virtual void Connect() = 0;
			virtual void WaitUntilConnect() = 0;
			virtual void Disconnect() = 0;

			virtual uint64_t GetCardUid() = 0;

			// abstract uint32_t read/write
			virtual uint32_t ReadPage(uint8_t addr) = 0;
			virtual void WritePage(uint8_t addr, uint32_t in) = 0;

			// raw data read/write
			virtual void ReadPage(uint8_t addr, uint8_t page_num, uint8_t *out) = 0;
			virtual void WritePage(uint8_t addr, uint8_t page_num, const uint8_t *in) = 0;

			virtual void SetDebugOutput(bool enable_debug) = 0;
		};
	}
}