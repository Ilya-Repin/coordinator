#include <hello.hpp>

#include <greeting.hpp>

#include <userver/ydb/exceptions.hpp>

#include <infra/components/coordination/coordination_gateway_component.hpp>
#include <infra/serializer/serializer.hpp>

#include <memory>

namespace coordinator {

Hello::Hello(const userver::components::ComponentConfig& config, const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context)
    , gateway(context.FindComponent<NCoordinator::NInfra::NComponents::TCoordinationGatewayComponent>().GetGateway())
{ }

userver::formats::json::Value Hello::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& ,
    const userver::formats::json::Value& ,
    userver::server::request::RequestContext&) const 
{
    auto result = gateway.GetPartitionMap();

    return NCoordinator::NInfra::SerializePartitionMap(result.value());
}

}  // namespace coordinator