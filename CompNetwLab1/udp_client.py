#!/usr/bin/env python3
"""UDP 实验客户端：向服务端连续发送带序号的数据报。

本机测试示例：
    python3 udp_client.py

局域网测试示例（IP 需改为服务器的实际 IPv4 地址）：
    python3 udp_client.py --host 192.168.1.100
"""

import argparse
import json
import socket
import time


def parse_args():
    """解析服务器地址、数据报数量和发送间隔等命令行参数。"""
    parser = argparse.ArgumentParser(description="UDP packet sender")
    # 本机测试使用 127.0.0.1；两台电脑时改为服务器的局域网 IP。
    parser.add_argument("--host", default="127.0.0.1", help="server IP address")
    parser.add_argument("--port", type=int, default=8080, help="server UDP port")
    parser.add_argument("--count", type=int, default=100, help="number of packets")
    parser.add_argument(
        "--interval",
        type=float,
        default=0.01,
        help="seconds between packets",
    )
    return parser.parse_args()


def encode(message):
    """把 Python 字典转换成可以通过 socket 发送的 UTF-8 字节。"""
    # separators 去掉 JSON 中不必要的空格，使 UDP 载荷更紧凑。
    return json.dumps(message, separators=(",", ":")).encode("utf-8")


def main():
    args = parse_args()

    # Python socket 用 (IP, 端口) 元组表示网络地址。
    destination = (args.host, args.port)

    # AF_INET 表示 IPv4；SOCK_DGRAM 表示 UDP。
    # UDP 是无连接协议，这里无需 TCP 客户端常用的 connect 操作。
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as client:
        print(f"Sending {args.count} datagrams to udp://{args.host}:{args.port}")

        # range 的结束值不包含在内，所以 count=100 时序号为 1..100。
        for seq in range(1, args.count + 1):
            # 每个数据报都包含类型、序号和发送时间。
            # 服务端主要使用 seq 判断数据报是否丢失、重复或乱序。
            message = {
                "type": "data",
                "seq": seq,
                # time_ns 返回纳秒单位的时间戳，也可用于后续分析延迟。
                "sent_ns": time.time_ns(),
            }

            # sendto 将一个完整 UDP 数据报发往 destination。
            # UDP 不会确认对方是否在线，返回成功也不代表服务端已收到。
            client.sendto(encode(message), destination)
            print(f"Sent seq={seq:03d}")

            # 默认每个包间隔 0.01 秒，方便终端和 Wireshark 观察。
            # 使用 --interval 0 可以不等待，快速连续发送。
            if args.interval > 0:
                time.sleep(args.interval)

        # UDP 没有内建的数据结束标志，所以额外发送一个 type=end 消息。
        # Wireshark 因此通常会看到 count + 1 个 UDP 数据报。
        client.sendto(encode({"type": "end", "count": args.count}), destination)
        print("Finished; end marker sent.")


# 直接执行此文件时进入 main；作为模块导入时不自动发包。
if __name__ == "__main__":
    main()
