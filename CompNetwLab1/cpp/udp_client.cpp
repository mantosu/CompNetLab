// udp_client.cpp — UDP 通信程序：客户端（发送端）
//
// 功能：
//   1. 创建 UDP 套接字；
//   2. 循环连续发送 N 个数据包（默认 100 个），每个数据包带有序号；
//   3. 发送完毕后关闭套接字。
//
// 用法： udp_client.exe [server_ip] [server_port] [count]
//   默认服务器地址为 127.0.0.1，端口 2000，发送 100 个数据包。
//
// 编译：需要链接 ws2_32.lib（已用 #pragma comment 指定，MinGW 下请加 -lws2_32）。

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_HOST    "127.0.0.1"  // 默认服务器 IP（本机）
#define DEFAULT_PORT    2000         // 默认服务器端口
#define DEFAULT_COUNT   100          // 默认连续发送的数据包个数

// 数据包结构：序号 + 数据内容（与服务器端保持一致）
struct Packet {
    int  seq;        // 数据包序号，从 1 开始
    char data[64];   // 数据内容
};

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET  sock;
    struct sockaddr_in serverAddr;   // 服务器地址
    int ret;

    const char *host  = DEFAULT_HOST;
    int         port  = DEFAULT_PORT;
    int         count = DEFAULT_COUNT;
    if (argc >= 2) host  = argv[1];
    if (argc >= 3) port  = atoi(argv[2]);
    if (argc >= 4) count = atoi(argv[3]);

    // ---------- 1. 初始化 Winsock 库 ----------
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup 失败，错误码：%d\n", WSAGetLastError());
        return 1;
    }

    // ---------- 2. 创建数据报套接字 ----------
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("socket 失败，错误码：%d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // ---------- 3. 填充服务器地址 ----------
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port   = htons(port);

    // 支持 IP 地址和主机名两种形式
    unsigned long ip = inet_addr(host);
    if (ip != INADDR_NONE) {
        serverAddr.sin_addr.s_addr = ip;            // 直接是 IP 地址
    } else {
        struct hostent *hp = gethostbyname(host);   // 主机名，如 "localhost"
        if (hp == NULL) {
            printf("无法解析主机名或 IP 地址：%s\n", host);
            closesocket(sock);
            WSACleanup();
            return 1;
        }
        memcpy(&serverAddr.sin_addr, hp->h_addr_list[0], hp->h_length);
    }

    // ---------- 4. 循环发送数据包 ----------
    // UDP 是无连接的，无需 connect()，直接 sendto() 指定目的地址发送。
    printf("开始向 %s:%d 连续发送 %d 个 UDP 数据包...\n\n", host, port, count);

    for (int i = 1; i <= count; i++) {
        Packet pkt;
        pkt.seq = i;
        sprintf(pkt.data, "hello UDP, packet %d", i);

        ret = sendto(sock, (const char *)&pkt, sizeof(pkt), 0,
                     (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        if (ret == SOCKET_ERROR) {
            printf("sendto 失败，错误码：%d\n", WSAGetLastError());
            break;
        }
        printf("已发送数据包：序号 %d/%d\n", i, count);

        // 如需观察丢包现象：可增大包数、在两台真实主机之间发送，
        // 或在此处加 Sleep() 调整发送速率。
    }

    printf("\n发送结束，共发送 %d 个数据包。\n", count);

    // ---------- 5. 释放资源 ----------
    closesocket(sock);
    WSACleanup();
    return 0;
}
