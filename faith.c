
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
		return color * 256;
	}
}

int* synth_mandel(int width, int height, float minx, float maxx, float miny, float maxy) {
	int* bitmap = (int*)malloc(sizeof(int) * width * height);
	int i, j;
	for (i=0; i<height; i++) {
		float y = ((i * 1.0) / height) * (maxy - miny) + miny;
		for (j=0; j<width; j++) {
			float x = ((j * 1.0) / width) * (maxx - minx) + minx;
			float v = lyapunov(x, y, 1000, "AABABBBA");
			float q = lyapunov(x, y, 1000, "AABABAAB");
			bitmap[i * width + j] = synth_color(v, 65536) + synth_color(q, 256);
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
			int red = 0, green = 0, blue = 0;
			if (j < width / 4) {
				blue = 255 - 192*j*4/width;
				red = 64*j*4/width;
				green = 64*j*4/width;
			} else if (j < width*2 / 4) {
				blue = 64 + 192*(j*4 - width)/width;
				red = blue;
				green = red;
			} else if (j < width*3 / 4) {
				// blue(1/2) = 64 + 64*(2w - w)/w = 128, blue(3/4) = 0
				// red(1/2) = green(1/2) = (blue(1/2) - 64) * 2 = 64
				// red(3/4) = green(3/4) = 255
				blue = 255 - 255*(j*4 - width*2)/width;
				red = 255;//128 + 128*(j*4 - width*2)/width;
				green = red;
			} else {
				blue = 0;
				red = 255 - 32*(j*4 - width*3)/width;
				green = 255 - 127*(j*4 - width*3)/width;
			}
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

