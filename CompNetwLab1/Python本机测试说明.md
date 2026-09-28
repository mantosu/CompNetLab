# Python UDP 实验本机测试

本机测试使用回环地址 `127.0.0.1` 和 UDP 端口 `8080`。客户端会发送 100 个带序号的数据报，服务端输出收到的序号并统计丢包、重复包、乱序包和格式错误。

## 1. 启动服务端

在终端 1 进入 `CompNetwLab1` 目录后执行：

```bash
python3 udp_server.py
```

出现 `Listening on udp://127.0.0.1:8080` 后保持该终端运行。`recvfrom()` 正在阻塞等待数据，不是程序卡死。

## 2. 启动 Wireshark（可选）

macOS 选择回环网卡 `Loopback: lo0`；Windows 选择 `Npcap Loopback Adapter`。开始抓包后再运行客户端。

显示过滤器：

```text
udp && udp.port == 8080
```

## 3. 启动客户端

在终端 2 进入 `CompNetwLab1` 目录后执行：

```bash
python3 udp_client.py
```

客户端发送完成后，服务端应输出类似结果：

```text
Expected:     100
Received:     100
Lost:         0
Loss rate:    0.00%
Duplicates:   0
Out of order: 0
Malformed:    0
Missing seq:  none
```

Wireshark 中通常会看到 101 个 UDP 数据报：100 个实验数据报和 1 个结束标记。统计丢包时只计前 100 个。

## 常用参数

快速连续发送（包间不等待）：

```bash
python3 udp_client.py --interval 0
```

更换目标 IP、端口或发包数：

```bash
python3 udp_client.py --host 127.0.0.1 --port 8080 --count 100
python3 udp_server.py --host 127.0.0.1 --port 8080 --count 100
```

## 两台电脑测试

服务端监听所有本地 IPv4 网卡：

```bash
python3 udp_server.py --host 0.0.0.0 --port 8080
```

客户端的 `--host` 改为服务器的局域网 IPv4 地址：

```bash
python3 udp_client.py --host 192.168.1.100 --port 8080
```

两台电脑需要连接同一个局域网，并确保防火墙允许 UDP 8080。
