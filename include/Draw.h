#ifndef DRAW_H
#define DRAW_H

namespace game
{
    // Draw a simple cylinder oriented along the Y axis.
    void draw_cylinder(float radius, float height, int segments = 48);

    // Draw an axis-aligned rectangular prism centered on the origin.
    void draw_box(float width, float height, float depth);
}

#endif // DRAW_H
