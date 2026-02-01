#include "get_context.hpp"

#include <core/coordination/coordination_state.hpp>

#include <userver/logging/log.hpp>

#include <fmt/format.h>

namespace NCoordinator::NApp::NUseCase {

////////////////////////////////////////////////////////////////////////////////

TGetContextUseCase::TGetContextUseCase(NCore::NDomain::ICoordinationRepository& coordinationRepository)
    : CoordinationRepository_(coordinationRepository)
{ }

NDto::TGetContextResponse TGetContextUseCase::Execute() const
{

    NCore::NDomain::TCoordinationContext context;
    
    try {
        context = CoordinationRepository_.GetCoordinationContext();
    } catch (std::exception& ex) {
        throw TGetContextTemporaryUnavailable(fmt::format("Failed to get coordination context: {}", ex.what()));
    }

    NDto::TGetContextResponse response{
        .Context = std::move(context),
    };

    return response;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NUseCase
