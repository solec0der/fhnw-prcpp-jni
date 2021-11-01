#include "jni.h"
#include "Image.h"
#include "math.h"

constexpr int numOfBands = 3;

JNIEXPORT void JNICALL Java_Image_inverting(
        JNIEnv *env, jclass, jint width, jint height, jintArray pixels
) {
    jboolean isCopy;
    jint *pixelValues = env->GetIntArrayElements(pixels, &isCopy);

    int arrLen = numOfBands * width * height;

    for (int i = 0; i < arrLen; i++) {
        pixelValues[i] = ~pixelValues[i];
    }

    env->ReleaseIntArrayElements(pixels, pixelValues, 0);
}

int getFilterCoeff(int *filter, int filterSize, int i, int j) {
    return filter[j * filterSize + i];
}

int clamp(int v) {
    return (v == (v & 0xFF)) ? v : (v < 0) ? 0 : 255;
}

JNIEXPORT void JNICALL Java_Image_filtering(
        JNIEnv *env, jclass, jint width, jint height, jintArray filter, jintArray pixelsIn, jintArray pixelsOut
) {
    jboolean isCopy;

    const int filterSize = (int) sqrt(env->GetArrayLength(filter));
    const int filterSizeD2 = filterSize / 2;

    int *filterValues = env->GetIntArrayElements(filter, &isCopy);
    int *pixelsInValues = env->GetIntArrayElements(pixelsIn, &isCopy);
    int *pixelsOutValues = env->GetIntArrayElements(pixelsOut, &isCopy);

    for (int v = 0; v < height; v++) {
        for (int u = 0; u < width; u++) {
            int sum[numOfBands] = {0};

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
                    const int index = numOfBands * (v0 * width + u0);

                    for (int c = 0; c < numOfBands; c++) {
                        sum[c] += pixelsInValues[index + numOfBands - 1 - c] * filterCoeff;
                    }
                }
            }
            const int index = numOfBands * (v * width + u);

            for (int c = 0; c < numOfBands; c++) {
                pixelsOutValues[index + numOfBands - 1 - c] = clamp(128 + sum[c]);
            }
        }
    }

    env->ReleaseIntArrayElements(pixelsOut, pixelsOutValues, 0);
}