#pragma once
#include "common/network/NetworkRequest.hpp"
#include "common/network/NetworkResult.hpp"
#include "controllers/plugins/LuaUtilities.hpp"
#include "controllers/plugins/PluginController.hpp"

#include <memory>
#ifdef CHATTERINO_HAVE_PLUGINS

namespace chatterino::lua::api {
// NOLINTBEGIN(readability-identifier-naming)

/*
 * @lua@class HTTPRequest
 */
class HTTPRequest : public std::enable_shared_from_this<HTTPRequest>
{
    // This type is private to prevent the accidental construction of HTTPRequest without a shared pointer
    struct ConstructorAccessTag {
    };

public:
    HTTPRequest(HTTPRequest::ConstructorAccessTag, NetworkRequest req);

private:
    NetworkRequest req_;

    static void createMetatable(lua_State *L);
    friend class chatterino::PluginController;

    /**
     * @brief Get the content of the top object on Lua stack, usually the first argument as an HTTPRequest
     *
     * If the object given is not a userdatum or the pointer inside that
     * userdatum doesn't point to a HTTPRequest, a lua error is thrown.
     */
    static std::shared_ptr<HTTPRequest> getOrError(lua_State *L,
                                                   StackIdx where = -1);
    /**
     * Pushes the private table onto the lua stack
     */
    StackIdx pushPrivate(lua_State *L);

    QString privateKey;
    int timeout_ = 10'000;
    bool done = false;

public:
    /**
     * Sets the success callback
     *
     * @lua@param callback HTTPCallback Function to call when the HTTP request succeeds
     * @exposed HTTPRequest:on_success
     */
    static int on_success_wrap(lua_State *L);
    int on_success(lua_State *L);

    /**
     * Sets the failure callback
     *
     * @lua@param callback HTTPCallback Function to call when the HTTP request fails or returns a non-ok status
     * @exposed HTTPRequest:on_error
     */
    static int on_error_wrap(lua_State *L);
    int on_error(lua_State *L);

    /**
     * Sets the finally callback
     *
     * @lua@param callback HTTPCallback Function to call when the HTTP request finishes
     * @exposed HTTPRequest:finally
     */
    static int finally_wrap(lua_State *L);
    int finally(lua_State *L);

    /**
     * Executes the HTTP request
     *
     * @exposed HTTPRequest:execute
     */
    static int execute_wrap(lua_State *L);
    int execute(lua_State *L);

    /**
     * Static functions
     */

    /**
     * Creates a new HTTPRequest
     *
     * @lua@param method HTTPMethod Method to use
     * @lua@param url string Where to send the request to
     */
    static int create(lua_State *L);
};

// NOLINTEND(readability-identifier-naming)
}  // namespace chatterino::lua::api

namespace chatterino::lua {
StackIdx push(lua_State *L, std::shared_ptr<api::HTTPRequest> request);
StackIdx push(lua_State *L, const NetworkResult &result);
}  // namespace chatterino::lua

#endif
