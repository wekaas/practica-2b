#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

int main() {

	int tc_fd;
	int tm_fd;

	tc_fd = open("multiple-tcs.bin", O_RDONLY);
	tm_fd = open("multiple-tms.bin", O_WRONLY | O_CREAT | O_TRUNC, 0664);

	uint8_t read_byte;
	uint8_t ntcs;

	read(tc_fd, &read_byte, 1);
	ntcs = read_byte;

	//VARIABLES DEL BUCLE:

	uint16_t tm_count = 0;
	uint8_t tc = 0;

	//BUCLE:

	while (tc < ntcs) {

		// TODO: Initialize variables

		uint16_t tc_packet_id;
		uint16_t tc_packet_seq_ctrl;
		uint16_t tc_packet_len;
		uint32_t tc_df_header;
		uint16_t tc_packet_err_ctrl;
		uint8_t tc_bytes[256];
		uint8_t byte_to_write;

		uint16_t tm_packet_id = 0;
		uint16_t tm_packet_seq_ctrl;
		uint16_t tm_packet_len;
		uint32_t tm_df_header = 0;
		uint32_t tm_source_data = 0;
		uint16_t error_type;

		// TODO: Read the telecomand, store its contents
		//       into the vector tc_data and the variables and
		//       print the values of the different fields

		//LECTURA:

		//Packet ID

		read(tc_fd, &read_byte, 1);

		tc_bytes[0] = read_byte;

		tc_packet_id = read_byte;
		tc_packet_id = tc_packet_id << 8;

		read(tc_fd, &read_byte, 1);

		tc_bytes[1] = read_byte;

		tc_packet_id = tc_packet_id | read_byte;

		//PACKET SEQUENCE CONTROL:

		read(tc_fd, &read_byte, 1);

		tc_bytes[2] = read_byte;

		tc_packet_seq_ctrl = read_byte;
		tc_packet_seq_ctrl = tc_packet_seq_ctrl << 8;

		read(tc_fd, &read_byte, 1);

		tc_bytes[3] = read_byte;

		tc_packet_seq_ctrl = tc_packet_seq_ctrl | read_byte;

		//PACKET LENGTH

		read(tc_fd, &read_byte, 1);

		tc_bytes[4] = read_byte;

		tc_packet_len = read_byte;
		tc_packet_len = tc_packet_len << 8;

		read(tc_fd, &read_byte, 1);

		tc_bytes[5] = read_byte;

		tc_packet_len = tc_packet_len | read_byte;

		//DATA FIELD HEADER

		read(tc_fd, &read_byte, 1);

		tc_bytes[6] = read_byte;

		tc_df_header = read_byte;
		tc_df_header = tc_df_header << 8;

		read(tc_fd, &read_byte, 1);

		tc_bytes[7] = read_byte;

		tc_df_header = tc_df_header | read_byte;
		tc_df_header = tc_df_header << 8;

		read(tc_fd, &read_byte, 1);

		tc_bytes[8] = read_byte;

		tc_df_header = tc_df_header | read_byte;
		tc_df_header = tc_df_header << 8;

		read(tc_fd, &read_byte, 1);

		tc_bytes[9] = read_byte;

		tc_df_header = tc_df_header | read_byte;

		// TODO: Calculate the telecommand's CRC

		uint16_t i = 0;

		while (i < tc_packet_len - 5) {

			read(tc_fd, &read_byte, 1);

			tc_bytes[10 + i] = read_byte;

			i = i + 1;

		}

		//PACKET ERROR CONTROL

		read(tc_fd, &read_byte, 1);        // Read MSB
		tc_packet_err_ctrl = read_byte;
		tc_packet_err_ctrl = tc_packet_err_ctrl << 8; // Shift it 8 bits to the left

		read(tc_fd, &read_byte, 1);               // Read LSB
		tc_packet_err_ctrl = tc_packet_err_ctrl | read_byte; // OR the MSB and the LSB

		uint16_t nbytes = tc_packet_len + 5;

		uint16_t crc_value = 0xFFFF;

		for (uint16_t i = 0; i < nbytes; i++) {

			crc_value = crc_value ^ (tc_bytes[i] << 8);

			for (uint8_t j = 0; j < 8; j++) {

				if ((crc_value & 0x8000) != 0) {
					crc_value = (crc_value << 1) ^ 0x1021;
				} else {
					crc_value = crc_value << 1;
				}
			}
		}

		//IMPRIMO LOS VALORES DE CADA COSA:

		//Cada uno de los paquetes:

		printf("Packet ID: 0x%X\n", tc_packet_id);
		printf("Packet sequence control: 0x%X\n", tc_packet_seq_ctrl);
		printf("Packet length: 0x%X\n", tc_packet_len);
		printf("Data Field Header: 0x%X\n", tc_df_header);
		printf("Packet error control: 0x%X\n", tc_packet_err_ctrl);

		//Los otros:

		printf("APID: 0x%X\n", tc_packet_id & 0x07FF);
		printf("Secuence flags: 0x%X\n", (tc_packet_seq_ctrl & 0xC000) >> 14);
		printf("Secuence count: %d\n", (tc_packet_seq_ctrl & 0x3FFF));
		printf("Ack: 0x%X\n", (tc_df_header & 0x0F000000) >> 24);
		printf("Service type: %d\n", (tc_df_header & 0x00FF0000) >> 16);
		printf("Service Subtype: %d\n", (tc_df_header & 0x0000FF00) >> 8);
		printf("Source ID: 0x%X\n", tc_df_header & 0x000000FF);

		// Compare the value of the calculated CRC with
		// the Packet Error Control field of the telecommand
		if (crc_value == tc_packet_err_ctrl) {

			// TODO: Output OK message

			printf("Expected CRC value 0x%X, Calculated CRC value 0x%X: OK\n", tc_packet_err_ctrl, crc_value);

			// TODO: Generate Acceptance Telemetry Packet (PERFECTA)

			//PACKET ID:

			tm_packet_id = tm_packet_id | (1 << 11);

			tm_packet_id = tm_packet_id | (0x32C);

			byte_to_write = (tm_packet_id & 0xFF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_packet_id & 0x00FF);
			write(tm_fd, &byte_to_write, 1);

			//PACKET SEQUENCE CONTROL:

			tm_packet_seq_ctrl = (3 << 14) | tm_count;

			byte_to_write = (tm_packet_seq_ctrl & 0xFF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_packet_seq_ctrl & 0x00FF);
			write(tm_fd, &byte_to_write, 1);

			//PACKET LENGTH:

			tm_packet_len = 0x0007;

			byte_to_write = (tm_packet_len & 0xFF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_packet_len & 0x00FF);
			write(tm_fd, &byte_to_write, 1);

			//PACKET DATA FIELD HEADER:

			tm_df_header = tm_df_header | (1 << 28) | (1 << 16) | (1 << 8) | 0x78;

			byte_to_write = (tm_df_header & 0xFF000000) >> 24;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_df_header & 0x00FF0000) >> 16;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_df_header & 0x0000FF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_df_header & 0x000000FF);
			write(tm_fd, &byte_to_write, 1);

			//SOURCE DATA:

			tm_source_data = tm_source_data | (1 << 28) | (1 << 27) | (0x32C << 16) | (0x3 << 14);

			byte_to_write = (tm_source_data & 0xFF000000) >> 24;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_source_data & 0x00FF0000) >> 16;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_source_data & 0x0000FF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_source_data & 0x000000FF);
			write(tm_fd, &byte_to_write, 1);

		} else {

			// TODO: Output FAIL message

			printf("Expected CRC value 0x%X, Calculated CRC value 0x%X: FAIL\n",
					tc_packet_err_ctrl, crc_value);

			// TODO: Generate Reject Telemetry Packet

			//PACKET ID:

			tm_packet_id = tm_packet_id | (1 << 11);

			tm_packet_id = tm_packet_id | (0x32C);

			byte_to_write = (tm_packet_id & 0xFF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_packet_id & 0x00FF);
			write(tm_fd, &byte_to_write, 1);

			//PACKET SEQUENCE CONTROL:

			tm_packet_seq_ctrl = (3 << 14) | tm_count;

			byte_to_write = (tm_packet_seq_ctrl & 0xFF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_packet_seq_ctrl & 0x00FF);
			write(tm_fd, &byte_to_write, 1);

			//PACKET LENGTH:

			tm_packet_len = 0x000D;

			byte_to_write = (tm_packet_len & 0xFF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_packet_len & 0x00FF);
			write(tm_fd, &byte_to_write, 1);

			//PACKET DATA FIELD HEADER:

			tm_df_header = tm_df_header | (1 << 28) | (1 << 16) | (2 << 8)
					| 0x78;

			byte_to_write = (tm_df_header & 0xFF000000) >> 24;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_df_header & 0x00FF0000) >> 16;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_df_header & 0x0000FF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tm_df_header & 0x000000FF);
			write(tm_fd, &byte_to_write, 1);

			//SOURCE DATA:

			byte_to_write = (tc_packet_id & 0xFF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tc_packet_id & 0x00FF);
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tc_packet_seq_ctrl & 0xFF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tc_packet_seq_ctrl & 0x00FF);
			write(tm_fd, &byte_to_write, 1);

			error_type = 0x0002;

			byte_to_write = (error_type & 0xFF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (error_type & 0x00FF);
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tc_packet_err_ctrl & 0xFF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (tc_packet_err_ctrl & 0x00FF);
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (crc_value & 0xFF00) >> 8;
			write(tm_fd, &byte_to_write, 1);

			byte_to_write = (crc_value & 0x00FF);
			write(tm_fd, &byte_to_write, 1);

		}

		// TODO: Increment telemetry count (tm_count = tm_count + 1)

		tm_count = tm_count + 1;

		// TODO: Increment index variable (tc = tc + 1)

		tc = tc + 1;

	}

	close(tc_fd);
	close(tm_fd);

	return 0;

}
