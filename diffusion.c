
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

typedef struct {
        float x1;
        float y1;
        float x2;
        float y2;
        int color;
} vector;

int* synth_cell(int width, int height, int steps) {
        int* bitmap = (int*)malloc(sizeof(int) * width * height);
        vector* dashes = (vector*)malloc(sizeof(vector) * steps);
        dashes[0].x1 = 0;
        dashes[0].y1 = 0;
        dashes[0].x2 = 1;
        dashes[0].y2 = 1;
	dashes[0].color = 0x808080;
	int i, fill = 1;
    float bottom = 1;
	for (i=0; ; i++) {
		float dx = - (dashes[i].y2 - dashes[i].y1) / 2;
		float dy = (dashes[i].x2 - dashes[i].x1) / 2;

        dx *= 1.1;
        dy *= 1.1;

		dashes[fill].x1 = (dashes[i].x1 + dashes[i].x2) / 2;
		dashes[fill].y1 = (dashes[i].y1 + dashes[i].y2) / 2;
		dashes[fill].x2 = dashes[fill].x1 + dx;
		dashes[fill].y2 = dashes[fill].y1 + dy;
		fill++;
		if (fill == steps) break;
		dashes[fill] = dashes[fill - 1];
		dashes[fill].x2 -= dx * 2;
		dashes[fill].y2 -= dy * 2;
		dashes[fill].color = dashes[i].color + 65536 * 15 + 256 * 5 - 10;
		fill++;
		if (fill == steps) break;

        dashes[fill-2].x1 = (dashes[i].x1 * 3 + dashes[i].x2) / 4;
        dashes[fill-2].y1 = (dashes[i].y1 * 3 + dashes[i].y2) / 4;
		dashes[fill-2].color = dashes[i].color - 65536 * 10 + 256 * 10 - 10;
        dashes[fill] = dashes[fill-2];
        dashes[fill].x1 = (dashes[i].x1 + dashes[i].x2 * 3) / 4;
        dashes[fill].y1 = (dashes[i].y1 + dashes[i].y2 * 3) / 4;
		dashes[fill].color = dashes[i].color - 65536 * 10 - 256 * 10 + 10;
        fill++;
        if (fill == steps) break;
	}
	float maxx = 0, maxy = 0, minx = 0, miny = 0;
	for (i=0; i<steps; i++) {
		if (maxx <= dashes[i].x1) maxx = dashes[i].x1;
		if (minx >= dashes[i].x1) minx = dashes[i].x1;
		if (maxx <= dashes[i].x2) maxx = dashes[i].x2;
		if (minx >= dashes[i].x2) minx = dashes[i].x2;
		//
		if (maxy <= dashes[i].y1) maxy = dashes[i].y1;
		if (miny >= dashes[i].y1) miny = dashes[i].y1;
		if (maxy <= dashes[i].y2) maxy = dashes[i].y2;
		if (miny >= dashes[i].y2) miny = dashes[i].y2;
	}

	maxx += 0.2;
	maxy += 0.2;
	minx -= 0.2;
	miny -= 0.2;

	int max = width + height;
	for (i=0; i<steps; i++) {
        float dx = dashes[i].x1 - dashes[i].x2;
        float dy = dashes[i].y1 - dashes[i].y2;
        float metric = fabs(dx) + fabs(dy);
        if (metric > 0.02) continue;
		int j;
        int screen_metric = ((dx / (maxx - minx)) * width +
                             (dy / (maxy - miny)) * height) * 2;
        int pixels = (screen_metric < max) ? screen_metric : max;
		for (j=0; j<=max; j++) {
			float x = (dashes[i].x1 * (max - j) + dashes[i].x2 * j) / max;
			float y = (dashes[i].y1 * (max - j) + dashes[i].y2 * j) / max;

			x = (x - minx) / (maxx - minx);
			y = (y - miny) / (maxy - miny);

            if ( !(x < 1.0))
            {
                printf("%f | %f..%f\n", x, dashes[i].x1, dashes[i].x2);
            }
			assert(x < 1.0);
			assert(x >= 0.0);
			if (y >= 1.0) {
				printf("y = %f [%f..%f]\n", y, miny, maxy);
				assert(y < 1.0);
			}
			assert(y >= 0.0);

			int screen_x = x * width;
			int screen_y = y * height;

			bitmap[screen_y * width + screen_x] = dashes[i].color;
		}
	}
	
	printf("\n");
	return bitmap;
}

int main() {
    sranddev();

	int scale = 256; //3072;
	int* bitmap = synth_cell(scale * 3, scale * 2, 250000);
	FILE* f = fopen("test1.bmp", "wb");
	write_bitmap(f, scale * 3, scale * 2, bitmap);
	fclose(f);
}

