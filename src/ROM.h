#pragma once

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
		mio0_decode(in_buf, segment.data(), NULL);

		segments[seg] = std::move(segment);
	}
};

std::vector<const char*> LevelNum  = {
	"LEVEL_NONE",
	"LEVEL_UNKNOWN_1",
	"LEVEL_UNKNOWN_2",
	"LEVEL_UNKNOWN_3",
	"LEVEL_BBH",
	"LEVEL_CCM",
	"LEVEL_CASTLE",
	"LEVEL_HMC",
	"LEVEL_SSL",
	"LEVEL_BOB",
	"LEVEL_SL",
	"LEVEL_WDW",
	"LEVEL_JRB",
	"LEVEL_THI",
	"LEVEL_TTC",
	"LEVEL_RR",
	"LEVEL_CASTLE_GROUNDS",
	"LEVEL_BITDW",
	"LEVEL_VCUTM",
	"LEVEL_BITFS",
	"LEVEL_SA",
	"LEVEL_BITS",
	"LEVEL_LLL",
	"LEVEL_DDD",
	"LEVEL_WF",
	"LEVEL_ENDING",
	"LEVEL_CASTLE_COURTYARD",
	"LEVEL_PSS",
	"LEVEL_COTMC",
	"LEVEL_TOTWC",
	"LEVEL_BOWSER_1",
	"LEVEL_WMOTR",
	"LEVEL_UNKNOWN_32",
	"LEVEL_BOWSER_2",
	"LEVEL_BOWSER_3",
	"LEVEL_UNKNOWN_35",
	"LEVEL_TTM",
	"LEVEL_UNKNOWN_37",
	"LEVEL_UNKNOWN_38",
};

