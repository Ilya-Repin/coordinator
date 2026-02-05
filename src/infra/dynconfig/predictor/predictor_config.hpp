#pragma once

#include <core/common/hub_params.hpp>

#include <userver/dynamic_config/snapshot.hpp>
#include <userver/formats/json/value.hpp>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

struct TPredictorSettings {
    NCore::NDomain::TLoadFactor DefaultFirstLoadFactor;
};

inline const userver::dynamic_config::Key<TPredictorSettings> PREDICTOR_CONFIG{
    "PREDICTOR_CONFIG",
    userver::dynamic_config::DefaultAsJsonString{R"(
{
    "default_first_load_factor": 5
}
)"}};

TPredictorSettings Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<TPredictorSettings>);

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
