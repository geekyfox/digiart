
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void write_bytes(FILE* f, int value, int bytes) {
	int i;
	for (i=0; i<bytes; i++) {
		fputc(value % 256, f);
		value /= 256;
	}
}

#define WRITESHO(x) write_bytes(f, x, 2)
#define WRITEINT(x) write_bytes(f, x, 4)

void write_header(FILE* f, int width, int height) {
	int byte_width = width * 3;
	while (byte_width % 4) byte_width++;
	int bitmap_size = byte_width * height;
	WRITESHO(0x4D42);
	WRITEINT(bitmap_size + 54);
	WRITEINT(0);
	WRITEINT(54);
	WRITEINT(40);
	WRITEINT(width);
	WRITEINT(height);
	WRITESHO(1);
	WRITESHO(24);
	WRITEINT(0);
	WRITEINT(bitmap_size);
	WRITEINT(2835);
	WRITEINT(2835);
	WRITEINT(0);
	WRITEINT(0);
}

void write_bitmap(FILE* f, int width, int height, int* data) {
	write_header(f, width, height);
	int i, j, pad;
	for (i=0; i<height; i++) {
		pad = 0;
		for (j=0; j<width; j++) {
			write_bytes(f, *data, 3);
			data++;
			pad += 3;
		}
		while(pad % 4) {
			write_bytes(f, 0, 1);
			pad++;
		}
	}
}

int synth_color(int x) {
	return x + 1;
}

#define PI 3.141592

int prime(int x) {
	int y = 2;
	while (1) {
		if (x < y*y) return 1;
		if (x % y == 0) return 0;
		y++;
	}
}

int* synth_cell(int width, int height, int steps) {
	int* bitmap = (int*)malloc(sizeof(int) * width * height);
	int i, j, k;

	for (i=height * width - 1; i>=0; i--) bitmap[i] = 48 + 256*16;
	int color = 0;
	int new_color;
	float radius = height / 20;
	float center_x = width / 2 - radius;
	float center_y = height / 2;
	float x = center_x + radius;
	float y = center_y;
	float angle = 0;
	float delta_angle = 2 * PI / 5;

	for (k=0; k<steps; k++) {
		if ( ! prime(k + 2) ) {
			center_x = 2 * x - center_x;
			center_y = 2 * y - center_y;
			angle = angle + PI;
			delta_angle = -delta_angle;
		}

		if (prime(2*k + 3)) {
			new_color = (color == 5) ? 0 : color+1;
		} else if (prime(3*k + 5)) {
			new_color = (color > 3) ? color - 4 : color + 2;
		} else {
			new_color = color;
		}

		for (i=0; i<501; i++) {
			int first = round(255.0 * (i / 500.0));
			int red = 0, green = 0, blue = 0;
			switch (new_color) {
			case 0:
				red += first; break;
			case 1:
				green += first; break;
			case 2:
				blue += first; break;
			case 3:
				red += first; green += first; break;
			case 4:
				red += first; blue += first; break;
			case 5:
				green += first; blue += first; break;
			}
			int second = round(255.0 * (500.0 - i) / 500.0);
			switch (color) {
			case 0:
				red += second; break;
			case 1:
				green += second; break;
			case 2:
				blue += second; break;
			case 3:
				red += second; green += second; break;
			case 4:
				red += second; blue += second; break;
			case 5:
				green += second; blue += second; break;
			}
			if (red > 255) red = 255;
			if (green > 255) green = 255;
			if (blue > 255) blue = 255;
			int screen_color = red * 65536 + green * 256 + blue;

			float current_angle = angle + delta_angle * i / 500.0;
			for (j=0; j<501; j++) {
				x = cos(current_angle) * radius * (0.8 + j / 1250.0) + center_x;
				y = sin(current_angle) * radius * (0.8 + j / 1250.0) + center_y;

				int screen_x = round(x);
				while (screen_x < 0) screen_x += width;
				while (screen_x >= width) screen_x -= width;

				int screen_y = round(y);
				while (screen_y < 0) screen_y += height;
				while (screen_y >= height) screen_y -= height;

				int cell = screen_y * width + screen_x;
				assert(screen_y < height);
				assert(cell < width * height);
				assert(cell >= 0);
				bitmap[cell] = screen_color;
			}
			x = cos(current_angle) * radius + center_x;
			y = sin(current_angle) * radius + center_y;
		}
		angle += delta_angle;
		color = new_color;

		printf(".");
		fflush(stdout);
	}
	printf("\n");
	return bitmap;
}

int main() {
	int scale = 2048;
	int* bitmap = synth_cell(scale * 3, scale * 2, 100);
	FILE* f = fopen("test1.bmp", "wb");
	write_bitmap(f, scale * 3, scale * 2, bitmap);
	fclose(f);
}

