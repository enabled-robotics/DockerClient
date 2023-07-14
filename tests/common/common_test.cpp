#include <gtest/gtest.h>

#include "docker.hpp"

#include <sstream>

using namespace docker;

namespace {
std::string runContainer() {
    docker::Docker client;

    request_params::RunContainer params;
    params.image = "senjun_courses_python";
    params.tty = true;
    params.memory = 7000000;

    auto result = client.runContainer(params);
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(!result.containerId.empty());
    return result.containerId;
}
}  // namespace

TEST(base, test_version) {
    Docker client;

    auto doc = client.dockerVersion();
    EXPECT_TRUE(!doc.json.empty());
}

TEST(base, test_list_images) {
    Docker client = Docker();

    auto imagesAnswer = client.listImages();
    EXPECT_TRUE(imagesAnswer.success);
}

TEST(base, test_create_container) {
    Docker client;

    docker::request_params::CreateContainer params;
    params.image = "senjun_courses_python";
    params.tty = true;
    params.memory = 7000000;

    auto result = client.createContainer(params);
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(!result.containerId.empty());

    docker::request_params::RemoveContainer removeParams;
    removeParams.containerId = std::move(result.containerId);
    auto resultDelete = client.deleteContainer(removeParams);
    EXPECT_TRUE(resultDelete.success);
}

TEST(base, test_run_container) {
    std::string id = runContainer();

    Docker client = Docker();

    std::cout << "Container created: " << id << std::endl;

    docker::request_params::KillContainer params;
    params.containerId = std::move(id);
    auto result = client.killContainer(params);
    EXPECT_TRUE(result.success);

    docker::request_params::RemoveContainer removeParams;
    removeParams.containerId = std::move(params.containerId);
    auto r = client.deleteContainer(removeParams);
    EXPECT_TRUE(r.success);
    std::cout << "Container removed: " << removeParams.containerId << std::endl;
}

TEST(base, test_put_archive) {
    std::string const id = runContainer();
    std::string const pathInContainer = "/home/code_runner";

    std::ostringstream stream(std::ios::binary | std::ios::trunc);
    auto tar = stream.str();

    Docker client;

    request_params::PutArchive params;
    params.containerId = id;
    params.path = pathInContainer;
    params.archive = tar;
    auto result = client.putArchive(params);
    EXPECT_TRUE(result.success);

    auto killResult = client.killContainer({params.containerId, {}});
    EXPECT_TRUE(killResult.success);

    request_params::RemoveContainer removeParams;
    removeParams.containerId = params.containerId;
    auto deleteResult = client.deleteContainer(removeParams);
    EXPECT_TRUE(deleteResult.success);
}

TEST(base, test_exec) {
    std::string id = runContainer();

    // m.py — file inside container for launching
    Docker client;

    request_params::ExecCreate params;
    params.containerId = std::move(id);
    params.cmd = {"sh", "run.sh", "m.py"};

    auto result = client.exec(params);
    EXPECT_TRUE(result.success);

    auto killResult = client.killContainer({params.containerId, {}});
    EXPECT_TRUE(killResult.success);

    auto deleteResult = client.deleteContainer({params.containerId, false, false});
    EXPECT_TRUE(deleteResult.success);
}
