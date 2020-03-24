// Local Headers
#include "shader.hpp"
#include "camera.hpp"
#include "Terrain.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Standard Headers
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <algorithm>

// debug options
bool DEBUG_FPS = false;
bool DEBUG_NUM_CHUNKS = false;
bool cmdOptionExists(char **begin, char **end, const std::string &option)
{
    return std::find(begin, end, option) != end;
}

// window width/height
const int mWidth = 1600;
const int mHeight = 900;

// initialize variables
void processInput(GLFWwindow *window);
float deltaTime = 0.0f;
float lastFrame = 0.0f;
Camera myCam;
bool firstMouse = true;
float lastX = mWidth / 2.0;
float lastY = mHeight / 2.0;
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.5f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    myCam.ProcessMouseMovement(xoffset, yoffset);
}

int main(int argc, char *argv[])
{
    // process arguments
    DEBUG_FPS = cmdOptionExists(argv, argv + argc, "--debug-fps");
    DEBUG_NUM_CHUNKS = cmdOptionExists(argv, argv + argc, "--debug-num-chunks");

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "Terrain Generation", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr)
    {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    glfwSetCursorPosCallback(mWindow, mouse_callback);
    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    // initialize shader
    Shader mainShader;
    mainShader.attach("main.vs").attach("main.fs");
    mainShader.link();

    // initialize Terrain object
    Terrain myTerrain;

    // player movement speed
    myCam.MovementSpeed = 20.0f;

    // draw nearer objects over farther objects
    glEnable(GL_DEPTH_TEST);

    // draw in wireframe mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Rendering Loop
    int nFrames = 0;
    int totalFrames = 0;
    int nSeconds = 0;
    while (glfwWindowShouldClose(mWindow) == false)
    {
        // timing
        float currentFrame = glfwGetTime();
        nFrames++;
        totalFrames++;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (currentFrame - nSeconds >= 1.0)
        {
            if (DEBUG_FPS)
                std::cout << nFrames << " fps" << std::endl;
            nFrames = 0;
            nSeconds++;
        }

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // input
        processInput(mWindow);

        // activate shader
        mainShader.activate();

        // per-frame transformations
        glm::mat4 projection = glm::perspective(
            glm::radians(100.0f),
            (float)mWidth / (float)mHeight, 0.1f, 200.0f);
        mainShader.bind("projection", projection);

        glm::mat4 view = glm::mat4(1.0f);
        view = myCam.GetViewMatrix();
        mainShader.bind("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        mainShader.bind("model", model);

        // check and generate new chunks
        myTerrain.checkChunks(myCam.Position, projection * view * model);

        // draw each chunk
        std::vector<Chunk> chunks = myTerrain.getChunks();
        if (DEBUG_NUM_CHUNKS)
            std::cout << "Drawing " << chunks.size() << " chunks on frame " << totalFrames << std::endl;
        for (int i = 0; i < (int)chunks.size(); i++)
        {
            int stride = 6;

            // send vertices to the GPU
            unsigned int VBO, VAO;
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(
                GL_ARRAY_BUFFER,
                chunks[i].vertices.size() * sizeof(chunks[i].vertices.front()),
                &chunks[i].vertices.front(),
                GL_STATIC_DRAW);

            // position attribute
            glVertexAttribPointer(
                0, 3, GL_FLOAT, GL_FALSE,
                stride * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);

            // color attribute
            glVertexAttribPointer(
                1, 3, GL_FLOAT, GL_FALSE,
                stride * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // render
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, chunks[i].vertices.size());

            // delete buffers
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }
    glfwTerminate();
    return EXIT_SUCCESS;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        myCam.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        myCam.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        myCam.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        myCam.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        myCam.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        myCam.ProcessKeyboard(UP, deltaTime);
}
