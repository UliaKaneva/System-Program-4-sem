#include "Traffic.h"

int main() {
    Traffic tr{3, 3};
    pthread_t p[2];
    pthread_create(&p[0], nullptr, Traffic::start_traffic, &tr);
    pthread_create(&p[1], nullptr, Traffic::analyze_traffic, &tr);

    for (unsigned long i : p) {
        pthread_join(i, nullptr);
    }
}