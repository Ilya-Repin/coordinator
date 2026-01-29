#pragma once

#include "coordination_context.hpp"

namespace NCoordinator::NCore::NDomain {

class ICoordinationRepository {
public:
    virtual TCoordinationContext GetCoordinationContext() const = 0; 
    virtual void SetCoordinationContext(const TCoordinationContext& context) const = 0;
    
    virtual ~ICoordinationRepository() = default;
};

}  // namespace NCoordinator::NCore::NDomain
