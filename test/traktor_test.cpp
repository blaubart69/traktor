#include "gtest/gtest.h"

#include "shared.h"
#include "calculate.h"

const int x_screen = 640;
const int y_screen = 480;

const int x_half = x_screen / 2;


TEST(CoordTest, Test_point_on_right_half)
{
    auto y_flucht = 100;
    CoordPoint coord = project_screen_point_to_coord(x_half + 100, 100, x_half, y_flucht);

    ASSERT_EQ(coord.x, 100);
    ASSERT_EQ(coord.y, y_flucht + 100);
}

TEST(CoordTest, Test_point_on_left_half)
{
    auto y_flucht = 100;
    CoordPoint coord = project_screen_point_to_coord(x_half - 100, 0, x_half, y_flucht);

    ASSERT_EQ( coord.x, -100);
    ASSERT_EQ( coord.y, y_flucht + 0);
}

TEST(ProjectOntoBaseline, Test1)
{
    CoordPoint coord;
    coord.x = 1;
    coord.y = 4;

    const int y_baseline = 10; // screensize + y_flucht
    float x_baseline = project_x_onto_baseline(coord, y_baseline);

    EXPECT_FLOAT_EQ( x_baseline, 2.5);
}

TEST(ProjectOntoBaseline, Test2)
{
    CoordPoint coord;
    coord.x = 2;
    coord.y = 4;

    const int y_baseline = 10; // screensize + y_flucht
    float x_baseline = project_x_onto_baseline(coord, y_baseline);

    EXPECT_FLOAT_EQ( x_baseline, 5);
}

TEST(ProjectOntoBaseline, Test_point_on_the_baseline_right)
{
    const int y_flucht = 0;

    int x_point = x_half + 160;
    int y_point = y_screen;
    CoordPoint coord = project_screen_point_to_coord(x_point, y_point, x_half, y_flucht);

    ASSERT_EQ( coord.x, 160);
    ASSERT_EQ( coord.y, 480);
    
    const int y_baseline = y_screen + y_flucht; // screensize + y_flucht
    float x_baseline = project_x_onto_baseline(coord, y_baseline);

    EXPECT_FLOAT_EQ( x_baseline, 160);
}

TEST(ProjectOntoBaseline, Test_point_on_the_baseline_left)
{
    const int y_flucht = 0;

    int x_point = x_half - 160;
    int y_point = y_screen;
    CoordPoint coord = project_screen_point_to_coord(x_point, y_point, x_half, y_flucht);

    ASSERT_EQ( coord.x, -160);
    ASSERT_EQ( coord.y, 480);
    
    const int y_baseline = y_screen + y_flucht; // screensize + y_flucht
    float x_baseline = project_x_onto_baseline(coord, y_baseline);

    EXPECT_FLOAT_EQ( x_baseline, -160);
}

TEST(ProjectOntoBaseline, Test_point_1)
{
    const int y_flucht = 0;

    int x_point = x_half - 80;
    int y_point = y_screen / 2;
    CoordPoint coord = project_screen_point_to_coord(x_point, y_point, x_half, y_flucht);

    ASSERT_EQ( coord.x, -80);
    ASSERT_EQ( coord.y, 240);
    
    const int y_baseline = y_screen + y_flucht; // screensize + y_flucht
    float x_baseline = project_x_onto_baseline(coord, y_baseline);

    EXPECT_FLOAT_EQ( x_baseline, -160);
}

TEST(ProjectOntoBaseline, Test_point_on_middle_line)
{
    const int y_flucht = 0;

    int x_point = x_half;
    int y_point = y_screen / 2;
    CoordPoint coord = project_screen_point_to_coord(x_point, y_point, x_half, y_flucht);

    ASSERT_EQ( coord.x, 0);
    ASSERT_EQ( coord.y, 240);
    
    const int y_baseline = y_screen + y_flucht; // screensize + y_flucht
    float x_baseline = project_x_onto_baseline(coord, y_baseline);

    EXPECT_FLOAT_EQ( x_baseline, 0);
}

TEST(ProjectOntoBaseline, Test_point_on_y_middle_x_10)
{
    const int y_flucht = 0;

    int x_point = x_half + 10;
    int y_point = y_screen / 2;
    CoordPoint coord = project_screen_point_to_coord(x_point, y_point, x_half, y_flucht);

    ASSERT_EQ( coord.x, 10);
    ASSERT_EQ( coord.y, 240);
    
    const int y_baseline = y_screen + y_flucht; // screensize + y_flucht
    float x_baseline = project_x_onto_baseline(coord, y_baseline);

    EXPECT_FLOAT_EQ( x_baseline, 20);
}

TEST(TestNegativMod, Test_1)
{
    ASSERT_EQ( -110 % 100, -10);
    ASSERT_EQ( -200 % 100, 0);
    ASSERT_EQ( -170 % 160, -10);
    ASSERT_EQ( -150 % 160, -150);
}