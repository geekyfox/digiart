
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
//		printf("%f / %f | %f, %f\n", value, sum, x, y);
		value = r*value*(1 - value);
//		printf("%f --- %f\n", value, r * (1 - 2*value));
		sum += log(fabs(r*(1 - 2*value)));
		code_ix++;
		if (code[code_ix] == '\0') code_ix = 0;
	}
	sum /= steps;
//	printf("result => %f\n", sum);
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
		color = -x * 50;
		if (color > 255) color = 255;
		return color * shift;
	} else {
		color = x * 600;
		if (color > 255) color = 255;
		return color;
	}
}

int* synth_mandel(int width, int height, float minx, float maxx, float miny, float maxy) {
	int* bitmap = (int*)malloc(sizeof(int) * width * height);
	int i, j;
	for (i=0; i<height; i++) {
		float y = ((i * 1.0) / height) * (maxy - miny) + miny;
		for (j=0; j<width; j++) {
			float x = ((j * 1.0) / width) * (maxx - minx) + minx;
			float v = lyapunov(x, y, 1000, "AABABBB");
			float q = lyapunov(x, y, 1000, "AABABAA");
			bitmap[i * width + j] = synth_color(v, 65536) + synth_color(q, 256);
		}
		printf(".");
		fflush(stdout);
	}
	printf("\n");
	return bitmap;
}

int main() {
	int* bitmap = synth_mandel(6144, 4096, 2.5, 4, 3, 4);
	FILE* f = fopen("test1.bmp", "wb");
	write_bitmap(f, 6144, 4096, bitmap);
	fclose(f);
}

