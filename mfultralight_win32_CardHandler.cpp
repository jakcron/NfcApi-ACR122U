#ifdef _MSC_VER

#include <nfc/mful/win32/CardHandler.h>
#include <nfc/Exception.h>

nfc::mful::win32::CardHandler::CardHandler()
{
	SetDebugOutput(false);
	connected_ = false;
	card_.EstablishContext(card_.CONTEXT_TYPE_USER);
}

nfc::mful::win32::CardHandler::~CardHandler()
{
	if (connected_ == true)
		card_.Disconnect();
}

void nfc::mful::win32::CardHandler::Connect()
{
	std::vector<std::string> reader_list;
	card_.GetReaderList(reader_list);

	for (size_t i = 0; i < reader_list.size(); i++)
	{
		if (reader_list[i].substr(0, 10) == "ACS ACR122")
		{
			card_.Connect(reader_list[i].c_str(), card_.ACCESS_MODE_SHARED);
			connected_ = true;
			return;
		}
	}
}

void nfc::mful::win32::CardHandler::WaitUntilConnect()
{
	std::vector<std::string> reader_list;
	card_.GetReaderList(reader_list);

	for (size_t i = 0; i < reader_list.size(); i++)
	{
		if (reader_list[i].substr(0, 10) == "ACS ACR122")
		{
		try_connect:
			try
			{
				card_.Connect(reader_list[i].c_str(), card_.ACCESS_MODE_SHARED);
			}
			catch (...)
			{
				goto try_connect;
			}
			//card_.SetBuzzerOnCardDetection(false);
			connected_ = true;
			return;
		}
	}
}

void nfc::mful::win32::CardHandler::Disconnect()
{
	card_.Disconnect();
}

uint64_t nfc::mful::win32::CardHandler::GetCardUid()
{
	if (connected_ == false) WaitUntilConnect();

	// Structure of response
	static const size_t datalen = 7;
	struct AdpuResponse {
		uint8_t value[datalen];
	} response;

	if (debug_output_) printf("[TX, CMD=GET_UID]\n");

	// Transmit
	card_.Transmit(GET_DATA, 0x00, 0x00, sizeof(AdpuResponse), nullptr, 0, (uint8_t*)&response, sizeof(AdpuResponse));


	// Process result
	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

		throw nfc::Exception("Failed to read value");
	}

	if (debug_output_) printf("[RX, RESULT = %04x, UID=%02x%02x%02x%02x]\n", card_.GetNfcResult(), response.value[0], response.value[1], response.value[2], response.value[3]);

	return (uint64_t)response.value[0] << 48 | (uint64_t)response.value[1] << 40 | (uint64_t)response.value[2] << 32 | (uint64_t)response.value[3] << 24 | (uint64_t)response.value[4] << 16 | (uint64_t)response.value[5] << 8 | (uint64_t)response.value[6] << 0;
}

uint32_t nfc::mful::win32::CardHandler::ReadPage(uint8_t addr)
{
	uint8_t page[kPageSize];
	ReadPage(addr, 1, page);
	return page[0] << 24 | page[1] << 16 | page[2] << 8 | page[3] << 0;
}

void nfc::mful::win32::CardHandler::WritePage(uint8_t addr, uint32_t in)
{
	uint8_t page[kPageSize] = { (in >> 24) & 0xff, (in >> 16) & 0xff, (in >> 8) & 0xff, (in) & 0xff };
	WritePage(addr, 1, page);
}

void nfc::mful::win32::CardHandler::ReadPage(uint8_t addr, uint8_t page_num, uint8_t *out)
{
	if (connected_ == false) WaitUntilConnect();

	if (debug_output_) printf("[TX, CMD=READ_PAGE, PAGE=%02d, NUM=%02d]\n", addr, page_num);

	// Transmit
	card_.Transmit(READ_BINARY_BLOCK, 0x00, addr, kPageSize*page_num, nullptr, 0, out, kPageSize*page_num);

	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

		throw nfc::Exception("Failed to read block");
	}

	if (debug_output_)
	{
		printf("[RX, RESULT = %04x, DATA=", card_.GetNfcResult());
		for (size_t i = 0; i < kPageSize*page_num; i++) printf("%02x", out[i]);
		printf("]\n");
	}
}

void nfc::mful::win32::CardHandler::WritePage(uint8_t addr, uint8_t page_num, const uint8_t *in)
{
	if (connected_ == false) WaitUntilConnect();

	if (debug_output_)
	{
		printf("[TX, CMD=WRITE_PAGE, PAGE=%02d, NUM=%02d, DATA=", addr, page_num);
		for (size_t i = 0; i < kPageSize*page_num; i++) printf("%02x", in[i]);
		printf("]\n");
	}

	// Transmit
	card_.Transmit(UPDATE_BINARY_BLOCK, 0x00, addr, kPageSize*page_num, in, kPageSize*page_num, nullptr, 0);

	if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		throw nfc::Exception("Failed to write block");
	}
}

void nfc::mful::win32::CardHandler::SetDebugOutput(bool enable_debug)
{
	debug_output_ = enable_debug;
	card_.SetAdpuDebug(debug_output_);
}

#endif