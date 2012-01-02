
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int mandel(float x, float y, int order, float LIMIT) {
	int ix = 0;
	float a = 0;
	float b = 0;
	float limit = 0;
	float limix = 5;

	while (ix < LIMIT) {
		float c = a;
		float d = b;
		int i;
		for (i=1; i<order; i++) {
			float t = c;
			c = a*c - b*d;
			d = a*d + b*t;
		}
		a = c - x;
		b = d - y;
		float t = sqrt(a*a + b*b);
		if (t > LIMIT) return ix;
		ix++;
		if (t > limit) {
			limit = t;
			limix = ix;
		}
		if (ix > limix * 200) return -1;
	}
	return -1;
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

int synth_color(int x) {
	return x + 1;
}

int* synth_mandel(int width, int height, float centerx, float centery, float dx) {
	float maxx = centerx + dx/2;
	float minx = centerx - dx/2;
	float dy = dx*height / width;
	float miny = centery - dy/2;
	float maxy = centery + dy/2;
	int* bitmap = (int*)malloc(sizeof(int) * width * height);
	int i, j;
	for (i=0; i<height; i++) {
		float y = ((i * 1.0) / height) * (maxy - miny) + miny;
		for (j=0; j<width; j++) {
			float x = ((j * 1.0) / width) * (maxx - minx) + minx;
			int v = mandel(x, y, 2, 1000);
			int q = mandel(x, y, 3, 1000);
			int p = mandel(x, y, 4, 1000);
			bitmap[i * width + j] = synth_color(v / 2) + synth_color(q) * 256 + synth_color(p) * 65536 * 2;
		}
		printf(".");
		fflush(stdout);
	}
	printf("\n");
	return bitmap;
}

int main() {
	int w = 1536, h = 1024;
	int* bitmap = synth_mandel(w, h, 0.58498, 0.4531, 0.00017);
	//int* bitmap = synth_mandel(1536, 1024, 0.57, 0.60, 0.442, 0.462);
	FILE* f = fopen("test1.bmp", "wb");
	write_bitmap(f, w, h, bitmap);
	fclose(f);
}

