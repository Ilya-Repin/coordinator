#include "load_factor_predictor_component.hpp"

#include <infra/load_factor_predictor/heuristic_predictor.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

TLoadFactorPredictorComponent::TLoadFactorPredictorComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context)
{
    // TODO other types of predictors
    // auto type = config["type"].As<std::string>();

    Predictor_ = std::make_unique<THeuristicPredictor>();
}

NCore::ILoadFactorPredictor& TLoadFactorPredictorComponent::GetPredictor()
{
    return *Predictor_;
}

userver::yaml_config::Schema TLoadFactorPredictorComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(
      R"(
type: object
description: Component for load factor predictor
additionalProperties: false
properties:
    type:
        type: string
        description: the type of predictor (heuristic, ...)
)");
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
