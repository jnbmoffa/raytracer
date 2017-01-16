#pragma once

#include <stdexcept>
#include <memory>

#define INDEX_NONE -1

// Custom dyamic array
// Provides optimized functionality and more usability than std::vector
template<typename T>
class Array
{
public:
    using value_type = T;

    Array() :
        Contents(nullptr),
        NumElements(0),
        MaxNumElements(0)
    {}

    // Returns the item at the given index
    inline T& operator[](int Index) const
    {
        return At(Index);
    }

    // The number of items in the array
    inline size_t Num() const
    {
        return NumElements;
    }

    // Adds an item to the end of the array
    void Add(const T& NewItem)
    {
        // Allocate more space if full
        if (IsAllocatedSpaceFull())
        {
            GrowAllocatedSpace();
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
    void Insert(const T& NewItem, size_t Index)
    {
        if (IsValidIndex(Index))
        {
            // One past the end
            size_t OriginalNum = NumElements;

            // Allocate more space if full
            if (IsAllocatedSpaceFull())
            {
                GrowAllocatedSpace();
            }

            // Shift everything forward to make room
            for (size_t i = OriginalNum; i > Index; --i)
            {
                Contents[i] = Contents[i - 1];
            }

            // Add the item
            Contents[Index] = NewItem;
            NumElements++;
        }
        else
        {
            // Bad index, add the item to the end
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
    void RemoveAt(size_t Index)
    {
        if (IsValidIndex(Index))
        {
            Contents[Index].~T();
            for (size_t i = Index; (i + 1) < NumElements; ++i)
            {
                Contents[i] = Contents[i + 1];
            }
            NumElements--;
        }
    }

    // Removes the item at the given index (order is not preserved)
    // Does nothing if the index is invalid
    void RemoveAtSwap(size_t Index)
    {
        if (IsValidIndex(Index))
        {
            Contents[Index].~T();
            if (Index != NumElements - 1)	// If removing last, just decrease counter
            {
                T last = Contents[NumElements - 1];
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
    void Reserve(size_t Number)
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
        for (size_t i = 0; i < Num(); ++i)
        {
            if (At(i) == Item)
            {
                return i;
            }
        }

        return INDEX_NONE;
    }

    // Split this array into two around the index
    // a = [0,Index), b = (Index,End]
    bool Split(size_t Index, Array<T>& a, Array<T>& b) const
    {
        if (IsValidIndex(Index) && NumElements > 1)
        {
            a.Clear();
            b.Clear();
            a.Reserve(Index);
            b.Reserve(NumElements - Index);
            for (size_t i = 0; i < Index; ++i)
            {
                a.Add(At(i));
            }
            for (size_t i = Index + 1; i < NumElements; ++i)
            {
                b.Add(At(i));
            }
            return true;
        }
        return false;
    }

    // Empty the array
    void Clear()
    {
        // Unique ptr cleans up memory
        Contents = nullptr;
        NumElements = 0;
        MaxNumElements = 0;
    }

    // Returns if the given index is valid in this array
    inline bool IsValidIndex(size_t Index) const
    {
        return Index < NumElements;
    }

    // Iterator for range-based looping
    class Iterator
    {
        size_t Index;
        const Array<T>* pArray;

    public:
        Iterator(const Array<T>* pArray, size_t Index = 0) : Index(Index), pArray(pArray) {}

        bool operator!= (const Iterator& rhs) const
        {
            return Index != rhs.Index;
        }

        T& operator*() const
        {
            return pArray->At(Index);
        }

        const Iterator& operator++()
        {
            ++Index;
            return *this;
        }

        Iterator operator+(int I) const
        {
            return Iterator(pArray, Index + I);
        }
    };

    // For range-based looping
    Iterator begin() const
    {
        return Iterator(this, 0);
    }

    // For range-based looping
    Iterator end() const
    {
        return Iterator(this, NumElements);
    }

private:

    std::unique_ptr<T[]> Contents;
    size_t NumElements;		// Number of elements in the array
    size_t MaxNumElements;	// Amount of space available in the allocated space

    // Allocate space for the given number of items
    // Truncates the array if the given number is less than the current number of elements
    void AllocateNewSpace(size_t NumberToAlllocate)
    {
        // Truncate the array if it's larger than the new size
        if (NumElements > NumberToAlllocate)
        {
            NumElements = NumberToAlllocate;
        }

        // Allocate new space and fill it with the old data
        std::unique_ptr<T[]> NewContents(new T[NumberToAlllocate]);
        if (Contents != nullptr)
        {
            for (size_t i = 0; i < NumElements; ++i)
            {
                NewContents[i] = Contents[i];
            }
            // Unique ptr cleans up memory
            Contents = std::move(NewContents);
        }
        else
        {
            // Unique ptr cleans up memory
            Contents = std::move(NewContents);
        }
        MaxNumElements = NumberToAlllocate;
    }

    bool IsAllocatedSpaceFull() const
    {
        return NumElements == MaxNumElements;
    }

    // Grow our space by powers of 2
    void GrowAllocatedSpace()
    {
        AllocateNewSpace(MaxNumElements > 0 ? MaxNumElements * 2 : 1);
    }
};