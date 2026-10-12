#!/usr/bin/env python3
"""UDP 实验服务端：接收带序号的数据报，并统计丢包情况。

本机测试示例：
    python3 udp_server.py

两台电脑测试时，服务端需要监听所有本地 IPv4 网卡：
    python3 udp_server.py --host 0.0.0.0
"""

import argparse
import json
import socket


def parse_args():
    """解析命令行参数，使 IP、端口和发包数无需写死在代码中。"""
    parser = argparse.ArgumentParser(description="UDP packet receiver")
    # 127.0.0.1 是回环地址，表示只接收这台电脑自己发来的数据。
    parser.add_argument("--host", default="127.0.0.1", help="local address to bind")
    parser.add_argument("--port", type=int, default=8080, help="local UDP port")
    parser.add_argument("--count", type=int, default=100, help="expected packet count")
    parser.add_argument(
        "--timeout",
        type=float,
        default=5.0,
        help="seconds to wait after traffic starts",
    )
    return parser.parse_args()


def print_summary(received, expected_count, duplicates, malformed, out_of_order):
    """将期望序号与实际序号比较，输出本轮实验的统计结果。"""
    # 若 expected_count=100，期望集合就是 {1, 2, ..., 100}。
    expected = set(range(1, expected_count + 1))

    # 集合差 expected - received 可以直接找出未收到的序号。
    missing = sorted(expected - received)

    # 只统计有效范围内的序号，避免异常序号影响结果。
    received_expected = len(received & expected)
    loss_rate = (len(missing) / expected_count * 100) if expected_count else 0.0

    print("\n--- Statistics ---")
    print(f"Expected:     {expected_count}")
    print(f"Received:     {received_expected}")
    print(f"Lost:         {len(missing)}")
    print(f"Loss rate:    {loss_rate:.2f}%")
    print(f"Duplicates:   {duplicates}")
    print(f"Out of order: {out_of_order}")
    print(f"Malformed:    {malformed}")
    print(f"Missing seq:  {missing if missing else 'none'}")


def main():
    args = parse_args()

    # set 会自动去重，因此很适合保存“已收到的序号”。
    expected = set(range(1, args.count + 1))
    received = set()
    duplicates = 0
    malformed = 0
    out_of_order = 0
    highest_seq = 0

    # AF_INET 表示使用 IPv4；SOCK_DGRAM 表示使用 UDP 数据报套接字。
    # with 代码块结束时会自动关闭 socket，释放端口和系统资源。
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as server:
        # bind 将 socket 与本地 IP 和端口绑定。客户端将数据发到这个地址。
        server.bind((args.host, args.port))
        print(f"Listening on udp://{args.host}:{args.port}")
        print(f"Waiting for {args.count} numbered datagrams ...")
        print("Keep this terminal open and run udp_client.py in another terminal.")

        while True:
            try:
                # recvfrom 会阻塞等待一个 UDP 数据报。
                # data 是收到的字节；peer 是发送方的 (IP, 端口) 元组。
                # 65535 是本次最多接收的字节数，足以容纳一个 UDP 数据报。
                data, peer = server.recvfrom(65535)

                # 第一个包到达前可以一直等待；开始收包后，超过指定时间
                # 没有新包就结束本轮实验，避免丢包时服务端永久等待。
                server.settimeout(args.timeout)
            except socket.timeout:
                print(f"No new datagram for {args.timeout:g} seconds; stopping.")
                break
            except KeyboardInterrupt:
                # Ctrl+C 是用户主动终止程序，不是网络错误。
                print("\nStopped by user (Ctrl+C).")
                break

            try:
                # 网络上收到的是 bytes：先按 UTF-8 转成字符串，再解析 JSON。
                message = json.loads(data.decode("utf-8"))
            except (UnicodeDecodeError, json.JSONDecodeError):
                malformed += 1
                print(f"Malformed datagram from {peer[0]}:{peer[1]}")
                continue

            # UDP 无连接，没有 TCP 那样的“关闭连接”通知。
            # 因此本实验自定义 type=end 数据报，表示客户端已发送完毕。
            if message.get("type") == "end":
                print(f"End marker received from {peer[0]}:{peer[1]}")

                # UDP 可能丢包或乱序。只有 1..count 全部到达才立即结束；
                # 若还有缺失，继续等待可能晚到的包，直到触发超时。
                if expected <= received:
                    break
                print("Some packets are still missing; waiting until timeout.")
                continue

            # 普通实验数据报必须是 type=data，且 seq 必须是整数。
            seq = message.get("seq")
            if message.get("type") != "data" or not isinstance(seq, int):
                malformed += 1
                print(f"Malformed message from {peer[0]}:{peer[1]}: {message!r}")
                continue

            # 同一序号已经出现过，说明收到了重复包。
            if seq in received:
                duplicates += 1
            # 新序号比之前见过的最大序号小，说明该包乱序到达。
            elif seq < highest_seq:
                out_of_order += 1
            else:
                highest_seq = seq

            received.add(seq)
            print(f"Received seq={seq:03d} from {peer[0]}:{peer[1]}")

    # socket 已关闭，根据收集的序号输出最终统计。
    print_summary(received, args.count, duplicates, malformed, out_of_order)


# 只有直接运行此文件时才调用 main；被其他 Python 文件导入时不会自动执行。
if __name__ == "__main__":
    main()
