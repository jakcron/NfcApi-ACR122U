#pragma once
#ifdef _MSC_VER
#include <string>
#include <vector>
#include <cstdint>
#include "WINSCARD.H"

namespace nfc
{
	namespace drv
	{
		namespace win32
		{
			class ACR122U
			{
			public:
				enum ContextType
				{
					CONTEXT_TYPE_USER = SCARD_SCOPE_USER,
					CONTEXT_TYPE_TERMINAL = SCARD_SCOPE_TERMINAL,
					CONTEXT_TYPE_SYSTEM = SCARD_SCOPE_SYSTEM,
				};

				enum AccessMode
				{
					ACCESS_MODE_EXCLUSIVE = SCARD_SHARE_EXCLUSIVE,
					ACCESS_MODE_SHARED = SCARD_SHARE_SHARED,
					ACCESS_MODE_DIRECT = SCARD_SHARE_DIRECT
				};

				ACR122U();
				~ACR122U();

				void Connect(const std::string& reader_name, uint32_t share_mode);
				void Disconnect();
				void EstablishContext(uint32_t scope);
				void GetReaderList(std::vector<std::string>& list);
				void Transmit(uint8_t instruction, uint8_t p1, uint8_t p2, uint8_t d_len, const uint8_t* payload, size_t payload_len, uint8_t* response, size_t response_len);
				uint16_t GetNfcResult() const;

				void SetAdpuDebug(bool enable_debug);

				// Configuration
				/*
				void SetLedBuzzerParam(uint8_t red_led, uint8_t green_led, uint8_t t1_blink_freq, uint8_t t2_blink_freq, uint8_t num_rep, uint8_t buzzer, uint8_t& post_led_state);
				void GetFirmwareVersion(char* out);
				void GetPiccOperatingParameters(uint8_t& param);
				void SetPiccOperatingParameters(uint8_t param);
				void SetCardTimeout(uint8_t timeout);
				void SetBuzzerOnCardDetection(bool enable);
				*/

			private:
				static const uint32_t kCardPreferedProtocol = SCARD_PROTOCOL_Tx;
				static const size_t kNfcResultSize = 2;
				static const size_t kTxBuffLen = 0x1000;
				static const uint8_t kDefaultCmdClass = 0xff;

				struct sAdpuCmd
				{
					uint8_t class_type;
					uint8_t instruction;
					uint8_t p1;
					uint8_t p2;
					uint8_t d_len; // size of data sent, or size of data intended to be read from device
				};

				DWORD active_protocol_;
				SCARDCONTEXT card_context_;
				SCARDHANDLE card_handle_;
				bool debug_output_;

				uint8_t send_[kTxBuffLen];
				uint8_t recv_[kTxBuffLen];

				uint16_t nfc_result_;

				void RawTransmit(const uint8_t* send, uint32_t send_len, uint8_t* recv, uint32_t& _recv_len);
			};
		}
	}
}

#endif