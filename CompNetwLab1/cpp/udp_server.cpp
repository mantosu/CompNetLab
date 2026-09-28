// udp_server.cpp — UDP 通信程序：服务器端（接收端）
//
// 功能：
//   1. 创建 UDP 套接字并绑定本地端口；
//   2. 循环接收客户端连续发送的数据包；
//   3. 根据数据包中的序号统计接收到的数据包和丢失的数据包数量。
//
// 用法： udp_server.exe [port] [expected_count]
//   默认端口为 2000，默认期望接收 100 个数据包。
//
// 编译：需要链接 ws2_32.lib（已用 #pragma comment 指定，MinGW 下请加 -lws2_32）。

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")   // MSVC 下自动链接 Winsock 库

#define DEFAULT_PORT     2000    // 默认监听端口（自定义端口建议使用 2000 以上）
#define DEFAULT_COUNT    100     // 默认期望接收的数据包个数
#define RECV_TIMEOUT_MS  5000    // 接收超时(毫秒)：超时未收到新包则判定发送结束
#define BUFSIZE          1024

// 数据包结构：序号 + 数据内容。
// 序号用于在接收端判断哪些数据包丢失。
struct Packet {
    int  seq;        // 数据包序号，从 1 开始
    char data[64];   // 数据内容
};

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET  sock;
    struct sockaddr_in serverAddr;   // 本地(服务器)地址
    struct sockaddr_in clientAddr;   // 发送方(客户端)地址
    int clientAddrLen;
    int ret;

    int port  = DEFAULT_PORT;
    int total = DEFAULT_COUNT;
    if (argc >= 2) port  = atoi(argv[1]);
    if (argc >= 3) total = atoi(argv[2]);

    // ---------- 1. 初始化 Winsock 库 ----------
    // WSAStartup 第一个参数指明请求使用的 Winsock 版本，MAKEWORD(2,2) 表示 2.2 版。
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup 失败，错误码：%d\n", WSAGetLastError());
        return 1;
    }

    // ---------- 2. 创建数据报套接字 ----------
    // UDP：domain = AF_INET，type = SOCK_DGRAM
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("socket 失败，错误码：%d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // ---------- 3. 绑定本地 IP 地址和端口号 ----------
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);   // 绑定到本机任意网卡地址
    serverAddr.sin_port        = htons(port);         // 端口号需转换为网络字节序

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("bind 失败，错误码：%d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // 设置接收超时：超过 RECV_TIMEOUT_MS 未收到数据则 recvfrom 返回超时，
    // 用于判断发送端是否已结束发送。
    int timeout = RECV_TIMEOUT_MS;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    printf("UDP 服务器已启动，监听端口 %d，期望接收 %d 个数据包...\n\n", port, total);

    // ---------- 4. 循环接收数据包并统计 ----------
    // UDP 是无连接的，不需要 listen() 和 accept()，直接 recvfrom() 接收数据。
    int *received = new int[total + 1]();   // received[i]=1 表示序号 i 已收到
    int  recvCount = 0;                     // 实际收到的不同序号的数据包个数

    while (recvCount < total) {
        Packet pkt;
        clientAddrLen = sizeof(clientAddr);
        ret = recvfrom(sock, (char *)&pkt, sizeof(pkt), 0,
                       (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (ret == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAETIMEDOUT) {
                printf("\n接收超时，判定发送已结束。\n");
                break;
            }
            printf("recvfrom 失败，错误码：%d\n", WSAGetLastError());
            break;
        }

        // 只统计有效序号，且每个序号只计一次（忽略重复包）
        if (pkt.seq >= 1 && pkt.seq <= total && !received[pkt.seq]) {
            received[pkt.seq] = 1;
            recvCount++;
        }
        printf("收到数据包：序号 %d，来源 %s:%d\n",
               pkt.seq, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    }

    // ---------- 5. 输出统计结果 ----------
    int lost = total - recvCount;
    printf("\n================ 统计结果 ================\n");
    printf("发送数据包总数：%d\n", total);
    printf("接收数据包总数：%d\n", recvCount);
    printf("丢失数据包总数：%d\n", lost);
    printf("丢包率：%.2f%%\n", lost * 100.0 / total);

    if (lost > 0) {
        printf("丢失的数据包序号：");
        for (int i = 1; i <= total; i++)
            if (!received[i])
                printf("%d ", i);
        printf("\n");
    }

    // ---------- 6. 释放资源 ----------
    delete[] received;
    closesocket(sock);
    WSACleanup();
    return 0;
}
