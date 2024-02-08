struct CoordPoint {
    int x;
    int y;
};

CoordPoint project_screen_point_to_coord(const int x_point_screen, const int y_point_screen, const int x_half, const int y_flucht);
float project_x_onto_baseline(const CoordPoint point, const int y_baseline);
