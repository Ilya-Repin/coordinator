#pragma once

#include <core/coordination/coordination_repository.hpp>
#include <core/coordination/coordination_context.hpp>

#include <userver/dynamic_config/source.hpp>
#include <userver/ydb/table.hpp>

namespace NCoordinator::NInfra::NRepository {

////////////////////////////////////////////////////////////////////////////////

class TYdbCoordinationRepository
    : public NCore::NDomain::ICoordinationRepository 
{
public:
    TYdbCoordinationRepository(
        std::shared_ptr<userver::ydb::TableClient> ydbClient,
        userver::dynamic_config::Source configSource);

    NCore::NDomain::TCoordinationContext GetCoordinationContext() const override;
    void SetCoordinationContext(const NCore::NDomain::TCoordinationContext& context) const override;

private:
    std::shared_ptr<userver::ydb::TableClient> YdbClient_;
    userver::dynamic_config::Source ConfigSource_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NRepository
