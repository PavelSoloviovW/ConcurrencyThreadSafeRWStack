#include "stdfx.h"
#include "RWLockStack.h"
#include "ThreadSafeException.h"

int main()
{
	int rInt = 10;
	std::stack<int> data;
	data.push(4);
	data.push(3);
	data.push(2);
	data.push(1);
	ThreadSafeStructs::RWLockStack<int> test(data);
	ThreadSafeStructs::RWLockStack<int> test_2(test);
	bool res = test_2.Empty();
	res;
	test_2.Push(rInt).Push(11);

	auto oops = test_2.TryPop();
	oops;

	std::stack<int> tmp;
	tmp.push(1);
	tmp.push(2);
	tmp.push(3);
	tmp.push(4);
	test.PushRange(std::move(tmp));
}