#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <FastNoise.h>

struct Chunk
{
    glm::vec3 center;
    std::vector<float> vertices;
};

class Terrain
{
private:
    FastNoise heightNoise;
    FastNoise moistureNoise;
    std::vector<Chunk> chunks;
    float halfW;
    bool chunkExists(float x, float z);
    void generateChunk(float x, float z);
    float generateHeight(float x, float z);
    glm::vec3 generateColor(float x, float y, float z);
    float seaLevel;
    int renderDistance;
    int chunkSize;
    glm::vec3 green;
    glm::vec3 blue;
    int mapWidth;
    int mapHeight;
    int mapComps;
    unsigned char *mapData;
public:
    std::vector<Chunk> getChunks();
    void checkChunks(glm::vec3 playerPos, glm::mat4 comboMatrix);
    Terrain();
    Terrain(int c, int r, float s);
};