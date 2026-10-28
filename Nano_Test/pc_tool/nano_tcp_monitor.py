"""Nano_Test TCP receiver, statistics monitor, and logger."""

from __future__ import annotations

import csv
import ctypes
import json
import os
import queue
import socket
import sys
import threading
import time
import traceback
from collections import deque
from datetime import datetime
from pathlib import Path
from typing import Dict, Optional, Tuple

import tkinter as tk
from tkinter import filedialog, messagebox, ttk

from protocol import Frame, MissingSpan, SequenceTracker, StreamParser


APP_TITLE = "Nano_Test TCP 数据监控"
DEFAULT_PORT = 15151
RECEIVE_SIZE = 64 * 1024
RAW_FILE_LIMIT = 512 * 1024 * 1024


def application_directory() -> Path:
    if getattr(sys, "frozen", False):
        return Path(sys.executable).resolve().parent
    return Path(__file__).resolve().parent


def hide_packaged_console() -> None:
    if not getattr(sys, "frozen", False) or sys.platform != "win32":
        return
    try:
        console = ctypes.windll.kernel32.GetConsoleWindow()
        if console:
            ctypes.windll.user32.ShowWindow(console, 0)
    except (AttributeError, OSError):
        pass


def wall_time_text() -> str:
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]


def session_stamp() -> str:
    return datetime.now().strftime("%Y%m%d_%H%M%S")


def format_bytes(value: float) -> str:
    units = ("B", "KiB", "MiB", "GiB", "TiB")
    number = float(value)
    for unit in units:
        if abs(number) < 1024.0 or unit == units[-1]:
            return "%.2f %s" % (number, unit)
        number /= 1024.0
    return "%.2f TiB" % number


def format_rate(value: float) -> str:
    return "%s/s" % format_bytes(value)


def format_duration(seconds: float) -> str:
    seconds = max(0, int(seconds))
    hours, remainder = divmod(seconds, 3600)
    minutes, seconds = divmod(remainder, 60)
    return "%02d:%02d:%02d" % (hours, minutes, seconds)


class Statistics:
    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._connected = False
        self._peer = "-"
        self._connected_since = 0.0
        self._connection_id = 0
        self.reset_counters()

    def reset_counters(self) -> None:
        with getattr(self, "_lock", threading.Lock()):
            self.total_bytes = 0
            self.payload_bytes = 0
            self.received_packets = 0
            self.unique_packets = 0
            self.complete_blocks = 0
            self.missing_packets = 0
            self.duplicate_packets = 0
            self.invalid_packets = 0
            self.discarded_bytes = 0
            self.truncated_bytes = 0
            self.connections = 0
            self.disconnections = 0
            self.last_block = None
            self.last_frame = None
            self.last_frame_count = None
            self.last_payload_length = None
            self.minimum_payload_length = None
            self.maximum_payload_length = None
            self._first_data_time = 0.0
            self._rate_events = deque()
            self._peak_rate = 0.0

    def connected(self, peer: Tuple[str, int]) -> int:
        with self._lock:
            self._connected = True
            self._peer = "%s:%d" % peer
            self._connected_since = time.monotonic()
            self._connection_id += 1
            self.connections += 1
            return self._connection_id

    def disconnected(self) -> None:
        with self._lock:
            if self._connected:
                self.disconnections += 1
            self._connected = False
            self._peer = "-"
            self._connected_since = 0.0

    def add_bytes(self, count: int) -> None:
        now = time.monotonic()
        with self._lock:
            self.total_bytes += count
            if not self._first_data_time:
                self._first_data_time = now
            self._rate_events.append((now, count))
            self._prune_rate_events(now)
            current = sum(item[1] for item in self._rate_events)
            if current > self._peak_rate:
                self._peak_rate = float(current)

    def add_frame(self, frame: Frame, duplicate: bool) -> None:
        with self._lock:
            self.received_packets += 1
            self.payload_bytes += frame.payload_length
            self.last_block = frame.block_index
            self.last_frame = frame.frame_number
            self.last_frame_count = frame.frame_count
            self.last_payload_length = frame.payload_length
            if duplicate:
                self.duplicate_packets += 1
            else:
                self.unique_packets += 1
            if self.minimum_payload_length is None or frame.payload_length < self.minimum_payload_length:
                self.minimum_payload_length = frame.payload_length
            if self.maximum_payload_length is None or frame.payload_length > self.maximum_payload_length:
                self.maximum_payload_length = frame.payload_length

    def add_missing(self, count: int) -> None:
        with self._lock:
            self.missing_packets += count

    def add_complete_block(self) -> None:
        with self._lock:
            self.complete_blocks += 1

    def add_parse_problem(self, invalid_packets: int, discarded_bytes: int) -> None:
        with self._lock:
            self.invalid_packets += invalid_packets
            self.discarded_bytes += discarded_bytes

    def add_truncated(self, count: int) -> None:
        with self._lock:
            self.truncated_bytes += count

    def snapshot(self) -> Dict[str, object]:
        now = time.monotonic()
        with self._lock:
            self._prune_rate_events(now)
            current_rate = float(sum(item[1] for item in self._rate_events))
            elapsed = now - self._first_data_time if self._first_data_time else 0.0
            average_rate = self.total_bytes / elapsed if elapsed > 0 else 0.0
            connection_time = now - self._connected_since if self._connected_since else 0.0
            return {
                "connected": self._connected,
                "peer": self._peer,
                "connection_id": self._connection_id,
                "connection_time": connection_time,
                "total_bytes": self.total_bytes,
                "payload_bytes": self.payload_bytes,
                "received_packets": self.received_packets,
                "unique_packets": self.unique_packets,
                "complete_blocks": self.complete_blocks,
                "missing_packets": self.missing_packets,
                "duplicate_packets": self.duplicate_packets,
                "invalid_packets": self.invalid_packets,
                "discarded_bytes": self.discarded_bytes,
                "truncated_bytes": self.truncated_bytes,
                "connections": self.connections,
                "disconnections": self.disconnections,
                "last_block": self.last_block,
                "last_frame": self.last_frame,
                "last_frame_count": self.last_frame_count,
                "last_payload_length": self.last_payload_length,
                "minimum_payload_length": self.minimum_payload_length,
                "maximum_payload_length": self.maximum_payload_length,
                "current_rate": current_rate,
                "average_rate": average_rate,
                "peak_rate": self._peak_rate,
            }

    def _prune_rate_events(self, now: float) -> None:
        cutoff = now - 1.0
        while self._rate_events and self._rate_events[0][0] < cutoff:
            self._rate_events.popleft()


class SessionLogger:
    def __init__(self, base_directory: Path, save_raw: bool, ui_queue: queue.Queue):
        self.directory = base_directory / session_stamp()
        suffix = 1
        while self.directory.exists():
            self.directory = base_directory / (session_stamp() + "_%02d" % suffix)
            suffix += 1
        self.directory.mkdir(parents=True, exist_ok=False)
        self._ui_queue = ui_queue
        self._lock = threading.Lock()
        self._text = (self.directory / "monitor.log").open("w", encoding="utf-8", buffering=1)
        self._missing_file = (self.directory / "missing_packets.csv").open(
            "w", encoding="utf-8-sig", newline=""
        )
        self._missing = csv.writer(self._missing_file)
        self._missing.writerow(
            [
                "time",
                "connection_id",
                "block_start",
                "block_end",
                "frame_start",
                "frame_end",
                "missing_count",
            ]
        )
        self._save_raw = save_raw
        self._raw = None
        self._raw_index = 0
        self._raw_size = 0

    def event(self, level: str, message: str) -> None:
        timestamp = wall_time_text()
        line = "[%s] [%s] %s" % (timestamp, level, message)
        with self._lock:
            self._text.write(line + "\n")
        self._ui_queue.put(("log", timestamp, level, message))

    def missing(self, connection_id: int, span: MissingSpan) -> None:
        timestamp = wall_time_text()
        with self._lock:
            self._missing.writerow(
                [
                    timestamp,
                    connection_id,
                    span.block_start,
                    span.block_end,
                    span.frame_start,
                    span.frame_end,
                    span.missing_count,
                ]
            )
            self._missing_file.flush()
        self._ui_queue.put(("missing", timestamp, connection_id, span))

    def raw(self, data: bytes) -> None:
        if not self._save_raw:
            return
        with self._lock:
            if self._raw is None or self._raw_size + len(data) > RAW_FILE_LIMIT:
                if self._raw is not None:
                    self._raw.close()
                self._raw_index += 1
                self._raw = (self.directory / ("raw_data_%04d.bin" % self._raw_index)).open("wb")
                self._raw_size = 0
            self._raw.write(data)
            self._raw_size += len(data)

    def close(self, summary: Dict[str, object]) -> None:
        with self._lock:
            with (self.directory / "session_summary.json").open("w", encoding="utf-8") as output:
                json.dump(summary, output, ensure_ascii=False, indent=2)
            if self._raw is not None:
                self._raw.close()
                self._raw = None
            self._missing_file.close()
            self._text.close()


class ReceiverServer:
    def __init__(
        self,
        bind_address: str,
        port: int,
        maximum_payload: int,
        log_directory: Path,
        save_raw: bool,
        ui_queue: queue.Queue,
    ):
        self.bind_address = bind_address
        self.port = port
        self.maximum_payload = maximum_payload
        self.log_directory = log_directory
        self.save_raw = save_raw
        self.ui_queue = ui_queue
        self.statistics = Statistics()
        self._stop_event = threading.Event()
        self._reset_event = threading.Event()
        self._thread = threading.Thread(target=self._run, name="nano-tcp-receiver", daemon=True)
        self._socket_lock = threading.Lock()
        self._listener = None
        self._client = None
        self._logger = None
        self._tracker = SequenceTracker()
        self._block_frames = {}
        self._block_order = deque()
        self._completed_blocks = set()

    @property
    def is_alive(self) -> bool:
        return self._thread.is_alive()

    def start(self) -> None:
        self._thread.start()

    def stop(self) -> None:
        self._stop_event.set()
        with self._socket_lock:
            sockets = (self._client, self._listener)
        for item in sockets:
            if item is not None:
                try:
                    item.shutdown(socket.SHUT_RDWR)
                except OSError:
                    pass
                try:
                    item.close()
                except OSError:
                    pass

    def request_statistics_reset(self) -> None:
        self._reset_event.set()

    def _run(self) -> None:
        try:
            self._logger = SessionLogger(self.log_directory, self.save_raw, self.ui_queue)
            self.ui_queue.put(("session", str(self._logger.directory)))
            self._logger.event(
                "INFO",
                "会话开始，协议总包长=16+payload_length，最大 payload=%d 字节"
                % self.maximum_payload,
            )
            listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            listener.settimeout(0.5)
            listener.bind((self.bind_address, self.port))
            listener.listen(1)
            with self._socket_lock:
                self._listener = listener
            self.ui_queue.put(("state", "等待连接", "listening"))
            self._logger.event("INFO", "监听 %s:%d" % (self.bind_address, self.port))

            while not self._stop_event.is_set():
                self._apply_reset_if_requested()
                try:
                    client, peer = listener.accept()
                except socket.timeout:
                    continue
                except OSError as error:
                    if not self._stop_event.is_set():
                        self._logger.event("ERROR", "accept 失败：%s" % error)
                    break
                with self._socket_lock:
                    self._client = client
                self._receive_client(client, peer)
                with self._socket_lock:
                    self._client = None
                if not self._stop_event.is_set():
                    self.ui_queue.put(("state", "等待重连", "listening"))
        except Exception as error:
            if self._logger is not None:
                self._logger.event("ERROR", "服务器启动或运行失败：%s" % error)
            else:
                self.ui_queue.put(("log", wall_time_text(), "ERROR", str(error)))
            self.ui_queue.put(("state", "启动失败", "error"))
        finally:
            with self._socket_lock:
                listener = self._listener
                self._listener = None
            if listener is not None:
                try:
                    listener.close()
                except OSError:
                    pass
            self.statistics.disconnected()
            if self._logger is not None:
                if self._stop_event.is_set():
                    self._logger.event("INFO", "服务器已停止")
                self._logger.close(self.statistics.snapshot())
            self.ui_queue.put(("state", "已停止", "stopped"))
            self.ui_queue.put(("server_finished",))

    def _receive_client(self, client: socket.socket, peer: Tuple[str, int]) -> None:
        parser = StreamParser(self.maximum_payload)
        connection_id = self.statistics.connected(peer)
        connection_bytes = 0
        started = time.monotonic()
        reason = "对端正常关闭"
        client.settimeout(0.5)
        try:
            client.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
            if hasattr(socket, "SIO_KEEPALIVE_VALS"):
                client.ioctl(socket.SIO_KEEPALIVE_VALS, (1, 5000, 1000))
        except OSError as error:
            self._logger.event("WARN", "设置 TCP keepalive 失败：%s" % error)

        self.ui_queue.put(("state", "已连接 %s:%d" % peer, "connected"))
        self._logger.event("INFO", "连接 #%d 建立：%s:%d" % (connection_id, peer[0], peer[1]))
        try:
            while not self._stop_event.is_set():
                self._apply_reset_if_requested()
                try:
                    data = client.recv(RECEIVE_SIZE)
                except socket.timeout:
                    continue
                except (ConnectionResetError, ConnectionAbortedError) as error:
                    reason = "连接被复位：%s" % error
                    break
                except OSError as error:
                    reason = "socket 接收异常：%s" % error
                    break

                if not data:
                    reason = "对端正常关闭"
                    break
                connection_bytes += len(data)
                self.statistics.add_bytes(len(data))
                self._logger.raw(data)
                result = parser.feed(data)
                if result.invalid_packets or result.discarded_bytes:
                    self.statistics.add_parse_problem(result.invalid_packets, result.discarded_bytes)
                    detail = result.messages[0] if result.messages else "重新查找帧头"
                    self._logger.event(
                        "WARN",
                        "协议重新同步：坏包=%d，丢弃=%d 字节，%s"
                        % (result.invalid_packets, result.discarded_bytes, detail),
                    )
                for frame in result.frames:
                    self._handle_frame(connection_id, frame)
        finally:
            pending = parser.reset()
            if pending:
                self.statistics.add_truncated(pending)
                self._logger.event("WARN", "连接结束时残留 %d 字节不完整数据" % pending)
            try:
                client.close()
            except OSError:
                pass
            elapsed = time.monotonic() - started
            self.statistics.disconnected()
            if self._stop_event.is_set():
                reason = "用户停止服务器"
            self._logger.event(
                "INFO",
                "连接 #%d 断开：%s；持续 %s；接收 %s"
                % (connection_id, reason, format_duration(elapsed), format_bytes(connection_bytes)),
            )

    def _handle_frame(self, connection_id: int, frame: Frame) -> None:
        tracked = self._tracker.process(frame)
        duplicate = tracked.status == "duplicate"
        self.statistics.add_frame(frame, duplicate)

        if tracked.status == "gap":
            for span in tracked.missing:
                self.statistics.add_missing(span.missing_count)
                self._logger.missing(connection_id, span)
                self._logger.event(
                    "LOSS", "缺包 %s，共 %d 包" % (span.describe(), span.missing_count)
                )
        elif tracked.status == "duplicate":
            self._logger.event(
                "DUP", "重复包 block=%d frame=%d/%d" % (
                    frame.block_index,
                    frame.frame_number,
                    frame.frame_count,
                )
            )
        elif tracked.status == "reset":
            self._block_frames.clear()
            self._block_order.clear()
            self._completed_blocks.clear()
            self._logger.event(
                "WARN",
                "序号基准重置：block=%d frame=%d/%d；%s"
                % (frame.block_index, frame.frame_number, frame.frame_count, tracked.detail),
            )

        if not duplicate and self._record_block_frame(frame):
            self.statistics.add_complete_block()

    def _record_block_frame(self, frame: Frame) -> bool:
        block = frame.block_index
        if block not in self._block_frames:
            self._block_frames[block] = (frame.frame_count, set())
            self._block_order.append(block)
        expected_count, numbers = self._block_frames[block]
        if expected_count != frame.frame_count:
            self._block_frames[block] = (frame.frame_count, set())
            expected_count, numbers = self._block_frames[block]
        numbers.add(frame.frame_number)
        completed = len(numbers) == expected_count and block not in self._completed_blocks
        if completed:
            self._completed_blocks.add(block)
        while len(self._block_order) > 512:
            old = self._block_order.popleft()
            self._block_frames.pop(old, None)
            self._completed_blocks.discard(old)
        return completed

    def _apply_reset_if_requested(self) -> None:
        if not self._reset_event.is_set():
            return
        self._reset_event.clear()
        self.statistics.reset_counters()
        self._tracker.reset()
        self._block_frames.clear()
        self._block_order.clear()
        self._completed_blocks.clear()
        if self._logger is not None:
            self._logger.event("INFO", "用户重置统计和序号基准")


class MonitorApp:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title(APP_TITLE)
        self.root.geometry("1120x760")
        self.root.minsize(940, 650)
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)
        self.ui_queue = queue.Queue()
        self.server: Optional[ReceiverServer] = None
        self.session_directory = ""

        self.bind_var = tk.StringVar(value="0.0.0.0")
        self.port_var = tk.StringVar(value=str(DEFAULT_PORT))
        self.max_payload_var = tk.StringVar(value="4")
        self.raw_var = tk.BooleanVar(value=False)
        self.log_directory_var = tk.StringVar(value=str(application_directory() / "logs"))
        self.state_var = tk.StringVar(value="未启动")
        self.session_var = tk.StringVar(value="日志目录：-")
        self.stat_vars = {}

        self._configure_style()
        self._build_ui()
        self.root.after(100, self._drain_events)
        self.root.after(250, self._refresh_statistics)

    def _configure_style(self) -> None:
        style = ttk.Style(self.root)
        try:
            style.theme_use("vista")
        except tk.TclError:
            pass
        self.root.option_add("*Font", ("Microsoft YaHei UI", 9))
        style.configure("Title.TLabel", font=("Microsoft YaHei UI", 13, "bold"))
        style.configure("State.TLabel", font=("Microsoft YaHei UI", 10, "bold"))
        style.configure("Value.TLabel", font=("Segoe UI", 12, "bold"))
        style.configure("Start.TButton", padding=(14, 6))
        style.configure("Stop.TButton", padding=(14, 6))

    def _build_ui(self) -> None:
        main = ttk.Frame(self.root, padding=12)
        main.pack(fill=tk.BOTH, expand=True)

        title_row = ttk.Frame(main)
        title_row.pack(fill=tk.X, pady=(0, 8))
        ttk.Label(title_row, text=APP_TITLE, style="Title.TLabel").pack(side=tk.LEFT)
        self.state_label = ttk.Label(title_row, textvariable=self.state_var, style="State.TLabel")
        self.state_label.pack(side=tk.RIGHT)

        config = ttk.LabelFrame(main, text="监听设置", padding=10)
        config.pack(fill=tk.X)
        ttk.Label(config, text="监听地址").grid(row=0, column=0, sticky=tk.W)
        self.bind_entry = ttk.Entry(config, textvariable=self.bind_var, width=16)
        self.bind_entry.grid(row=1, column=0, sticky=tk.EW, padx=(0, 10))
        ttk.Label(config, text="端口").grid(row=0, column=1, sticky=tk.W)
        self.port_entry = ttk.Entry(config, textvariable=self.port_var, width=9)
        self.port_entry.grid(row=1, column=1, sticky=tk.EW, padx=(0, 10))
        ttk.Label(config, text="最大 payload (MiB)").grid(row=0, column=2, sticky=tk.W)
        self.max_payload_entry = ttk.Entry(config, textvariable=self.max_payload_var, width=14)
        self.max_payload_entry.grid(row=1, column=2, sticky=tk.EW, padx=(0, 10))
        ttk.Label(config, text="日志根目录").grid(row=0, column=3, sticky=tk.W)
        self.log_entry = ttk.Entry(config, textvariable=self.log_directory_var)
        self.log_entry.grid(row=1, column=3, sticky=tk.EW, padx=(0, 6))
        self.browse_button = ttk.Button(config, text="浏览", command=self._choose_log_directory)
        self.browse_button.grid(row=1, column=4, padx=(0, 12))
        self.raw_check = ttk.Checkbutton(config, text="保存原始数据", variable=self.raw_var)
        self.raw_check.grid(row=1, column=5, padx=(0, 12))
        self.start_button = ttk.Button(config, text="开始监听", style="Start.TButton", command=self._start)
        self.start_button.grid(row=1, column=6, padx=(0, 6))
        self.stop_button = ttk.Button(config, text="停止", style="Stop.TButton", command=self._stop, state=tk.DISABLED)
        self.stop_button.grid(row=1, column=7)
        config.columnconfigure(3, weight=1)

        stats = ttk.LabelFrame(main, text="实时统计", padding=8)
        stats.pack(fill=tk.X, pady=(10, 0))
        stat_items = [
            ("current_rate", "当前速率"),
            ("average_rate", "平均速率"),
            ("peak_rate", "峰值速率"),
            ("total_bytes", "TCP 总字节"),
            ("payload_bytes", "Payload 字节"),
            ("received_packets", "收到包数"),
            ("unique_packets", "唯一包数"),
            ("complete_blocks", "完整 Block"),
            ("missing_packets", "缺失包数"),
            ("duplicate_packets", "重复包数"),
            ("invalid_packets", "坏包数"),
            ("discarded_bytes", "重同步丢弃"),
            ("connections", "连接/断开"),
            ("connection_time", "连接时长"),
            ("last_sequence", "最新序号"),
        ]
        for index, (key, label) in enumerate(stat_items):
            row = index // 5
            column = index % 5
            cell = ttk.Frame(stats, padding=(6, 3))
            cell.grid(row=row, column=column, sticky=tk.EW)
            ttk.Label(cell, text=label).pack(anchor=tk.W)
            variable = tk.StringVar(value="-")
            self.stat_vars[key] = variable
            ttk.Label(cell, textvariable=variable, style="Value.TLabel").pack(anchor=tk.W)
        for column in range(5):
            stats.columnconfigure(column, weight=1, uniform="stats")

        actions = ttk.Frame(main)
        actions.pack(fill=tk.X, pady=(8, 6))
        ttk.Button(actions, text="重置统计", command=self._reset_statistics).pack(side=tk.LEFT)
        ttk.Button(actions, text="清空界面日志", command=self._clear_views).pack(side=tk.LEFT, padx=(6, 0))
        ttk.Button(actions, text="打开日志目录", command=self._open_log_directory).pack(side=tk.LEFT, padx=(6, 0))
        self.packet_size_var = tk.StringVar(value="Payload 长度：-")
        ttk.Label(actions, textvariable=self.packet_size_var).pack(side=tk.RIGHT)

        notebook = ttk.Notebook(main)
        notebook.pack(fill=tk.BOTH, expand=True)

        log_frame = ttk.Frame(notebook)
        notebook.add(log_frame, text="事件日志")
        self.log_text = tk.Text(
            log_frame,
            wrap=tk.NONE,
            state=tk.DISABLED,
            bg="#101418",
            fg="#D8DEE9",
            insertbackground="#FFFFFF",
            font=("Consolas", 9),
            borderwidth=0,
        )
        log_y = ttk.Scrollbar(log_frame, orient=tk.VERTICAL, command=self.log_text.yview)
        log_x = ttk.Scrollbar(log_frame, orient=tk.HORIZONTAL, command=self.log_text.xview)
        self.log_text.configure(yscrollcommand=log_y.set, xscrollcommand=log_x.set)
        self.log_text.grid(row=0, column=0, sticky=tk.NSEW)
        log_y.grid(row=0, column=1, sticky=tk.NS)
        log_x.grid(row=1, column=0, sticky=tk.EW)
        log_frame.rowconfigure(0, weight=1)
        log_frame.columnconfigure(0, weight=1)
        self.log_text.tag_configure("ERROR", foreground="#FF6B6B")
        self.log_text.tag_configure("WARN", foreground="#FFD166")
        self.log_text.tag_configure("LOSS", foreground="#FF8C42")
        self.log_text.tag_configure("DUP", foreground="#8ECAE6")
        self.log_text.tag_configure("INFO", foreground="#D8DEE9")

        missing_frame = ttk.Frame(notebook)
        notebook.add(missing_frame, text="缺包明细")
        columns = ("time", "connection", "blocks", "frames", "count")
        self.missing_tree = ttk.Treeview(missing_frame, columns=columns, show="headings")
        headings = {
            "time": "时间",
            "connection": "连接",
            "blocks": "Block 范围",
            "frames": "Frame 范围",
            "count": "缺失数量",
        }
        widths = {"time": 180, "connection": 75, "blocks": 200, "frames": 180, "count": 100}
        for column in columns:
            self.missing_tree.heading(column, text=headings[column])
            self.missing_tree.column(column, width=widths[column], anchor=tk.CENTER)
        missing_y = ttk.Scrollbar(missing_frame, orient=tk.VERTICAL, command=self.missing_tree.yview)
        self.missing_tree.configure(yscrollcommand=missing_y.set)
        self.missing_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        missing_y.pack(side=tk.RIGHT, fill=tk.Y)

        ttk.Label(main, textvariable=self.session_var).pack(fill=tk.X, pady=(6, 0))

    def _start(self) -> None:
        try:
            port = int(self.port_var.get())
            maximum_mib = int(self.max_payload_var.get())
            if not 1 <= port <= 65535:
                raise ValueError("端口必须在 1~65535 之间")
            if not 1 <= maximum_mib <= 512:
                raise ValueError("最大 payload 必须在 1~512 MiB 之间")
            bind_address = self.bind_var.get().strip() or "0.0.0.0"
            socket.inet_aton(bind_address)
            log_directory = Path(self.log_directory_var.get()).expanduser().resolve()
        except (ValueError, OSError) as error:
            messagebox.showerror("参数错误", str(error), parent=self.root)
            return

        self.server = ReceiverServer(
            bind_address,
            port,
            maximum_mib * 1024 * 1024,
            log_directory,
            self.raw_var.get(),
            self.ui_queue,
        )
        self._set_controls_running(True)
        self.state_var.set("正在启动")
        self.server.start()

    def _stop(self) -> None:
        if self.server is not None:
            self.state_var.set("正在停止")
            self.stop_button.configure(state=tk.DISABLED)
            self.server.stop()

    def _reset_statistics(self) -> None:
        if self.server is not None and self.server.is_alive:
            self.server.request_statistics_reset()

    def _choose_log_directory(self) -> None:
        selected = filedialog.askdirectory(initialdir=self.log_directory_var.get(), parent=self.root)
        if selected:
            self.log_directory_var.set(selected)

    def _open_log_directory(self) -> None:
        target = self.session_directory or self.log_directory_var.get()
        try:
            Path(target).mkdir(parents=True, exist_ok=True)
            os.startfile(str(Path(target).resolve()))
        except OSError as error:
            messagebox.showerror("无法打开目录", str(error), parent=self.root)

    def _clear_views(self) -> None:
        self.log_text.configure(state=tk.NORMAL)
        self.log_text.delete("1.0", tk.END)
        self.log_text.configure(state=tk.DISABLED)
        for item in self.missing_tree.get_children():
            self.missing_tree.delete(item)

    def _set_controls_running(self, running: bool) -> None:
        entry_state = tk.DISABLED if running else tk.NORMAL
        for control in (
            self.bind_entry,
            self.port_entry,
            self.max_payload_entry,
            self.log_entry,
            self.browse_button,
            self.raw_check,
        ):
            control.configure(state=entry_state)
        self.start_button.configure(state=tk.DISABLED if running else tk.NORMAL)
        self.stop_button.configure(state=tk.NORMAL if running else tk.DISABLED)

    def _drain_events(self) -> None:
        try:
            while True:
                event = self.ui_queue.get_nowait()
                kind = event[0]
                if kind == "log":
                    _, timestamp, level, message = event
                    self._append_log(timestamp, level, message)
                elif kind == "missing":
                    _, timestamp, connection_id, span = event
                    block_text = str(span.block_start)
                    if span.block_start != span.block_end:
                        block_text = "%d~%d" % (span.block_start, span.block_end)
                    self.missing_tree.insert(
                        "",
                        tk.END,
                        values=(
                            timestamp,
                            "#%d" % connection_id,
                            block_text,
                            "%d~%d" % (span.frame_start, span.frame_end),
                            span.missing_count,
                        ),
                    )
                elif kind == "state":
                    self.state_var.set(event[1])
                elif kind == "session":
                    self.session_directory = event[1]
                    self.session_var.set("日志目录：%s" % event[1])
                elif kind == "server_finished":
                    self._set_controls_running(False)
        except queue.Empty:
            pass
        if self.root.winfo_exists():
            self.root.after(100, self._drain_events)

    def _append_log(self, timestamp: str, level: str, message: str) -> None:
        self.log_text.configure(state=tk.NORMAL)
        self.log_text.insert(tk.END, "[%s] [%-5s] %s\n" % (timestamp, level, message), level)
        line_count = int(self.log_text.index("end-1c").split(".")[0])
        if line_count > 5000:
            self.log_text.delete("1.0", "%d.0" % (line_count - 4500))
        self.log_text.see(tk.END)
        self.log_text.configure(state=tk.DISABLED)

    def _refresh_statistics(self) -> None:
        snapshot = self.server.statistics.snapshot() if self.server is not None else None
        if snapshot is not None:
            self.stat_vars["current_rate"].set(format_rate(snapshot["current_rate"]))
            self.stat_vars["average_rate"].set(format_rate(snapshot["average_rate"]))
            self.stat_vars["peak_rate"].set(format_rate(snapshot["peak_rate"]))
            self.stat_vars["total_bytes"].set(format_bytes(snapshot["total_bytes"]))
            self.stat_vars["payload_bytes"].set(format_bytes(snapshot["payload_bytes"]))
            for key in (
                "received_packets",
                "unique_packets",
                "complete_blocks",
                "missing_packets",
                "duplicate_packets",
                "invalid_packets",
            ):
                self.stat_vars[key].set("{:,}".format(snapshot[key]))
            self.stat_vars["discarded_bytes"].set(format_bytes(snapshot["discarded_bytes"]))
            self.stat_vars["connections"].set(
                "%d / %d" % (snapshot["connections"], snapshot["disconnections"])
            )
            self.stat_vars["connection_time"].set(format_duration(snapshot["connection_time"]))
            if snapshot["last_block"] is None:
                self.stat_vars["last_sequence"].set("-")
            else:
                self.stat_vars["last_sequence"].set(
                    "%d : %d/%d"
                    % (
                        snapshot["last_block"],
                        snapshot["last_frame"],
                        snapshot["last_frame_count"],
                    )
                )
            if snapshot["last_payload_length"] is None:
                self.packet_size_var.set("Payload 长度：-")
            else:
                self.packet_size_var.set(
                    "Payload 长度：当前 %s，最小 %s，最大 %s"
                    % (
                        format_bytes(snapshot["last_payload_length"]),
                        format_bytes(snapshot["minimum_payload_length"]),
                        format_bytes(snapshot["maximum_payload_length"]),
                    )
                )
        if self.root.winfo_exists():
            self.root.after(250, self._refresh_statistics)

    def _on_close(self) -> None:
        if self.server is not None and self.server.is_alive:
            self.server.stop()
            self.root.after(100, self._wait_and_close)
        else:
            self.root.destroy()

    def _wait_and_close(self) -> None:
        if self.server is not None and self.server.is_alive:
            self.root.after(100, self._wait_and_close)
        else:
            self.root.destroy()


def main() -> None:
    hide_packaged_console()
    root = tk.Tk()
    MonitorApp(root)
    root.mainloop()


if __name__ == "__main__":
    try:
        main()
    except Exception:
        crash_text = traceback.format_exc()
        try:
            with (application_directory() / "crash.log").open("a", encoding="utf-8") as crash_log:
                crash_log.write("[%s]\n%s\n" % (wall_time_text(), crash_text))
        finally:
            raise
