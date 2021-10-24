#include "iostream"
#include "jni.h"
#include "Image.h"
#include "math.h"

#define NUM_OF_BANDS 3

JNIEXPORT void JNICALL Java_Image_inverting(
        JNIEnv *env, jclass, jint width, jint height, jintArray pixels
) {
    jboolean isCopy = false;
    jint *pixelValues = env->GetIntArrayElements(pixels, &isCopy);

    int arrLen = NUM_OF_BANDS * width * height;

    for (int i = 0; i < arrLen; i++) {
        pixelValues[i] = ~pixelValues[i];
    }

    env->ReleaseIntArrayElements(pixels, pixelValues, 1);
}

int getFilterCoeff(int *filter, int filterSize, int i, int j) {
    return filter[j * filterSize + i];
}

void getRgb(const int *pixels, int width, int u, int v, int *rgb) {
    // TODO: Add length assert for rgb

    const int index = NUM_OF_BANDS * (v * width + u);

    rgb[0] = pixels[index + 2];
    rgb[1] = pixels[index + 1];
    rgb[2] = pixels[index + 0];
}

void setRgb(int *pixels, int width, int u, int v, const int *rgb) {
    // TODO: Add length assert for rgb

    const int index = NUM_OF_BANDS * (v * width + u);

    pixels[index + 2] = rgb[0];
    pixels[index + 1] = rgb[1];
    pixels[index + 0] = rgb[2];
}

int clamp(int v) {
    return (v == (v & 0xFF)) ? v : (v < 0) ? 0 : 255;
}

JNIEXPORT void JNICALL Java_Image_filtering(
        JNIEnv *env, jclass, jint width, jint height, jintArray filter, jintArray pixelsIn, jintArray pixelsOut
) {
    jboolean isCopy = false;

    const int filterSize = (int) sqrt(env->GetArrayLength(filter));
    const int filterSizeD2 = filterSize / 2;

    int *filterValues = env->GetIntArrayElements(filter, &isCopy);
    int *pixelsInValues = env->GetIntArrayElements(pixelsIn, &isCopy);
    int *pixelsOutValues = env->GetIntArrayElements(pixelsOut, &isCopy);

    int *rgb = new int[NUM_OF_BANDS];

    for (int v = 0; v < height; v++) {
        for (int u = 0; u < width; u++) {
            int *sum = new int[NUM_OF_BANDS];

            for (int j = 0; j < filterSize; j++) {
                int v0 = v + j - filterSizeD2;
                if (v0 < 0) {
                    v0 = -v0;
                } else if (v0 >= height) {
                    v0 = 2 * height - v0 - 1;
                }

                for (int i = 0; i < filterSize; i++) {
                    const int filterCoeff = getFilterCoeff(filterValues, filterSize, i, j);
                    int u0 = u + i - filterSizeD2;
                    if (u0 < 0) {
                        u0 = -u0;
                    } else if (u0 >= width) {
                        u0 = 2 * width - u0 - 1;
                    }

                    getRgb(pixelsInValues, width, u0, v0, rgb);

                    for (int c = 0; c < NUM_OF_BANDS; c++) {
                        sum[c] += rgb[c] * filterCoeff;
                    }
                }

                for (int c = 0; c < NUM_OF_BANDS; c++) {
                    sum[c] = clamp(128 + sum[c]);
                }
                setRgb(pixelsOutValues, width, u, v, sum);
            }
        }
    }

    env->ReleaseIntArrayElements(pixelsOut, pixelsOutValues, 1);
}