#pragma once
#include <string>
#include <vector>
#include <cstdint>

#include "WINSCARD.H"

class ACR122UHandler
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

	ACR122UHandler();
	~ACR122UHandler();

	int Connect(const std::string& reader_name, uint32_t share_mode);
	int Disconnect();
	int EstablishContext(uint32_t scope);
	int GetReaderList(std::vector<std::string>& list);


	int Transmit(const uint8_t* send, DWORD send_len, uint8_t* recv, DWORD& _recv_len);

private:
	static const uint32_t kCardPreferedProtocol = SCARD_PROTOCOL_Tx;

	DWORD active_protocol_;
	SCARDCONTEXT card_context_;
	SCARDHANDLE card_handle_;
};