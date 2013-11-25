
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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

int power(int enc)
{
    int blue = enc & 0xFF;
    int green = (enc >> 8) & 0xFF;
    int red = enc >> 16;
    int max = (blue < green) ? green : blue;
    max = (max < red) ? red : max;
    int min = (blue < green) ? blue : green;
    min = (min < red) ? min : red;
    return blue + green + red;
}

int pre(void)
{
    int blue = random() % 255;
    int green = random() % 255;
    int red = random() % 255;

    int max = (blue < green) ? green : blue;
    max = (max < red) ? red : max;
    if (blue == max) blue /= 1.5;
    if (green == max) green /= 1.5;
    if (red == max) red /= 1.5;

    return (red << 16) | (green << 8) | blue;
}

int sharpen(x)
{
    if (x < 192) {
        return x;
    }
    return (x - 128) * 2;
}

int post(int enc)
{
    int blue = enc & 0xFF;
    int green = (enc >> 8) & 0xFF;
    int red = enc >> 16;

    blue = sharpen(blue);
    red = sharpen(red);
    green = sharpen(green);

    return (red << 16) | (green << 8) | blue;
}

int blend(int enc1, int w1, int enc2, int w2)
{
    if (enc1 == enc2) return enc1;
    if ((enc1 == 0) && (enc2 == 0)) return 0;

    int blue1 = enc1 & 0xFF;
    int green1 = (enc1 >> 8) & 0xFF;
    int red1 = enc1 >> 16;

    int blue2 = enc2 & 0xFF;
    int green2 = (enc2 >> 8) & 0xFF;
    int red2 = enc2 >> 16;

    int blue = (blue1 * w1 + blue2 * w2) / (w1 + w2);
    int green = (green1 * w1 + green2 * w2) / (w1 + w2);
    int red = (red1 * w1 + red2 * w2) / (w1 + w2);

    return (red << 16) | (green << 8) | blue;
}

int* synth(int width, int height, int iters)
{
    int* data = malloc(width * height * sizeof(int));
    int* data_tmp = malloc(width * height * sizeof(int));
    int i, x, y, ix;
    for (i=0; i<width*height; i++)
    {
        data[i] = pre();
    }
    for (i=1; i<=iters; i++)
    {
        for (y=0; y<height; y++)
        {
            for (x=0; x<width; x++)
            {
                ix = y * width + x;
                int value = data[ix];
                int pwr = power(value), pwr2;
                if (x > 0) {
                    pwr2 = power(data[ix - 1]);
                    if (pwr2 > pwr) {
                        value = data[ix - 1];
                        pwr = pwr2;
                    }
                }
                if (x < (width - 1)) {
                    pwr2 = power(data[ix + 1]);
                    if (pwr2 > pwr) {
                        value = data[ix + 1];
                        pwr = pwr2;
                    }
                }
                if (y > 0) {
                    pwr2 = power(data[ix - width]);
                    if (pwr2 > pwr) {
                        value = data[ix - width];
                        pwr = pwr2;
                    }
                }
                if (y < (height - 1)) {
                    pwr2 = power(data[ix + width]);
                    if (pwr2 > pwr) {
                        value = data[ix + width];
                        pwr = pwr2;
                    }
                }
                if (i == iters)
                {
                    if (data[ix] == value)
                    {
                        value = post(value);
                    }
                    else
                    {
                        value = 0;
                    }
                }
                data_tmp[ix] = value; //blend(data[ix], value);
            }
        }
        int* t = data;
        data = data_tmp;
        data_tmp = t;
    }
    free(data_tmp);
    return data;
}

int* scale(int* src, int boost, int width, int height)
{
    int full_width = (width - 1) * boost + 1;
    int full_height = (height - 1) * boost + 1;
    int* result = malloc(full_width * full_height * sizeof(int));
    int i, x, y;
    for (i=0; i<full_width * full_height; i++)
    {
        result[i] = 0;
    }
    for (x=0; x<width; x++) {
        for (y=0; y<height; y++) {
            int s_ix = x + y * width;
            int d_ix = x * boost + y * boost * full_width;
            result[d_ix] = src[s_ix];
            if (y == 0) continue;
            int u_ix = s_ix - width;
            for (i=1; i<boost; i++)
            {
                d_ix -= full_width;
                result[d_ix] = blend(src[s_ix], boost - i, src[u_ix], i);
            }
        }
    }
    for (x=0; x<full_width; x++) {
        int p = x % boost;
        if (p == 0) continue;
        int q = boost - p;
        for (y=0; y<full_height; y++) {
            int a_ix = (x - p) + y * full_width;
            int b_ix = (x + q) + y * full_width;
            int c_ix = x + y * full_width;
            result[c_ix] = blend(result[a_ix], q, result[b_ix], p);
        }
    }
    free(src);
    return result;
}

int main() {
    srandom(0);
    int multi = 512;
    int boost = 4;
	int* bitmap = synth(multi * 3 + 1, multi * 2 + 1, multi / 8);
    if (boost != 1) {
        bitmap = scale(bitmap, boost, multi * 3 + 1, multi * 2 + 1);
    }
	FILE* f = fopen("test1.bmp", "wb");
	write_bitmap(f, multi * 3 * boost + 1, multi * 2 * boost + 1, bitmap);
    free(bitmap);
	fclose(f);
}

