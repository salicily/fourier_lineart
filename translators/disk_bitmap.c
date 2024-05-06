#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

#include "disk_bitmap.h"

static uint16_t read_16le(uint8_t *data, size_t *offset);
static uint32_t read_32le(uint8_t *data, size_t *offset);
static void write_16le(uint8_t *data, size_t *offset, uint16_t val);
static void write_32le(uint8_t *data, size_t *offset, uint32_t val);
static int parse_bitmap_info_(uint8_t *data, size_t data_size, struct raw_bitmap_info *rbi, struct rgba **color_map, uint8_t **bitmap);
static void dump_bitmap_info_(uint8_t *data, size_t data_size, struct raw_bitmap_info *rbi, struct rgba **color_map, uint8_t **bitmap);

struct raw_bitmap *disk_to_bitmap(const char *fname) {
    int fd = open(fname, O_RDONLY);
    if (fd == -1) {
        dprintf(2, "Cannot open file %s (%s)\n", fname, strerror(errno));
        return NULL;
    }
    off_t fsize = lseek(fd, 0, SEEK_END);
    if (fsize < 0) {
        dprintf(2, "Cannot determine file size (%s)\n", strerror(errno));
        return NULL;
    }
    size_t data_size = (size_t)fsize;
    uint8_t *data = malloc(data_size);
    if (data == NULL) {
        dprintf(2, "Cannot allocate the image in memory\n");
        return NULL;
    }
    lseek(fd, 0, SEEK_SET);
    ssize_t rd = read(fd, data, (size_t)fsize);
    if (rd != (ssize_t)fsize) {
        dprintf(2, "Cannot read the file (%s)\n", strerror(errno));
        free(data);
        return NULL;
    }
    struct raw_bitmap_info rbi;
    struct rgba *color_map;
    uint8_t *bitmap;
    int r = parse_bitmap_info_(data, data_size, &rbi, &color_map, &bitmap);
    if (r != 0) {
        dprintf(2, "Unsupported file format\n");
        free(data);
        return NULL;
    }
    struct raw_bitmap *bm = create_raw_bitmap(rbi);
    if (bm == NULL) {
        free(data);
        return NULL;
    }
    size_t line_width = ((rbi.width * rbi.bits_per_pixel + 31) >> 5) << 2;
    size_t bitmap_size = line_width * rbi.height;
    (void)set_color_map(bm, color_map, rbi.colors_in_color_map);
    (void)set_bitmap(bm, bitmap, bitmap_size);
    free(data);
    return bm;
}

int bitmap_to_disk(const struct raw_bitmap *bm, const char *fname) {
    if (bm == NULL) {
        dprintf(2, "No bitmap provided\n");
        return -1;
    }
    int fd = open(fname, O_CREAT | O_WRONLY | O_EXCL, 0664);
    if (fd == -1) {
        dprintf(2, "Cannot open file %s (%s)\n", fname, strerror(errno));
        return -1;
    }
    struct raw_bitmap_info rbi = get_raw_bitmap_info(bm);
    size_t line_width = ((rbi.width * rbi.bits_per_pixel + 31) >> 5) << 2;
    size_t bitmap_size = line_width * rbi.height;
    size_t file_size = 54 + sizeof(struct rgba) * rbi.colors_in_color_map + bitmap_size;
    uint8_t *data = malloc(file_size);
    if (data == NULL) {
        dprintf(2, "Cannot allocate %zu bytes\n", file_size);
        return -1;
    }
    struct rgba *color_map;
    uint8_t *bitmap;
    dump_bitmap_info_(data, file_size, &rbi, &color_map, &bitmap);
    (void)get_color_map(bm, color_map, rbi.colors_in_color_map);
    (void)get_bitmap(bm, bitmap, bitmap_size);

    ssize_t rd = write(fd, data, file_size);
    free(data);
    if (rd != (ssize_t)file_size) {
        dprintf(2, "Cannot write the file (%s)\n", strerror(errno));
        return -1;
    }
    return 0;
}

static uint16_t read_16le(uint8_t *data, size_t *offset) {
    uint16_t res_low = data[*offset];
    ++*offset;
    uint16_t res_high = data[*offset];
    ++*offset;
    return (res_high << 8) | res_low;
}

static uint32_t read_32le(uint8_t *data, size_t *offset) {
    uint32_t res_low = read_16le(data, offset);
    uint32_t res_high = read_16le(data, offset);
    return (res_high << 16) | res_low;
}

static void write_16le(uint8_t *data, size_t *offset, uint16_t val) {
    data[*offset] = val;
    ++*offset;
    data[*offset] = val >> 8;
    ++*offset;
    return;
}

static void write_32le(uint8_t *data, size_t *offset, uint32_t val) {
    write_16le(data, offset, val);
    write_16le(data, offset, val >> 16);
    return;
}

static int parse_bitmap_info_(uint8_t *data, size_t data_size, struct raw_bitmap_info *rbi, struct rgba **color_map, uint8_t **bitmap) {
    if (data_size < 54) {
        return -1;
    }
    size_t offset = 0;
    /* Check the magic number */
    uint16_t magic = read_16le(data, &offset);
    if (magic != UINT16_C(0x4d42)) {
        dprintf(2, "Invalid magic number, expecting 'BM'\n");
        return -1;
    }
    dprintf(2, "Found expected magic number\n");
    uint32_t check_size = read_32le(data, &offset);
    if (check_size != data_size) {
        dprintf(2, "Invalid file size, expecting %zu, got %" PRIu32 "\n", data_size, check_size);
        return -1;
    }
    dprintf(2, "Bitmap file size is %" PRIu32 "\n", check_size);
    (void)read_32le(data, &offset);
    uint32_t bitmap_array_offset = read_32le(data, &offset);
    if (bitmap_array_offset > data_size) {
        dprintf(2, "Bitmap starts beyond the file\n");
        return -1;
    }
    dprintf(2, "Bitmap starts at offset %" PRIu32 "\n", bitmap_array_offset);
    uint32_t header_size = read_32le(data, &offset);
    if (header_size != 40) {
        dprintf(2, "Header size is %" PRIu32 ", was expecting 40\n", header_size);
        return -1;
    }
    rbi->width = read_32le(data, &offset);
    dprintf(2, "Image is %" PRIu32 " pixels wide\n", rbi->width);
    rbi->height = read_32le(data, &offset);
    dprintf(2, "Image is %" PRIu32 " pixels high\n", rbi->height);
    uint16_t planes = read_16le(data, &offset);
    if (planes != 1) {
        dprintf(2, "Unexpected number of planes: %" PRIu16 ", was expecting 1\n", planes);
        return -1;
    }
    rbi->bits_per_pixel = read_16le(data, &offset);
    switch (rbi->bits_per_pixel) {
        case 1:
        case 4:
        case 8:
        case 16:
        case 24:
        case 32:
            break;
        default:
            dprintf(2, "Unsupported bits per pixels (%" PRIu16 ")\n", rbi->bits_per_pixel);
            return -1;
    }
    dprintf(2, "Bits per pixel: %" PRIu32 "\n", rbi->bits_per_pixel);
    uint32_t compression = read_32le(data, &offset);
    if (compression != 0) {
        dprintf(2, "Non raw format (%" PRIu32 "), not supported\n", compression);
        return -1;
    }
    (void)read_32le(data, &offset);
    uint32_t line_width = ((rbi->width * rbi->bits_per_pixel + 31) >> 5) << 2;
    uint32_t theoretic_bitmap_size = line_width * rbi->height;
    if ((theoretic_bitmap_size + bitmap_array_offset) > check_size) {
        dprintf(2, "The bitmap overflows the file\n");
        return -1;
    }
    rbi->w_ppm = read_32le(data, &offset);
    rbi->h_ppm = read_32le(data, &offset);
    dprintf(2, "Width: %" PRIu32 " pixels per meter, Height: %" PRIu32 " pixels per meter\n", rbi->w_ppm, rbi->h_ppm);
    rbi->colors_in_color_map = read_32le(data, &offset);
    (void)read_32le(data, &offset);
    if (rbi->bits_per_pixel <= 8) {
        if (rbi->colors_in_color_map == 0) {
            rbi->colors_in_color_map = UINT32_C(1) << rbi->bits_per_pixel;
        }
        if (((rbi->colors_in_color_map - 1) >> rbi->bits_per_pixel) > 0) {
            return -1;
        }
    } else {
        if (rbi->colors_in_color_map > 0) {
            return -1;
        }
    }
    dprintf(2, "Using a color table of %" PRIu32 " colors\n", rbi->colors_in_color_map);
    size_t color_map_size = rbi->colors_in_color_map * sizeof(struct rgba);
    if (bitmap_array_offset < (color_map_size + offset)) {
        dprintf(2, "Colormap overflows to bitmap array\n");
        return -1;
    }
    *color_map = (struct rgba *)(data + offset);
    *bitmap = data + bitmap_array_offset;
    return 0;
}

static void dump_bitmap_info_(uint8_t *data, size_t data_size, struct raw_bitmap_info *rbi, struct rgba **color_map, uint8_t **bitmap) {
    size_t offset = 0;
    /* Check the magic number */
    write_16le(data, &offset, UINT16_C(0x4d42));
    write_32le(data, &offset, data_size);
    write_32le(data, &offset, 0);
    uint32_t bitmap_array_offset = 54 + rbi->colors_in_color_map * sizeof(struct rgba);
    write_32le(data, &offset, bitmap_array_offset);
    write_32le(data, &offset, 40);
    write_32le(data, &offset, rbi->width);
    write_32le(data, &offset, rbi->height);
    write_16le(data, &offset, 1);
    write_16le(data, &offset, rbi->bits_per_pixel);
    write_32le(data, &offset, 0);
    write_32le(data, &offset, 0);
    write_32le(data, &offset, rbi->w_ppm);
    write_32le(data, &offset, rbi->h_ppm);
    write_32le(data, &offset, rbi->colors_in_color_map);
    write_32le(data, &offset, 0);
    *color_map = (struct rgba *)(data + offset);
    *bitmap = data + bitmap_array_offset;
    return;
}

