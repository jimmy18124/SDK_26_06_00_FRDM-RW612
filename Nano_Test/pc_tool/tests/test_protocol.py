import struct
import unittest

from protocol import FRAME_HEADER, FRAME_TAIL, SequenceTracker, StreamParser


def make_frame(block, count, number, payload):
    return FRAME_HEADER + struct.pack("!IHH", block, count, number) + payload + FRAME_TAIL


class StreamParserTests(unittest.TestCase):
    def test_handles_fragmentation_and_coalescing(self):
        parser = StreamParser()
        one = make_frame(7, 3, 1, b"abc")
        two = make_frame(7, 3, 2, b"payload with another size")

        self.assertEqual(parser.feed(one[:5]).frames, [])
        result = parser.feed(one[5:] + two)

        self.assertEqual([(f.block_index, f.frame_number) for f in result.frames], [(7, 1), (7, 2)])
        self.assertEqual(result.frames[0].payload, b"abc")
        self.assertEqual(result.frames[1].payload, b"payload with another size")

    def test_resynchronizes_after_noise(self):
        parser = StreamParser()
        result = parser.feed(b"noise" + make_frame(1, 1, 1, b"x"))
        self.assertEqual(len(result.frames), 1)
        self.assertEqual(result.discarded_bytes, 5)

    def test_accepts_variable_size(self):
        parser = StreamParser()
        data = make_frame(1, 2, 1, b"a") + make_frame(1, 2, 2, b"b" * 27)
        result = parser.feed(data)
        self.assertEqual([f.payload_length for f in result.frames], [1, 27])


class SequenceTrackerTests(unittest.TestCase):
    def setUp(self):
        self.parser = StreamParser()
        self.tracker = SequenceTracker()

    def frame(self, block, count, number):
        return self.parser.feed(make_frame(block, count, number, b"x")).frames[0]

    def test_reports_missing_frames_in_same_block(self):
        self.tracker.process(self.frame(10, 5, 1))
        result = self.tracker.process(self.frame(10, 5, 4))
        self.assertEqual(result.status, "gap")
        self.assertEqual(result.missing[0].missing_count, 2)
        self.assertEqual((result.missing[0].frame_start, result.missing[0].frame_end), (2, 3))

    def test_reports_cross_block_gap(self):
        self.tracker.process(self.frame(10, 5, 4))
        result = self.tracker.process(self.frame(12, 5, 3))
        self.assertEqual(sum(item.missing_count for item in result.missing), 8)

    def test_retransmission_is_duplicate(self):
        self.tracker.process(self.frame(20, 100, 1))
        self.tracker.process(self.frame(20, 100, 2))
        result = self.tracker.process(self.frame(20, 100, 1))
        self.assertEqual(result.status, "duplicate")

    def test_device_restart_is_sequence_reset(self):
        self.tracker.process(self.frame(0, 2, 1))
        self.tracker.process(self.frame(0, 2, 2))
        self.tracker.process(self.frame(1, 2, 1))
        result = self.tracker.process(self.frame(0, 2, 1))
        self.assertEqual(result.status, "reset")


if __name__ == "__main__":
    unittest.main()
