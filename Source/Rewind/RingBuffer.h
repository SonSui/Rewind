#pragma once
template<typename T>
class TRingBuffer
{
public:
	explicit TRingBuffer(int32 InMax)
		: MaxElements(InMax){
		Data.SetNum(InMax);
	}

	void Push(const T& Item)
	{
		Data[Head] = Item;
		Head = (Head + 1) % MaxElements;
		if (Count < MaxElements) ++Count;
	}

	bool Pop(T& Out)
	{
		if (Count == 0) return false;
		Head = (Head - 1 + MaxElements) % MaxElements;
		Out = Data[Head];
		--Count;
		return true;
	}

	int32 Num() const { return Count; }

private:
	int32 Head = 0;
	int32 Count = 0;
	int32 MaxElements;
	TArray<T> Data;
};
