#!/usr/bin/env python3
"""Convert PNG files to 32-bit BMP using only the Python standard library."""
import struct
import zlib
import os
import sys


def read_png(path):
    with open(path, 'rb') as f:
        data = f.read()
    assert data[:8] == b'\x89PNG\r\n\x1a\n', "Not a PNG file"

    pos = 8
    width = height = None
    bit_depth = color_type = None
    idat = b''

    while pos < len(data):
        length = struct.unpack('>I', data[pos:pos + 4])[0]
        chunk_type = data[pos + 4:pos + 8]
        chunk_data = data[pos + 8:pos + 8 + length]
        if chunk_type == b'IHDR':
            width, height, bit_depth, color_type, _, _, _ = struct.unpack(
                '>IIBBBBB', chunk_data)
        elif chunk_type == b'IDAT':
            idat += chunk_data
        elif chunk_type == b'IEND':
            break
        pos += 12 + length

    raw = zlib.decompress(idat)

    channels = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}[color_type]
    bpp = (bit_depth * channels + 7) // 8
    stride = width * bpp + 1  # filter byte per row

    # De-filter (support Sub, Up, Average, Paeth)
    def paeth(a, b, c):
        p = a + b - c
        pa = abs(p - a)
        pb = abs(p - b)
        pc = abs(p - c)
        if pa <= pb and pa <= pc:
            return a
        if pb <= pc:
            return b
        return c

    rows = []
    prev = bytearray(width * bpp)
    for y in range(height):
        row = bytearray(raw[y * stride + 1:(y + 1) * stride])
        ftype = raw[y * stride]
        if ftype == 1:  # Sub
            for i in range(bpp, len(row)):
                row[i] = (row[i] + row[i - bpp]) & 0xFF
        elif ftype == 2:  # Up
            for i in range(len(row)):
                row[i] = (row[i] + prev[i]) & 0xFF
        elif ftype == 3:  # Average
            for i in range(len(row)):
                left = row[i - bpp] if i >= bpp else 0
                row[i] = (row[i] + ((left + prev[i]) >> 1)) & 0xFF
        elif ftype == 4:  # Paeth
            for i in range(len(row)):
                a = row[i - bpp] if i >= bpp else 0
                b = prev[i]
                c = prev[i - bpp] if i >= bpp else 0
                row[i] = (row[i] + paeth(a, b, c)) & 0xFF
        rows.append(bytes(row))
        prev = row

    # Build RGBA pixel buffer
    pixels = bytearray(width * height * 4)
    for y in range(height):
        row = rows[y]
        for x in range(width):
            idx = x * bpp
            if color_type == 6:  # RGBA
                r, g, b, a = row[idx], row[idx + 1], row[idx + 2], row[idx + 3]
            elif color_type == 2:  # RGB
                r, g, b = row[idx], row[idx + 1], row[idx + 2]
                a = 255
            elif color_type == 0:  # Grayscale
                r = g = b = row[idx]
                a = 255
            elif color_type == 4:  # Grayscale + alpha
                r = g = b = row[idx]
                a = row[idx + 1]
            else:
                raise ValueError("Unsupported color type %d" % color_type)
            dst = (y * width + x) * 4
            pixels[dst] = b
            pixels[dst + 1] = g
            pixels[dst + 2] = r
            pixels[dst + 3] = a

    return width, height, pixels


def write_bmp(path, width, height, pixels):
    row_size = width * 4
    data_size = row_size * height
    file_size = 54 + data_size
    with open(path, 'wb') as f:
        # BITMAPFILEHEADER (14 bytes)
        f.write(b'BM')
        f.write(struct.pack('<I', file_size))
        f.write(struct.pack('<HH', 0, 0))
        f.write(struct.pack('<I', 54))
        # BITMAPINFOHEADER (40 bytes)
        f.write(struct.pack('<I', 40))
        f.write(struct.pack('<i', width))
        f.write(struct.pack('<i', height))
        f.write(struct.pack('<HH', 1, 32))
        f.write(struct.pack('<I', 0))
        f.write(struct.pack('<I', data_size))
        f.write(struct.pack('<i', 2835))
        f.write(struct.pack('<i', 2835))
        f.write(struct.pack('<I', 0))
        f.write(struct.pack('<I', 0))
        # Pixel data (bottom-up, BGRA)
        for y in range(height - 1, -1, -1):
            row = pixels[y * row_size:(y + 1) * row_size]
            f.write(row)


def main():
    mapping = {
        'protagonist.png': 'assets/player.bmp',
        'bomb.png': 'assets/bomb.bmp',
        'enemy.png': 'assets/enemy.bmp',
    }
    for png, bmp in mapping.items():
        if not os.path.exists(png):
            print("SKIP (not found): %s" % png)
            continue
        w, h, px = read_png(png)
        os.makedirs('assets', exist_ok=True)
        write_bmp(bmp, w, h, px)
        print("OK: %s -> %s (%dx%d)" % (png, bmp, w, h))


if __name__ == '__main__':
    main()