#pragma once

template<typename CallbackFunction, typename CallbackFunctionResultType>
class SeparatedThreadCallbackExecutor
{
private:
	std::promise<void> threadReadyToActionFlag;
	std::future<void> threadActionFinished;
	const CallbackFunction testingActionCallback;
	std::shared_future<void> startProcessFlag;

public:
	SeparatedThreadCallbackExecutor(const CallbackFunction& testingActionCallback, std::shared_future<void> startProcessFlag);
	SeparatedThreadCallbackExecutor(const SeparatedThreadCallbackExecutor&& executor);
	SeparatedThreadCallbackExecutor(const SeparatedThreadCallbackExecutor& executor) = delete;

	bool WaitForThreadReady();
	bool WaitForThreadFinished();
	std::future<CallbackFunctionResultType> GetThreadResult();
	
	SeparatedThreadCallbackExecutor operator=(const SeparatedThreadCallbackExecutor&& executor);
	SeparatedThreadCallbackExecutor operator=(const SeparatedThreadCallbackExecutor& executor) = delete;
};

template<typename CallbackFunction, typename CallbackFunctionResultType>
SeparatedThreadCallbackExecutor<CallbackFunction, CallbackFunctionResultType>::SeparatedThreadCallbackExecutor(
	const CallbackFunction& testingActionCallback,
	std::shared_future<void> startProcessFlag
)
	:testingActionCallback(testingActionCallback),
	startProcessFlag(startProcessFlag),
	threadReadyToActionFlag(),
	threadActionFinished()
{
	auto initAndProcessFunction = [this]()
		{
			this->threadReadyToActionFlag.set_value();
			this->startProcessFlag.wait();
			this->testingActionCallback();
		};
	threadActionFinished = std::async(std::launch::async, initAndProcessFunction);
}

template<typename CallbackFunction, typename CallbackFunctionResultType>
bool SeparatedThreadCallbackExecutor<CallbackFunction, CallbackFunctionResultType>::WaitForThreadReady()
{
	try
	{
		this->threadReadyToActionFlag.get_future().get();
	}
	catch (const std::exception&)
	{
		return false;
	}
	return true;
}

template<typename CallbackFunction, typename CallbackFunctionResultType>
bool SeparatedThreadCallbackExecutor<CallbackFunction, CallbackFunctionResultType>::WaitForThreadFinished()
{
	try
	{
		this->threadActionFinished.get();
	}
	catch (const std::exception&)
	{
		return false;
	}
	return true;
}

template<typename CallbackFunction, typename CallbackFunctionResultType>
std::future<CallbackFunctionResultType> SeparatedThreadCallbackExecutor<CallbackFunction, CallbackFunctionResultType>::GetThreadResult()
{
	WaitForThreadFinished();
	return std::move(threadActionFinished);
}

template<typename CallbackFunction, typename CallbackFunctionResultType>
SeparatedThreadCallbackExecutor<CallbackFunction, CallbackFunctionResultType>
	SeparatedThreadCallbackExecutor<CallbackFunction, CallbackFunctionResultType>::operator=
	(
		const SeparatedThreadCallbackExecutor&& executor
	)
{
	startProcessFlag = std::move(executor->startProcessFlag);
	testingActionCallback = std::move(executor->testingActionCallback);
	threadActionFinished = std::move(executor->threadActionFinished);
	threadReadyToActionFlag = std::move(executor->threadReadyToActionFlag);
}

template<typename CallbackFunction, typename CallbackFunctionResultType>
SeparatedThreadCallbackExecutor<CallbackFunction, CallbackFunctionResultType>::SeparatedThreadCallbackExecutor(const SeparatedThreadCallbackExecutor&& executor)
	:testingActionCallback(std::move(executor.testingActionCallback)),
	startProcessFlag(std::move(executor.startProcessFlag)),
	threadActionFinished(std::move(executor.threadActionFinished)),
	threadReadyToActionFlag(std::move(executor.threadReadyToActionFlag))	//WTF
{
}
