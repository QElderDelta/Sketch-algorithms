#include "segment.hpp"

std::vector<ShardData> getShardDataFromDependencies(const Dependencies& aDependencies)
{
    std::vector<ShardData> sShardData;
    sShardData.reserve(aDependencies.prototypes.size() + 1);

    const auto sMain
        = generateShardData(aDependencies.size, aDependencies.shard_count, aDependencies.shard_distribution);
    sShardData.push_back(sMain);

    for (const auto& sPrototype : aDependencies.prototypes)
    {
        sShardData.push_back(
            generateShardDataUsingExisting(sShardData.front(), sPrototype, aDependencies.shard_distribution));
    }

    return sShardData;
}