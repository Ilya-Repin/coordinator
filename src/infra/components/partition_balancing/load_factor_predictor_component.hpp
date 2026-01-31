#pragma once

#include <core/partition_balancing/load_factor_predictor.hpp>

#include <userver/components/loggable_component_base.hpp>

#include <memory>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

class TLoadFactorPredictorComponent
    : public userver::components::LoggableComponentBase
{
public:
    static constexpr std::string_view kName = "load-factor-predictor";

    TLoadFactorPredictorComponent(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context);

    NCore::ILoadFactorPredictor& GetPredictor();

    static userver::yaml_config::Schema GetStaticConfigSchema();

private:
    std::unique_ptr<NCore::ILoadFactorPredictor> Predictor_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
