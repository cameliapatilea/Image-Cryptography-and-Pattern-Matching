

#ifndef CRYPTOGRAPHY_PATTERN_RECOGNITION_ON_BMP_IMAGES_BMPIMAGEHELPER_H
#define CRYPTOGRAPHY_PATTERN_RECOGNITION_ON_BMP_IMAGES_BMPIMAGEHELPER_H


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct __attribute__((__packed__)) {
    uint16_t type;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
    uint32_t headerSize;
    uint32_t width;
    uint32_t height;
    uint16_t numberOfPlanes;
    uint16_t biti_per_pixel;
    uint32_t compressionType;
    uint32_t sizeAfterCompression;
    uint32_t horizontalResolution;
    uint32_t verticalResolution;
    uint32_t numberOfColors;
    uint32_t importantColors;
} BMPHeader;

typedef struct {
    uint8_t R, G, B;
} Pixel;

typedef struct {
    BMPHeader header;
    uint32_t padding;
    Pixel *pixels;
} BMPImage;

/*
 * Utility function used to read and load BMP images in memory
 */
BMPImage ReadBMPImage (char* imageSourcePath) {

    BMPImage image;

    FILE* input = fopen(imageSourcePath, "rb");

    /// Check if image failed to open
    if (input == NULL) {
        printf("Image not found. Please check image path.");
        return;
    }

    /// Load header if image was loaded inside the input stream
    fread(&image.header, sizeof(BMPHeader), 1, input);

    uint32_t width = image.header.width;
    uint32_t height = image.header.height;

    /// Memory allocation
    image.pixels = (Pixel*) malloc(width * height * sizeof(Pixel));

    /// Calculate padding
    if (width % 4 != 0) {
        image.padding = 4 - (3 * width) % 4;
    }
    else {
        image.padding = 0;
    }

    /// Set cursor to the starting position of the payload/effective data
    fseek(input, 54, SEEK_SET);

    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < width; j++) {
            /// Read pixels from input file
            uint8_t pixel[3];

            fread(pixel, 3, 1, input);

            image.pixels[i * width + j].B = pixel[0];
            image.pixels[i * width + j].G = pixel[1];
            image.pixels[i * width + j].R = pixel[2];
        }
        /// Jump over padding bytes
        fseek(input, image.padding, SEEK_CUR);
    }

    fclose(input);

    return image;
}

/*
 * This method is used to write given BMP images to disk using the destination path
 */
void WriteBMPImage (BMPImage image, char* imageDestinationPath) {

    uint8_t paddingByte = 0;

    FILE* output = fopen(imageDestinationPath, "wb+");

    /// Write header to the destination file
    fwrite(&image.header, sizeof(BMPHeader), 1, output);

    uint32_t width = image.header.width;
    uint32_t height = image.header.height;

    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < width; j++) {
            /// Write pixels to input file
            fwrite(&image.pixels[i * width + j].B, 1, 1, output);
            fwrite(&image.pixels[i * width + j].G, 1, 1, output);
            fwrite(&image.pixels[i * width + j].R, 1, 1, output);
        }
        /// Write padding bytes, if necessary
        if (image.padding > 0) {
            fwrite(&paddingByte, 1, image.padding, output);
        }
    }

    fclose(output);
}


#endif //CRYPTOGRAPHY_PATTERN_RECOGNITION_ON_BMP_IMAGES_BMPIMAGEHELPER_H
