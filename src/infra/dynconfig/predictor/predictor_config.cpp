#include "predictor_config.hpp"

#include <core/common/hub_params.hpp>

#include <userver/dynamic_config/value.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/ydb/settings.hpp>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

TPredictorSettings Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<TPredictorSettings>)
{
    const auto defaultFirstLoadFactor = NCore::NDomain::TLoadFactor{
        value["default_first_load_factor"].As<NCore::NDomain::TLoadFactor::UnderlyingType>()
    };

    return TPredictorSettings{
        .DefaultFirstLoadFactor = defaultFirstLoadFactor,
    };
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
