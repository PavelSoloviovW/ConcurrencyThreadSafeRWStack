#pragma once

namespace ThreadSafeStructs
{
	class ThreadSafetyException : public std::runtime_error
	{
	public:
		ThreadSafetyException(const std::string& msg)
			: runtime_error(msg)
		{}

		ThreadSafetyException(const char* msg)
			: runtime_error(msg)
		{}
	};
}