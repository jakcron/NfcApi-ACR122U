#pragma once
#include "mfc.h"

namespace nfc
{
	namespace mfc
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

			enum SectorKeyType
			{
				KEY_A,
				KEY_B,
			};

			enum KeyBankId
			{
				KEY_BANK_SLOT_0,
				KEY_BANK_SLOT_1
			};

			virtual void Connect() = 0;
			virtual void WaitUntilConnect() = 0;
			virtual void Disconnect() = 0;

			virtual uint32_t GetCardUid() = 0;

			virtual void LoadKey(const uint8_t key[kKeySize], KeyBankId key_bank) = 0;
			virtual void AuthenticateWithBlock(uint8_t block, KeyBankId key_bank, SectorKeyType key_type) = 0;

			virtual void ReadBlock(uint8_t block, uint8_t out[kBlockSize]) = 0;
			virtual void WriteBlock(uint8_t block, const uint8_t in[kBlockSize]) = 0;

			virtual uint32_t ReadValue(uint8_t block) = 0;
			virtual void WriteValue(uint8_t block, uint32_t value) = 0;
			virtual void IncrementValue(uint8_t block) = 0;
			virtual void DecrementValue(uint8_t block) = 0;
			virtual void CopyValue(uint8_t src_block, uint8_t dst_block) = 0;

			virtual void SetDebugOutput(bool enable_debug) = 0;
		};
	}
}


