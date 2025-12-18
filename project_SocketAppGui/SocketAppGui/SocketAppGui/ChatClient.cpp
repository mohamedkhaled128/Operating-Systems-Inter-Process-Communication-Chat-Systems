#include "ChatClient.h"
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

ChatClient::ChatClient() {}

ChatClient::~ChatClient() {
    disconnect();
}

void ChatClient::setOnMessage(OnMessage cb) {
    onMessage = cb;
}

void ChatClient::setOnDisconnect(OnDisconnect cb) {
    onDisconnect = cb;
}

bool ChatClient::connectTo(const std::string& ip, int port) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return false;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
        return false;

    sockaddr_in srv{};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &srv.sin_addr);

    if (connect(sock, (sockaddr*)&srv, sizeof(srv)) == SOCKET_ERROR) {
        closesocket(sock);
        sock = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    running = true;
    recvThread = std::thread(&ChatClient::recvLoop, this);
    return true;
}

void ChatClient::disconnect() {
    if (!running) return;

    running = false;
    shutdown(sock, SD_BOTH);

    if (recvThread.joinable())
        recvThread.join();

    closesocket(sock);
    sock = INVALID_SOCKET;
    WSACleanup();

    if (onDisconnect)
        onDisconnect();
}

bool ChatClient::send_message_socket(const std::string& payload) {
    uint32_t len = htonl((uint32_t)payload.size());
    if (send(sock, (char*)&len, sizeof(len), 0) != sizeof(len))
        return false;

    int sent = 0;
    while (sent < (int)payload.size()) {
        int n = send(sock,
            payload.data() + sent,
            (int)payload.size() - sent, 0);
        if (n <= 0) return false;
        sent += n;
    }
    return true;
}

bool ChatClient::recv_message_socket(std::string& out) {
    uint32_t lenNet = 0;
    int r = recv(sock, (char*)&lenNet, sizeof(lenNet), 0);
    if (r <= 0) return false;

    uint32_t len = ntohl(lenNet);
    if (len > 20000) return false;

    out.resize(len);
    int received = 0;

    while (received < (int)len) {
        int n = recv(sock, &out[received],
            (int)len - received, 0);
        if (n <= 0) return false;
        received += n;
    }
    return true;
}

void ChatClient::recvLoop() {
    while (running) {
        std::string msg;
        if (!recv_message_socket(msg))
            break;

        if (onMessage)
            onMessage(msg);
    }

    running = false;
    if (onDisconnect)
        onDisconnect();
}

bool ChatClient::sendJson(const std::string& json) {
    if (!running || sock == INVALID_SOCKET)
        return false;

    return send_message_socket(json);
}
