#pragma once
#include <cstdint>

namespace nfc
{
namespace mfc
{
	static const size_t kKeySize = 6;
	static const size_t kBlockSize = 0x10;

	struct sBlock0 {

		uint8_t uid[4];     // unique to each SR
		uint8_t bcc;        // bcc should == uid[0] ^ uid[1] ^ uid[2] ^ uid[3]
		uint8_t sak;       // 88
		uint8_t atqa[2];    // 04 00
		uint8_t manufacturer_data[8];    // same for cards in same batch, likely batch manufacture timestamp
	};

	struct sSectorTrailer
	{
		uint8_t key_a[kKeySize];
		uint8_t permissions[4];
		uint8_t key_b[kKeySize];
	};

	struct sSector64Byte
	{
		uint8_t data[kBlockSize*3];
		sSectorTrailer trailer;
	};

	struct sSector256Byte
	{
		uint8_t data[kBlockSize*0xf];
		sSectorTrailer trailer;
	};

	struct sCard1K
	{
		sSector64Byte sector[0x10];
	};

	struct sCard2K
	{
		sSector64Byte sector[0x20];
	};

	struct sCard4K
	{
		sSector64Byte sector[0x20];
		sSector256Byte extended_sector[8];
	};

	inline uint32_t sector_size(uint8_t sector) { return (sector < 0x20) ? sizeof(sSector64Byte) : sizeof(sSector256Byte); }
	inline uint8_t sector_to_block(uint8_t sector) { return sector_size(sector) == 0x40 ? (sector * 4) : (0x20 * 4 + (sector - 0x20) * 16); }
	inline uint8_t block_to_sector(uint8_t block) { return  (block < 0x80) ? (block / 0x4) : (0x20 + ((block - 0x80) / 0x10)); }
}
}