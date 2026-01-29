#pragma once

#include <core/coordination/coordination_repository.hpp>
#include <core/coordination/coordination_context.hpp>

#include <userver/ydb/table.hpp>

namespace NCoordinator::NInfra::NRepository {

////////////////////////////////////////////////////////////////////////////////

class TYdbCoordinationRepository
    : public NCore::NDomain::ICoordinationRepository 
{
public:
    TYdbCoordinationRepository(std::shared_ptr<userver::ydb::TableClient> ydbClient);

    NCore::NDomain::TCoordinationContext GetCoordinationContext() const override;
    void SetCoordinationContext(const NCore::NDomain::TCoordinationContext& context) const override;

private:
    std::shared_ptr<userver::ydb::TableClient> YdbClient_;
    // TODO addd dynconfig
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NRepository
