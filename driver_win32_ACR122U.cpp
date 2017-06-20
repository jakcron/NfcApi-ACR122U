#ifdef _MSC_VER

#include <nfc/drv/win32/ACR122U.h>
#include <nfc/Exception.h>

nfc::drv::win32::ACR122U::ACR122U()
{
	SetAdpuDebug(false);
}

nfc::drv::win32::ACR122U::~ACR122U()
{
}

void nfc::drv::win32::ACR122U::Connect(const std::string & reader_name, uint32_t share_mode)
{
	if (SCardConnectA(card_context_, reader_name.c_str(), share_mode, kCardPreferedProtocol, &card_handle_, &active_protocol_) != SCARD_S_SUCCESS)
	{
		throw nfc::Exception("Failed to connect to card");
	}
}

void nfc::drv::win32::ACR122U::Disconnect()
{
	if (SCardDisconnect(card_handle_, SCARD_UNPOWER_CARD) != SCARD_S_SUCCESS)
	{
		throw nfc::Exception("Failed to disconnect card (power down)");
	}

	if (SCardReleaseContext(card_context_) != SCARD_S_SUCCESS)
	{
		throw nfc::Exception("Failed to release WINCARD API context");
	}
}

void nfc::drv::win32::ACR122U::EstablishContext(uint32_t scope)
{
	if (SCardEstablishContext(scope, NULL, NULL, &card_context_) != SCARD_S_SUCCESS)
	{
		throw nfc::Exception("EstablishContect() failed");
	}
}

void nfc::drv::win32::ACR122U::GetReaderList(std::vector<std::string>& list)
{
	DWORD size = 256;
	char tmp[256];


	if (SCardListReadersA(card_context_, NULL, tmp, &size) != SCARD_S_SUCCESS)
	{
		throw nfc::Exception("Failed to retrieve list of card readers");
	}

	// separate the reader names, and add to list of strings
	char *p = tmp;
	while (p[0])
	{
		list.push_back(p);
		p = &p[strlen(p) + 1];
	}
}

void nfc::drv::win32::ACR122U::Transmit(uint8_t instruction, uint8_t p1, uint8_t p2, uint8_t d_len, const uint8_t * payload, size_t payload_len, uint8_t * response, size_t response_len)
{
	// Prepare command
	sAdpuCmd cmd;
	cmd.class_type = kDefaultCmdClass;
	cmd.instruction = instruction;
	cmd.p1 = p1;
	cmd.p2 = p2;
	cmd.d_len = d_len;


	// Calculate send/recv len 
	uint32_t send_len = sizeof(sAdpuCmd) + ((payload != nullptr) ? payload_len : 0);
	uint32_t recv_len = ((response != nullptr) ? response_len : 0) + kNfcResultSize;

	// Check size of in/out
	if (send_len > kTxBuffLen || recv_len > (kTxBuffLen - kNfcResultSize))
	{
		throw nfc::Exception("Too large Adpu In/Out sizes");
	}

	// Prepare Transmit
	memset(send_, 0, kTxBuffLen);
	memset(recv_, 0, kTxBuffLen);
	memcpy(send_, &cmd, sizeof(sAdpuCmd));
	if (payload != nullptr)
	{
		memcpy(send_ + sizeof(sAdpuCmd), payload, payload_len);
	}

	if (debug_output_)
	{
		printf("[ADPU SEND:");
		for (size_t i = 0; i < send_len; i++)
		{
			printf(" %02X", send_[i]);
		}
		printf("]\n");
	}

	// Transmit
	RawTransmit(send_, send_len, recv_, recv_len);

	if (debug_output_)
	{
		printf("[ADPU RECV:");
		for (size_t i = 0; i < recv_len; i++)
		{
			printf(" %02X", recv_[i]);
		}
		printf("]\n");
	}
	

	// save nfc result
	nfc_result_ = (recv_[recv_len - kNfcResultSize] << 8) | (recv_[recv_len - kNfcResultSize + 1]);

	// if recv data has expected size
	if (recv_len == (response_len + kNfcResultSize))
	{
		// is response isn't nullptr, copy to it
		if (response != nullptr)
		{
			memcpy(response, recv_, response_len);
		}
	}
	else if (recv_len == kNfcResultSize) // else if only the error was returned
	{

	}
	else // the response did not have the expected size
	{
		printf("recv_len = %d (expected %d + 2)\n", recv_len, response_len);
		throw nfc::Exception("Adpu response has unexpected size");
	}
}

uint16_t nfc::drv::win32::ACR122U::GetNfcResult() const
{
	return nfc_result_;
}

void nfc::drv::win32::ACR122U::SetAdpuDebug(bool enable_debug)
{
	debug_output_ = enable_debug;
}

void nfc::drv::win32::ACR122U::RawTransmit(const uint8_t* send, uint32_t send_len, uint8_t* recv, uint32_t& recv_len)
{
	DWORD len = recv_len;
	if (SCardTransmit(card_handle_, NULL, send, send_len, NULL, recv, &len) != SCARD_S_SUCCESS)
	{
		throw nfc::Exception("Card transmission failed");
	}
	recv_len = len;
}

/*
void nfc::drv::win32::ACR122U::SetLedBuzzerParam(uint8_t red_led, uint8_t green_led, uint8_t t1_blink_freq, uint8_t t2_blink_freq, uint8_t num_rep, uint8_t buzzer, uint8_t& post_led_state)
{
	enum LedStateInternal
	{
		RED_FINAL_ON = 1 << 0,
		GREEN_FINAL_ON = 1 << 1,
		RED_UPDATE = 1 << 2,
		GREEN_UPDATE = 1 << 3,
		RED_INITIAL_BLINK_ON = 1 << 4,
		GREEN_INITIAL_BLINK_ON = 1 << 5,
		RED_BLINK = 1 << 6,
		GREEN_BLINK = 1 << 7,
	};

	struct AdpuPayload {
		uint8_t t1_blink_freq;
		uint8_t t2_blink_freq;
		uint8_t num_repetition;
		uint8_t buzzer_state;
	} payload;

	uint8_t led_state;
	if (red_led & UPDATE)
	{
		led_state |= RED_UPDATE;
		led_state |= (red_led & FINAL_ON) ? RED_FINAL_ON : 0;
		led_state |= (red_led & INITIAL_BLINK_ON) ? RED_INITIAL_BLINK_ON : 0;
		led_state |= (red_led & BLINK_ON) ? RED_BLINK : 0;
	}
	if (green_led & UPDATE)
	{
		led_state |= GREEN_UPDATE;
		led_state |= (green_led & FINAL_ON) ? GREEN_FINAL_ON : 0;
		led_state |= (green_led & INITIAL_BLINK_ON) ? GREEN_INITIAL_BLINK_ON : 0;
		led_state |= (green_led & BLINK_ON) ? GREEN_BLINK : 0;
	}

	payload.t1_blink_freq = t1_blink_freq;
	payload.t2_blink_freq = t2_blink_freq;
	payload.num_repetition = num_rep;
	payload.buzzer_state = buzzer & 3;

	Transmit(0x00, 0x40, led_state, sizeof(AdpuPayload), (const uint8_t*)payload, sizeof(AdpuPayload), nullptr, 0);

	if ((GetNfcResult() >> 8) == 0x90)
	{
		post_led_state = GetNfcResult() & 3;
	}
	else
	{
		throw SmartCardException("Failed to set LED/Buzzer State");
	}
}

void nfc::drv::win32::ACR122U::GetFirmwareVersion(char * out)
{
	struct AdpuSend {
		AdpuCmdHeader hdr = { 0xFF, 0x00, 0x48, 0x00, 0x00 };
	} send;

	struct AdpuRecv {
		char firmare_version[10];
		uint8_t res_code[2];
		uint16_t result() { return (uint16_t)res_code[0] << 8 | (uint16_t)res_code[1]; }
	} recv;


	DWORD recv_size = sizeof(AdpuRecv);
	if (Transmit((const uint8_t*)&send, sizeof(AdpuSend), (uint8_t*)&recv, recv_size) != 0)
	{
		printf("fail!\n");
		return;
	}

	if (recv.result() == 0x9000)
	{
		memcpy(out, recv.firmare_version, 10);
	}
	else
	{
		out[0] = '\0';
	}
}

void nfc::drv::win32::ACR122U::GetPiccOperatingParameters(uint8_t & param)
{
	struct AdpuSend {
		AdpuCmdHeader hdr = { 0xFF, 0x00, 0x50, 0x00, 0x00 };
	} send;

	struct AdpuRecv {
		uint8_t operating_parameter;
		uint8_t res_code[2];
		uint16_t result() { return (uint16_t)res_code[0] << 8 | (uint16_t)res_code[1]; }
	} recv;

	DWORD recv_size = sizeof(AdpuRecv);
	if (Transmit((const uint8_t*)&send, sizeof(AdpuSend), (uint8_t*)&recv, recv_size) != 0)
	{
		return;
	}

	if (recv.result() == 0x9000)
	{
		param = recv.operating_parameter;
	}
}

void nfc::drv::win32::ACR122U::SetPiccOperatingParameters(uint8_t param)
{
	struct AdpuSend {
		AdpuCmdHeader hdr = { 0xFF, 0x00, 0x51, 0x00, 0x00 };
	} send;

	struct AdpuRecv {
		uint8_t operating_parameter;
		uint8_t res_code[2];
		uint16_t result() { return (uint16_t)res_code[0] << 8 | (uint16_t)res_code[1]; }
	} recv;

	send.hdr.p2 = param;

	DWORD recv_size = sizeof(AdpuRecv);
	if (Transmit((const uint8_t*)&send, sizeof(AdpuSend), (uint8_t*)&recv, recv_size) != 0)
	{
		return;
	}
}

void nfc::drv::win32::ACR122U::SetCardTimeout(uint8_t timeout)
{
	struct AdpuSend {
		AdpuCmdHeader hdr = { 0xFF, 0x00, 0x41, 0x00, 0x00 };
	} send;

	struct AdpuRecv {
		uint8_t res_code[2];
		uint16_t result() { return (uint16_t)res_code[0] << 8 | (uint16_t)res_code[1]; }
	} recv;

	send.hdr.p2 = timeout;

	DWORD recv_size = sizeof(AdpuRecv);
	if (Transmit((const uint8_t*)&send, sizeof(AdpuSend), (uint8_t*)&recv, recv_size) != 0)
	{
		return;
	}
}

void nfc::drv::win32::ACR122U::SetBuzzerOnCardDetection(bool enable)
{
	Transmit(0x00, 0x50, enable ? 0xff : 0x00, 0x00, nullptr, 0, nullptr, 0);

	if (GetNfcResult() != 0x9000)
	{
		throw SmartCardException("Failed card detection buzzer configuration");
	}
}
*/

#endif