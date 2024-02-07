#include "stdfx.h"
#include "RWLockStack.h"
#include "ThreadSafeException.h"
#include "ThreadToTest.h"

namespace
{
	const std::pair<int32_t, int32_t> MIN_MAX_RANDOM_VALUES(1, 100);

	int32_t GetRandomNumber(const std::pair<int32_t, int32_t> minMax)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distribution(minMax.first, minMax.second);

		return distribution(gen);
	}

	std::stack<int> GetRandomStack(const int32_t size, const std::pair<int32_t, int32_t> minMax)
	{
		std::stack<int> generatingStack;
		for (int numbersGenerated = 0; numbersGenerated < size; ++numbersGenerated)
		{
			generatingStack.push(GetRandomNumber(minMax));
		}
		return generatingStack;
	}

	void AddValueToContainerWithCopy(
		const int32_t value,
		ThreadSafeStructs::RWLockStack<int>& outThreadSafeContainer,
		std::stack<int>& outOrignContainer
	)
	{
		outOrignContainer.push(value);
		outThreadSafeContainer.Push(value);
	}

	void AddValueToContainerWithMove(
		int32_t value,
		ThreadSafeStructs::RWLockStack<int>& outThreadSafeContainer,
		std::stack<int>& outOrignContainer
	)
	{
		outOrignContainer.push(value);
		outThreadSafeContainer.Push(std::move(value));
	}

	void FillThreadSafeAndOriginStackWithRandomNumbers(
		const int32_t numberOfGeneratedNumbers,
		const std::pair<int32_t, int32_t> minMax,
		const bool isNeedInCopy,
		ThreadSafeStructs::RWLockStack<int>& outThreadSafeContainer, 
		std::stack<int>& outOrignContainer
	)
	{
		for (int numbersGenerated = 0; numbersGenerated < numberOfGeneratedNumbers; ++numbersGenerated)
		{
			auto randomNumber = GetRandomNumber(minMax);
			if (isNeedInCopy)
			{
				AddValueToContainerWithCopy(randomNumber, outThreadSafeContainer, outOrignContainer);
			}
			else
			{
				AddValueToContainerWithMove(randomNumber, outThreadSafeContainer, outOrignContainer);
			}
		}
	}

	std::vector<int> ConvertStackToVector(std::stack<int>& stack)
	{
		std::vector<int> buffer;

		while (!stack.empty())
		{
			buffer.push_back(stack.top());
			stack.pop();
		}
		return buffer;
	}

	std::stack<int> PushBackStack(const std::stack<int>& first, const std::stack<int>& second)
	{
		auto outStack = first;
		auto bufferSecondPart = ConvertStackToVector(second);

		for (auto pointer = bufferSecondPart.rbegin(); pointer != bufferSecondPart.rend(); ++pointer)
		{
			outStack.push(*pointer);
		}
		return outStack;
	}
}

TEST(RWLockStack, CreateContainer_Empty)
{
	ThreadSafeStructs::RWLockStack<int> container;

	EXPECT_TRUE(container.Empty());
}

TEST(RWLockStack, CreateContainer_FromStack)
{
	auto stackOrign = GetRandomStack(10, MIN_MAX_RANDOM_VALUES);

	ThreadSafeStructs::RWLockStack<int> container(stackOrign);

	ASSERT_EQ(stackOrign.size(), container.Size());

	while (true)
	{
		if (stackOrign.empty())
		{
			break;
		}
		ASSERT_EQ(stackOrign.top(), container.TryPop());
		stackOrign.pop();
	}
}

TEST(RWLockStack, CreateContainer_FromStackMoved)
{
	auto stackOrign = GetRandomStack(10, MIN_MAX_RANDOM_VALUES);
	auto stackCopy = stackOrign;

	ThreadSafeStructs::RWLockStack<int> container(std::move(stackCopy));

	ASSERT_EQ(stackOrign.size(), container.Size());

	while (true)
	{
		if (stackOrign.empty())
		{
			break;
		}
		ASSERT_EQ(stackOrign.top(), container.TryPop());
		stackOrign.pop();
	}
}

TEST(RWLockStack, SizeOfContainer)
{
	auto stackOrign = GetRandomStack(10, MIN_MAX_RANDOM_VALUES);
	ThreadSafeStructs::RWLockStack<int> container(stackOrign);
	EXPECT_EQ(10, container.Size());
}

TEST(RWLockStack, EmptyOfContainer)
{
	auto stackOrign = GetRandomStack(0, MIN_MAX_RANDOM_VALUES);
	ThreadSafeStructs::RWLockStack<int> container(stackOrign);
	EXPECT_TRUE(container.Empty());
}

TEST(RWLockStack, CopyContainerContainer_OneThread)
{
	ThreadSafeStructs::RWLockStack<int> container(GetRandomStack(10, MIN_MAX_RANDOM_VALUES));
	ThreadSafeStructs::RWLockStack<int> containerCopy(container);

	ASSERT_EQ(containerCopy.Size(), container.Size());

	while (true)
	{
		if (containerCopy.Empty())
		{
			break;
		}
		ASSERT_EQ(containerCopy.TryPop(), container.TryPop());
	}
}

TEST(RWLockStack, MoveContainerContainer_OneThread)
{
	ThreadSafeStructs::RWLockStack<int> container(GetRandomStack(10, MIN_MAX_RANDOM_VALUES));
	ThreadSafeStructs::RWLockStack<int> containerTmpCopy(container);
	ThreadSafeStructs::RWLockStack<int> containerCopy(std::move(containerTmpCopy));

	ASSERT_EQ(containerCopy.Size(), container.Size());

	while (true)
	{
		if (containerCopy.Empty())
		{
			break;
		}
		ASSERT_EQ(containerCopy.TryPop(), container.TryPop());
	}
}

TEST(RWLockStack, PopItemFromEmptyContainer_OneThread)
{
	ThreadSafeStructs::RWLockStack<int> container;
	EXPECT_THROW(container.TryPop(), ThreadSafeStructs::ThreadSafetyException);
}

TEST(RWLockStack, PopItemsFromNotEmptyContainer_OneThread)
{
	auto stackOrign = GetRandomStack(10, MIN_MAX_RANDOM_VALUES);
	ThreadSafeStructs::RWLockStack<int> container(stackOrign);

	ASSERT_EQ(stackOrign.size(), container.Size());

	while (true)
	{
		if (stackOrign.empty())
		{
			break;
		}
		ASSERT_EQ(stackOrign.top(), container.TryPop());
		stackOrign.pop();
	}
}

TEST(RWLockStack, PopMoreItemsFromNotEmptyContainerThenSize_OneThread)
{
	auto stackOrign = GetRandomStack(10, MIN_MAX_RANDOM_VALUES);
	ThreadSafeStructs::RWLockStack<int> container(stackOrign);

	ASSERT_EQ(stackOrign.size(), container.Size());

	while (true)
	{
		if (stackOrign.empty())
		{
			break;
		}
		ASSERT_EQ(stackOrign.top(), container.TryPop());
		stackOrign.pop();
	}
	EXPECT_THROW(container.TryPop(), ThreadSafeStructs::ThreadSafetyException);
	EXPECT_THROW(container.TryPop(), ThreadSafeStructs::ThreadSafetyException);
}

TEST(RWLockStack, PushItemsWithCopyToRWStack_OneThread)
{
	auto numberOfGeneratedNumbers = 10;
	ThreadSafeStructs::RWLockStack<int> container;
	std::stack<int> orignContainer;

	for (int numbersGenerated = 0; numbersGenerated < numberOfGeneratedNumbers; ++numbersGenerated)
	{
		auto randomNumber = GetRandomNumber(MIN_MAX_RANDOM_VALUES);
		AddValueToContainerWithCopy(randomNumber, container, orignContainer);
		EXPECT_EQ(container.Size(), orignContainer.size());
	}
}

TEST(RWLockStack, PushItemsWithMoveToRWStack_OneThread)
{
	auto numberOfGeneratedNumbers = 10;
	ThreadSafeStructs::RWLockStack<int> container;
	std::stack<int> orignContainer;

	for (int numbersGenerated = 0; numbersGenerated < numberOfGeneratedNumbers; ++numbersGenerated)
	{
		auto randomNumber = GetRandomNumber(MIN_MAX_RANDOM_VALUES);
		AddValueToContainerWithMove(randomNumber, container, orignContainer);
		EXPECT_EQ(container.Size(), orignContainer.size());
	}
}

TEST(RWLockStack, PushPopItemWithCopyToEmptyRWStack_OneThread)
{
	ThreadSafeStructs::RWLockStack<int> container;
	std::stack<int> orignContainer;
	FillThreadSafeAndOriginStackWithRandomNumbers(1, MIN_MAX_RANDOM_VALUES, true, container, orignContainer);

	ASSERT_EQ(orignContainer.size(), container.Size());

	while (true)
	{
		if (orignContainer.empty())
		{
			break;
		}
		ASSERT_EQ(orignContainer.top(), container.TryPop());
		orignContainer.pop();
	}
}

TEST(RWLockStack, PushPopItemsWithCopyToRWStack_OneThread)
{
	ThreadSafeStructs::RWLockStack<int> container;
	std::stack<int> orignContainer;
	FillThreadSafeAndOriginStackWithRandomNumbers(100, MIN_MAX_RANDOM_VALUES, true, container, orignContainer);

	ASSERT_EQ(orignContainer.size(), container.Size());

	while (true)
	{
		if (orignContainer.empty())
		{
			break;
		}
		ASSERT_EQ(orignContainer.top(), container.TryPop());
		orignContainer.pop();
	}
}

TEST(RWLockStack, PushPopItemWithMoveToEmptyRWStack_OneThread)
{
	ThreadSafeStructs::RWLockStack<int> container;
	std::stack<int> orignContainer;
	FillThreadSafeAndOriginStackWithRandomNumbers(1, MIN_MAX_RANDOM_VALUES, false, container, orignContainer);

	ASSERT_EQ(orignContainer.size(), container.Size());

	while (true)
	{
		if (orignContainer.empty())
		{
			break;
		}
		ASSERT_EQ(orignContainer.top(), container.TryPop());
		orignContainer.pop();
	}
}

TEST(RWLockStack, PushPopItemsWithMoveToRWStack_OneThread)
{
	ThreadSafeStructs::RWLockStack<int> container;
	std::stack<int> orignContainer;
	FillThreadSafeAndOriginStackWithRandomNumbers(100, MIN_MAX_RANDOM_VALUES, false, container, orignContainer);

	ASSERT_EQ(orignContainer.size(), container.Size());

	while (true)
	{
		if (orignContainer.empty())
		{
			break;
		}
		ASSERT_EQ(orignContainer.top(), container.TryPop());
		orignContainer.pop();
	}
}

TEST(RWLockStack, PushStackRangeToEmptyRWStackWithCopy_OneThread)
{
	auto stackOrign = GetRandomStack(10, MIN_MAX_RANDOM_VALUES);

	ThreadSafeStructs::RWLockStack<int> container;

	container.PushRange(stackOrign);

	ASSERT_EQ(stackOrign.size(), container.Size());

	while (true)
	{
		if (stackOrign.empty())
		{
			break;
		}
		ASSERT_EQ(stackOrign.top(), container.TryPop());
		stackOrign.pop();
	}
}

TEST(RWLockStack, PushStackRangeToEmptyRWStackWithMove_OneThread)
{
	auto stackOrign = GetRandomStack(10, MIN_MAX_RANDOM_VALUES);
	auto stackOrignCopy = stackOrign;

	ThreadSafeStructs::RWLockStack<int> container;

	container.PushRange(std::move(stackOrignCopy));

	ASSERT_EQ(stackOrign.size(), container.Size());

	while (true)
	{
		if (stackOrign.empty())
		{
			break;
		}
		ASSERT_EQ(stackOrign.top(), container.TryPop());
		stackOrign.pop();
	}
}

TEST(RWLockStack, PushRWStackRangeToEmptyRWStackWithCopy_OneThread)
{
	ThreadSafeStructs::RWLockStack<int> containerToPush(GetRandomStack(10, MIN_MAX_RANDOM_VALUES));
	ThreadSafeStructs::RWLockStack<int> container;

	container.PushRange(containerToPush);

	ASSERT_EQ(containerToPush.Size(), container.Size());

	while (true)
	{
		if (containerToPush.Empty())
		{
			break;
		}
		ASSERT_EQ(containerToPush.TryPop(), container.TryPop());
	}
}

TEST(RWLockStack, PushRWStackRangeToEmptyRWStackWithMove_OneThread)
{
	ThreadSafeStructs::RWLockStack<int> containerToPush(GetRandomStack(10, MIN_MAX_RANDOM_VALUES));
	ThreadSafeStructs::RWLockStack<int> containerToPushCopy(containerToPush);
	ThreadSafeStructs::RWLockStack<int> container;

	container.PushRange(std::move(containerToPushCopy));

	ASSERT_EQ(containerToPush.Size(), container.Size());

	while (true)
	{
		if (containerToPush.Empty())
		{
			break;
		}
		ASSERT_EQ(containerToPush.TryPop(), container.TryPop());
	}
}

TEST(RWLockStack, PushRangeStackToNotEmptyRWStackWithCopy_OneThread)
{
	auto stackFirstPart = GetRandomStack(10, MIN_MAX_RANDOM_VALUES);
	auto stackSecondPart = GetRandomStack(10, MIN_MAX_RANDOM_VALUES);

	ThreadSafeStructs::RWLockStack<int> container(stackFirstPart);
	container.PushRange(stackSecondPart);

	auto stackOrign = PushBackStack(stackFirstPart, stackSecondPart);

	ASSERT_EQ(stackOrign.size(), container.Size());

	while (true)
	{
		if (stackOrign.empty())
		{
			break;
		}
		EXPECT_EQ(stackOrign.top(), container.TryPop());
		stackOrign.pop();
	}
}

TEST(RWLockStack, PushRangeStackToNotEmptyRWStackWithMove_OneThread)
{
	auto stackFirstPart = GetRandomStack(10, MIN_MAX_RANDOM_VALUES);
	auto stackSecondPart = GetRandomStack(10, MIN_MAX_RANDOM_VALUES);

	ThreadSafeStructs::RWLockStack<int> container(stackFirstPart);
	container.PushRange(std::move(stackSecondPart));

	auto stackOrign = PushBackStack(stackFirstPart, stackSecondPart);

	ASSERT_EQ(stackOrign.size(), container.Size());

	while (true)
	{
		if (stackOrign.empty())
		{
			break;
		}
		EXPECT_EQ(stackOrign.top(), container.TryPop());
		stackOrign.pop();
	}
}

//MultiThread tests!

TEST(RWLockStack, PushWithSingleAdditionalThread)
{
	ThreadSafeStructs::RWLockStack<int> container;
	const auto numberOfGeneratedNumbers = 10;

	auto checkingFunction = [](const int numberOfAddedItems, ThreadSafeStructs::RWLockStack<int>& container)
		{
			for (int numbersGenerated = 0; numbersGenerated < numberOfAddedItems; ++numbersGenerated)
			{
				container.Push(GetRandomNumber(MIN_MAX_RANDOM_VALUES));
			}
		};

	std::promise<void> testingThreadReady;
	std::promise<void> testerThreadReady;
	std::future<void> whaitForTesterThreadReady(testerThreadReady.get_future());
	std::future<void> whaitForTestingThreadReady(testingThreadReady.get_future());
	auto workDone = std::async(std::launch::async, [checkingFunction, numberOfGeneratedNumbers, &container, &testingThreadReady, &whaitForTesterThreadReady]() //TODO think how to cut catch params)
		{
			testingThreadReady.set_value();
			whaitForTesterThreadReady.wait();
			checkingFunction(numberOfGeneratedNumbers, container);
		});

	whaitForTestingThreadReady.wait();
	testerThreadReady.set_value();

	workDone.get();
	EXPECT_EQ(container.Size(), 10);
}

TEST(RWLockStack, PushWithAdditionalThreads_SOME_PROBLEM)
{
	ThreadSafeStructs::RWLockStack<int> container;
	const auto numberOfGeneratedNumbers = 10;
	const auto numberOfTestingThreads = 2;

	auto checkingFunction = [](const int numberOfAddedItems, ThreadSafeStructs::RWLockStack<int>& container)
		{
			for (int numbersGenerated = 0; numbersGenerated < numberOfAddedItems; ++numbersGenerated)
			{
				container.Push(GetRandomNumber(MIN_MAX_RANDOM_VALUES));
			}
		};

	std::promise<void> mainThreadReadyPromise;
	std::shared_future<void> mainThreadReadyFeature(mainThreadReadyPromise.get_future());
	std::list<std::future<void>> threadReadyFeatures;
	std::list<std::future<void>> threadProcessFinishedFeatures;

	for (int threadToTest = 0; threadToTest < numberOfTestingThreads; ++threadToTest)
	{
		auto currentThreadPromise = std::promise<void>();
		threadReadyFeatures.push_back(currentThreadPromise.get_future());

		auto threadDoneFeature = std::async(std::launch::async, [checkingFunction, numberOfGeneratedNumbers, &container, &currentThreadPromise, &mainThreadReadyFeature]()
			{
				//currentThreadPromise.set_value(); //Some problem, wait is not works
				mainThreadReadyFeature.wait();
				checkingFunction(numberOfGeneratedNumbers, container);
			}
		);
		threadProcessFinishedFeatures.push_back(std::move(threadDoneFeature));
	}

	for (auto featureReady = threadReadyFeatures.begin(); featureReady != threadReadyFeatures.end(); ++featureReady)
	{
		featureReady->wait(); //Some problem, wait is not works
	}
	mainThreadReadyPromise.set_value();

	for (auto featureReady = threadProcessFinishedFeatures.begin(); featureReady != threadProcessFinishedFeatures.end(); ++featureReady)
	{
		featureReady->wait();
	}

	EXPECT_EQ(container.Size(), 100);
}

TEST(RWLockStack, PushWithAdditionalThreads)
{
	ThreadSafeStructs::RWLockStack<int> container;
	const auto numberOfGeneratedNumbers = 10;
	const auto numberOfTestingThreads = 10;

	auto checkingFunction = [](const int numberOfAddedItems, ThreadSafeStructs::RWLockStack<int>& container)
		{
			for (int numbersGenerated = 0; numbersGenerated < numberOfAddedItems; ++numbersGenerated)
			{
				container.Push(GetRandomNumber(MIN_MAX_RANDOM_VALUES));
			}
		};

	std::promise<void> mainThreadReadyPromise;
	std::shared_future<void> mainThreadReadyFeature(mainThreadReadyPromise.get_future());
	std::list<std::unique_ptr<std::promise<void>>> threadReadyPromise;
	std::list<std::future<void>> threadProcessFinishedFeatures;

	for (int threadToTest = 0; threadToTest < numberOfTestingThreads; ++threadToTest)
	{
		auto currentThreadPromise = std::make_unique<std::promise<void>>();

		auto threadDoneFeature = std::async(std::launch::async, [checkingFunction, numberOfGeneratedNumbers, &container, &mainThreadReadyFeature](std::promise<void>& currentThreadPromise)
			{
				currentThreadPromise.set_value();
				mainThreadReadyFeature.wait();
				checkingFunction(numberOfGeneratedNumbers, container);
			},
			std::ref(*currentThreadPromise.get())
		);
		threadReadyPromise.push_back(std::move(currentThreadPromise));
		threadProcessFinishedFeatures.push_back(std::move(threadDoneFeature));
	}

	for (auto featureReady = threadReadyPromise.begin(); featureReady != threadReadyPromise.end(); ++featureReady)
	{
		featureReady->get()->get_future().wait(); //but works in this configuration
	}
	mainThreadReadyPromise.set_value();

	for (auto featureReady = threadProcessFinishedFeatures.begin(); featureReady != threadProcessFinishedFeatures.end(); ++featureReady)
	{
		featureReady->wait();
	}

	EXPECT_EQ(container.Size(), 100);
}