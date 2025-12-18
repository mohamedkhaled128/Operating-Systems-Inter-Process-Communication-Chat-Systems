#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct Client {
    SOCKET sock;
    string user;
};

vector<Client> clients;
mutex clients_mtx;

bool send_message(SOCKET sock, const string& payload) {
    uint32_t len = htonl((uint32_t)payload.size());

    if (send(sock, (char*)&len, sizeof(len), 0) <= 0)
        return false;

    int sent = 0;
    while (sent < (int)payload.size()) {
        int n = send(sock,
            payload.data() + sent,
            (int)payload.size() - sent,
            0);
        if (n <= 0) return false;
        sent += n;
    }
    return true;
}

bool recv_message(SOCKET sock, string& out) {
    uint32_t len_net;
    int r = recv(sock, (char*)&len_net, sizeof(len_net), 0);
    if (r <= 0) return false;

    uint32_t len = ntohl(len_net);
    if (len > 10000) return false;

    out.resize(len);
    int received = 0;

    while (received < (int)len) {
        int n = recv(sock, &out[received], len - received, 0);
        if (n <= 0) return false;
        received += n;
    }
    return true;
}

string get_json_value(const string& json, const string& key) {
    string k = "\"" + key + "\":\"";
    size_t start = json.find(k);
    if (start == string::npos) return "";

    start += k.length();
    size_t end = json.find("\"", start);
    if (end == string::npos) return "";

    return json.substr(start, end - start);
}

void broadcast(const string& payload) {
    vector<SOCKET> targets;

    {
        lock_guard<mutex> lock(clients_mtx);
        for (auto& c : clients)
            targets.push_back(c.sock);
    }

    for (SOCKET s : targets) {
        if (!send_message(s, payload)) {
            lock_guard<mutex> lock(clients_mtx);
            for (auto it = clients.begin(); it != clients.end(); ++it) {
                if (it->sock == s) {
                    closesocket(it->sock);
                    clients.erase(it);
                    break;
                }
            }
        }
    }
}

void handle_client(SOCKET sock) {
    string first_msg;

    if (!recv_message(sock, first_msg)) {
        closesocket(sock);
        return;
    }

    string user = get_json_value(first_msg, "user");
    if (user.empty())
        user = "Unknown";

    {
        lock_guard<mutex> lock(clients_mtx);
        clients.push_back({ sock, user });
    }

    cout << "[+] " << user << " joined\n";

    string join_msg =
        "{\"type\":\"chat\",\"user\":\"Server\",\"text\":\"" +
        user + " joined\"}";
    broadcast(join_msg);

    while (true) {
        string payload;
        if (!recv_message(sock, payload))
            break;

        broadcast(payload);

        string text = get_json_value(payload, "text");
        cout << "[MSG] " << user << ": " << text << endl;
    }

    {
        lock_guard<mutex> lock(clients_mtx);
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (it->sock == sock) {
                clients.erase(it);
                break;
            }
        }
    }

    closesocket(sock);
    cout << "[-] " << user << " disconnected\n";

    string left_msg =
        "{\"type\":\"chat\",\"user\":\"Server\",\"text\":\"" +
        user + " left\"}";
    broadcast(left_msg);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    int port = 5000;
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listen_sock, (sockaddr*)&addr, sizeof(addr));
    listen(listen_sock, SOMAXCONN);

    cout << "[SERVER] Listening on port " << port << "...\n";

    while (true) {
        SOCKET client_sock = accept(listen_sock, nullptr, nullptr);
        cout << "[NEW CONNECTION]\n";
        thread(handle_client, client_sock).detach();
    }

    closesocket(listen_sock);
    WSACleanup();
    return 0;
}
