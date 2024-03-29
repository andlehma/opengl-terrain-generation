![](https://user-images.githubusercontent.com/26948028/77484899-ba69c680-6df9-11ea-977e-d8347c28c174.png)

# Terrain Generation
Final project for my independent study in computer graphics. Lots of code from [learnopengl.com](https://learnopengl.com/) and [Glitter](https://github.com/Polytonic/Glitter), with much of the terrain generation algorithm coming from [this Red Blob Games blog post](https://www.redblobgames.com/maps/terrain-from-noise/).

# Video Demo
[![Video Demo](https://img.youtube.com/vi/MbxDeQu9QoE/0.jpg)](https://youtu.be/MbxDeQu9QoE)

# Dependencies
* glad
* glfw
* glm
* fastnoise
* stb_image.h

install them all with `install_libraries.sh`

# Installation
```
mkdir build
cd build
cmake ..
make
./terrain-generation
```

# Usage
WASD to move, Space to ascend, Left Shift to descend
Mouse to look
