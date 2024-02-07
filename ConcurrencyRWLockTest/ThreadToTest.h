#pragma once

namespace TestUtil
{
	struct ThreadToTest //TODO extend to class
	{
	public:
		std::promise<void> threadPromise;
		std::future<void> threadFuture;
		
	};
}