#pragma once
#include "ThreadSafeException.h"

namespace
{
	template<typename T>
	std::vector<T> ConvertStackToVector(std::stack<T> stack)
	{
		std::vector<int> buffer;

		while (!stack.empty())
		{
			buffer.push_back(stack.top());
			stack.pop();
		}
		return buffer;
	}

	template<typename T>
	void AppendStackToStack(std::stack<T>& first, const std::stack<T>& second)
	{
		auto bufferSecondPart = ConvertStackToVector(second);

		for (auto pointer = bufferSecondPart.rbegin(); pointer != bufferSecondPart.rend(); ++pointer)
		{
			first.push(*pointer);
		}
	}

	template<typename T>
	void AppendStackToStack(std::stack<T>& first, const std::stack<T>&& second)
	{
		auto bufferSecondPart = ConvertStackToVector(std::move(second));

		for (auto pointer = bufferSecondPart.rbegin(); pointer != bufferSecondPart.rend(); ++pointer)
		{
			first.push(*pointer);
		}
	}
}

namespace ThreadSafeStructs
{
	template<typename T>
	class RWLockStack
	{
	public:
		RWLockStack() noexcept;
		explicit RWLockStack(RWLockStack<T>& stack) noexcept;
		explicit RWLockStack(RWLockStack<T>&& stack) noexcept;
		explicit RWLockStack(const std::stack<T>& stack) noexcept;
		explicit RWLockStack(std::stack<T>&& stack) noexcept;

		RWLockStack<T>& Push(const T& item);
		RWLockStack<T>& Push(const T&& item);
		RWLockStack<T>& PushRange(RWLockStack<T>& stack);
		RWLockStack<T>& PushRange(RWLockStack<T>&& stack);
		RWLockStack<T>& PushRange(const std::stack<T>& stack);
		RWLockStack<T>& PushRange(std::stack<T>&& stack);
		T TryPop();
		T WhaitAndPop();

		bool Empty() noexcept;
		uint32_t Size() noexcept;

	private:
		std::stack<T> data;
		boost::shared_mutex mutex;
		std::condition_variable condVar;
	};

	template<typename T>
	RWLockStack<T>::RWLockStack() noexcept
	{
	}

	template<typename T>
	RWLockStack<T>::RWLockStack(RWLockStack<T>& stack) noexcept
	{
		const std::shared_lock<boost::shared_mutex> lock(stack.mutex);
		data = stack.data;
	}

	template<typename T>
	RWLockStack<T>::RWLockStack(RWLockStack<T>&& stack) noexcept
	{
		const std::lock_guard<boost::shared_mutex> lock(stack.mutex);
		data = std::move(stack.data);
	}

	template<typename T>
	RWLockStack<T>::RWLockStack(const std::stack<T>& stack) noexcept
		: data(stack)
	{
	}

	template<typename T>
	RWLockStack<T>::RWLockStack(std::stack<T>&& stack) noexcept
		: data(std::forward<std::stack<T>>(stack))
	{
	}

	template<typename T>
	bool RWLockStack<T>::Empty() noexcept 
	{
		const std::shared_lock<boost::shared_mutex> lock(mutex);
		return data.empty();
	}

	template<typename T>
	uint32_t RWLockStack<T>::Size() noexcept
	{
		const std::shared_lock<boost::shared_mutex> lock(mutex);
		return static_cast<uint32_t>(data.size());
	}

	template<typename T>
	RWLockStack<T>& RWLockStack<T>::Push(const T& item)
	{
		const std::lock_guard<boost::shared_mutex> lock(mutex);
		data.push(item);
		condVar.notify_one();
		return *this;
	}

	template<typename T>
	RWLockStack<T>& RWLockStack<T>::Push(const T&& item)
	{
		const std::lock_guard<boost::shared_mutex> lock(mutex);
		data.push(std::move(item));
		condVar.notify_one();
		return *this;
	}

	template<typename T>
	RWLockStack<T>& RWLockStack<T>::PushRange(RWLockStack<T>& rwLockStack)
	{
		std::unique_lock<boost::shared_mutex> dataToCopy(rwLockStack.mutex, std::defer_lock);
		std::unique_lock<boost::shared_mutex> orignData(mutex, std::defer_lock);
		std::lock(dataToCopy, orignData);

		// we emulate data.push_range(stack); from unsupported C++23
		AppendStackToStack(data, rwLockStack.data);

		condVar.notify_all();
		return *this;
	}

	template<typename T>
	RWLockStack<T>& RWLockStack<T>::PushRange(RWLockStack<T>&& rwLockStack)
	{
		std::unique_lock<boost::shared_mutex> dataToCopy(rwLockStack.mutex, std::defer_lock);
		std::unique_lock<boost::shared_mutex> orignData(mutex, std::defer_lock);
		std::lock(dataToCopy, orignData);

		// we emulate data.push_range(stack); from unsupported C++23
		AppendStackToStack(data, std::forward<std::stack<T>>(rwLockStack.data));

		condVar.notify_all();
		return *this;
	}

	template<typename T>
	RWLockStack<T>& RWLockStack<T>::PushRange(const std::stack<T>& stack)
	{
		std::lock_guard<boost::shared_mutex> lock(mutex);

		// we emulate data.push_range(stack); from unsupported C++23
		AppendStackToStack(data, stack);

		condVar.notify_all();
		return *this;
	}

	template<typename T>
	RWLockStack<T>& RWLockStack<T>::PushRange(std::stack<T>&& stack)
	{
		std::lock_guard<boost::shared_mutex> lock(mutex);

		// we emulate data.push_range(stack); from unsupported C++23
		AppendStackToStack(data, std::forward<std::stack<T>>(stack));

		condVar.notify_all();
		return *this;
	}

	template<typename T>
	T RWLockStack<T>::TryPop()
	{
		const std::lock_guard<boost::shared_mutex> lock(mutex);
		if (data.empty())
		{
			throw ThreadSafeStructs::ThreadSafetyException("Item can not be poped from stack, stack is empty.");
		}
		auto dataItem = data.top();
		data.pop();
		return dataItem;
	}

	template<typename T>
	T RWLockStack<T>::WhaitAndPop()
	{
		condVar.wait(std::unique_lock(mutex), [data]() { return data.empty(); });
		const std::lock_guard<boost::shared_mutex> lock(mutex);

		auto dataItem = data.top();
		data.pop();
		return dataItem;
	}
}