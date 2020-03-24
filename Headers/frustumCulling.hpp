// https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf

#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Plane
{
    float a, b, c, d;
};

void NormalizePlane(Plane &plane)
{
    float mag = sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
    plane.a = plane.a / mag;
    plane.b = plane.b / mag;
    plane.c = plane.c / mag;
    plane.d = plane.d / mag;
}

std::vector<Plane> ExtractPlanesGLM(
    const glm::mat4 &comboMatrix,
    bool normalize)
{
    std::vector<Plane> p_planes;
    // left clipping plane
    Plane left;
    left.a = comboMatrix[0][3] + comboMatrix[0][0];
    left.b = comboMatrix[1][3] + comboMatrix[1][0];
    left.c = comboMatrix[2][3] + comboMatrix[2][0];
    left.d = comboMatrix[3][3] + comboMatrix[3][0];
    p_planes.push_back(left);

    // right clipping plane
    Plane right;
    right.a = comboMatrix[0][3] - comboMatrix[0][0];
    right.b = comboMatrix[1][3] - comboMatrix[1][0];
    right.c = comboMatrix[2][3] - comboMatrix[2][0];
    right.d = comboMatrix[3][3] - comboMatrix[3][0];
    p_planes.push_back(right);

    // top clipping plane
    Plane top;
    top.a = comboMatrix[0][3] - comboMatrix[0][1];
    top.b = comboMatrix[1][3] - comboMatrix[1][1];
    top.c = comboMatrix[2][3] - comboMatrix[2][1];
    top.d = comboMatrix[3][3] - comboMatrix[3][1];
    p_planes.push_back(top);

    // bottom clipping plane
    Plane bottom;
    bottom.a = comboMatrix[0][3] + comboMatrix[0][1];
    bottom.b = comboMatrix[1][3] + comboMatrix[1][1];
    bottom.c = comboMatrix[2][3] + comboMatrix[2][1];
    bottom.d = comboMatrix[3][3] + comboMatrix[3][1];
    p_planes.push_back(bottom);

    // near clipping plane
    Plane near;
    near.a = comboMatrix[0][3] + comboMatrix[0][2];
    near.b = comboMatrix[1][3] + comboMatrix[1][2];
    near.c = comboMatrix[2][3] + comboMatrix[2][2];
    near.d = comboMatrix[3][3] + comboMatrix[3][2];
    p_planes.push_back(near);

    // far clipping plane
    Plane far;
    far.a = comboMatrix[0][3] - comboMatrix[0][2];
    far.b = comboMatrix[1][3] - comboMatrix[1][2];
    far.c = comboMatrix[2][3] - comboMatrix[2][2];
    far.d = comboMatrix[3][3] - comboMatrix[3][2];
    p_planes.push_back(far);

    if (normalize)
    {
        NormalizePlane(p_planes[0]);
        NormalizePlane(p_planes[1]);
        NormalizePlane(p_planes[2]);
        NormalizePlane(p_planes[3]);
        NormalizePlane(p_planes[4]);
        NormalizePlane(p_planes[5]);
    }
    return p_planes;
}

float DistanceToPoint(const Plane &plane, const glm::vec3 &pt)
{
    return (plane.a * pt.x) + (plane.b * pt.y) + (plane.c * pt.z) + plane.d;
}