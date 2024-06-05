#pragma once

#include "shard_data.hpp"

#include <set>
#include <vector>

class BaselineCommon
{
public:
    void                      addShardData(const std::vector<ShardData>& aShardData, uint32_t aPassCondition);
    uint64_t                  estimateMemoryUsage() const;
    const std::set<uint64_t>& getIds() const;

private:
    std::set<uint64_t>                 ids;
    std::vector<std::vector<uint64_t>> mergeShardData(const std::vector<ShardData>& aShardData) const;
};