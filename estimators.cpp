#include "estimators.hpp"

#include <chrono>
#include <iostream>
#include <numeric>

uint64_t BaselineEstimator::estimateCoverage() const
{
    return getIds().size();
}

uint64_t BaselineEstimator::estimateMemoryUsage() const
{
    return estimator.estimateMemoryUsage();
}

void BaselineEstimator::addShardData(const std::vector<ShardData>& aShardData, uint32_t aPassCondition)
{
    estimator.addShardData(aShardData, aPassCondition);
}

const std::set<uint64_t>& BaselineEstimator::getIds() const
{
    return estimator.getIds();
}

RangeMinHashEstimator::RangeMinHashEstimator(uint64_t aSketchSize)
: sketch_size(aSketchSize), min_hash(sketch_size)
{}

RangeMinHashEstimator::InternalStateType RangeMinHashEstimator::constructDefault() const
{
    return InternalStateType(sketch_size);
}

RangeMinHashEstimator::InternalStateType& RangeMinHashEstimator::getInternalState()
{
    return min_hash;
}

const RangeMinHashEstimator::InternalStateType& RangeMinHashEstimator::getInternalState() const
{
    return min_hash;
}

uint64_t RangeMinHashEstimator::estimateMemoryUsageImpl() const
{
    return sketch_size * sizeof(uint64_t);
}

BBitMinHashEstimator::BBitMinHashEstimator(uint64_t aHashBitCount)
: hash_bit_count(aHashBitCount), min_hash(hash_bit_count)
{}

BBitMinHashEstimator::InternalStateType BBitMinHashEstimator::constructDefault() const
{
    return InternalStateType(hash_bit_count);
}

BBitMinHashEstimator::InternalStateType& BBitMinHashEstimator::getInternalState()
{
    return min_hash;
}

const BBitMinHashEstimator::InternalStateType& BBitMinHashEstimator::getInternalState() const
{
    return min_hash;
}

uint64_t BBitMinHashEstimator::estimateMemoryUsageImpl() const
{
    return min_hash.size() * sizeof(uint64_t);
}

HyperLogLogEstimator::HyperLogLogEstimator(uint64_t sBucketCountLog2)
: bucket_count_log2(sBucketCountLog2), hyper_log_log(bucket_count_log2, sketch::hll::ORIGINAL)
{}

HyperLogLogEstimator::InternalStateType HyperLogLogEstimator::constructDefault() const
{
    return sketch::hll_t(bucket_count_log2, sketch::hll::ORIGINAL);
}

HyperLogLogEstimator::InternalStateType& HyperLogLogEstimator::getInternalState()
{
    return hyper_log_log;
}
const HyperLogLogEstimator::InternalStateType& HyperLogLogEstimator::getInternalState() const
{
    return hyper_log_log;
}

uint64_t HyperLogLogEstimator::estimateMemoryUsageImpl() const
{
    auto sRes = hyper_log_log.est_memory_usage();
    return sRes.first + sRes.second;
}