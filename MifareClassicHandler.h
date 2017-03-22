#pragma once
#include "ACR122UHandler.h"

class MifareClassicHandler
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

	static const int kKeySize = 6;
	static const int kBlockSize = 0x10;
	static const int kNfcResultSize = 2;

	MifareClassicHandler();
	~MifareClassicHandler();

	int Connect();
	int Disconnect();

	int GetCardUid(uint32_t& uid);

	int LoadKey(const uint8_t key[kKeySize], KeyBankId key_bank);
	int AuthenticateWithBlock(uint8_t block, KeyBankId key_bank, SectorKeyType key_type);
	//int AuthenticateWithBlock_Old(uint8_t block, KeyBankId key_bank, SectorKeyType key_type);

	int ReadBlock(uint8_t block, uint8_t out[kBlockSize]);
	int WriteBlock(uint8_t block, const uint8_t in[kBlockSize]);
	
	int ReadValue(uint8_t block, uint32_t& value);
	int WriteValue(uint8_t block, uint32_t value);
	int IncrementValue(uint8_t block);
	int DecrementValue(uint8_t block);
	int CopyValue(uint8_t src_block, uint8_t dst_block);

	inline uint16_t nfc_result(void) const { return nfc_result_; }
	inline uint32_t sector_size(uint8_t sector) const { return (sector < 0x20) ? 0x40 : 0x100; }
	inline uint8_t sector_to_block(uint8_t sector) const { return sector_size(sector) == 0x40 ? (sector * 4) : (0x20 * 4 + (sector - 0x20) * 16); }
	inline uint8_t block_to_sector(uint8_t block) const { return  (block < 0x80) ? (block / 0x4) : (0x20 + ((block - 0x80) / 0x10)); }

private:
	static const int kTxBuffLen = 0x1000;
	
	enum AdpuClassType
	{
		DEFAULT_CLASS = 0xFF
	};

	enum AdpuInstructionId
	{
		LOAD_AUTH_KEYS = 0x82,
		AUTHENTICATE_BLOCK = 0x86,
		//AUTHENTICATE_BLOCK_OLD = 0x88,
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

	struct AdpuCmdHeader
	{
		uint8_t class_type;
		uint8_t instruction;
		uint8_t p1;
		uint8_t p2;
		uint8_t d_len; // size of data sent, or size of data intended to be read from device
	};

	ACR122UHandler card_;
	uint8_t send_[kTxBuffLen];
	uint8_t recv_[kTxBuffLen];

	int adpu_result_;
	uint16_t nfc_result_;

	int AdpuTransmit(void* in, uint32_t in_size, void* out, uint32_t out_size);
	void SaveResult(uint32_t recv_len);
	void ClearTxBuff(void);
};
