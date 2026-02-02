#pragma once

#include <userver/server/handlers/exceptions.hpp>

namespace NCoordinator::NApi::NHandlers {

////////////////////////////////////////////////////////////////////////////////

userver::formats::json::Value MakeError(std::string_view error);

userver::formats::json::Value MakeServerError(std::optional<std::string_view> message = std::nullopt);

// HTTP 500
class TServerException
    : public userver::server::handlers::ExceptionWithCode<userver::server::handlers::HandlerErrorCode::kServerSideError>
{
public:
    TServerException(std::string_view msg);
};

////////////////////////////////////////////////////////////////////////////////

}  // namespace NCoordinator::NApi::NHandlers
