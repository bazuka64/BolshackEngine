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
		mio0_decode(in_buf, segment.data());

		segments[seg] = std::move(segment);
	}
};

std::vector<const char*> LevelNum = {
	"LEVEL_NONE",
	"LEVEL_UNKNOWN_1",
	"LEVEL_UNKNOWN_2",
	"LEVEL_UNKNOWN_3",
	"Big Boo's Haunt",
	"Cool, Cool Mountain",
	"Castle Inside",
	"Hazy Maze Cave",
	"Shifting Sand Land",
	"Bob-omb Battlefield",
	"Snowman's Land",
	"Wet-Dry World",
	"Jolly Roger Bay",
	"Tiny-Huge Island",
	"Tick Tock Clock",
	"Rainbow Ride",
	"Castle Grounds",
	"Bowser in the Dark World",
	"Vanish Cap Under the Moat",
	"Bowser in the Fire Sea",
	"The Secret Aquarium",
	"Bowser in the Sky",
	"Lethal Lava Land",
	"Dire, Dire Docks",
	"Whomp's Fortress",
	"Ending Cake",
	"Castle Courtyard",
	"The Princess's Secret Slide",
	"Cavern of the Metal Cap",
	"Tower of the Wing Cap",
	"Bowser 1",
	"Wing Mario Over the Rainbow",
	"LEVEL_UNKNOWN_32",
	"Bowser 2",
	"Bowser 3",
	"LEVEL_UNKNOWN_35",
	"Tall, Tall Mountain",
	"LEVEL_UNKNOWN_37",
	"LEVEL_UNKNOWN_38",
};

