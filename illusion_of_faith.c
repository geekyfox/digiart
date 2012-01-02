
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

float lyapunov(float x, float y, int steps, char* code) {
	int code_ix = 0;
	float sum = 0;
	int i;
	float value = 0.5;
	for (i=0; i<steps; i++) {
		float r = (code[code_ix] == 'A' ? x : y);
		value = r*value*(1 - value);
		sum += log(fabs(r*(1 - 2*value)));
		code_ix++;
		if (code[code_ix] == '\0') code_ix = 0;
	}
	sum /= steps;
	return sum;
}

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

int synth_color(float x, int shift) {
	int color;
	if (x < 0) {
		color = -x * 400;
		if (color > 255) color = 255;
		return color * shift;
	} else {
		color = x * 400;
		if (color > 255) color = 255;
		return color * 256;
	}
}

int* synth_mandel(int width, int height, float minx, float maxx, float miny, float maxy) {
	int* bitmap = (int*)malloc(sizeof(int) * width * height);
	int i, j;
	for (i=0; i<height; i++) {
		float y = ((i * 1.0) / height) * (maxy - miny) + miny;
		float y2 = maxy + miny - y;
		for (j=0; j<width; j++) {
			float x = ((j * 1.0) / width) * (maxx - minx) + minx;
			float x2 = maxx + maxy - x;
			float v = lyapunov(x, y, 1000, "AABABBBABB");
//			float q = lyapunov(x, y, 1000, "AABABAABAA");
			float t = - lyapunov(x, y, 1000, "BABABBBA");
			bitmap[i * width + j] = synth_color(v, 65536) | synth_color(t, 1);
		}
		printf(".");
		fflush(stdout);
	}
	printf("\n");
	return bitmap;
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

int* synth_image(int width, int height, int radius) {
	int* bitmap = synth_mandel(width, height, 3.55, 4, 2.9, 3.2);
	int i, j;
	int cx = width / 2;
	int cy = height/2 + height/8;

	int cell = width / 32;

	for (i = 0; i<width*16; i++) {
		double phi = i*PI*2 / (width * 16);
		double r = 2*radius*cos(2*phi);
		int x = r*cos(phi);
		int y = r*sin(phi);
		int level = 255;
		if ((PI/4 <= phi) && (phi <= 3*PI/4)) y = y*3 / 2; 
		for (j = 0; j < width; j++) {
			int x1 = (x * j) / width + cx;
			int y1 = (y * j) / width + cy;
			int red, green, blue;
			//
			int ix_x = x1 - cx;
			ix_x = (ix_x < 0) ? (ix_x / cell) : ((ix_x / cell) - 1);
			int ix_y = y1 - cy;
			ix_y = (ix_y < 0) ? (ix_y / cell) : ((ix_y / cell) - 1);
			if ( (ix_x + ix_y) % 2 == 0) {
				red = green = blue = 0;
			} else {
				red = green = blue = 255;
			}
			//
			bitmap[x1 + y1*width] = red * 65536 + green * 256 + blue;
		}
	}
	return bitmap;
}

int main() {
	int scale = 2048;
	int* bitmap = synth_image(scale * 3, scale * 2, scale / 3);
	FILE* f = fopen("test1.bmp", "wb");
	write_bitmap(f, scale * 3, scale * 2, bitmap);
	fclose(f);
}

