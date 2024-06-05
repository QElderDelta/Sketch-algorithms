#include "presence_checkers.hpp"

#include "common.hpp"

void BaselinePresenceChecker::addShardData(const std::vector<ShardData>& aShardData, uint32_t aPassCondition)
{
    presence_checker.addShardData(aShardData, aPassCondition);
}

uint64_t BaselinePresenceChecker::estimateMemoryUsage() const
{
    return presence_checker.estimateMemoryUsage();
}

bool BaselinePresenceChecker::isPresent(uint64_t aId) const
{
    return getIds().contains(aId);
}

const std::set<uint64_t>& BaselinePresenceChecker::getIds() const
{
    return presence_checker.getIds();
}

BloomFilterPresenceChecker::BloomFilterPresenceChecker(uint64_t aSecondLevelSize,
                                                       uint32_t aNumberOfHashFunctions)
: second_level_size(aSecondLevelSize)
, number_of_hash_functions(aNumberOfHashFunctions)
, bloom_filter(second_level_size, number_of_hash_functions)
{}

void BloomFilterPresenceChecker::addShardData(const std::vector<ShardData>& aShardData,
                                              uint32_t                      aPassCondition)
{
    const auto sConvertDependencyToBloomFilter = [this](const ShardData& aData)
    {
        const auto sConvertShardResponseToBloomFilter = [&](const std::unordered_set<uint64_t>& aShardData)
        {
            sketch::bf_t sFilter(second_level_size, number_of_hash_functions);

            for (auto sId : aShardData)
            {
                sFilter.addh(sId);
            }

            return sFilter;
        };

        std::vector<sketch::bf_t> sShardBloomFilters;
        std::transform(aData.data.begin(),
                       aData.data.end(),
                       std::back_inserter(sShardBloomFilters),
                       [&sConvertShardResponseToBloomFilter](const auto& aShardData)
                       { return sConvertShardResponseToBloomFilter(aShardData); });

        Timer        sTimer("BloomFilterPresenceChecker merge shard data");
        sketch::bf_t sFilter(second_level_size, number_of_hash_functions);

        for (const auto& sShard : sShardBloomFilters)
        {
            sFilter |= sShard;
        }

        return sFilter;
    };

    Timer sTimer("BloomFilterPresenceChecker coverage calculation");
    if (aPassCondition == 2 && aShardData.size() == 2)
    {
        bloom_filter
            = sConvertDependencyToBloomFilter(aShardData[0]) & sConvertDependencyToBloomFilter(aShardData[1]);
    }
    else
    {
        for (const auto& sDep : aShardData)
        {
            bloom_filter |= sConvertDependencyToBloomFilter(sDep);
        }
    }
}

bool BloomFilterPresenceChecker::isPresent(uint64_t aId) const
{
    return bloom_filter.may_contain(aId);
}

uint64_t BloomFilterPresenceChecker::estimateMemoryUsage() const
{
    auto sRes = bloom_filter.est_memory_usage();
    return sRes.first + sRes.second;
}

uint64_t BloomFilterPresenceChecker::estimateCardinality() const
{
    return static_cast<uint64_t>(bloom_filter.cardinality_estimate());
}

HyperLogLogPresenceChecker::HyperLogLogPresenceChecker(uint64_t aHllCount, uint32_t aHllBucketCountLog2)
: hll_count(aHllCount)
, hll_bucket_count_log2(aHllBucketCountLog2)
, filter(hll_count, 1337, hll_bucket_count_log2)
{}
void HyperLogLogPresenceChecker::addShardData(const std::vector<ShardData>& aShardData, uint32_t)
{
    const auto sConvertDependencyToBloomFilter = [this](const ShardData& aData)
    {
        const auto sConvertShardResponseToBloomFilter = [&](const std::unordered_set<uint64_t>& aShardData)
        {
            sketch::hlf_t sFilter(hll_count, 1337, hll_bucket_count_log2);

            for (auto sId : aShardData)
            {
                sFilter.addh(sId);
            }

            return sFilter;
        };

        std::vector<sketch::hlf_t> sShardBloomFilters;
        std::transform(aData.data.begin(),
                       aData.data.end(),
                       std::back_inserter(sShardBloomFilters),
                       [&sConvertShardResponseToBloomFilter](const auto& aShardData)
                       { return sConvertShardResponseToBloomFilter(aShardData); });

        Timer         sTimer("HyperLogLogChecker merge shard data");
        sketch::hlf_t sFilter(hll_count, 1337, hll_bucket_count_log2);

        for (const auto& sShard : sShardBloomFilters)
        {
            sFilter += sShard;
        }

        return sFilter;
    };

    Timer sTimer("HyperLogLogPresenceChecker coverage calculation");
    for (const auto& sDep : aShardData)
    {
        filter += sConvertDependencyToBloomFilter(sDep);
    }
}
bool HyperLogLogPresenceChecker::isPresent(uint64_t aId) const
{
    return filter.may_contain(aId);
}
uint64_t HyperLogLogPresenceChecker::estimateMemoryUsage() const
{
    return filter.est_memory_usage();
}

uint64_t HyperLogLogPresenceChecker::estimateCardinality() const
{
    return 0;
}