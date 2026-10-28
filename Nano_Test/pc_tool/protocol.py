"""TCP stream framing and sequence tracking for Nano_Test."""

from __future__ import annotations

import struct
from dataclasses import dataclass, field
from typing import List, Optional, Tuple


FRAME_HEADER = b"\xA0\xB0\xC0\xD0"
FRAME_TAIL = b"\xAF\xBF\xCF\xDF"
SEQUENCE_HEADER_SIZE = 12
FRAME_OVERHEAD = 16


@dataclass(frozen=True)
class Frame:
    block_index: int
    frame_count: int
    frame_number: int
    payload_length: int
    packet_length: int
    raw: bytes = field(repr=False)

    @property
    def payload(self) -> bytes:
        return self.raw[SEQUENCE_HEADER_SIZE:-len(FRAME_TAIL)]


@dataclass
class ParseResult:
    frames: List[Frame] = field(default_factory=list)
    discarded_bytes: int = 0
    invalid_packets: int = 0
    messages: List[str] = field(default_factory=list)


class StreamParser:
    """Parse arbitrary TCP chunks using fixed header and tail markers.

    Packet length is 16 + the variable payload length. This framing is only
    unambiguous when the payload cannot contain the complete tail marker.
    """

    def __init__(self, max_payload_length: int = 4 * 1024 * 1024):
        if max_payload_length <= 0:
            raise ValueError("max_payload_length must be positive")
        self.max_payload_length = max_payload_length
        self._buffer = bytearray()

    @property
    def buffered_bytes(self) -> int:
        return len(self._buffer)

    def reset(self) -> int:
        pending = len(self._buffer)
        self._buffer.clear()
        return pending

    def feed(self, data: bytes) -> ParseResult:
        result = ParseResult()
        if data:
            self._buffer.extend(data)

        while True:
            header_offset = self._buffer.find(FRAME_HEADER)
            if header_offset < 0:
                keep = min(len(self._buffer), len(FRAME_HEADER) - 1)
                discard = len(self._buffer) - keep
                if discard:
                    del self._buffer[:discard]
                    result.discarded_bytes += discard
                break

            if header_offset:
                del self._buffer[:header_offset]
                result.discarded_bytes += header_offset

            frame = self._parse_frame(result)
            if frame is None:
                break
            if frame is False:
                continue
            result.frames.append(frame)

        return result

    def _parse_frame(self, result: ParseResult):
        tail_offset = self._buffer.find(FRAME_TAIL, SEQUENCE_HEADER_SIZE)
        if tail_offset < 0:
            maximum_packet_length = self.max_payload_length + FRAME_OVERHEAD
            if len(self._buffer) > maximum_packet_length:
                result.invalid_packets += 1
                result.messages.append("packet exceeds configured maximum payload length")
                del self._buffer[0]
                result.discarded_bytes += 1
                return False
            return None

        packet_length = tail_offset + len(FRAME_TAIL)
        if packet_length < FRAME_OVERHEAD:
            result.invalid_packets += 1
            del self._buffer[0]
            result.discarded_bytes += 1
            return False

        block_index, frame_count, frame_number = struct.unpack("!IHH", self._buffer[4:SEQUENCE_HEADER_SIZE])
        raw = bytes(self._buffer[:packet_length])
        del self._buffer[:packet_length]
        if not self._sequence_valid(frame_count, frame_number):
            result.invalid_packets += 1
            result.messages.append(
                "invalid sequence fields: block=%d count=%d number=%d"
                % (block_index, frame_count, frame_number)
            )
            return False
        payload_length = packet_length - FRAME_OVERHEAD
        return Frame(block_index, frame_count, frame_number, payload_length, packet_length, raw)

    @staticmethod
    def _sequence_valid(frame_count: int, frame_number: int) -> bool:
        return frame_count > 0 and 0 < frame_number <= frame_count


@dataclass(frozen=True)
class MissingSpan:
    block_start: int
    block_end: int
    frame_start: int
    frame_end: int
    missing_count: int

    def describe(self) -> str:
        if self.block_start == self.block_end:
            return "block=%d, frame=%d~%d" % (
                self.block_start,
                self.frame_start,
                self.frame_end,
            )
        return "block=%d~%d, frame=%d~%d" % (
            self.block_start,
            self.block_end,
            self.frame_start,
            self.frame_end,
        )


@dataclass
class TrackResult:
    status: str
    missing: List[MissingSpan] = field(default_factory=list)
    detail: str = ""


class SequenceTracker:
    """Track gaps using the application block/frame sequence."""

    def __init__(self, maximum_gap_blocks: int = 100000):
        self.maximum_gap_blocks = maximum_gap_blocks
        self.expected: Optional[Tuple[int, int, int]] = None
        self.last: Optional[Tuple[int, int]] = None
        self._recent = set()
        self._recent_order: List[Tuple[int, int]] = []
        self._recent_limit = 20000

    def reset(self) -> None:
        self.expected = None
        self.last = None
        self._recent.clear()
        self._recent_order = []

    def process(self, frame: Frame) -> TrackResult:
        key = (frame.block_index, frame.frame_number)
        if self.expected is None:
            self._remember(key)
            self._advance(frame)
            return TrackResult("first")

        expected_block, expected_frame, expected_count = self.expected
        if frame.frame_count != expected_count:
            old_count = expected_count
            self._recent.clear()
            self._recent_order = []
            self._remember(key)
            self._advance(frame)
            return TrackResult(
                "reset",
                detail="frame_count changed from %d to %d" % (old_count, frame.frame_count),
            )

        if frame.block_index == expected_block and frame.frame_number == expected_frame:
            self._remember(key)
            self._advance(frame)
            return TrackResult("ok")

        if key in self._recent:
            if self.last is not None and frame.block_index == self.last[0]:
                return TrackResult("duplicate", detail="repeated application frame")
            previous = self.last
            self._recent.clear()
            self._recent_order = []
            self._remember(key)
            self._advance(frame)
            return TrackResult(
                "reset",
                detail="sequence restarted: previous=%s current=(%d,%d)"
                % (previous, frame.block_index, frame.frame_number),
            )

        self._remember(key)

        block_delta = (frame.block_index - expected_block) & 0xFFFFFFFF
        if frame.block_index == expected_block and frame.frame_number > expected_frame:
            missing = [
                MissingSpan(
                    expected_block,
                    expected_block,
                    expected_frame,
                    frame.frame_number - 1,
                    frame.frame_number - expected_frame,
                )
            ]
            self._advance(frame)
            return TrackResult("gap", missing=missing)

        if 0 < block_delta < 0x80000000 and block_delta <= self.maximum_gap_blocks:
            missing = self._build_cross_block_gap(
                expected_block,
                expected_frame,
                frame.block_index,
                frame.frame_number,
                expected_count,
                block_delta,
            )
            self._advance(frame)
            return TrackResult("gap", missing=missing)

        previous = self.last
        self._advance(frame)
        return TrackResult(
            "reset",
            detail="sequence moved backward or jumped too far: previous=%s current=(%d,%d)"
            % (previous, frame.block_index, frame.frame_number),
        )

    def _advance(self, frame: Frame) -> None:
        self.last = (frame.block_index, frame.frame_number)
        if frame.frame_number < frame.frame_count:
            self.expected = (frame.block_index, frame.frame_number + 1, frame.frame_count)
        else:
            self.expected = ((frame.block_index + 1) & 0xFFFFFFFF, 1, frame.frame_count)

    def _remember(self, key: Tuple[int, int]) -> None:
        self._recent.add(key)
        self._recent_order.append(key)
        if len(self._recent_order) > self._recent_limit:
            old = self._recent_order.pop(0)
            self._recent.discard(old)

    @staticmethod
    def _build_cross_block_gap(
        expected_block: int,
        expected_frame: int,
        current_block: int,
        current_frame: int,
        frame_count: int,
        block_delta: int,
    ) -> List[MissingSpan]:
        spans = []
        if expected_frame <= frame_count:
            spans.append(
                MissingSpan(
                    expected_block,
                    expected_block,
                    expected_frame,
                    frame_count,
                    frame_count - expected_frame + 1,
                )
            )

        intermediate_blocks = block_delta - 1
        if intermediate_blocks > 0:
            start = (expected_block + 1) & 0xFFFFFFFF
            end = (current_block - 1) & 0xFFFFFFFF
            spans.append(
                MissingSpan(start, end, 1, frame_count, intermediate_blocks * frame_count)
            )

        if current_frame > 1:
            spans.append(
                MissingSpan(current_block, current_block, 1, current_frame - 1, current_frame - 1)
            )
        return spans
