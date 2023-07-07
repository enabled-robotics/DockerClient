#include <gtest/gtest.h>

#include "docker.hpp"

#include <sstream>

namespace {
std::string runContainer() {
    Docker client = Docker();

    std::string const runParams =
        R"({"Image": "senjun_courses_python", "Tty": true, "HostConfig": {"Memory": 7000000}})";
    rapidjson::Document params;
    if (params.Parse(runParams.c_str()).HasParseError()) {
        EXPECT_TRUE(false);
    }
    auto result = client.run_container(params);
    EXPECT_TRUE(result["success"].GetBool());
    EXPECT_TRUE(result["data"].GetObject()["Id"].GetString());
    return result["data"].GetObject()["Id"].GetString();
}

bool deleteContainer(std::string const & id) {
    Docker client;
    auto result = client.delete_container(id);
    EXPECT_TRUE(result["success"].IsBool());

    auto const r = result["success"].GetBool();
    EXPECT_TRUE(r);
    return r;
}
}  // namespace

TEST(base, test_version) {
    Docker client = Docker();

    JSON_DOCUMENT doc = client.docker_version();
    auto version = jsonToString(doc);
    std::cout << jsonToString(doc) << std::endl;
    EXPECT_TRUE(!version.empty());
}

TEST(base, test_list_images) {
    Docker client = Docker();

    auto images = client.list_images();
    for (auto & image : images.GetObject()) {
        std::cout << jsonToString(image.name) << ':';
        std::cout << jsonToString(image.value) << std::endl;
    }
    auto const r = images["success"].GetBool();
    EXPECT_TRUE(r);
}

TEST(base, test_create_container) {
    Docker client = Docker();

    std::string const createParams =
        R"({"Image": "senjun_courses_python", "Tty": true, "HostConfig": {"Memory": 7000000}})";
    rapidjson::Document params;
    if (params.Parse(createParams.c_str()).HasParseError()) {
        assert(false);
    }
    auto result = client.create_container(params);
    std::cout << jsonToString(result["success"]) << ": ";
    std::cout << jsonToString(result["data"]);

    std::string const id = result["data"].GetObject()["Id"].GetString();
    result = client.delete_container(id);
    EXPECT_TRUE(result["success"].IsBool());

    auto const r = result["success"].GetBool();
    EXPECT_TRUE(r);
}

TEST(base, test_run_container) {
    std::string id = runContainer();

    Docker client = Docker();

    std::cout << "Container created: " << id << std::endl;

    auto result = client.kill_container(id);
    EXPECT_TRUE(result["success"].GetBool());

    result = client.delete_container(id);
    EXPECT_TRUE(result["success"].GetBool());
    std::cout << "Container removed: " << id << std::endl;
}

TEST(base, test_put_archive) {
    std::string const id = runContainer();
    std::string const pathInContainer = "/home/code_runner";

    std::ostringstream stream(std::ios::binary | std::ios::trunc);

    Docker client;
    auto result = client.put_archive(id, pathInContainer, stream);
    assert(result["success"].GetBool());

    client.kill_container(id);

    auto deleteResult = client.delete_container(id);
    EXPECT_TRUE(deleteResult["success"].IsBool());

    auto const r = deleteResult["success"].GetBool();
    EXPECT_TRUE(r);
}

TEST(base, test_exec) {
    std::string const id = runContainer();

    // m.py — file inside container for launching
    std::string const strExecParams =
        R"({"AttachStderr": true, "AttachStdout": true, "Detach": false, "Tty": true, "Cmd": ["sh",  "run.sh", " m.py"]})";
    rapidjson::Document execParams;
    if (execParams.Parse(strExecParams.c_str()).HasParseError()) {
        assert(false);
    }

    rapidjson::Document startParams;

    std::string const strStartParams = R"({"Detach": false, "Tty": false})";
    if (startParams.Parse(strStartParams.c_str()).HasParseError()) {
        assert(false);
    }

    Docker client;
    auto result = client.exec(execParams, startParams, id);
    if (!result["success"].GetBool()) {
        assert(false);
    }

    std::string const answer = jsonToString(result["data"]);
    std::cout << answer << std::endl;

    client.kill_container(id);

    auto deleteResult = client.delete_container(id);
    EXPECT_TRUE(deleteResult["success"].IsBool());

    auto const r = deleteResult["success"].GetBool();
    EXPECT_TRUE(r);
}
