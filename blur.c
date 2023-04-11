#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define NUM_THREADS 4

int main()
{
    FILE *image, *outputImage;
    image = fopen("sample.bmp", "rb");
    outputImage = fopen("blurred_%d.bmp", "wb");
    long ancho, alto;
    unsigned char r, g, b;
    unsigned char *ptr;
    unsigned char xx[54];
    int cuenta = 0;

    // Cabecera
    for (int i = 0; i < 54; i++)
    {
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    // Dimensiones de la imagen
    ancho = (long)xx[20] * 65536 + (long)xx[19] * 256 + (long)xx[18];
    alto = (long)xx[24] * 65536 + (long)xx[23] * 256 + (long)xx[22];
    printf("largo img %li\n", alto);
    printf("ancho img %li\n", ancho);

    // Memoria para la imagen
    ptr = (unsigned char *)malloc(alto * ancho * 3 * sizeof(unsigned char));
    fread(ptr, sizeof(unsigned char), alto * ancho * 3, image);

    for (int k = 0; k < 40; k++)
    {
        // Desenfoque
        omp_set_num_threads(NUM_THREADS);
#pragma omp parallel for shared(ptr) private(r, g, b) schedule(dynamic)
        for (int i = 0; i < alto * ancho * 3; i += 3)
        {
            if (i < ancho * 3 || i > alto * ancho * 3 - ancho * 3 || i % (ancho * 3) == 0 || i % (ancho * 3) == ancho * 3 - 3)
            {
                continue; // no aplicar efecto en bordes
            }

            r = (ptr[i - 3] + ptr[i] + ptr[i + 3] + ptr[i - (ancho * 3)] + ptr[i + (ancho * 3)]) / 5;
            g = (ptr[i - 2] + ptr[i + 1] + ptr[i + 4] + ptr[i - (ancho * 3) + 1] + ptr[i + (ancho * 3) + 1]) / 5;
            b = (ptr[i - 1] + ptr[i + 2] + ptr[i + 5] + ptr[i - (ancho * 3) + 2] + ptr[i + (ancho * 3) + 2]) / 5;

            ptr[i - 2] = g;
            ptr[i - 1] = b;
            ptr[i] = r;
        }

        // Escribir imagen en archivo
        char filename[50];
        sprintf(filename, "blurred_%d.bmp", k);
        FILE *outputImage = fopen(filename, "wb");
        fwrite(xx, sizeof(unsigned char), 54, outputImage);
        fwrite(ptr, sizeof(unsigned char), alto * ancho * 3, outputImage);
        fclose(outputImage);
    }

    // Liberar
    // Liberar memoria y cerrar archivos
    free(ptr);
    fclose(image);
    fclose(outputImage);
    return 0;
}