
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
    int i, ax, bx, mid, x, y, c;
    long long state = 5;
    int radius = width / steps;
    for (i=0; i < steps * steps; i++) {
        state += i;
        state = (state * state) % (41 * 2047);
        int up = state % steps;
        state = (state * state) % (41 * 2047);
        int down = state % steps;
        state = (state * state) % (41 * 2047);
        int color = state % 3;
        printf ("Step %d : %d / %d => %d\n", i, up, down, color);
        up = up * (width - radius) / steps;
        down = down * (width - radius) / steps;
        for (y = 0; y < height; y++) {
            ax = (y * up + (height - y) * down) / height;
            bx = ax + radius;
            for (x = ax; x < bx; x++) {
                int now = bitmap[y * width + x];
                int red = (now >> 16) & 0xFF;
                int green = (now >> 8) & 0xFF;
                int blue = now & 0xFF;

                float blend = 0.90;
                /*
                if (x*5 < width) blend = 0;
                else if (x * 3 > width * 2) blend = 1.01;
                else {
                    //blend = x * 3;
                    //blend /= width*2;
                    blend = 0.90;
                }
                */

                red   *= blend;
                green *= blend;
                blue  *= blend;

                switch (color) {
                case 0: red   += 32 + 233 * (1 - blend); break;
                case 1: green += 32 + 233 * (1 - blend); break;
                case 2: blue  += 32 + 233 * (1 - blend); break;
                }

                if (red > 255) red = 255;
                if (green > 255) green = 255;
                if (blue > 255) blue = 255;

                int val = (red << 16) | (green << 8) | blue;
            
                bitmap[y * width + x] = val;
            }
        }
    }

    int* ref = bitmap;
    bitmap = (int*)malloc(sizeof(int) * width * height);
    for (x = 0; x < width; x++) {
        //for (y = 0; y < height / 2; y++) {
        //    bitmap[y * width + x] = 0x1FFFFFF ^ ref[y * width + x];
        //    bitmap[y * width + x] &= 0x7F7F7F;
        //}
        for (y = 0; y <height; y++) {
            bitmap[y * width + x] = 0xFFFFFF ^ ref[y * width + x];
            bitmap[y * width + x] &= 0xFEFEFE;
            bitmap[y * width + x] >>= 1;
        }
    }

	for (i = 0; i<width*100; i++) {
		double phi = i*PI*2 / (width * 100);

		double r = width/4;
        int x = r*sin(phi)*sin(phi)*sin(phi);
        int y = r*(13*cos(phi) - 5*cos(phi * 2) -
                          2*cos(phi * 3) - cos(phi * 4))/16;
        int dx = 1, dy = 1;
        int j;
        for (j = 0; j < width; j++) {
            int sx = width / 2 + (x * j) / width;
            int sy = height / 2 + (y * j) / width;
            float redf = width/2 + radius*1.7 - sx;
            redf /= radius * 2.8;
            int red = (int)(redf * 255);
            int blue = 255 - red;
            float greenf = sy;
            greenf /= height;
            int green = (int)(greenf * 255);
            bitmap[sx + sy*width] = ref[sy * width + sx];
        }
	}
	return bitmap;
}

int main() {
	int scale = 2048;
	int* bitmap = synth_cell(scale * 3, scale * 2, 100);
	FILE* f = fopen("test1.bmp", "wb");
	write_bitmap(f, scale * 3, scale * 2, bitmap);
	fclose(f);
}

