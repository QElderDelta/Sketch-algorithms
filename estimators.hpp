#pragma once

#include "baseline_common.hpp"
#include "common.hpp"
#include "shard_data.hpp"

#include <optional>
#include <set>

#include <sketch/bbmh.h>
#include <sketch/hll.h>
#include <sketch/mh.h>

class CoverageEstimator
{
public:
    virtual uint64_t estimateCoverage() const                                                        = 0;
    virtual void     addShardData(const std::vector<ShardData>& aShardData, uint32_t aPassCondition) = 0;
    virtual uint64_t estimateMemoryUsage() const                                                     = 0;
};

class BaselineEstimator : public CoverageEstimator
{
public:
    uint64_t estimateCoverage() const override;
    void     addShardData(const std::vector<ShardData>& aShardData, uint32_t aPassCondition) override;
    uint64_t estimateMemoryUsage() const override;
    const std::set<uint64_t>& getIds() const;

private:
    BaselineCommon estimator;
};

template <typename CustomEstimatorType>
class CustomEstimatorBase : public CoverageEstimator
{
public:
    uint64_t estimateCoverage() const override
    {
        if (intersection_size.has_value())
        {
            return *intersection_size;
        }

        return getDerived()->getInternalState().cardinality_estimate();
    }

    void addShardData(const std::vector<ShardData>& aShardData, uint32_t aPassCondition) override
    {
        const auto sConvertDependencyToInternalState = [this](const ShardData& aData)
        {
            const auto sConvertShardResponseToInternalState
                = [&](const std::unordered_set<uint64_t>& aShardData)
            {
                typename CustomEstimatorType::InternalStateType sInternalState
                    = getDerived()->constructDefault();

                for (auto sId : aShardData)
                {
                    sInternalState.addh(sId);
                }

                return sInternalState;
            };

            std::vector<typename CustomEstimatorType::InternalStateType> sShardInternalStates;
            std::transform(aData.data.begin(),
                           aData.data.end(),
                           std::back_inserter(sShardInternalStates),
                           [&sConvertShardResponseToInternalState](const auto& aShardData)
                           { return sConvertShardResponseToInternalState(aShardData); });

            Timer sTimer(CustomEstimatorType::Name + " merge shard data");
            typename CustomEstimatorType::InternalStateType sInternalState = sShardInternalStates[0];

            for (size_t i = 1; i < sShardInternalStates.size(); ++i)
            {
                sInternalState += sShardInternalStates[i];
            }

            return sInternalState;
        };

        Timer sTimer(CustomEstimatorType::Name + " coverage calculation");
        if (aPassCondition == 2 && aShardData.size() == 2)
        {
            const auto sFirstConverted = sConvertDependencyToInternalState(aShardData[0]);
            const auto sSecondCoverted = sConvertDependencyToInternalState(aShardData[1]);
            intersection_size          = sFirstConverted.cardinality_estimate()
                              + sSecondCoverted.cardinality_estimate()
                              - (sFirstConverted + sSecondCoverted).cardinality_estimate();
        }
        else
        {
            for (const auto& sDep : aShardData)
            {
                getDerived()->getInternalState() += sConvertDependencyToInternalState(sDep);
            }
        }
    }

    uint64_t estimateMemoryUsage() const { return getDerived()->estimateMemoryUsageImpl(); }

private:
    CustomEstimatorType* getDerived() { return static_cast<CustomEstimatorType*>(this); }

    const CustomEstimatorType* getDerived() const { return static_cast<const CustomEstimatorType*>(this); }

    std::optional<uint64_t> intersection_size;
};

class RangeMinHashEstimator : public CustomEstimatorBase<RangeMinHashEstimator>
{
public:
    using InternalStateType              = sketch::RangeMinHash<uint64_t>;
    inline static const std::string Name = "RangeMinHashEstimator";

    RangeMinHashEstimator(uint64_t aSketchSize);

    InternalStateType        constructDefault() const;
    InternalStateType&       getInternalState();
    const InternalStateType& getInternalState() const;
    uint64_t                 estimateMemoryUsageImpl() const;

private:
    uint64_t                       sketch_size{0};
    sketch::RangeMinHash<uint64_t> min_hash;
};

class BBitMinHashEstimator : public CustomEstimatorBase<BBitMinHashEstimator>
{
public:
    using InternalStateType              = sketch::BBitMinHasher<uint64_t>;
    inline static const std::string Name = "BBitMinHashEstimator";

    BBitMinHashEstimator(uint64_t aHashBitCount);

    InternalStateType        constructDefault() const;
    InternalStateType&       getInternalState();
    const InternalStateType& getInternalState() const;
    uint64_t                 estimateMemoryUsageImpl() const;

private:
    uint64_t                        hash_bit_count{0};
    sketch::BBitMinHasher<uint64_t> min_hash;
};

class HyperLogLogEstimator : public CustomEstimatorBase<HyperLogLogEstimator>
{
public:
    using InternalStateType              = sketch::hll_t;
    inline static const std::string Name = "HyperLogLogEstimator";

    HyperLogLogEstimator(uint64_t sBucketCountLog2);

    InternalStateType        constructDefault() const;
    InternalStateType&       getInternalState();
    const InternalStateType& getInternalState() const;
    uint64_t                 estimateMemoryUsageImpl() const;

private:
    uint64_t      bucket_count_log2{0};
    sketch::hll_t hyper_log_log;
};