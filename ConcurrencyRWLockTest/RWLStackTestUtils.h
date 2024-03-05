#pragma once
#include "SeparatedThreadCallbackExecutor.h"

template<typename CallbackFunction, typename CallbackFunctionResultType>
class TestThreadsManager
{
public:
	TestThreadsManager();
	void AddThreadExecutor(std::unique_ptr<SeparatedThreadCallbackExecutor<CallbackFunction, CallbackFunctionResultType>> executor);
	void WaitThreadFinished();
	std::shared_future<void> GetMainThreadReadyFuture() const;
	int32_t GetWhaitForThreadsReadyExceptionsCount() const;
	int32_t GetThreadsProcessedExceptionsCount() const;

private:
	std::promise<void> mainThreadReadyPromise;
	std::shared_future<void> mainThreadReadyFeature;
	std::list<std::unique_ptr<SeparatedThreadCallbackExecutor<CallbackFunction, CallbackFunctionResultType>>> threadsExecutors;
	int32_t whaitForThreadsReadyExceptionsCount;
	int32_t threadsProcessedExceptionsCount;
};

template<typename CallbackFunction, typename CallbackFunctionResultType>
TestThreadsManager<CallbackFunction, CallbackFunctionResultType>::TestThreadsManager()
	: mainThreadReadyPromise(),
	mainThreadReadyFeature(mainThreadReadyPromise.get_future()),
	threadsExecutors(),
	whaitForThreadsReadyExceptionsCount(0),
	threadsProcessedExceptionsCount(0)
{
}

template<typename CallbackFunction, typename CallbackFunctionResultType>
void TestThreadsManager<CallbackFunction, CallbackFunctionResultType>::AddThreadExecutor(
	std::unique_ptr<SeparatedThreadCallbackExecutor<CallbackFunction, CallbackFunctionResultType>> executor
)
{
	threadsExecutors.push_back(std::move(executor));
}

template<typename CallbackFunction, typename CallbackFunctionResultType>
void TestThreadsManager<CallbackFunction, CallbackFunctionResultType>::WaitThreadFinished()
{
	for (auto& featureReady : threadsExecutors)
	{
		if (!featureReady->WaitForThreadReady())
		{
			++whaitForThreadsReadyExceptionsCount;
		}
	}
	mainThreadReadyPromise.set_value();
	for (auto& featureReady : threadsExecutors)
	{
		if (!featureReady->WaitForThreadFinished())
		{
			++threadsProcessedExceptionsCount;
		}
	}
}

template<typename CallbackFunction, typename CallbackFunctionResultType>
std::shared_future<void> TestThreadsManager<CallbackFunction, CallbackFunctionResultType>::GetMainThreadReadyFuture() const
{
	return mainThreadReadyFeature;
}

template<typename CallbackFunction, typename CallbackFunctionResultType>
int32_t TestThreadsManager<CallbackFunction, CallbackFunctionResultType>::GetWhaitForThreadsReadyExceptionsCount() const
{
	return whaitForThreadsReadyExceptionsCount;
}

template<typename CallbackFunction, typename CallbackFunctionResultType>
int32_t TestThreadsManager<CallbackFunction, CallbackFunctionResultType>::GetThreadsProcessedExceptionsCount() const
{
	return threadsProcessedExceptionsCount;
}


//TODO move
class ThreadIndexParamCheckCallback
{
public:
	void operator()()
	{
		for (int numbersGenerated = 0; numbersGenerated < numberOfGeneratedNumbers; ++numbersGenerated)
		{
			container.Push(threadIndex);
		}
	}

	ThreadIndexParamCheckCallback(const int32_t threadIndex, const int32_t numberOfGeneratedNumbers, ThreadSafeStructs::RWLockStack<int>& container)
		: threadIndex(threadIndex),
		numberOfGeneratedNumbers(numberOfGeneratedNumbers),
		container(container)
	{
	}
private:
	const int32_t threadIndex;
	ThreadSafeStructs::RWLockStack<int>& container;
	const int32_t numberOfGeneratedNumbers;
};