#pragma once
#include "config.h"
#include <vector>

class ETTHandler {
  private:
    std::vector<int> _ints;
    std::vector<int*> _pointers;
  public:
    size_t AddETT()
    {
        _ints.push_back(-1);
        return _ints.size() - 1;
    }

    void LinkETT()
    {
        _pointers.resize(_ints.size());
        for (size_t i = 0; i < _ints.size(); i++)
            _pointers[i] = &_ints[i];
    }

    int* const* GetData()
    {
        return _pointers.data();
    }

    int GetIntByIndex(size_t index) const
    {
        return _ints[index];
    }

    size_t Size() const {
        return _ints.size();
    }
};