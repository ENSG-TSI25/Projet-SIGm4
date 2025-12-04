#include <gtest/gtest.h>
#include <core/Project.hpp>
#include <core/Layer.hpp>

class ProjectTest : public ::testing::Test {
protected:
    Layer *layer1;
    Layer *layer2;
    Project *project1;
    Project *project2;

    void SetUp() override {
        layer1 = new Layer("Layer1");
        layer2 = new Layer("Layer2");
        project1 = new Project("TestProject1", 2020.0);
        project2 = new Project("TestProject2", 2021.0, "EPSG:3857", {*layer1, *layer2});
    }

    void TearDown() override {
        delete project1;
        delete project2;
        delete layer1;
        delete layer2;
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
    EXPECT_EQ(project2->getLayers()[0].getName(), "Layer1");
    EXPECT_EQ(project2->getLayers()[1].getName(), "Layer2");
}

TEST_F(ProjectTest, AddLayer) {
    project1->addLayer(*layer1);
    EXPECT_EQ(project1->getLayers().size(), 1);
    EXPECT_EQ(project1->getLayers()[0].getName(), "Layer1");
}

TEST_F(ProjectTest, AddEmptyLayer) {
    Layer emptyLayer("");
    project1->addLayer(emptyLayer);
    EXPECT_EQ(project1->getLayers().size(), 1);
}

TEST_F(ProjectTest, RemoveLayer) {
    project2->rmLayer(*layer1);
    EXPECT_EQ(project2->getLayers().size(), 1);
    EXPECT_EQ(project2->getLayers()[0].getName(), "Layer2");
}

TEST_F(ProjectTest, RemoveNonExistentLayer) {
    Layer layer3("Layer3");
    project2->rmLayer(layer3);
    EXPECT_EQ(project2->getLayers().size(), 2);
}

TEST_F(ProjectTest, SettersAndGetters) {
    project1->setName("NewProjectName");
    project1->setCrs("EPSG:32633");
    project1->setEpoch0(2022.0);

    EXPECT_EQ(project1->getName(), "NewProjectName");
    EXPECT_EQ(project1->getCrs(), "EPSG:32633");
    EXPECT_EQ(project1->getEpoch0(), 2022.0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}