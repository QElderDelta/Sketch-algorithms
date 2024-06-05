#include "baseline_common.hpp"

#include <algorithm>
#include <numeric>

#include "common.hpp"

void BaselineCommon::addShardData(const std::vector<ShardData>& aShardData, uint32_t aPassCondition)
{
    ids.clear();

    const auto sDeps = mergeShardData(aShardData);
    {
        Timer                 sTimer("Default estimator coverage calculation");
        std::vector<uint64_t> sTmp;
        size_t                sCount = std::accumulate(sDeps.begin(),
                                        sDeps.end(),
                                        0,
                                        [](const auto& acc, const auto& e) { return acc + e.size(); });
        sTmp.reserve(sCount);

        for (const auto& x : sDeps)
            sTmp.insert(sTmp.end(), x.begin(), x.end());

        std::sort(sTmp.begin(), sTmp.end());

        uint64_t sLastId = 0;
        sCount           = 0;
        for (const auto& x : sTmp)
        {
            if (sLastId != x)
            {
                if (sCount >= aPassCondition)
                    ids.insert(sLastId);
                sLastId = x;
                sCount  = 1;
            }
            else
            {
                sCount++;
            }
        }
        if (!sTmp.empty())
        {
            if (sCount >= aPassCondition)
                ids.insert(sLastId);
        }
    }
}

uint64_t BaselineCommon::estimateMemoryUsage() const
{
    return sizeof(uint64_t) * ids.size();
}

const std::set<uint64_t>& BaselineCommon::getIds() const
{
    return ids;
}

std::vector<std::vector<uint64_t>> BaselineCommon::mergeShardData(const std::vector<ShardData>& aShardData) const
{
    std::vector<std::vector<uint64_t>> sResult;
    sResult.reserve(aShardData.size());

    for (const auto& sDependency : aShardData)
    {
        sResult.push_back({});
        Timer sTimer("DefaultEstimator merge sharded data");
        for (const auto& sShard : sDependency.data)
        {
            sResult.back().insert(sResult.back().end(), sShard.begin(), sShard.end());
        }
    }

    return sResult;
}
