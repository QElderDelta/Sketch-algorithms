#include "shard_data.hpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace
{
    ShardData createShardDataUsingExistingIds(const std::vector<uint64_t> &aIds,
                                              uint32_t                     aShardCount,
                                              ShardDataDistribution        aDistributionType)
    {
        auto sResponseSizes
            = details::generateShardResponseSizes(aIds.size(), aShardCount, aDistributionType);
        for (size_t i = 1; i < sResponseSizes.size(); ++i)
        {
            sResponseSizes[i] += sResponseSizes[i - 1];
        }

        std::vector<std::unordered_set<uint64_t>> sResult(aShardCount);

        for (size_t i = 0; i < sResponseSizes.size(); ++i)
        {
            const auto sBegin = !i ? aIds.begin() : std::next(aIds.begin(), sResponseSizes[i - 1]);
            const auto sEnd
                = i == sResponseSizes.size() - 1 ? aIds.end() : std::next(aIds.begin(), sResponseSizes[i]);

            sResult[i].insert(sBegin, sEnd);
        }

        return {std::move(sResult), aIds.size()};
    }
}  // namespace

namespace details
{
    std::vector<uint32_t> generateShardResponseSizes(uint32_t              aResponseSize,
                                                     uint32_t              aShardCount,
                                                     ShardDataDistribution aDistributionType)
    {
        if (aShardCount == 0)
        {
            return {};
        }

        std::vector<uint32_t> sResult;
        sResult.reserve(aShardCount);

        if (aDistributionType == ShardDataDistribution::EVEN)
        {
            auto sPerShardSize = (aResponseSize + aShardCount - 1) / aShardCount;

            uint32_t sTotal = 0;
            for (uint32_t i = 0; i < aShardCount; ++i)
            {
                if (sTotal + sPerShardSize <= aResponseSize)
                {
                    sResult.push_back(sPerShardSize);
                    sTotal += sPerShardSize;
                }
                else
                {
                    sResult.push_back(aResponseSize - sTotal);
                    sTotal += aResponseSize - sTotal;
                }
            }

            if (sTotal < aResponseSize)
            {
                sResult.back() += aResponseSize - sTotal;
            }
        }
        else if (aDistributionType == ShardDataDistribution::RANDOM)
        {
            std::random_device         sDevice;
            std::default_random_engine sEngine(sDevice());

            uint32_t sTotal = 0;
            for (uint32_t i = 0; i < aShardCount; ++i)
            {
                if (sTotal == aResponseSize || i == aShardCount - 1)
                {
                    sResult.push_back(aResponseSize - sTotal);
                    continue;
                }

                std::uniform_int_distribution<uint32_t> sRng(0, aResponseSize - sTotal);
                sResult.push_back(sRng(sEngine));
                sTotal += sResult.back();
            }
        }

        return sResult;
    }
}  // namespace details

ShardData generateShardData(uint32_t              aResponseSize,
                            uint32_t              aShardCount,
                            ShardDataDistribution aDistributionType)
{
    if (aShardCount == 0)
    {
        return {};
    }

    return createShardDataUsingExistingIds(generateIds(aResponseSize), aShardCount, aDistributionType);
}

ShardData generateShardDataUsingExisting(const ShardData      &aExisting,
                                         ShardDataPrototype    aPrototype,
                                         ShardDataDistribution aDistributionType)
{
    std::vector<uint64_t> sIds;
    sIds.reserve(aPrototype.response_size);

    if (aPrototype.operation == ShardDataPrototype::INTERSECTION)
    {
        if (aPrototype.operation_result_size > aExisting.total_size
            || aPrototype.operation_result_size > aPrototype.response_size)
        {
            throw std::invalid_argument("Invalid operation result size for intersection");
        }

        uint32_t sCounter = 0;
        for (const auto &sShard : aExisting.data)
        {
            for (auto sId : sShard)
            {
                if (sCounter++ < aPrototype.operation_result_size)
                {
                    sIds.push_back(sId);
                }
                else
                {
                    break;
                }
            }
        }
    }
    else if (aPrototype.operation == ShardDataPrototype::UNION)
    {
        if (aPrototype.operation_result_size < aExisting.total_size
            || aPrototype.operation_result_size - aExisting.total_size > aPrototype.response_size)
        {
            throw std::invalid_argument("Invalid operation result size for union");
        }

        uint32_t sCounter = 0;
        for (const auto &sShard : aExisting.data)
        {
            for (auto sId : sShard)
            {
                if (sCounter++
                    < aPrototype.response_size - (aPrototype.operation_result_size - aExisting.total_size))
                {
                    sIds.push_back(sId);
                }
                else
                {
                    break;
                }
            }
        }
    }

    const auto sGeneratedIds = generateIds(aPrototype.response_size - sIds.size());
    sIds.insert(sIds.end(), sGeneratedIds.begin(), sGeneratedIds.end());

    return createShardDataUsingExistingIds(sIds, aExisting.data.size(), aDistributionType);
}

std::vector<uint64_t> generateIds(uint32_t aCount, const std::set<uint64_t> &sExcludedIds)
{
    std::vector<uint64_t> sIds;
    sIds.reserve(aCount);

    constexpr uint64_t MIN_ID = 35033762171394u;
    constexpr uint64_t MAX_ID = 18446735718737694272u;

    std::random_device                      sDevice;
    std::default_random_engine              sEngine(sDevice());
    std::uniform_int_distribution<uint64_t> sRng(MIN_ID, MAX_ID);

    for (uint32_t i = 0; i < aCount; ++i)
    {
        while (true)
        {
            if (auto sId = sRng(sEngine); !sExcludedIds.contains(sId))
            {
                sIds.push_back(sRng(sEngine));
                break;
            }
        }
    }

    return sIds;
}
