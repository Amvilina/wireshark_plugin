#pragma once
#include "config.h"
#include <vector>

class HFHandler {
  private:
    std::vector<int> _ints;
    std::vector<hf_register_info> _infos;
  public:
    size_t AddHF(hf_register_info info)
    {
        _infos.push_back(info);
        _ints.push_back(-1);
        return _infos.size() - 1;
    }

    void LinkHF()
    {
        for (size_t i = 0; i < _infos.size(); ++i)
            _infos[i].p_id = &_ints[i];
    }

    hf_register_info* GetData()
    {
        return _infos.data();
    }

    int GetIntByIndex(size_t index) const
    {
        return _ints[index];
    }

    int& GetIntByIndex(size_t index)
    {
        return _ints[index];
    }

    size_t Size() const {
        return _infos.size();
    }
};