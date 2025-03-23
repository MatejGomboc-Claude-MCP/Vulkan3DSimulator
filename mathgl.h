#pragma once

namespace mgl {
    // Simple 3D vector implementation for the triangle renderer
    struct vec3 {
        float x, y, z;
        
        vec3() : x(0.0f), y(0.0f), z(0.0f) {}
        vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    };
}
