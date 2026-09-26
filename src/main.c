#include "sentinel.h"

int main(int argc, char *argv[]) {
    return sentinel_run(argc >= 2 ? argv[1] : NULL);
}