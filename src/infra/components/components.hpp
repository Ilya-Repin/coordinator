#pragma once

#include <userver/components/component_list.hpp>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

void RegisterUserverComponents(userver::components::ComponentList&);

// Clients
void RegisterYdbComponents(userver::components::ComponentList&);

// Infra
void RegisterInfraComponents(userver::components::ComponentList&);

// Services
void RegisterServices(userver::components::ComponentList&);

// Handlers
void RegisterHandlers(userver::components::ComponentList& list);

////////////////////////////////////////////////////////////////////////////////

}  // namespace NChat::NInfra::NComponents
