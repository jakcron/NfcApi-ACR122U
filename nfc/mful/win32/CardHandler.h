#pragma once
#ifdef _MSC_VER
#include "../CardHandlerInterface.h"
#include "../../drv/win32/ACR122U.h"

namespace nfc
{
	namespace mful
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

				uint64_t GetCardUid();

				// abstract uint32_t read/write
				uint32_t ReadPage(uint8_t addr);
				void WritePage(uint8_t addr, uint32_t in);

				// raw data read/write
				void ReadPage(uint8_t page_addr, uint8_t page_num, uint8_t *out);
				void WritePage(uint8_t page_addr, uint8_t page_num, const uint8_t *in); // max page num 4 words max (aka 16bytes)

				void SetDebugOutput(bool enable_debug);

			private:
				enum AdpuInstructionId
				{
					READ_BINARY_BLOCK = 0xB0,
					GET_DATA = 0xCA,
					UPDATE_BINARY_BLOCK = 0xD6,
				};

				bool debug_output_;
				bool connected_;
				drv::win32::ACR122U card_;
			};
		}

	}
}
#endif