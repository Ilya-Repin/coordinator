#include <hello.hpp>

#include <greeting.hpp>

#include <userver/ydb/exceptions.hpp>

#include <infra/kesus_gateway/kesus_gateway.hpp>
#include <infra/serializer/serializer.hpp>

#include <memory>

namespace coordinator {

Hello::Hello(const userver::components::ComponentConfig& config, const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
        ydb_client_(context.FindComponent<userver::ydb::YdbComponent>().GetCoordinationClient("chat_db"))
        , dist_(context.FindComponent<NCoordinator::NInfra::NComponents::TCoordinationDistLockComponent>())
{   
    gateway = std::make_unique<NCoordinator::NInfra::NGateway::TKesusGateway>(
        ydb_client_, "chat-coordination", "partition-map-lock", false);
}

userver::formats::json::Value Hello::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& ,
    const userver::formats::json::Value& ,
    userver::server::request::RequestContext& 
) const 
{
    auto result = gateway->GetPartitionMap();

    return NCoordinator::NInfra::SerializePartitionMap(result);
}

}  // namespace coordinator