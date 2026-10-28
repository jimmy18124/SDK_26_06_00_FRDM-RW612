# Nano_Test TCP 数据监控上位机

该工具使用 Python 3.8 自带的 `tkinter` 和标准库，不需要安装第三方包。

协议对外说明见：[Nano_Test_TCP协议说明.md](Nano_Test_TCP协议说明.md)。

## 协议

每个应用包长度可以不同，包格式为：

```text
偏移       长度                 内容
0          4                    A0 B0 C0 D0
4          4                    block_index，大端 uint32
8          2                    frame_count，大端 uint16
10         2                    frame_number，大端 uint16
12         payload_length       payload
12+长度     4                    AF BF CF DF
```

总包长为 `16 + payload_length`。协议中没有单独的长度字段，上位机通过帧头和帧尾进行定界。因此 payload 不得包含完整字节序列 `AF BF CF DF`；如果业务数据可能包含该序列，板端协议需要增加长度字段或转义规则。

TCP 是字节流。程序不会把一次 `recv()` 当成一个应用包，可以处理任意拆包和粘包。

## 启动

直接运行打包版本：

```text
dist\NanoTcpMonitor.exe
```

也可以使用 Python 源码版本。双击：

```text
run_monitor.bat
```

或在该目录执行：

```powershell
python nano_tcp_monitor.py
```

默认监听所有本机 IPv4 地址的 `15151` 端口。电脑连接 `Nano_Test` SoftAP 后，需要保证电脑 Wi-Fi 地址为板端配置的 TCP 服务器地址，当前固件为 `192.168.167.2`。

## 统计口径

- 缺包：根据 `block_index` 和 `frame_number` 的前向跳变统计。
- 重复包：已经接收过的 block/frame 再次到达。板端断线后重发未完成 block 时可能出现。
- 序号复位：序号回退、跳变过大或 `frame_count` 改变。
- 坏包：序号字段非法或超过最大 payload 仍未找到帧尾。
- 当前速率：最近 1 秒内收到的 TCP 字节数。
- 平均速率：本次统计开始后总 TCP 字节数除以经过时间。

TCP 会自动重传底层丢失的数据，因此这里统计的是应用层序号缺失，不等同于 Wi-Fi 空口丢包率。

## 日志

每次开始监听会创建一个时间戳目录：

```text
logs/YYYYMMDD_HHMMSS/
    monitor.log
    missing_packets.csv
    session_summary.json
    raw_data_0001.bin
```

EXE 版本默认在 `NanoTcpMonitor.exe` 所在目录创建 `logs`；源码版本默认在源码目录创建 `logs`。

`raw_data_*.bin` 仅在勾选“保存原始数据”后生成，并按 512 MiB 分卷。高速长时间测试时建议关闭原始数据保存。

## 测试

```powershell
python -m unittest discover -s tests -v
```
