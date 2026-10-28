# Nano_Test TCP 数据协议说明

## 1. 协议用途

本协议用于 Nano_Test 设备通过 TCP 向上位机连续发送测试数据。

TCP 本身是字节流协议。一次 TCP `send()` 对应的数据，在上位机一次 `recv()` 中可能被拆开，也可能和后续数据合并。因此，上位机必须按照本协议的帧头、序号字段和帧尾重新组包，不能把一次 `recv()` 直接当成一个应用数据包。

## 2. 名词定义

本协议中，一个“逻辑帧”可以拆分为多个“数据包”发送：

- `block_index`：逻辑帧号。相同 `block_index` 的数据包属于同一个逻辑帧。
- `frame_count`：当前逻辑帧总共被拆分成多少个数据包。
- `frame_number`：当前数据包在该逻辑帧中的序号，从 `1` 开始，最大为 `frame_count`。
- `payload`：当前数据包携带的实际数据，长度可以变化。

例如：

```text
block_index = 25
frame_count = 3
```

表示逻辑帧 `25` 被拆成 3 个数据包发送，这 3 个数据包的 `frame_number` 分别为 `1`、`2`、`3`。

## 3. 数据包格式

每个 TCP 应用数据包格式如下：

```text
+----------+----------+----------+----------+----------------+----------+
| 帧头     | 逻辑帧号 | 总包数   | 当前包号 | 可变长度数据   | 帧尾     |
| 4 bytes   | 4 bytes  | 2 bytes  | 2 bytes  | payload        | 4 bytes  |
+----------+----------+----------+----------+----------------+----------+
```

字段明细：

| 字段 | 偏移 | 长度 | 字节序 | 说明 |
|---|---:|---:|---|---|
| `header` | 0 | 4 bytes | 固定值 | `A0 B0 C0 D0` |
| `block_index` | 4 | 4 bytes | 大端序 | 逻辑帧号；无符号 `uint32` |
| `frame_count` | 8 | 2 bytes | 大端序 | 当前逻辑帧总包数；无符号 `uint16` |
| `frame_number` | 10 | 2 bytes | 大端序 | 当前包序号，范围 `1 ~ frame_count`；无符号 `uint16` |
| `payload` | 12 | 可变 | 原始字节 | 当前包的有效数据 |
| `tail` | `12 + payload_length` | 4 bytes | 固定值 | `AF BF CF DF` |

协议中没有单独的 `payload_length` 字段。`payload_length` 表示根据帧头和帧尾解析出来的实际数据长度。

## 4. 数据包总长度

固定字段长度为：

```text
帧头          4 bytes
block_index   4 bytes
frame_count   2 bytes
frame_number  2 bytes
帧尾          4 bytes
固定部分合计  16 bytes
```

因此：

```text
数据包总长度 = 16 + payload_length
```

例如 `payload_length = 100` 时：

```text
数据包总长度 = 16 + 100 = 116 bytes
```

## 5. 字节序要求

所有整数均使用网络字节序，也就是大端序。

例如：

```text
block_index = 25
```

发送到 TCP 字节流中的 4 个字节为：

```text
00 00 00 19
```

```text
frame_count = 3
```

发送为：

```text
00 03
```

```text
frame_number = 2
```

发送为：

```text
00 02
```

C 语言发送端可以使用 `htonl()` 和 `htons()`；接收端使用 `ntohl()` 和 `ntohs()`。

## 6. 完整示例

假设：

```text
block_index  = 25
frame_count  = 3
frame_number = 2
payload      = 01 02 03 04 05
```

则：

```text
payload_length = 5
数据包总长度   = 16 + 5 = 21 bytes
```

完整字节流为：

```text
A0 B0 C0 D0    # header
00 00 00 19    # block_index = 25
00 03          # frame_count = 3
00 02          # frame_number = 2
01 02 03 04 05 # payload，长度 5
AF BF CF DF    # tail
```

## 7. 合法性检查

上位机收到数据后，应至少检查：

1. 帧头是否为 `A0 B0 C0 D0`。
2. `frame_count` 是否大于 0。
3. `frame_number` 是否满足 `1 <= frame_number <= frame_count`。
4. 帧尾是否为 `AF BF CF DF`。
5. `payload_length` 是否超过上位机配置的最大允许值。
6. 相同 `block_index` 的包是否按 `frame_number=1,2,...,frame_count` 到达。

## 8. 缺包判断

当收到同一个逻辑帧中的包时：

```text
期望：block_index=25, frame_number=3
实际：block_index=25, frame_number=5
```

则认为：

```text
block_index=25 的 frame_number=3~4 缺失，共 2 包
```

跨逻辑帧时：

```text
最后收到：block_index=25, frame_number=3/4
下一包：  block_index=27, frame_number=2/4
```

则认为缺少：

```text
block_index=25, frame_number=4
block_index=26, frame_number=1~4
block_index=27, frame_number=1
```

如果断线后设备重新发送尚未完成的逻辑帧，之前已经收到过的 `block_index/frame_number` 会被识别为重复包，不计入缺包。

## 9. 重要注意事项

### 9.1 TCP 接收长度不等于协议包长度

上位机不能假设：

```text
一次 recv() = 一个完整数据包
```

正确做法是：

1. 把收到的字节追加到缓存。
2. 在缓存中查找帧头。
3. 从帧头之后查找帧尾。
4. 找到完整帧后取出一个应用数据包。
5. 继续解析缓存中剩余的数据。

### 9.2 Payload 不能包含完整帧尾

当前协议没有长度字段，使用帧尾 `AF BF CF DF` 定位数据包结束位置。因此，`payload` 中不能自然出现完整的：

```text
AF BF CF DF
```

否则接收端可能提前判断数据包结束，导致后续数据解析错位。

如果 payload 可能包含任意二进制内容，建议后续版本改为以下方式之一：

- 在序号字段后增加 `payload_length` 字段；或
- 对 payload 中的帧头、帧尾和转义字节进行转义；或
- 使用 CRC 加长度字段的完整二进制帧格式。

### 9.3 TCP 丢包与应用层缺包不是同一个概念

TCP 会自动重传底层丢失的数据。因此上位机根据 `block_index/frame_number` 统计的是应用层序号缺失、重复和复位，不是 Wi-Fi 空口的原始丢包率。

## 10. 参考伪代码

```c
send(header, 4);
send_u32_be(block_index);
send_u16_be(frame_count);
send_u16_be(frame_number);
send(payload, payload_length);
send(tail, 4);
```

其中：

```text
header = A0 B0 C0 D0
tail   = AF BF CF DF
```
