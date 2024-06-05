#pragma once

#include "estimators.hpp"
#include "shard_data.hpp"

#include <memory>

struct Dependencies
{
    uint32_t                        size{0};
    ShardDataDistribution           shard_distribution = ShardDataDistribution::EVEN;
    std::vector<ShardDataPrototype> prototypes;
    uint32_t                        shard_count{40};
};

std::vector<ShardData> getShardDataFromDependencies(const Dependencies& aDependencies);