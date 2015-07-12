#pragma once

#define INDEX_NONE -1

template<typename T>
class Array
{
	T* Contents;
	unsigned int NumElements;		// Number of elements in the array
	unsigned int MaxNumElements;	// Amount of space available in the array

	// Allocate space for the given number of items
	// Truncates the array if the given number is less than the current number of elements
	void AllocateNewSpace(unsigned int NumberToAlllocate)
	{
		// Truncate the array if it's larger than the new size
		if (NumElements > NumberToAlllocate)
		{
			NumElements = NumberToAlllocate;
		}

		// Allocate new space and fill it with the old data
		T* NewContents = new T[NumberToAlllocate];
		if (Contents != nullptr)
		{
			for (unsigned int i = 0; i < NumElements; i++)
			{
				NewContents[i] = Contents[i];
			}
			FreeContents();
			Contents = NewContents;
		}
		else
		{
			Contents = NewContents;
		}
		MaxNumElements = NumberToAlllocate;
	}

	// Free the memory allocated for the contents
	void FreeContents()
	{
		if (Contents != nullptr)
		{
			delete[] Contents;
		}
		Contents = nullptr;
	}

public:
	Array() : Contents(nullptr), NumElements(0), MaxNumElements(0){}

	~Array()
	{
		FreeContents();
	}

	// Returns the item at the given index
	inline T& operator[](int Index) const
	{
		return this->At(Index);
	}
	
	// The number of items in the array
	inline unsigned int Num() const
	{
		return NumElements;
	}

	// Adds an item to the end of the array
	void Add(const T& NewItem)
	{
		// Allocate more space if full
		if (NumElements == MaxNumElements)
		{
			AllocateNewSpace(NumElements + 1);
		}

		Contents[NumElements++] = NewItem;
	}

	// Add an item to the end of the array if it is not
	// already in the array
	void AddUnique(const T& NewItem)
	{
		if (Find(NewItem) == INDEX_NONE)
		{
			Add(NewItem);
		}
	}

	// Insert an item at the given index (preserving order)
	void Insert(const T& NewItem, unsigned int Index)
	{
		if (IsValidIndex(Index))
		{
			unsigned int OriginalNum = NumElements;

			// Allocate more space if full
			if (NumElements == MaxNumElements)
			{
				AllocateNewSpace(NumElements + 1);
			}

			// Shift everything forward to make room
			for (unsigned int i = OriginalNum; i > Index; i--)
			{
				Contents[i] = Contents[i - 1];
			}

			// Add the item
			Contents[Index] = NewItem;
			NumElements++;
		}
		else
		{
			Add(NewItem);
		}
	}

	// Remove the provided item if it's in the array
	bool Remove(const T& Item)
	{
		int Index = Find(Item);

		if (Index != INDEX_NONE)
		{
			RemoveAt(Index);
			return true;
		}

		return false;
	}

	// Removes the item at the given index (preserving order)
	// Does nothing if the index is invalid
	void RemoveAt(unsigned int Index)
	{
		if (IsValidIndex(Index))
		{
			for (unsigned int i = Index; (i + 1) < NumElements; i++)
			{
				Contents[i] = Contents[i + 1];
			}
			NumElements--;
		}
	}

	// Removes the item at the given index (order is not preserved)
	// Does nothing if the index is invalid
	void RemoveAtSwap(unsigned int Index)
	{
		if (IsValidIndex(Index))
		{
			if (Index != NumElements-1)	// If removing last, just decrease counter
			{
				T last = Contents[NumElements-1];
				Contents[Index] = last;
			}
			NumElements--;
		}
	}

	// Returns the item at the given index
	T& At(int Index) const
	{
		if (IsValidIndex(Index))
		{
			return Contents[Index];
		}

		throw std::out_of_range(" Array index out of range.");
	}

	// Reserve space for the given number of elements
	void Reserve(unsigned int Number)
	{
		if (Number > MaxNumElements)
		{
			AllocateNewSpace(Number);
		}
	}

	// Finds the index of an item in the array
	// Returns INDEX_NONE if the item is not in the array
	int Find(const T& Item) const
	{
		for (unsigned int i = 0; i < Num(); i++)
		{
			if (this->At(i) == Item)
			{
				return i;
			}
		}

		return INDEX_NONE;
	}

	// Empty the array
	void Clear()
	{
		FreeContents();
		NumElements = 0;
		MaxNumElements = 0;
	}

	// Returns if the given index is valid in this array
	inline bool IsValidIndex(unsigned int Index) const
	{
		return Index < NumElements;
	}

	// Iterator for range-based looping
	class Iterator
	{
		unsigned int Index;
		const Array<T>* pArray;

	public:
		Iterator(const Array<T>* pArray, unsigned int Index = 0) : Index(Index), pArray(pArray) {}

		bool operator!= (const Iterator& rhs)
		{
			return Index != rhs.Index;
		}

		T& operator* () const
		{
			return pArray->At(Index);
		}

		const Iterator& operator++()
		{
			++Index;
			return *this;
		}
	};

	// For range-based looping
	inline Iterator begin()
	{
		return Iterator(this, 0);
	}

	// For range-based looping
	inline Iterator end()
	{
		return Iterator(this, NumElements);
	}
};