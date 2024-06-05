#include "estimators.hpp"
#include "segment.hpp"

#include <iostream>

int main()
{
    for (uint32_t sSize :
         {1'000'000, 2'000'000, 3'000'000, 4'000'000, 5'000'000, 10'000'000, 20'000'000, 30'000'000, 60'000'000})
    {
        constexpr uint32_t PASS_CONDITION = 1;

        Dependencies sDeps;

        sDeps.size               = sSize / 4;
        sDeps.shard_count        = 40;
        sDeps.shard_distribution = ShardDataDistribution::RANDOM;

        sDeps.prototypes = {ShardDataPrototype{.operation             = ShardDataPrototype::UNION,
                                               .operation_result_size = sSize / 2,
                                               .response_size         = sSize / 4},
                            ShardDataPrototype{.operation             = ShardDataPrototype::UNION,
                                               .operation_result_size = sSize / 2,
                                               .response_size         = sSize / 4},
                            ShardDataPrototype{.operation             = ShardDataPrototype::UNION,
                                               .operation_result_size = sSize / 2,
                                               .response_size         = sSize / 4}};

        const auto sShardData = getShardDataFromDependencies(sDeps);

        BaselineEstimator sBaseline;
        sBaseline.addShardData(sShardData, PASS_CONDITION);

        std::cout << "Baseline memory usage: " << sBaseline.estimateMemoryUsage() << std::endl;

        for (uint64_t sBucketCountLog2 = 10; sBucketCountLog2 <= 20; ++sBucketCountLog2)
        {
            HyperLogLogEstimator sEstimator(sBucketCountLog2);
            sEstimator.addShardData(sShardData, PASS_CONDITION);

            std::cout << "Bucket count, actual size, estimated size, error in %, memory usage: "
                      << sBucketCountLog2 << ' ' << sBaseline.estimateCoverage() << ' '
                      << sEstimator.estimateCoverage() << ' '
                      << std::fabs(static_cast<double>(sBaseline.estimateCoverage())
                                   - static_cast<double>(sEstimator.estimateCoverage()))
                             / sBaseline.estimateCoverage() * 100
                      << ' ' << sEstimator.estimateMemoryUsage() << std::endl;
        }
    }

    return 0;
}