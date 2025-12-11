#include <gtest/gtest.h>
#include <core/Project.hpp>
#include <core/Layer.hpp>
#include <fstream>
#include <cstdio>

class ProjectTest : public ::testing::Test {
protected:
    Layer *layer1;
    Layer *layer2;
    Project *project1;
    Project *project2;

    std::string path;

    void SetUp() override {
        layer1 = new Layer("Layer1");
        layer2 = new Layer("Layer2");

        project1 = new Project("TestProject1", 2020.0);
        project2 = new Project("TestProject2", 2021.0, "EPSG:3857", {*layer1, *layer2});

        path = "/tmp/test_project.sigm4";
    }

    void TearDown() override {
        delete project1;
        delete project2;
        delete layer1;
        delete layer2;
        std::remove(path.c_str());
    }
};

TEST_F(ProjectTest, DefaultConstructor) {
    EXPECT_EQ(project1->getName(), "TestProject1");
    EXPECT_EQ(project1->getEpoch0(), 2020.0);
    EXPECT_EQ(project1->getCrs(), "EPSG:4326");
    EXPECT_TRUE(project1->getLayers().empty());
}

TEST_F(ProjectTest, ConstructorWithLayers) {
    EXPECT_EQ(project2->getName(), "TestProject2");
    EXPECT_EQ(project2->getEpoch0(), 2021.0);
    EXPECT_EQ(project2->getCrs(), "EPSG:3857");
    EXPECT_EQ(project2->getLayers().size(), 2);
}

TEST_F(ProjectTest, AddLayer) {
    project1->addLayer(*layer1);
    ASSERT_EQ(project1->getLayers().size(), 1);
    EXPECT_EQ(project1->getLayers()[0].getName(), "Layer1");
}

TEST_F(ProjectTest, RemoveLayer) {
    project2->rmLayer(*layer1);
    ASSERT_EQ(project2->getLayers().size(), 1);
    EXPECT_EQ(project2->getLayers()[0].getName(), "Layer2");
}

TEST_F(ProjectTest, SettersAndGetters) {
    project1->setName("NewName");
    project1->setCrs("EPSG:32633");
    project1->setEpoch0(2022.0);

    EXPECT_EQ(project1->getName(), "NewName");
    EXPECT_EQ(project1->getCrs(), "EPSG:32633");
    EXPECT_DOUBLE_EQ(project1->getEpoch0(), 2022.0);
}


// TESTS SAVE / LOAD

// Save empty
TEST_F(ProjectTest, SaveEmptyProject) {
    Project p("Test", 2024.0, "EPSG:4326");
    ASSERT_TRUE(p.save(path));

    std::ifstream f(path);
    EXPECT_TRUE(f.good());
}

// Save + Load 
TEST_F(ProjectTest, SaveAndLoadSimple) {
    Project p("MonProjet", 2023.5, "EPSG:2154");
    ASSERT_TRUE(p.save(path));

    Project loaded = Project::load(path);
    EXPECT_EQ(loaded.getName(), "MonProjet");
    EXPECT_EQ(loaded.getCrs(), "EPSG:2154");
    EXPECT_DOUBLE_EQ(loaded.getEpoch0(), 2023.5);
    EXPECT_TRUE(loaded.getLayers().empty());
}

// Save + Load layers
TEST_F(ProjectTest, SaveAndLoadLayers) {
    Project p("Projet", 2024.0, "EPSG:4326");
    p.addLayer(Layer("L1", "EPSG:4326", 2024.0));
    p.addLayer(Layer("L2", "EPSG:2154", 2023.0));

    ASSERT_TRUE(p.save(path));

    Project loaded = Project::load(path);
    ASSERT_EQ(loaded.getLayers().size(), 2);
    EXPECT_EQ(loaded.getLayers()[0].getName(), "L1");
    EXPECT_EQ(loaded.getLayers()[1].getName(), "L2");
}

// Caractères spéciaux
TEST_F(ProjectTest, SaveLoadSpecialCharacters) {
    Project p("Nom \"test\" \\ok", 2024.0, "EPSG:4326");
    p.addLayer(Layer("Couche\nMulti\tLine", "EPSG:2154", 2023.5));

    ASSERT_TRUE(p.save(path));

    Project loaded = Project::load(path);
    EXPECT_EQ(loaded.getName(), "Nom \"test\" \\ok");
    EXPECT_EQ(loaded.getLayers()[0].getName(), "Couche\nMulti\tLine");
}

// Load file missing
TEST_F(ProjectTest, LoadMissingFile) {
    EXPECT_THROW(Project::load("/tmp/not_exists_abcdef.sigm4"), std::runtime_error);
}

// Save invalid path
TEST_F(ProjectTest, SaveInvalidPath) {
    Project p("X", 2024.0, "EPSG:4326");
    EXPECT_FALSE(p.save("/wrong/path/does/not/exist/proj.sigm4"));
}

// Overwrite
TEST_F(ProjectTest, OverwriteExistingFile) {
    Project p1("V1", 2024.0, "EPSG:4326");
    ASSERT_TRUE(p1.save(path));

    Project p2("V2", 2025.0, "EPSG:2154");
    p2.addLayer(Layer("NewLayer", "EPSG:2154", 2025.0));
    ASSERT_TRUE(p2.save(path));

    Project loaded = Project::load(path);
    EXPECT_EQ(loaded.getName(), "V2");
    EXPECT_EQ(loaded.getCrs(), "EPSG:2154");
    EXPECT_EQ(loaded.getLayers().size(), 1);
}

// JSON structure attendue
TEST_F(ProjectTest, CheckJSONStructure) {
    Project p("TestJSON", 2024.0, "EPSG:4326");
    p.addLayer(Layer("L", "EPSG:2154", 2023.0));

    ASSERT_TRUE(p.save(path));

    std::ifstream f(path);
    ASSERT_TRUE(f.is_open());
    std::string s((std::istreambuf_iterator<char>(f)), {});

    EXPECT_NE(s.find("\"version\""), std::string::npos);
    EXPECT_NE(s.find("\"format\""), std::string::npos);
    EXPECT_NE(s.find("\"name\""), std::string::npos);
    EXPECT_NE(s.find("\"layers\""), std::string::npos);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
