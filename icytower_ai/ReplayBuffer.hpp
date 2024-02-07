#include "tiny_dnn/tinier_dnn.h"
#include "RL.h"

// implemented as composition
template <typename T>
class ReplayBuffer
{
public:
	ReplayBuffer(): buf(maxSize) {};

	// vector interface

	void push_back(const T& experience)
	{
		buf[idx] = experience;
		idx = (idx + 1) % maxSize;
		currentSize = std::max(idx + 1, currentSize);
	}

	size_t size() { return currentSize; }

	// works more like vector::at()
	T& operator [](size_t n)
	{
		if (n + 1 > currentSize)
			throw "Invalid element";
		return buf[n];
	}
	// vector interface end

	/// <summary>
	/// After running a frame, update next state in the previous experience
	/// </summary>
	/// <typeparam name="T"></typeparam>
	void provideNextState(const tiny_dnn::vec_t& state)
	{
		buf[idx==0?maxSize-1:idx-1].nextState = state; //copy
	}

private:
	unsigned currentSize = 0;
	unsigned maxSize = 5000;
	unsigned idx = 0;
	std::vector<T> buf;

};