# CLAUDE.md

本文件是 Claude Code 在本仓库中工作的指南。

## 项目概述

中山大学《计算机网络》实验课程仓库。实验按序号分目录存放（`CompNetwLabN/`），当前包含：

- **实验 1**：UDP 通信程序设计 —— 客户端循环发送 100 个带序号的数据报，服务器统计丢失/重复/乱序/格式错误，并用 Wireshark 分析。

后续还会有实验 2（TCP）、实验 3 等，且大概率以 **Python** 为主实现（实验 1 的 C++/Winsock 仅用于演示底层 Socket API）。新实验优先编写 Python 版。

## 小组成员（组号 6）

| 角色 | 姓名 | 学号 |
| :--- | :--- | :--- |
| 组长 | 闫维岳 | 24325288 |
| 组员 | 苏文韬 | 24325237 |
| 组员 | 薛淞 | 24325286 |

## 目录结构

```
CompNetwLab1/
├── udp_client.py              # Python 客户端（默认 127.0.0.1:8080，100 包）
├── udp_server.py              # Python 服务器
├── cpp/
│   ├── udp_client.cpp         # C++ (Winsock) 客户端（默认端口 2000）
│   └── udp_server.cpp         # C++ (Winsock) 服务器
├── Python本机测试说明.md        # Python 版本机/跨机测试说明
├── 实验1-网络编程实验-2026/     # 指导手册 PDF、报告/心得模板 docx
└── 实验1-网络编程实验报告.md     # 实验报告（Markdown，导出 PDF 提交）
```

## 常用命令

### Python（Windows 使用 `py`）

```bash
py udp_server.py                                 # 启动服务器
py udp_client.py                                 # 启动客户端
py udp_client.py --interval 0                    # 包间不等待，快速连续发送
py udp_server.py --host 0.0.0.0 --port 8080      # 跨机测试：监听所有网卡
py udp_client.py --host <局域网IP> --port 8080    # 跨机测试：指定服务器 IP
```

### C++（MinGW g++，必须显式链接 `-lws2_32`）

```bash
g++ cpp/udp_server.cpp -o udp_server.exe -lws2_32
g++ cpp/udp_client.cpp -o udp_client.exe -lws2_32
./udp_server.exe [port] [count]                  # 默认端口 2000、100 包
./udp_client.exe [server_ip] [port] [count]
```

> 源码中的 `#pragma comment(lib, "ws2_32.lib")` 仅对 MSVC 生效；MinGW 编译必须手动加 `-lws2_32`。
> VSCode 默认构建任务（`.vscode/tasks.json`）只调 g++ 未链接 ws2_32，编译 Winsock 程序请用上面的命令或修改任务配置。

## 约定

- 编译产物（`*.exe`、`*.o` 等）已被 `.gitignore` 忽略，不要提交。
- 实验报告先用 Markdown 编写，确认后再导出 PDF 提交；文件名按课程要求：
  - 小组报告（每组一份）：`组号_实验序号_实验报告.pdf`，如 `6_1_实验报告.pdf`
  - 个人心得（每人一份）：`学号_实验序号_心得体会.pdf`
- 小组实验报告内容：实验步骤、输入指令、输出截图、实验结果、遇到问题、解决方法、实验完成情况、改进建议等。
- 个人心得内容：承担的任务、实现和执行方法、实施细节、实施中各种截图和信息、遇到的问题与困难、解决方法、心得体会、改进建议等，每位同学单独完成一份。
- 代码注释、文档、报告均使用中文。
- 每个实验目录下应保留：源码（优先 Python）、实验报告 md、测试/使用说明。
-  md文档里面的图片应该用相对路径。
  
