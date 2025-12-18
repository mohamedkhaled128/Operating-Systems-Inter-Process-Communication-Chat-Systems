#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <winsock2.h>

class ChatClient {
public:
    using OnMessage = std::function<void(const std::string&)>;
    using OnDisconnect = std::function<void()>;

    ChatClient();
    ~ChatClient();

    bool connectTo(const std::string& ip, int port);
    void disconnect();
    bool sendJson(const std::string& json);

    void setOnMessage(OnMessage cb);
    void setOnDisconnect(OnDisconnect cb);

private:
    void recvLoop();
    bool send_message_socket(const std::string& payload);
    bool recv_message_socket(std::string& out);

private:
    SOCKET sock = INVALID_SOCKET;
    std::thread recvThread;
    std::atomic_bool running = false;

    OnMessage onMessage;
    OnDisconnect onDisconnect;
};
