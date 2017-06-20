#pragma once
#include <cstdint>

namespace nfc
{
namespace mfc
{
	// key size
	static const size_t kKeySize = 6;
	static const size_t kBlockSize = 0x10;
	static const size_t kSector64ByteNum = 0x20;
	static const size_t kSector256ByteNum = 0x8;


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
		uint8_t data[kBlockSize*15];
		sSectorTrailer trailer;
	};

	struct sCard1K
	{
		sSector64Byte sector[0x10];
	};

	struct sCard2K
	{
		sSector64Byte sector[kSector64ByteNum];
	};

	struct sCard4K
	{
		sSector64Byte sector[kSector64ByteNum];
		sSector256Byte extended_sector[kSector256ByteNum];
	};

	static const size_t kSector64ByteBlockNum = sizeof(sSector64Byte) / kBlockSize;
	static const size_t kSector256ByteBlockNum = sizeof(sSector256Byte) / kBlockSize;

	inline uint32_t sector_size(uint8_t sector) { return (sector < kSector64ByteNum) ? sizeof(sSector64Byte) : sizeof(sSector256Byte); }
	inline uint8_t sector_to_block(uint8_t sector) { return sector_size(sector) == sizeof(sSector64Byte) ? (sector * kSector64ByteBlockNum) : (kSector64ByteNum * kSector64ByteBlockNum + (sector - kSector64ByteNum) * kSector256ByteBlockNum); }
	inline uint8_t block_to_sector(uint8_t block) { return  (block < (kSector64ByteNum * kSector64ByteBlockNum)) ? (block / kSector64ByteBlockNum) : (kSector64ByteNum + ((block - (kSector64ByteNum * kSector64ByteBlockNum)) / kSector256ByteBlockNum)); }
}
}