#include <nfc/Exception.h>

nfc::Exception::Exception()
{
}

nfc::Exception::Exception(const std::string & what)
{
	what_ = what;
}


nfc::Exception::~Exception()
{
}

const char * nfc::Exception::what() const noexcept
{
	return what_.c_str();
}
