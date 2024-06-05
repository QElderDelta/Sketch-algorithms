#pragma once

#include <cstdint>
#include <set>
#include <unordered_set>
#include <vector>

enum class ShardDataDistribution
{
    EVEN,
    RANDOM
};

namespace details
{
    std::vector<uint32_t> generateShardResponseSizes(uint32_t              aResponseSize,
                                                     uint32_t              aShardCount,
                                                     ShardDataDistribution aDistributionType);
}

struct ShardData
{
    std::vector<std::unordered_set<uint64_t>> data;
    uint64_t                                  total_size{0};
};

ShardData generateShardData(uint32_t              aResponseSize,
                            uint32_t              aShardCount,
                            ShardDataDistribution aDistributionType);

struct ShardDataPrototype
{
    enum Operation
    {
        INTERSECTION,
        UNION
    };

    Operation operation = INTERSECTION;
    uint64_t  operation_result_size{0};
    uint32_t  response_size{0};
};

ShardData generateShardDataUsingExisting(const ShardData&      aExisting,
                                         ShardDataPrototype    aPrototype,
                                         ShardDataDistribution aDistributionType);

std::vector<uint64_t> generateIds(uint32_t aCount, const std::set<uint64_t>& sExcludedIds = {});
