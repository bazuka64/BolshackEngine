#pragma once
#include "File.h"
#include "libmio0.h"

struct ROM {

	std::vector<byte> bytes;
	std::map<byte, std::vector<byte>> segments;

	ROM(const std::string& path) {
		bytes = File::ReadAllBytes(path);
	}

	void SetSegment(byte seg, uint start, uint end) {
		if (start > end)return;

		uint size = end - start;
		std::vector<byte> segment(size);
		memcpy(segment.data(), &bytes[start], size);
		segments[seg] = std::move(segment);
	}

	void SetSegmentMIO0(byte seg, uint start, uint end) {

		mio0_header_t head;
		byte* in_buf = &bytes[start];
		mio0_decode_header(in_buf, &head);
		std::vector<byte> segment(head.dest_size);
		mio0_decode(in_buf, segment.data());

		segments[seg] = std::move(segment);
	}

	static std::vector<const char*> LevelNames;
};