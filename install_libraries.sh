#!/bin/bash

rm -rf Vendor
mkdir Vendor
git clone -b c https://github.com/Dav1dde/glad Vendor/glad
git clone https://github.com/glfw/glfw Vendor/glfw
git clone https://github.com/g-truc/glm/ Vendor/glm
git clone https://github.com/nothings/stb Vendor/stb
