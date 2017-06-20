#pragma once
#ifdef _MSC_VER
#include "../CardHandlerInterface.h"
#include "../../drv/win32/ACR122U.h"

namespace nfc
{
	namespace mfc
	{
		namespace win32
		{
			class CardHandler : public CardHandlerInterface
			{
			public:
				CardHandler();
				~CardHandler();

				void Connect();
				void WaitUntilConnect();
				void Disconnect();

				uint32_t GetCardUid();

				void LoadKey(const uint8_t key[kKeySize], KeyBankId key_bank);
				void AuthenticateWithBlock(uint8_t block, KeyBankId key_bank, SectorKeyType key_type);

				void ReadBlock(uint8_t block, uint8_t out[kBlockSize]);
				void WriteBlock(uint8_t block, const uint8_t in[kBlockSize]);

				uint32_t ReadValue(uint8_t block);
				void WriteValue(uint8_t block, uint32_t value);
				void IncrementValue(uint8_t block);
				void DecrementValue(uint8_t block);
				void CopyValue(uint8_t src_block, uint8_t dst_block);

				void SetDebugOutput(bool enable_debug);

			private:
				enum AdpuInstructionId
				{
					LOAD_AUTH_KEYS = 0x82,
					AUTHENTICATE_BLOCK = 0x86,
					READ_BINARY_BLOCK = 0xB0,
					READ_VALUE_BLOCK = 0xB1,
					GET_DATA = 0xCA,
					UPDATE_BINARY_BLOCK = 0xD6,
					UPDATE_VALUE_BLOCK = 0xD7,
				};

				enum ValueUpdateType
				{
					WRITE_VALUE = 0x00,
					INCREMENT_VALUE = 0x01,
					DECREMENT_VALUE = 0x02,
					COPY_VALUE = 0x03,
				};

				bool debug_output_;
				bool connected_;
				drv::win32::ACR122U card_;
			};
		}
		
	}
}
#endif