#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h> 
#include <ws2tcpip.h>
#include <cstring>
enum NetEvent {
    NET_NONE, // 没有新消息到达
    NET_STEP, // 收到对方落子（配合 r、c 两个参数使用）
    NET_RESTART, // 收到对方"重新开始"请求
    NET_QUIT, // 收到对方"退出"通知
    NET_DISCONNECT, // 连接断开（对方关闭程序 / 网络错误 / 连接失败）
    NET_CONNECTED // 客机连上主机成功（仅客机轮询连接状态时返回）
};
class Network {
    static const int PORT = 12345;

public:
    ~Network() {
        if (m_listen != INVALID_SOCKET) {
            closesocket(m_listen);
        }
        if (m_client != INVALID_SOCKET) {
            closesocket(m_client);
        }
        if (wsaStarted) {
            WSACleanup();
        }
    }
    bool startHost() {
        if (!ensureWSAStartup()) {
            return false;
        }
        is_host = true;
        m_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_listen == INVALID_SOCKET) {
            return false;
        }

        int opt = 1;
        setsockopt(m_listen, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        addr.sin_addr.S_un.S_addr = INADDR_ANY;

        if (bind(m_listen, (sockaddr*)&addr,sizeof(addr)) == SOCKET_ERROR) {
            closesocket(m_listen);
            return false;
        }
        if (listen(m_listen, 1) == SOCKET_ERROR) {
            closesocket(m_listen);
            return false;
        }

        setNonBlock(m_listen);
        return true;
    }
    bool connectTo(const char* ip)
    {
        if (!ensureWSAStartup()) {
            return false;
        }
        is_host = false;
        m_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_client == INVALID_SOCKET) {
            return false;
        }
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        inet_pton(AF_INET, ip, &addr.sin_addr);
        if (connect(m_client, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            closesocket(m_client);
            return false;
        }
        return true;
    }
    bool isHost() const {
        return is_host;
    }
    bool isConnected() const {
        return is_connected;
    }
    bool pollAccept() {
        if (!is_host || m_listen == INVALID_SOCKET) {
            return false;
        }
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_listen, &readfds);
        timeval timeout = { 0, 0 };
        int ret = select(0, &readfds, nullptr, nullptr, &timeout);
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            return false;
        }
        if (!FD_ISSET(m_listen, &readfds)) {
            return false;
        }
        m_client = accept(m_listen, nullptr, nullptr);
        if (m_client == INVALID_SOCKET) {
            return false;
        }
        setNonBlock(m_client);
        is_connected = true;
        return true;
    }
    int pollConnect() {
        if (is_connected) {
            return NET_NONE;
        }
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(m_client, &writefds);
        timeval timeout = { 0, 0 };
        select(0, nullptr, &writefds, nullptr, &timeout);
        int error = 0;
        int len = sizeof(error);
        getsockopt(m_client, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
        if (error == 0) {
            is_connected = true;
            return NET_CONNECTED;
        } else if (error == WSAEWOULDBLOCK) {
            return NET_NONE;
        } else {
            closesocket(m_client);
            m_client = INVALID_SOCKET;
            return NET_DISCONNECT;
        }
    }
    void close() {
        if (m_client != INVALID_SOCKET) {
            shutdown(m_client, SD_BOTH); 
            closesocket(m_client);
            m_client = INVALID_SOCKET;
        }
        if (m_listen != INVALID_SOCKET) {
            closesocket(m_listen);
            m_listen = INVALID_SOCKET;
        }
        is_connected = false;
        sendHead = 0;
        sendTail = 0;
        if (wsaStarted) {
            WSACleanup();
            wsaStarted = false;
        }
    }
    void enquene(char cmd, int r, int c) {
        if ((sendTail + 1) % 32 == sendHead) {
            return;
        }
        sendBuffer[sendTail][0] = cmd;
        sendBuffer[sendTail][1] = static_cast<char>(r);
        sendBuffer[sendTail][2] = static_cast<char>(c);
        sendTail = (sendTail + 1) % 32;
    }
    void sendRestart() {
        enquene('R', 0, 0);
    }
    void sendQuit() {
        enquene('Q', 0, 0);
    }
    void flushSend() {
        while (sendHead != sendTail) {
            int bytesSent = send(m_client, sendBuffer[sendHead], 3, 0);
            if (bytesSent == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    break;
                } else {
                    sendHead = sendTail;
                    is_connected = false;
                    break;
                }
            }
            sendHead = (sendHead + 1) % 32;
        }
    }
    NetEvent pollReceive(int& r, int& c) {
        if (!isConnected()) {
            return NET_NONE;
        }
        flushSend();
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_client, &readfds);
        timeval timeout = { 0, 0 };
        select(0, &readfds, nullptr, nullptr, &timeout);
        if (!FD_ISSET(m_client, &readfds)) {
            return NET_NONE;
        }
        int bytesRead = recv(m_client, recvBuffer + recvBufferLen, sizeof(recvBuffer) - recvBufferLen, 0);
        if (bytesRead == 0) {
            is_connected = false;
            return NET_DISCONNECT;
        }
        if (bytesRead == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) {
                return NET_NONE;
            } else {
                is_connected = false;
                return NET_DISCONNECT;
            }
        }
        recvBufferLen += bytesRead;
        while (recvBufferLen >= 3) {
            char cmd = recvBuffer[0];
            int row = static_cast<int>(recvBuffer[1]);
            int col = static_cast<int>(recvBuffer[2]);
            memmove(recvBuffer, recvBuffer + 3, recvBufferLen - 3);
            recvBufferLen -= 3;
            if (cmd == 'S') {
                r = row;
                c = col;
                return NET_STEP;
            } else if (cmd == 'R') {
                return NET_RESTART;
            } else if (cmd == 'Q') {
                return NET_QUIT;
            }
        }
        return NET_NONE;
    }
    void sendStep(int r, int c) {
        enquene('S', r, c);
    }

private:
    
    void setNonBlock(SOCKET sock) {
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);
    }
    bool ensureWSAStartup() {
        if (wsaStarted) {
            return true;
        }
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return false;
        }
        wsaStarted = true;
        return true;
    }
    bool wsaStarted = false;
    bool is_host = false;
    SOCKET m_listen = INVALID_SOCKET;
    SOCKET m_client = INVALID_SOCKET;
    bool is_connected = false;
    char recvBuffer[128];
    int recvBufferLen = 0;
    char sendBuffer[32][3];
    int sendHead = 0;
    int sendTail = 0;
};