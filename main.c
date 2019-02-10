#include <stdio.h>
#include "BMPImageHelper.h"
#include "CryptographyHelper.h"

int main() {

    char sourceImagePath[] = "peppers.bmp";
    char copyImagePath[] = "peppers_copy.bmp";
    char encryptedImagePath[] = "peppers_encrypted.bmp";
    char decryptedImagePath[] = "peppers_decrypted.bmp";

    char secretKeyFilePath[] = "secret_key.txt";

    BMPImage image = ReadBMPImage(sourceImagePath);
    WriteBMPImage(image, copyImagePath);

    EncryptImage(sourceImagePath, encryptedImagePath, secretKeyFilePath);
    DecryptImage(encryptedImagePath, decryptedImagePath, secretKeyFilePath);

    return 0;
}