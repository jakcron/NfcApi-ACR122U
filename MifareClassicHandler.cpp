#include "MifareClassicHandler.h"

MifareClassicHandler::MifareClassicHandler()
{
	card_.EstablishContext(card_.CONTEXT_TYPE_USER);
}

MifareClassicHandler::~MifareClassicHandler()
{
	card_.Disconnect();
}

int MifareClassicHandler::Connect()
{
	std::vector<std::string> reader_list;
	if (card_.GetReaderList(reader_list) != 0)
	{
		return 1;
	}

	for (size_t i = 0; i < reader_list.size(); i++)
	{
		if (reader_list[i].substr(0,10) == "ACS ACR122")
		{
			return card_.Connect(reader_list[i].c_str(), card_.ACCESS_MODE_SHARED);
		}
	}

	return 1;
}

int MifareClassicHandler::Disconnect()
{
	card_.Disconnect();
	return 0;
}

int MifareClassicHandler::GetCardUid(uint32_t& uid)
{
	struct AdpuSend {
		AdpuCmdHeader hdr = { DEFAULT_CLASS, GET_DATA, 0x00, 0x00, sizeof(uint32_t) };
	} cmd;

	struct AdpuRecv {
		uint8_t uid[sizeof(uint32_t)];
	} recv;

	// Transmit
	int ret = 0;
	if ((ret = AdpuTransmit(&cmd, sizeof(cmd), &recv, sizeof(recv))) != SCARD_S_SUCCESS) {
		return ret;
	}
	
	// Process result
	if (nfc_result() == NFC_ERROR_OK) {
		uid = recv.uid[0] << 24 | recv.uid[1] << 16 | recv.uid[2] << 8 | recv.uid[3] << 0;
	}

	return 0;
}

int MifareClassicHandler::LoadKey(const uint8_t key[kKeySize], KeyBankId key_bank)
{
	// Structures of command and response
	struct AdpuCmd {
		AdpuCmdHeader hdr = { DEFAULT_CLASS, LOAD_AUTH_KEYS, 0x00, 0x00, kKeySize };

		uint8_t key[kKeySize];
	} cmd;

	// Set arguments
	cmd.hdr.p2 = key_bank;
	memcpy(cmd.key, key, kKeySize);

	// Transmit
	int ret = 0;
	if ((ret = AdpuTransmit(&cmd, sizeof(cmd), NULL, 0)) != SCARD_S_SUCCESS) {
		return ret;
	}

	return 0;
}

int MifareClassicHandler::AuthenticateWithBlock(uint8_t block, KeyBankId key_bank, SectorKeyType key_type)
{
	// Structures of command and response
	struct AdpuCmd {
		AdpuCmdHeader hdr = { DEFAULT_CLASS, AUTHENTICATE_BLOCK, 0x00, 0x00, 5 };

		uint8_t auth_version = 1;
		uint8_t reserved = 0;
		uint8_t block;
		uint8_t key_type;
		uint8_t key_bank;
	} cmd;

	// Set arguments
	cmd.block = block;
	cmd.key_type = key_type == KEY_A? 0x60 : 0x61;
	cmd.key_bank = key_bank;

	// Transmit
	int ret = 0;
	if ((ret = AdpuTransmit(&cmd, sizeof(cmd), NULL, 0)) != SCARD_S_SUCCESS) {
		return ret;
	}

	return 0;
}

/*
int MifareClassicHandler::AuthenticateWithBlock_Old(uint8_t block, KeyBankId key_bank, SectorKeyType key_type)
{
	// Structures of command and response
	struct AdpuCmd {
		uint8_t adpu_class = DEFAULT_CLASS;
		uint8_t instruction = AUTHENTICATE_BLOCK_OLD;
		uint8_t p1 = 0x00;
		uint8_t block; // p2
		uint8_t key_type; // p3
		uint8_t key_bank; // data in
	} cmd;

	// Set arguments
	cmd.block = block;
	cmd.key_type = key_type == KEY_A ? 0x60 : 0x61;
	cmd.key_bank = key_bank;

	// Transmit
	int ret = 0;
	if ((ret = AdpuTransmit(&cmd, sizeof(cmd), NULL, 0)) != SCARD_S_SUCCESS) {
		return ret;
	}

	return 0;
}
*/

int MifareClassicHandler::ReadBlock(uint8_t block, uint8_t out[kBlockSize])
{
	// Structures of command and response
	struct AdpuCmd {
		AdpuCmdHeader hdr = { DEFAULT_CLASS, READ_BINARY_BLOCK, 0x00, 0x00, kBlockSize };
	} cmd;

	struct AdpuRecv {
		uint8_t block[kBlockSize];
	} recv;

	// Set arguments
	cmd.hdr.p2 = block;

	// Transmit
	int ret = 0;
	if ((ret = AdpuTransmit(&cmd, sizeof(cmd), &recv, sizeof(recv))) != SCARD_S_SUCCESS) {
		return ret;
	}

	// Process result
	if (nfc_result() == NFC_ERROR_OK) {
		memcpy(out, recv.block, kBlockSize);
	}
	
	return 0;
}

int MifareClassicHandler::WriteBlock(uint8_t block, const uint8_t in[kBlockSize])
{
	// Structures of command and response
	struct AdpuCmd {
		AdpuCmdHeader hdr = { DEFAULT_CLASS, UPDATE_BINARY_BLOCK, 0x00, 0x00, kBlockSize };
		uint8_t block[kBlockSize];
	} cmd;

	// Set arguments
	cmd.hdr.p2 = block;
	memcpy(cmd.block, in, kBlockSize);

	// Transmit
	int ret = 0;
	if ((ret = AdpuTransmit(&cmd, sizeof(cmd), NULL, 0)) != SCARD_S_SUCCESS) {
		return ret;
	}

	return 0;
}

int MifareClassicHandler::ReadValue(uint8_t block, uint32_t& value)
{
	// Structures of command and response
	struct AdpuCmd {
		AdpuCmdHeader hdr = { DEFAULT_CLASS, READ_VALUE_BLOCK, 0x00, 0x00, sizeof(uint32_t) };
	} cmd;

	struct AdpuRecv {
		uint8_t value[sizeof(uint32_t)];
	} recv;

	// Set arguments
	cmd.hdr.p2 = block;

	// Transmit
	int ret = 0;
	if ((ret = AdpuTransmit(&cmd, sizeof(cmd), &recv, sizeof(recv))) != SCARD_S_SUCCESS) {
		return ret;
	}

	// Process result
	if (nfc_result() == NFC_ERROR_OK) {
		value = recv.value[0] << 24 | recv.value[1] << 16 | recv.value[2] << 8 | recv.value[3] << 0;
	}

	return 0;
}

int MifareClassicHandler::WriteValue(uint8_t block, uint32_t value)
{
	// Structures of command and response
	struct AdpuCmd {
		AdpuCmdHeader hdr = { DEFAULT_CLASS, UPDATE_VALUE_BLOCK, 0x00, 0x00, 5 };

		uint8_t val_update_type = ValueUpdateType::WRITE_VALUE;
		uint8_t value[4];
	} cmd;

	// Set arguments
	cmd.hdr.p2 = block;
	cmd.value[0] = (value >> 24) & 0xFF;
	cmd.value[1] = (value >> 16) & 0xFF;
	cmd.value[2] = (value >> 8) & 0xFF;
	cmd.value[3] = (value >> 0) & 0xFF;

	// Transmit
	int ret = 0;
	if ((ret = AdpuTransmit(&cmd, sizeof(cmd), NULL, 0)) != SCARD_S_SUCCESS) {
		return ret;
	}

	return 0;
}

int MifareClassicHandler::IncrementValue(uint8_t block)
{
	// Structures of command and response
	struct AdpuCmd {
		AdpuCmdHeader hdr = { DEFAULT_CLASS, UPDATE_VALUE_BLOCK, 0x00, 0x00, 1 };

		uint8_t val_update_type = ValueUpdateType::INCREMENT_VALUE;
	} cmd;

	// Set arguments
	cmd.hdr.p2 = block;

	// Transmit
	int ret = 0;
	if ((ret = AdpuTransmit(&cmd, sizeof(cmd), NULL, 0)) != SCARD_S_SUCCESS) {
		return ret;
	}

	return 0;
}

int MifareClassicHandler::DecrementValue(uint8_t block)
{
	// Structures of command and response
	struct AdpuCmd {
		AdpuCmdHeader hdr = { DEFAULT_CLASS, UPDATE_VALUE_BLOCK, 0x00, 0x00, 1 };

		uint8_t val_update_type = ValueUpdateType::DECREMENT_VALUE;
	} cmd;

	// Set arguments
	cmd.hdr.p2 = block;

	// Transmit
	int ret = 0;
	if ((ret = AdpuTransmit(&cmd, sizeof(cmd), NULL, 0)) != SCARD_S_SUCCESS) {
		return ret;
	}

	return 0;
}

int MifareClassicHandler::CopyValue(uint8_t src_block, uint8_t dst_block)
{
	// Structures of command and response
	struct AdpuCmd {
		AdpuCmdHeader hdr = { DEFAULT_CLASS, UPDATE_VALUE_BLOCK, 0x00, 0x00, 2 };

		uint8_t val_update_type = ValueUpdateType::COPY_VALUE;
		uint8_t dst_block;
	} cmd;

	// Set arguments
	cmd.hdr.p2 = src_block;
	cmd.dst_block = dst_block;

	// Transmit
	int ret = 0;
	if ((ret = AdpuTransmit(&cmd, sizeof(cmd), NULL, 0)) != SCARD_S_SUCCESS) {
		return ret;
	}

	return 0;
}

int MifareClassicHandler::AdpuTransmit(void* in, uint32_t in_size, void* out, uint32_t out_size)
{
	DWORD send_len, recv_len;
	int ret = 0;

	// check size of in/out
	if (in_size > kTxBuffLen || out_size > (kTxBuffLen - kNfcResultSize)) {
		return 1;
	}

	
	// Prepare Transmit
	ClearTxBuff();
	memcpy(send_, in, in_size);
	send_len = in_size;
	recv_len = out_size + kNfcResultSize;

	// transmit
	if ((ret = card_.Transmit(send_, send_len, recv_, recv_len)) != SCARD_S_SUCCESS) {
		return ret;
	}

	// save nfc result
	SaveResult(recv_len);

	// the response was expected size, copy to out
	if (recv_len == (out_size + kNfcResultSize) && out != NULL) {
		memcpy(out, recv_, out_size);
	}

	return 0;
}

void MifareClassicHandler::SaveResult(uint32_t recv_len)
{
	nfc_result_ = (recv_[recv_len - kNfcResultSize] << 8) | (recv_[recv_len - kNfcResultSize + 1]);
}

void MifareClassicHandler::ClearTxBuff(void)
{
	memset(send_, 0, kTxBuffLen);
	memset(recv_, 0, kTxBuffLen);
}
