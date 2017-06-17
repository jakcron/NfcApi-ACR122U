#ifdef _MSC_VER

#include "nfc/mfc/win32/CardHandler.h"
#include "nfc/Exception.h"

nfc::mfc::win32::CardHandler::CardHandler()
{
	SetDebugOutput(false);
	connected_ = false;
	card_.EstablishContext(card_.CONTEXT_TYPE_USER);
}

nfc::mfc::win32::CardHandler::~CardHandler()
{
	if (connected_ == true)
		card_.Disconnect();
}

void nfc::mfc::win32::CardHandler::Connect()
{
	std::vector<std::string> reader_list;
	card_.GetReaderList(reader_list);

	for (size_t i = 0; i < reader_list.size(); i++)
	{
		if (reader_list[i].substr(0,10) == "ACS ACR122")
		{
			card_.Connect(reader_list[i].c_str(), card_.ACCESS_MODE_SHARED);
			connected_ = true;
			return;
		}
	}
}

void nfc::mfc::win32::CardHandler::WaitUntilConnect()
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
			catch (const nfc::Exception& e)
			{
				goto try_connect;
			}
			//card_.SetBuzzerOnCardDetection(false);
			connected_ = true;
			return;
		}
	}
}

void nfc::mfc::win32::CardHandler::Disconnect()
{
	card_.Disconnect();
}

uint32_t nfc::mfc::win32::CardHandler::GetCardUid()
{
	if (connected_ == false) WaitUntilConnect();

	// Structure of response
	static const size_t datalen = 4;
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


	return response.value[0] << 24 | response.value[1] << 16 | response.value[2] << 8 | response.value[3] << 0;
}

void nfc::mfc::win32::CardHandler::LoadKey(const uint8_t key[kKeySize], KeyBankId key_bank)
{
	if (connected_ == false) WaitUntilConnect();

	if (debug_output_) printf("[TX, CMD=LOAD_AUTH_KEYS, KEY_BANK=%01d, KEY=%02x%02x%02x%02x%02x%02x]\n", key_bank, key[0], key[1], key[2], key[3], key[4], key[5]);

	// Transmit
	card_.Transmit(LOAD_AUTH_KEYS, 0x00, key_bank, kKeySize, key, kKeySize, nullptr, 0);

	if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		throw nfc::Exception("Failed to load auth key");
	}
}

void nfc::mfc::win32::CardHandler::AuthenticateWithBlock(uint8_t block, KeyBankId key_bank, SectorKeyType key_type)
{
	if (connected_ == false) WaitUntilConnect();

	// Structure of payload
	struct AdpuPayload {
		uint8_t auth_version = 1;
		uint8_t reserved = 0;
		uint8_t block;
		uint8_t key_type;
		uint8_t key_bank;
	} payload;

	payload.block = block;
	payload.key_type = key_type == KEY_A ? 0x60 : 0x61;
	payload.key_bank = key_bank;

	if (debug_output_) printf("[TX, CMD=AUTHENTICATE_BLOCK, BLOCK=%d, KEY_BANK=%01d, KEY_TYPE=%s]\n", block, key_bank, key_type == KEY_A? "KEY_A" : "KEY_B");

	// Transmit
	card_.Transmit(AUTHENTICATE_BLOCK, 0x00, 0x00, sizeof(AdpuPayload), (const uint8_t*)&payload, sizeof(AdpuPayload), nullptr, 0);

	if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		throw nfc::Exception("Failed to authenticate with block");
	}
}

void nfc::mfc::win32::CardHandler::ReadBlock(uint8_t block, uint8_t out[kBlockSize])
{
	if (connected_ == false) WaitUntilConnect();

	if (debug_output_) printf("[TX, CMD=READ_BLOCK, BLOCK=%02d]\n", block);

	// Transmit
	card_.Transmit(READ_BINARY_BLOCK, 0x00, block, kBlockSize, nullptr, 0, out, kBlockSize);

	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

		throw nfc::Exception("Failed to write block");
	}

	if (debug_output_) printf("[RX, RESULT = %04x, DATA=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n", card_.GetNfcResult(), out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7], out[8], out[9], out[10], out[11], out[12], out[13], out[14], out[15]);
}

void nfc::mfc::win32::CardHandler::WriteBlock(uint8_t block, const uint8_t in[kBlockSize])
{
	if (connected_ == false) WaitUntilConnect();

	if (debug_output_) printf("[TX, CMD=WRITE_BLOCK, BLOCK = %d, DATA=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n", block, in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15]);

	// Transmit
	card_.Transmit(UPDATE_BINARY_BLOCK, 0x00, block, kBlockSize, in, kBlockSize, nullptr, 0);

	if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		throw nfc::Exception("Failed to write block");
	}
}

uint32_t nfc::mfc::win32::CardHandler::ReadValue(uint8_t block)
{
	if (connected_ == false) WaitUntilConnect();

	// Structure of response
	struct AdpuResponse {
		uint8_t value[4];
	} response;

	if (debug_output_) printf("[TX, CMD=READ_VALUE, BLOCK=%d]\n", block);

	// Transmit
	card_.Transmit(READ_VALUE_BLOCK, 0x00, block, sizeof(AdpuResponse), nullptr, 0, (uint8_t*)&response, sizeof(AdpuResponse));

	// Process result
	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

		throw nfc::Exception("Failed to read value");
	}

	if (debug_output_) printf("[RX, RESULT = %04x, UID=%02x%02x%02x%02x]\n", card_.GetNfcResult(), response.value[0], response.value[1], response.value[2], response.value[3]);

	return response.value[0] << 24 | response.value[1] << 16 | response.value[2] << 8 | response.value[3] << 0;
}

void nfc::mfc::win32::CardHandler::WriteValue(uint8_t block, uint32_t value)
{
	if (connected_ == false) WaitUntilConnect();

	// Structure of payload
	struct AdpuPayload {
		uint8_t val_update_type = ValueUpdateType::WRITE_VALUE;
		uint8_t value[4];
	} payload;

	payload.value[0] = (value >> 24) & 0xFF;
	payload.value[1] = (value >> 16) & 0xFF;
	payload.value[2] = (value >> 8) & 0xFF;
	payload.value[3] = (value >> 0) & 0xFF;

	if (debug_output_) printf("[TX, CMD=WRITE_VALUE, BLOCK=%d, VALUE=%08x]\n", block, value);

	// Transmit
	card_.Transmit(UPDATE_VALUE_BLOCK, 0x00, block, sizeof(AdpuPayload), (const uint8_t*)&payload, sizeof(AdpuPayload), nullptr, 0);

	if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		throw nfc::Exception("Failed to write value");
	}
}

void nfc::mfc::win32::CardHandler::IncrementValue(uint8_t block)
{
	if (connected_ == false) WaitUntilConnect();

	// Structure of payload
	struct AdpuPayload {
		uint8_t val_update_type = ValueUpdateType::INCREMENT_VALUE;
	} payload;

	if (debug_output_) printf("[TX, CMD=INCREMENT_VALUE, BLOCK=%d]\n", block);

	// Transmit
	card_.Transmit(UPDATE_VALUE_BLOCK, 0x00, block, sizeof(AdpuPayload), (const uint8_t*)&payload, sizeof(AdpuPayload), nullptr, 0);

	if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		throw nfc::Exception("Failed to increment value");
	}
}

void nfc::mfc::win32::CardHandler::DecrementValue(uint8_t block)
{
	if (connected_ == false) WaitUntilConnect();

	// Structure of payload
	struct AdpuPayload {
		uint8_t val_update_type = ValueUpdateType::DECREMENT_VALUE;
	} payload;

	if (debug_output_) printf("[TX, CMD=DECREMENT_VALUE, BLOCK=%d]\n", block);

	// Transmit
	card_.Transmit(UPDATE_VALUE_BLOCK, 0x00, block, sizeof(AdpuPayload), (const uint8_t*)&payload, sizeof(AdpuPayload), nullptr, 0);

	if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		throw nfc::Exception("Failed to decrement value");
	}
}

void nfc::mfc::win32::CardHandler::CopyValue(uint8_t src_block, uint8_t dst_block)
{
	if (connected_ == false) WaitUntilConnect();

	// Structure of payload
	struct AdpuPayload {
		uint8_t val_update_type = ValueUpdateType::COPY_VALUE;
		uint8_t dst_block;
	} payload;

	// Set arguments
	payload.dst_block = dst_block;

	if (debug_output_) printf("[TX, CMD=COPY_VALUE, SRC_BLOCK=%d, DST_BLOCK=%d]\n", src_block, dst_block);

	// Transmit
	card_.Transmit(UPDATE_VALUE_BLOCK, 0x00, src_block, sizeof(AdpuPayload), (const uint8_t*)&payload, sizeof(AdpuPayload), nullptr, 0);

	if (debug_output_) printf("[RX, RESULT = %04x]\n", card_.GetNfcResult());

	if (card_.GetNfcResult() != NFC_ERROR_OK)
	{
		throw nfc::Exception("Failed to copy value");
	}
}

void nfc::mfc::win32::CardHandler::SetDebugOutput(bool enable_debug)
{
	debug_output_ = enable_debug;
}

#endif