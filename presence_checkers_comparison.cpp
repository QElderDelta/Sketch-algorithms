#include "presence_checkers.hpp"
#include "segment.hpp"

#include <iostream>

int main()
{
    for (uint32_t sSize :
         {1'000'000, 2'000'000, 3'000'000, 4'000'000, 5'000'000, 10'000'000, 20'000'000, 30'000'000, 60'000'000})
    {
        constexpr uint32_t PASS_CONDITION = 1;

        Dependencies sDeps;

        sDeps.size               = sSize * 8 / 10;
        sDeps.shard_count        = 40;
        sDeps.shard_distribution = ShardDataDistribution::RANDOM;

        sDeps.prototypes = {ShardDataPrototype{.operation             = ShardDataPrototype::UNION,
                                               .operation_result_size = sSize,
                                               .response_size         = sSize * 6 / 10}};

        const auto sShardData = getShardDataFromDependencies(sDeps);

        BaselinePresenceChecker sBaseline;
        sBaseline.addShardData(sShardData, PASS_CONDITION);

        std::cout << "Baseline memory usage: " << sBaseline.estimateMemoryUsage() << std::endl;

        for (uint32_t sHllCount = 1; sHllCount <= 20; ++sHllCount)
        {
            constexpr uint32_t BUCKET_COUNT = 20;

            HyperLogLogPresenceChecker sBloomFilter(sHllCount, BUCKET_COUNT);
            sBloomFilter.addShardData(sShardData, PASS_CONDITION);

            uint32_t sFalseNegative  = 0;
            uint32_t sFalsePostitive = 0;

            const auto sNotPresentIds = generateIds(1'000'000, sBaseline.getIds());

            for (auto sId : sNotPresentIds)
            {
                if (sBloomFilter.isPresent(sId))
                {
                    ++sFalsePostitive;
                }
            }

            std::cout << "Deps size, second lvl size, false negative, false "
                         "positive, memory usage: "
                      << sSize << ' ' << sHllCount << ' ' << sFalseNegative << ' ' << sFalsePostitive << ' '
                      << sBloomFilter.estimateMemoryUsage() << std::endl;
        }
    }

    return 0;
}