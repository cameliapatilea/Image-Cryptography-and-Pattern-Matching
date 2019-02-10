

#ifndef CRYPTOGRAPHY_PATTERN_RECOGNITION_ON_BMP_IMAGES_CRYPTOGRAPHYHELPER_H
#define CRYPTOGRAPHY_PATTERN_RECOGNITION_ON_BMP_IMAGES_CRYPTOGRAPHYHELPER_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "BMPImageHelper.h"

Pixel XORPixelWithPixel (Pixel pixel1, Pixel pixel2) {

    Pixel result;

    result.R = pixel1.R ^ pixel2.R;
    result.G = pixel1.G ^ pixel2.G;
    result.B = pixel1.B ^ pixel2.B;

    return result;
}

Pixel XORPixelWithUint32 (Pixel pixel1, uint32_t number) {

    Pixel result;

    result.B = pixel1.B ^ (uint8_t) (number & 255);
    result.G = pixel1.G ^ (uint8_t) (number & ((1 << 16) - 1)) >> 8;
    result.R = pixel1.R ^ (uint8_t) (number & ((1 << 24) - 1)) >> 16;

    return result;
}

uint32_t XORShift32 (uint32_t x)
{
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    return x;
}

uint32_t* RandomSequenceGenerator (int n, uint32_t seed) {

    uint32_t* randomSequence = (uint32_t*) malloc((n + 1) * sizeof(uint32_t));

    randomSequence[0] = seed;

    for (int i = 1; i <= n; i++) {
        randomSequence[i] = XORShift32(randomSequence[i - 1]);
    }

    return randomSequence;
}

uint32_t* DurstenfeldRandomPermutation (int n, uint32_t* randomSequence) {

    uint32_t* permutation;

    permutation = (uint32_t*) malloc((n + 1) * sizeof(uint32_t));

    for (int i = 0; i < n; i++) {
        permutation[i] = i;
    }

    for (int i = n - 1; i > 0; i--) {
        int index = randomSequence[n - i] % (i + 1);

        uint32_t aux = permutation[i];
        permutation[i] = permutation[index];
        permutation[index] = aux;
    }

    return permutation;
}

uint32_t* ComputeInversePermutation (int n, uint32_t* permutation) {

    uint32_t* inverse;

    inverse = (uint32_t*) malloc((n + 1) * sizeof(uint32_t));

    for (int i = 0; i < n; i++) {
        inverse[permutation[i]] = i;
    }

    return inverse;
}

BMPImage ImagePixelShuffler (BMPImage input, uint32_t* permutation) {

    BMPImage output;
    uint32_t width = input.header.width;
    uint32_t height = input.header.height;

    output.header = input.header;
    output.padding = input.padding;
    output.pixels = (Pixel*) malloc(width * height * sizeof(Pixel));

    for (int i = 0; i < width * height; i++) {
        output.pixels[permutation[i]] = input.pixels[i];
    }

    return output;
}

BMPImage ImageEncrypter (BMPImage input, uint32_t* randomSequence, uint32_t SV) {

    BMPImage output;
    uint32_t width = input.header.width;
    uint32_t height = input.header.height;

    output.header = input.header;
    output.padding = input.padding;
    output.pixels = (Pixel*) malloc(width * height * sizeof(Pixel));

    uint32_t numberOfPixels = width * height;

    output.pixels[0] = XORPixelWithUint32(XORPixelWithUint32(input.pixels[0], SV), randomSequence[numberOfPixels]);

    for (int i = 1; i < numberOfPixels; i++) {
        output.pixels[i] = XORPixelWithUint32(XORPixelWithPixel(output.pixels[i - 1], input.pixels[i]), randomSequence[numberOfPixels + i]);
    }

    return output;
}

BMPImage ImageDecrypter (BMPImage input, uint32_t* randomSequence, uint32_t SV) {

    BMPImage output;
    uint32_t width = input.header.width;
    uint32_t height = input.header.height;

    output.header = input.header;
    output.padding = input.padding;
    output.pixels = (Pixel*) malloc(width * height * sizeof(Pixel));

    uint32_t numberOfPixels = width * height;

    output.pixels[0] = XORPixelWithUint32(XORPixelWithUint32(input.pixels[0], SV), randomSequence[numberOfPixels]);

    for (int i = 1; i < numberOfPixels; i++) {
        output.pixels[i] = XORPixelWithUint32(XORPixelWithPixel(input.pixels[i - 1], input.pixels[i]), randomSequence[numberOfPixels + i]);
    }

    return output;
}

BMPImage EncryptImage (char* imageSourcePath, char* imageDestinationPath, char* secretKeyPath) {

    uint32_t R0, SV;

    FILE *secretKeyInput = fopen(secretKeyPath, "r+");

    /// Read secret key (R0 and SV = Starting Value)
    fscanf(secretKeyInput, "%u", &R0);
    fscanf(secretKeyInput, "%u", &SV);

    fclose(secretKeyInput);

    BMPImage input = ReadBMPImage(imageSourcePath);

    uint32_t width = input.header.width;
    uint32_t height = input.header.height;

    uint32_t* randomSequence = RandomSequenceGenerator(2 * width * height, R0);
    uint32_t* randomPermutation = DurstenfeldRandomPermutation(width * height, randomSequence);

    BMPImage shuffledImage = ImagePixelShuffler(input, randomPermutation);
    BMPImage output = ImageEncrypter(shuffledImage, randomSequence, SV);

    WriteBMPImage(output, imageDestinationPath);

    return output;
}

BMPImage DecryptImage (char* imageSourcePath, char* imageDestinationPath, char* secretKeyPath) {

    uint32_t R0, SV;

    FILE *secretKeyInput = fopen(secretKeyPath, "r+");

    /// Read secret key (R0 and SV = Starting Value)
    fscanf(secretKeyInput, "%u", &R0);
    fscanf(secretKeyInput, "%u", &SV);

    fclose(secretKeyInput);

    BMPImage input = ReadBMPImage(imageSourcePath);

    uint32_t width = input.header.width;
    uint32_t height = input.header.height;

    uint32_t* randomSequence = RandomSequenceGenerator(2 * width * height, R0);
    uint32_t* randomPermutation = DurstenfeldRandomPermutation(width * height, randomSequence);
    uint32_t* inversePermutation = ComputeInversePermutation(width * height, randomPermutation);

    BMPImage partialOutput = ImageDecrypter(input, randomSequence, SV);
    BMPImage output = ImagePixelShuffler(partialOutput, inversePermutation);

    WriteBMPImage(output, imageDestinationPath);

    return output;
}


#endif //CRYPTOGRAPHY_PATTERN_RECOGNITION_ON_BMP_IMAGES_CRYPTOGRAPHYHELPER_H
