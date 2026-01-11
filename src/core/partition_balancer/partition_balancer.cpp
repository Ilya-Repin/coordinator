#include "partition_balancer.hpp"

namespace {

////////////////////////////////////////////////////////////////////////////////

inline constexpr std::size_t MAX_TOLERABLE_LAG = 1; // TODO подумать может все таки 0 или добавить budget на однократные лаги?

////////////////////////////////////////////////////////////////////////////////

std::size_t CalculateCV(const std::vector<NCoordinator::NCore::NDomain::TLoadFactor>& loadFactors) { // TODO вынести в utils math и написать тесты
    if (loadFactors.empty()) {
        return 0;
    }

    long double sum = 0.0L;
    for (const auto& lf : loadFactors) {
        sum += static_cast<long double>(lf.GetUnderlying());
    }

    const long double mean = sum / loadFactors.size();
    if (mean == 0.0L) {
        return 0;
    }

    long double variance = 0.0L;
    for (const auto& lf : loadFactors) {
        const long double diff =
            static_cast<long double>(lf.GetUnderlying()) - mean;
        variance += diff * diff;
    }
    variance /= loadFactors.size();

    const long double stddev = std::sqrt(variance);

    const long double cv = stddev / mean;

    return static_cast<std::size_t>(std::llround(cv));
}

////////////////////////////////////////////////////////////////////////////////

} // anonymous namespace

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

bool TPartitionBalancer::GetIsLeader() const
{
    return IsLeader_;
}

NDomain::TEpoch TPartitionBalancer::GetCoordinationEpoch() const
{
    return CoordinationEpoch_;
}

NDomain::TPartitionMap TPartitionBalancer::GetPartitionMap() const
{
    NDomain::TPartitionMap result;
    result.Epoch = CoordinationEpoch_;
    result.Partitions.reserve(HashRing_.GetAllPartitions().size());

    for (const auto& [hub, partitions] : PartitionsByHub_) {
        for (const auto& id : partitions) {
            result.Partitions.emplace_back(id, hub);
        }
    }

    std::sort(result.Partitions.begin(), result.Partitions.end(), 
        [](const NDomain::TPartition& a, const NDomain::TPartition& b) {
            return a.Id < b.Id;
        });

    return result;
}

const TPartitionBalancer::TCoordinationState& TPartitionBalancer::GetCoordinationState() const
{
    return CoordinationState_;
}

void TPartitionBalancer::Rebalance(const TClusterSnapshot& snapshot, const TBalancingSettings& settings)
{
    if (!HashRing_.IsInitialized()) {
        throw std::runtime_error("Partitions are not initialized!");
        // if (!CoordinationState_.StatByPartition.empty()) {
        //     std::vector<NDomain::TPartitionId> partitions;
        //     partitions.reserve(CoordinationState_.StatByPartition.size());
        //     for (const auto& [id, state] : CoordinationState_.StatByPartition) {
        //         partitions.emplace_back(id);
        //         CoordinationEpoch_ = std::max(CoordinationEpoch_, state.ObservedAt);
        //     }
        //     HashRing_.LoadPartitions(partitions);
        // } else {
        //     HashRing_.InitializeWithCount(10); // TODO replace with default from config
        // }    
    }

    NDomain::TEpoch previousEpoch{(CoordinationEpoch_.GetUnderlying())++};

    std::unordered_set<NDomain::TPartitionId> orphanedPartitions{
        HashRing_.GetAllPartitions().begin(),
        HashRing_.GetAllPartitions().end(),
    };

    for (auto it = PartitionsByHub_.begin(); it != PartitionsByHub_.end(); ) {
        const auto& endpoint = it->first;

        if (snapshot.find(endpoint) == snapshot.end()) {
            it = PartitionsByHub_.erase(it);
        } else {
            for (const auto& partition : it->second) {
                orphanedPartitions.erase(partition);
            }
            ++it;
        }
    }

    std::vector<NDomain::TLoadFactor> loadFactors;
    std::unordered_set<NDomain::THubEndpoint> activeHubs;
    std::unordered_set<NDomain::THubEndpoint> overloadedHubs;

    for (const auto& [endpoint, report] : snapshot) {
        auto& state = CoordinationState_.StateByHub[endpoint];
    
        state.Endpoint = endpoint;
        state.ObservedAt = CoordinationEpoch_;
        state.LaggedEpochs = (previousEpoch.GetUnderlying() - report.AppliedEpoch.GetUnderlying() > MAX_TOLERABLE_LAG
            ? 0
            : state.LaggedEpochs + 1);
        state.Status = DetermineHubStatus(state, report, settings);

        switch (state.Status) {
            case NDomain::EHubStatus::HEALTHY:
            case NDomain::EHubStatus::OVERLOADED:
                activeHubs.insert(state.Endpoint);
                loadFactors.push_back(report.LoadFactor);
                break; 
            case NDomain::EHubStatus::DRAINING:
            case NDomain::EHubStatus::LAGGED:
            case NDomain::EHubStatus::UNKNOWN:
                DrainHub(state.Endpoint, orphanedPartitions);
                break;
        };
    }

    const std::size_t cv = CalculateCV(loadFactors);

    if (!orphanedPartitions.empty() ||  !overloadedHubs.empty() || cv > settings.MinBalancingCV ) {
        BalancePartitions(orphanedPartitions);
    }
}

void TPartitionBalancer::SetIsLeader(const bool isLeader)
{
    IsLeader_ = isLeader;
}

void TPartitionBalancer::SetEpoch(const NDomain::TEpoch epoch)
{
    CoordinationEpoch_ = epoch;
}

void TPartitionBalancer::InitializePartitions(const std::vector<NDomain::TPartitionId>& partitions)
{
    HashRing_.LoadPartitions(partitions);
}

void TPartitionBalancer::InitializePartitions(const std::size_t partitionsCount)
{
    HashRing_.InitializeWithCount(partitionsCount);
}

void TPartitionBalancer::ApplyPartitionMap(const NDomain::TPartitionMap& partitionMap)
{
    PartitionsByHub_.clear();
    CoordinationEpoch_ = partitionMap.Epoch;

    std::vector<NDomain::TPartitionId> partitions;
    partitions.reserve(partitionMap.Partitions.size());
    for (const auto& partition : partitionMap.Partitions) {
        partitions.emplace_back(partition.Id);
        PartitionsByHub_[partition.Hub].emplace(partition.Id);
    }
    HashRing_.LoadPartitions(partitions);
}

void TPartitionBalancer::ApplyPartitionStates(const TPartitionStates& stat)
{
    CoordinationState_.StateByPartition = stat;
}

void TPartitionBalancer::ApplyHubStates(const THubStates& stat)
{
    CoordinationState_.StateByHub = stat;
}

NDomain::EHubStatus TPartitionBalancer::DetermineHubStatus(
    const NDomain::THubState& hubState,
    const NDomain::THubReport& report,
    const TBalancingSettings& settings) const
{
    if (settings.BlockedDCs.contains(report.DC) || settings.BlockedHubs.contains(report.Endpoint)) {
        return NDomain::EHubStatus::DRAINING;
    }
    if (hubState.LaggedEpochs != 0) {
        return NDomain::EHubStatus::LAGGED;
    }
    if (report.LoadFactor >= settings.OverloadThreshold) {
        return NDomain::EHubStatus::OVERLOADED;
    }
    return NDomain::EHubStatus::HEALTHY;
}

void TPartitionBalancer::DrainHub(
    const NDomain::THubEndpoint& hub,
    std::unordered_set<NDomain::TPartitionId>& orphanedPartitions)
{
    for (const auto& partition : PartitionsByHub_.at(hub)) {
        orphanedPartitions.insert(partition);
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
