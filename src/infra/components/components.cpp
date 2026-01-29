#include "components.hpp"

#include <infra/components/coordination/coordination_dist_lock_component.hpp>
#include <infra/components/coordination/coordination_gateway_component.hpp>
#include <infra/components/coordination/coordination_repository_component.hpp>

#include <hello.hpp>

#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/dynamic_config/updater/component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/server_monitor.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/ydb/component.hpp>

namespace NCoordinator::NInfra::NComponents {

void RegisterUserverComponents(userver::components::ComponentList& list)
{
    list.Append<userver::server::handlers::Ping>()
        .Append<userver::components::TestsuiteSupport>()
        .AppendComponentList(userver::clients::http::ComponentList())
        .Append<userver::clients::dns::Component>()
        .Append<userver::server::handlers::TestsControl>()
        .Append<userver::congestion_control::Component>();
}

// Clients
void RegisterYdbComponents(userver::components::ComponentList& list)
{
    list.Append<userver::components::DefaultSecdistProvider>()
        .Append<userver::components::Secdist>()
        .Append<userver::ydb::YdbComponent>();
}

// Infra
void RegisterInfraComponents(userver::components::ComponentList& list)
{
    list.Append<TCoordinationGatewayComponent>()
        .Append<TCoordinationRepositoryComponent>()
        .Append<TCoordinationDistLockComponent>();
}

// Handlers
void RegisterHandlers(userver::components::ComponentList& list)
{
    list.Append<coordinator::Hello>();
}

}  // namespace NCoordinator::NInfra::NComponents
