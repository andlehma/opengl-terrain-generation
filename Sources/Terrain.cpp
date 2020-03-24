#include "Terrain.hpp"
#include "frustumCulling.hpp"
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// rounds number to nearest multiple of another number
// used for determining which chunk the player is in
int roundToMultiple(int number, int multiple)
{
    bool neg = false;
    if (number < 0)
    {
        neg = true;
        number = -number;
    }
    int result = ((number + multiple / 2) / multiple) * multiple;
    if (neg)
        result = -result;
    return result;
}

// get the noise values from (-1, 1) to (0, 1)
float normalizeNoise(float n)
{
    n += 1;
    n = n / 2;
    return n;
}

// default constructor
Terrain::Terrain()
{
    heightNoise.SetNoiseType(FastNoise::Simplex);
    heightNoise.SetFrequency(0.005);
    moistureNoise.SetNoiseType(FastNoise::Simplex);
    moistureNoise.SetFrequency(0.01);
    chunkSize = 16;
    halfW = (float)chunkSize / 2;
    renderDistance = 7;
    seaLevel = 6;
    mapData = stbi_load("heightmoisturemap.png", &mapWidth, &mapHeight, &mapComps, 3);
}

// parameterized constructor
Terrain::Terrain(int c, int r, float s) : Terrain()
{
    chunkSize = c;
    renderDistance = r;
    seaLevel = s;
}

// generate realistic terrain using multiple frequencies of noise
// https://www.redblobgames.com/maps/terrain-from-noise/
float Terrain::generateHeight(float x, float z)
{
    float heightMult = 15;
    float y = normalizeNoise(heightNoise.GetNoise(x, z));
    y += (0.5 * normalizeNoise(heightNoise.GetNoise(2 * x, 2 * z)));
    y += (0.25 * normalizeNoise(heightNoise.GetNoise(4 * x, 4 * z)));
    y += (0.125 * normalizeNoise(heightNoise.GetNoise(8 * x, 8 * z)));
    y += (0.0625 * normalizeNoise(heightNoise.GetNoise(16 * x, 16 * z)));
    y = pow(y, 2.3);
    y *= heightMult;

    // if height is low enough, make it water
    if (y < seaLevel)
        y = seaLevel;

    return y;
}

// use heightmoisturemap.png to assign each point on the grid a
// color based on elevation (y axis in the image) and moisture (x axis)
glm::vec3 Terrain::generateColor(float x, float y, float z)
{
    float mountainTopHeight = 65.0f;
    float range = mountainTopHeight - seaLevel;

    // get map x coordinate
    float m = normalizeNoise(moistureNoise.GetNoise(x, z));
    float moistureIndex = ceil(m * (float)mapWidth);
    if (moistureIndex > mapWidth - 1)
        moistureIndex = mapWidth - 1;

    // get map y coordinate
    float heightSlope = (float)(mapHeight / range);
    int heightIndex = ceil((heightSlope * y) - (seaLevel * heightSlope));
    if (heightIndex > mapHeight - 1)
        heightIndex = mapHeight - 1;

    int dataIndex = (mapWidth * heightIndex * 3) + (moistureIndex * 3);
    glm::vec3 color;
    color.x = (float)mapData[dataIndex + 0] / 255;
    color.y = (float)mapData[dataIndex + 1] / 255;
    color.z = (float)mapData[dataIndex + 2] / 255;
    return color;
}

// check if a chunk with center x,z exists in our chunk vector
bool Terrain::chunkExists(float x, float z)
{
    bool result = false;
    for (int i = 0; i < (int)chunks.size(); i++)
    {
        if (chunks[i].center.x == x && chunks[i].center.z == z)
            result = true;
    }
    return result;
}

// generate chunk with center x, z and push it to our chunk vector
void Terrain::generateChunk(float x, float z)
{
    std::vector<float> newVertices;

    // generate two triangles for each grid square
    for (int i = x; i < x + chunkSize; i++)
    {
        for (int j = z; j < z + chunkSize; j++)
        {
            // x and z values all exist on the grid
            float left = i - halfW;
            float right = left + 1;
            float top = j - halfW;
            float bottom = top + 1;

            // y value defined by noise function
            float topLeftHeight = generateHeight(left, top);
            float topRightHeight = generateHeight(right, top);
            float bottomLeftHeight = generateHeight(left, bottom);
            float bottomRightHeight = generateHeight(right, bottom);

            // first triangle
            // ____
            // |  /
            // | /
            // |/
            newVertices.push_back(left);
            newVertices.push_back(topLeftHeight);
            newVertices.push_back(top);
            glm::vec3 color = generateColor(left, topLeftHeight, top);
            newVertices.push_back(color.x);
            newVertices.push_back(color.y);
            newVertices.push_back(color.z);

            newVertices.push_back(left);
            newVertices.push_back(bottomLeftHeight);
            newVertices.push_back(bottom);
            color = generateColor(left, bottomLeftHeight, bottom);
            newVertices.push_back(color.x);
            newVertices.push_back(color.y);
            newVertices.push_back(color.z);

            newVertices.push_back(right);
            newVertices.push_back(topRightHeight);
            newVertices.push_back(top);
            color = generateColor(right, topRightHeight, top);
            newVertices.push_back(color.x);
            newVertices.push_back(color.y);
            newVertices.push_back(color.z);

            // second triangle
            //   /|
            //  / |
            // /__|
            newVertices.push_back(right);
            newVertices.push_back(topRightHeight);
            newVertices.push_back(top);
            color = generateColor(right, topRightHeight, top);
            newVertices.push_back(color.x);
            newVertices.push_back(color.y);
            newVertices.push_back(color.z);

            newVertices.push_back(right);
            newVertices.push_back(bottomRightHeight);
            newVertices.push_back(bottom);
            color = generateColor(right, bottomRightHeight, bottom);
            newVertices.push_back(color.x);
            newVertices.push_back(color.y);
            newVertices.push_back(color.z);

            newVertices.push_back(left);
            newVertices.push_back(bottomLeftHeight);
            newVertices.push_back(bottom);
            color = generateColor(left, bottomLeftHeight, bottom);
            newVertices.push_back(color.x);
            newVertices.push_back(color.y);
            newVertices.push_back(color.z);
        }
    }

    // generate new chunk object and push to chunk vector
    Chunk chunk;
    chunk.center = glm::vec3(x, generateHeight(x, z), z);
    chunk.vertices = newVertices;
    chunks.push_back(chunk);
}

// getter for chunk vector
std::vector<Chunk> Terrain::getChunks()
{
    return chunks;
}

// check if necessary chunks exist, and generate them if not
void Terrain::checkChunks(glm::vec3 playerPos, glm::mat4 comboMatrix)
{
    // center chunk is located at player position rounded to
    // nearest multiple of chunkSize
    int centerChunkX = roundToMultiple(playerPos.x, chunkSize);
    int centerChunkZ = roundToMultiple(playerPos.z, chunkSize);

    // check/generate center chunk
    if (!chunkExists(centerChunkX, centerChunkZ))
    {
        generateChunk(centerChunkX, centerChunkZ);
    }

    // generate list of chunk centers
    // defined by renderDistance
    std::vector<glm::vec3> centers;
    std::vector<Plane> frustumPlanes = ExtractPlanesGLM(comboMatrix, true);
    float chunkHypotenuse = 1.41421356237 * chunkSize; // sqrt(2)
    for (int i = centerChunkX - (chunkSize * renderDistance);
         i <= centerChunkX + (chunkSize * renderDistance);
         i += chunkSize)
    {
        for (int j = centerChunkZ - (chunkSize * renderDistance);
             j <= centerChunkZ + (chunkSize * renderDistance);
             j += chunkSize)
        {
            // check if chunk is in view frustum
            glm::vec3 center = glm::vec3(i, generateHeight(i, j), j);
            bool cull = false;
            for (int k = 0; k < 6; k++)
            {
                if (DistanceToPoint(frustumPlanes[k], center) <= -(chunkHypotenuse / 2))

                    cull = true;
            }
            if (!cull)
                centers.push_back(center);
        }
    }

    // check if each chunk exists from our list of chunk centers
    for (int i = 0; i < (int)centers.size(); i++)
    {
        if (!chunkExists(centers[i].x, centers[i].z))
            generateChunk(centers[i].x, centers[i].z);
    }

    // cull chunks not in render distance or view frustum
    std::vector<Chunk> newChunks;
    for (int i = 0; i < (int)chunks.size(); i++)
    {
        bool keepChunk = false;
        for (int j = 0; j < (int)centers.size(); j++)
        {
            if (chunks[i].center.x == centers[j].x && chunks[i].center.z == centers[j].z)
                keepChunk = true;
        }
        if (keepChunk)
            newChunks.push_back(chunks[i]);
    }
    chunks = newChunks;
}