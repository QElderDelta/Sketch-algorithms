#pragma once

#include "baseline_common.hpp"
#include "shard_data.hpp"

#include <set>

#include <sketch/bf.h>
#include <sketch/hll.h>

class PresenceChecker
{
public:
    virtual void     addShardData(const std::vector<ShardData>& aShardData, uint32_t aPassCondition) = 0;
    virtual bool     isPresent(uint64_t aId) const                                                   = 0;
    virtual uint64_t estimateMemoryUsage() const                                                     = 0;
};

class BaselinePresenceChecker : public PresenceChecker
{
public:
    void     addShardData(const std::vector<ShardData>& aShardData, uint32_t aPassCondition) override;
    uint64_t estimateMemoryUsage() const override;
    bool     isPresent(uint64_t aId) const override;
    const std::set<uint64_t>& getIds() const;

private:
    BaselineCommon presence_checker;
};

class BloomFilterPresenceChecker : public PresenceChecker
{
public:
    BloomFilterPresenceChecker(uint64_t aSecondLevelSize, uint32_t aNumberOfHashFunctions);
    void     addShardData(const std::vector<ShardData>& aShardData, uint32_t aPassCondition);
    bool     isPresent(uint64_t aId) const;
    uint64_t estimateMemoryUsage() const;
    uint64_t estimateCardinality() const;

private:
    uint64_t     second_level_size{0};
    uint32_t     number_of_hash_functions{0};
    sketch::bf_t bloom_filter;
};

class HyperLogLogPresenceChecker : public PresenceChecker
{
public:
    HyperLogLogPresenceChecker(uint64_t aHllCount, uint32_t aHllBucketCountLog2);
    void     addShardData(const std::vector<ShardData>& aShardData, uint32_t aPassCondition);
    bool     isPresent(uint64_t aId) const;
    uint64_t estimateMemoryUsage() const;
    uint64_t estimateCardinality() const;

private:
    uint64_t      hll_count{0};
    uint32_t      hll_bucket_count_log2{0};
    sketch::hlf_t filter;
};