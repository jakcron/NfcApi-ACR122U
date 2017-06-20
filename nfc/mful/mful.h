#pragma once
#include <cstdint>

namespace nfc
{
	namespace mful
	{
		static const uint8_t kCascadeTag = 0x88;
		static const size_t kPageSize = 4;
		static const uint8_t kUserPageNum = 12;

		struct sCard {
			struct sSerial {
				uint8_t serial_0[3];
				uint8_t bcc_0;
				uint8_t serial_1[4];
				uint8_t bcc_1;
			} serial;
			uint8_t internal_byte;
			uint8_t lock_bytes[2]; // [0] bit3-7 > lock page 3-7; bit0 bl OTP, bit1 BL 4-9, bit2 BL 10-15, but [1] bit0-7 > lock page 8-15 
			uint8_t otp[kPageSize]; // page 3
			uint8_t user_data[kUserPageNum][kPageSize]; // page 4-15
		};

		static const size_t kCardSize = sizeof(sCard);
	}
}