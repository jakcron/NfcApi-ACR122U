#include "ACR122UHandler.h"

ACR122UHandler::ACR122UHandler()
{
}

ACR122UHandler::~ACR122UHandler()
{
}

int ACR122UHandler::Connect(const std::string & reader_name, uint32_t share_mode)
{
	return SCardConnectA(card_context_, reader_name.c_str(), share_mode, kCardPreferedProtocol, &card_handle_, &active_protocol_);
}

int ACR122UHandler::Disconnect()
{
	int ret_code;

	if ((ret_code = SCardDisconnect(card_handle_, SCARD_UNPOWER_CARD)) != SCARD_S_SUCCESS)
	{
		return ret_code;
	}

	if ((ret_code = SCardReleaseContext(card_context_)) != SCARD_S_SUCCESS)
	{
		return ret_code;
	}

	return 0;
}

int ACR122UHandler::EstablishContext(uint32_t scope)
{
	return SCardEstablishContext(scope, NULL, NULL, &card_context_);
}

int ACR122UHandler::GetReaderList(std::vector<std::string>& list)
{
	DWORD size = 256;
	int ret_code;
	char tmp[256];


	if ((ret_code = SCardListReadersA(card_context_, NULL, tmp, &size)) != 0)
	{
		return ret_code;
	}

	// separate the reader names, and add to list of strings
	char *p = tmp;
	while (p[0])
	{
		list.push_back(p);
		p = &p[strlen(p) + 1];
	}

	return 0;
}

int ACR122UHandler::Transmit(const uint8_t* send, DWORD send_len, uint8_t* recv, DWORD& recv_len)
{
	return SCardTransmit(card_handle_, NULL, send, send_len, NULL, recv, &recv_len);
}
