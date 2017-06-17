#pragma once
#include <exception>
#include <string>

namespace nfc
{
	class Exception :
		public std::exception
	{
	public:
		Exception();
		Exception(const std::string& what);
		~Exception();

		const char* what() const noexcept;
	private:
		std::string what_;
	};
}



