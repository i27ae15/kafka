#include <string>
#include <cstdint>

#include <core/exceptions.h>
#include <utils.h>


namespace ServerException {

    BaseException::BaseException(const std::string& msg) : msg {msg + "\x0A"} {
        PRINT_ERROR(msg);
    }

    const char* BaseException::what() const noexcept {
        return msg.c_str();
    }

    SocketConnFailed::SocketConnFailed() : BaseException("Failed to create server socket") {}
    SetSockOptConnFailed::SetSockOptConnFailed() : BaseException("Setsockopt failed") {}

    BindToPortFailed::BindToPortFailed(std::string port) :
        BaseException("Failed to bin to port " + port) {}

    BindToPortFailed::BindToPortFailed(uint16_t port) :
        BaseException("Failed to bin to port " + std::to_string(port)) {}

    ListenFailed::ListenFailed() : BaseException("Listen failed") {}

    ApiNotSupported::ApiNotSupported() : BaseException("API not supported") {}

}