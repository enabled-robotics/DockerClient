#include "docker.h"

int main() {
    Docker client = Docker();

    JSON_DOCUMENT doc = client.docker_version();
    std::cout << jsonToString(doc) << std::endl;

    return 0;
}