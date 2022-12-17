#include "docker.h"

void test_version() {
    Docker client = Docker();

    JSON_DOCUMENT doc = client.docker_version();
    std::cout << jsonToString(doc) << std::endl;
}

void test_list_images() {
    Docker client = Docker();

    auto images = client.list_images();
    for (auto & image : images.GetObject()) {
        std::cout << jsonToString(image.name) << ':';
        std::cout << jsonToString(image.value) << std::endl;
    }
}

int main() {
    test_version();
    test_list_images();

    return 0;
}