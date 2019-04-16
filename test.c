#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>


// Algoritmo Marsaglia
float getNumDistrNormal(int max, int min){
    float value, x, y, rsq, f;

    do {
        x = 2.0 * rand() / (float)RAND_MAX - 1.0;
        y = 2.0 * rand() / (float)RAND_MAX - 1.0;
        rsq = x * x + y * y;
    } while( rsq >= 1. || rsq == 0. );
    
    f = sqrt( -2.0 * log(rsq) / rsq );
    // (x * f) is a number between [-3, 2.9]
    // (x * f) + 3 to get a number between [0, 5.9]
    // ((x * f) + 3) * 100 / 5.9 to get a number between [0, 100]
    value = ((x * f) + 3) * max / 5.9; // maxNum = highest number
    if (value > max) value = max; // In case value is > 100
    else if (value < min) value = min; // In case value is < 0

    return value;
}

int determinarViabilidad(float dificultad){
    if (dificultad >= 85.01) { // 100 - 85.01 = 0 hijos
        return 0;
    } else {
        if (85 >= dificultad && dificultad > 45){ // 85 - 45.01 = min 1 hijo
            return getNumDistrNormal(45, 1);
        } else { // 45 - 0 = min 3 hijos
            return getNumDistrNormal(85, 3);
        }
    }
}

void main() {

    printf("%d", determinarViabilidad(40));
}
