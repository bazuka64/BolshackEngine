#pragma once

#define MIO0_HEADER_LENGTH 16
#define GET_BIT(buf, bit) ((buf)[(bit) / 8] & (1 << (7 - ((bit) % 8))))
#define read_u32_be(buf) (unsigned int)(((buf)[0] << 24) + ((buf)[1] << 16) + ((buf)[2] << 8) + ((buf)[3]))

typedef struct {
	unsigned int dest_size;
	unsigned int comp_offset;
	unsigned int uncomp_offset;
} mio0_header_t;

void mio0_decode_header(const unsigned char* buf, mio0_header_t* head) {
	head->dest_size = read_u32_be(&buf[4]);
	head->comp_offset = read_u32_be(&buf[8]);
	head->uncomp_offset = read_u32_be(&buf[12]);
}

void mio0_decode(const unsigned char* in, unsigned char* out) {

	mio0_header_t head;
	unsigned int bytes_written = 0;
	int bit_idx = 0;
	int comp_idx = 0;
	int uncomp_idx = 0;

	mio0_decode_header(in, &head);

	while (bytes_written < head.dest_size) {
		if (GET_BIT(&in[MIO0_HEADER_LENGTH], bit_idx)) {
			// 1 - pull uncompressed data
			out[bytes_written] = in[head.uncomp_offset + uncomp_idx];
			bytes_written++;
			uncomp_idx++;
		}
		else {
			// 0 - read compressed data
			int idx;
			int length;
			int i;
			const unsigned char* vals = &in[head.comp_offset + comp_idx];
			comp_idx += 2;
			length = ((vals[0] & 0xF0) >> 4) + 3;
			idx = ((vals[0] & 0x0F) << 8) + vals[1] + 1;
			for (i = 0; i < length; i++) {
				out[bytes_written] = out[bytes_written - idx];
				bytes_written++;
			}
		}
		bit_idx++;
	}
}