#pragma once

#include <userver/components/component_list.hpp>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

void RegisterUserverComponents(userver::components::ComponentList&);

// Clients
void RegisterYdbComponent(userver::components::ComponentList&);

// Components
void RegisterServiceComponents(userver::components::ComponentList&);

// Handlers
void RegisterHandlers(userver::components::ComponentList& list);

////////////////////////////////////////////////////////////////////////////////

}  // namespace NChat::NInfra::NComponents
