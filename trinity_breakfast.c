
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

int* synth_image(int width, int height, int radius) {
	int* bitmap = (int*)malloc(sizeof(int) * height * width);
	int i, j;

    int focus = (int)floor(((float)radius) / sqrt(2));

    for (i = 0; i<width; i++) {
        for (j=0; j<height; j++) {
            int p = i - (width / 2);
            int q = j - (height / 2);

            p = p*p + q*q;

            if (p < radius*radius*5) {
                if (p < radius*radius*2) {
                    bitmap[i + j * width] = 0xFFFFD0;
                } else {
                    double phi = q*q;
                    phi = acos(sqrt(phi/p)) * 20;
                    int shift = (int)(width / 20.0) * sin(phi);

                    if (p + shift*shift*shift < radius*radius*3) {
                        bitmap[i + j * width] = 0x000080;
                    } else { 
                        bitmap[i + j * width] = 0xFFFF00;
                    }
                }
                continue;
            } else {
                p = (i * 20) / width;
                q = (j * 13) / height;
                if ((p + q) % 2 == 0) {
                    bitmap[i + j * width] = 0xFFFFFF;
                } else {
                    bitmap[i + j * width] = 0x60FF40;
                }
            }
        }
    }

	for (i = 0; i<width*100; i++) {
		double phi = i*PI*2 / (width * 100);
		//if ((PI/4 <= phi) && (phi <= 3*PI/4)) continue;
		//if ((PI*5/4 <= phi) && (phi <= 7*PI/4)) continue;

		double r = radius*cos(3*phi);
		int x = width/2 + r*cos(phi)*3/2;
		int y = height/2 + r*sin(phi)*3/2;
        int dx, dy;
        dy = y - height/2;
        if (x > width/2) {
            dx = (x - width/2 - focus);
            dy = (int)(dy * 1.9);
        } else {
            dx = (x - width/2 + focus);
        }
        //dx *= sin(phi * 7 + 1)/2 + 0.5;
        //dy *= sin(phi * 7 + 1)/2 + 0.5;
        for (j = -width; j < width; j++) {
            int sx = x + (dx * j) / (width * 10);
            int sy = y + (dy * j) / (width * 10);
            float redf = width/2 + radius*1.7 - sx;
            redf /= radius * 2.8;
            int red = (int)(redf * 255);
            int blue = 255 - red;
            float greenf = sy;
            greenf /= height;
            int green = (int)(greenf * 255);
            bitmap[sx + sy*width] = blue + green * 256 + red * 65536;
        }
	}
	return bitmap;
}

int main() {
	int scale = 2048;
	int* bitmap = synth_image(scale * 3, scale * 2, scale / 2);
	FILE* f = fopen("test1.bmp", "wb");
	write_bitmap(f, scale * 3, scale * 2, bitmap);
	fclose(f);
}

