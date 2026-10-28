import queue
import socket
import struct
import tempfile
import time
import unittest
from pathlib import Path

from nano_tcp_monitor import ReceiverServer
from protocol import FRAME_HEADER, FRAME_TAIL


def make_frame(block, count, number, payload):
    return FRAME_HEADER + struct.pack("!IHH", block, count, number) + payload + FRAME_TAIL


def wait_until(predicate, timeout=3.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return True
        time.sleep(0.02)
    return False


class ReceiverServerTests(unittest.TestCase):
    def test_variable_payload_gap_and_log_files(self):
        probe = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        probe.bind(("127.0.0.1", 0))
        port = probe.getsockname()[1]
        probe.close()

        with tempfile.TemporaryDirectory() as temporary:
            events = queue.Queue()
            server = ReceiverServer(
                "127.0.0.1",
                port,
                1024 * 1024,
                Path(temporary),
                False,
                events,
            )
            server.start()
            self.assertTrue(wait_until(lambda: server.is_alive))

            client = socket.create_connection(("127.0.0.1", port), timeout=2.0)
            first = make_frame(3, 4, 1, b"abc")
            third = make_frame(3, 4, 3, b"z" * 111)
            client.sendall(first[:7])
            client.sendall(first[7:] + third)
            client.shutdown(socket.SHUT_RDWR)
            client.close()

            self.assertTrue(
                wait_until(lambda: server.statistics.snapshot()["received_packets"] == 2)
            )
            self.assertEqual(server.statistics.snapshot()["missing_packets"], 1)
            self.assertEqual(server.statistics.snapshot()["payload_bytes"], 114)

            server.stop()
            self.assertTrue(wait_until(lambda: not server.is_alive))
            sessions = list(Path(temporary).iterdir())
            self.assertEqual(len(sessions), 1)
            self.assertTrue((sessions[0] / "monitor.log").is_file())
            self.assertTrue((sessions[0] / "missing_packets.csv").is_file())
            self.assertTrue((sessions[0] / "session_summary.json").is_file())


if __name__ == "__main__":
    unittest.main()
