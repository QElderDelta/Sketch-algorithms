#include <gtest/gtest.h>

#include "shard_data.hpp"

#include <array>
#include <numeric>
#include <ranges>

namespace
{
    template <typename DataProvider, typename ResultProvider>
    void EvenDistributionTest(DataProvider &&aDataProvider, ResultProvider &&aResultProvider)
    {
        {
            auto sData = aDataProvider(100, 4, ShardDataDistribution::EVEN);

            for (const auto &sPart : sData)
            {
                EXPECT_EQ(aResultProvider(sPart), 25);
            }
        }

        {
            auto sData = aDataProvider(98, 4, ShardDataDistribution::EVEN);

            for (auto sPart : std::ranges::subrange(sData.begin(), std::prev(sData.end())))
            {
                EXPECT_EQ(aResultProvider(sPart), 25);
            }

            EXPECT_EQ(aResultProvider(sData.back()), 23);
        }
    }

    template <typename DataProvider, typename Reducer>
    void RandomDistributionTest(DataProvider &&aDataProvider, Reducer &&aReducer)
    {
        constexpr auto SIZES        = std::array{1, 10, 100, 10000, 777, 322, 1337, 420};
        constexpr auto SHARDS_COUNT = std::array{2, 3, 4, 5, 10, 20, 40};

        for (auto sSize : SIZES)
        {
            for (auto sShardCount : SHARDS_COUNT)
            {
                auto sGeneratedData = aDataProvider(sSize, sShardCount, ShardDataDistribution::RANDOM);
                EXPECT_EQ(std::accumulate(sGeneratedData.begin(),
                                          sGeneratedData.end(),
                                          0,
                                          std::forward<Reducer>(aReducer)),
                          sSize);
            }
        }
    }

    void ShardDataPrototypeTest(const ShardData   &aExisting,
                                const ShardData   &aGenerated,
                                ShardDataPrototype aPrototype)
    {
        ASSERT_EQ(aGenerated.data.size(), aExisting.data.size());
        ASSERT_EQ(aGenerated.total_size, aPrototype.response_size);

        std::unordered_set<uint64_t> sExistingIds;

        for (const auto &sShard : aExisting.data)
        {
            sExistingIds.insert(sShard.begin(), sShard.end());
        }

        bool     sIsIntersection = aPrototype.operation == ShardDataPrototype::INTERSECTION;
        uint32_t sOperationSize  = sIsIntersection ? 0 : aExisting.total_size;
        for (const auto &sShard : aGenerated.data)
        {
            for (auto sId : sShard)
            {
                if (sExistingIds.find(sId) != sExistingIds.end())
                {
                    if (sIsIntersection)
                    {
                        ++sOperationSize;
                    }
                }
                else
                {
                    if (!sIsIntersection)
                    {
                        ++sOperationSize;
                    }
                }
            }
        }

        ASSERT_EQ(sOperationSize, aPrototype.operation_result_size);
    }
}  // namespace

TEST(DistributionTest, EvenDistribution)
{
    EvenDistributionTest(details::generateShardResponseSizes, [](auto aSize) { return aSize; });
}

TEST(DistributionTest, RandomDistribution)
{
    RandomDistributionTest(details::generateShardResponseSizes,
                           [](const auto &aResult, const auto &aCurr) { return aResult + aCurr; });
}

TEST(ShardDataTest, EvenDistribution)
{
    EvenDistributionTest(
        [](uint32_t aResponseSize, uint32_t aShardCount, ShardDataDistribution aDistributionType)
        { return generateShardData(aResponseSize, aShardCount, aDistributionType).data; },
        [](const auto &aData) { return aData.size(); });
}

TEST(ShardDataTest, RandomDistribution)
{
    RandomDistributionTest(
        [](uint32_t aResponseSize, uint32_t aShardCount, ShardDataDistribution aDistributionType)
        { return generateShardData(aResponseSize, aShardCount, aDistributionType).data; },
        [](const auto &aResult, const auto &aCurr) { return aResult + aCurr.size(); });
}

TEST(ShardDataPrototypeTests, Intersection)
{
    const auto sExisting = generateShardData(100, 4, ShardDataDistribution::EVEN);
    {
        const ShardDataPrototype sPrototype{ShardDataPrototype::INTERSECTION, 100, 100};
        ShardDataPrototypeTest(
            sExisting,
            generateShardDataUsingExisting(sExisting, sPrototype, ShardDataDistribution::EVEN),
            sPrototype);
    }
    {
        const ShardDataPrototype sPrototype{ShardDataPrototype::INTERSECTION, 30, 100};
        ShardDataPrototypeTest(
            sExisting,
            generateShardDataUsingExisting(sExisting, sPrototype, ShardDataDistribution::EVEN),
            sPrototype);
    }
    {
        const ShardDataPrototype sPrototype{ShardDataPrototype::INTERSECTION, 0, 100};
        ShardDataPrototypeTest(
            sExisting,
            generateShardDataUsingExisting(sExisting, sPrototype, ShardDataDistribution::EVEN),
            sPrototype);
    }
}

TEST(ShardDataPrototypeTests, Union)
{
    const auto sExisting = generateShardData(100, 4, ShardDataDistribution::EVEN);
    {
        const ShardDataPrototype sPrototype{ShardDataPrototype::UNION, 100, 100};
        ShardDataPrototypeTest(
            sExisting,
            generateShardDataUsingExisting(sExisting, sPrototype, ShardDataDistribution::EVEN),
            sPrototype);
    }
    {
        const ShardDataPrototype sPrototype{ShardDataPrototype::UNION, 100, 20};
        ShardDataPrototypeTest(
            sExisting,
            generateShardDataUsingExisting(sExisting, sPrototype, ShardDataDistribution::EVEN),
            sPrototype);
    }
    {
        const ShardDataPrototype sPrototype{ShardDataPrototype::UNION, 120, 40};
        ShardDataPrototypeTest(
            sExisting,
            generateShardDataUsingExisting(sExisting, sPrototype, ShardDataDistribution::EVEN),
            sPrototype);
    }
    {
        const ShardDataPrototype sPrototype{ShardDataPrototype::UNION, 150, 120};
        ShardDataPrototypeTest(
            sExisting,
            generateShardDataUsingExisting(sExisting, sPrototype, ShardDataDistribution::EVEN),
            sPrototype);
    }
}