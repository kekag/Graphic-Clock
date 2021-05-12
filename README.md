# Graphic-Clock
Lightweight graphic wall-clock using OpenGL and FreeGLUT. Instead of drawing each hand based on the current system time each frame, this generates an inital X, Y, ΔX, and ΔY for each hand from a single snapshot, then adjusts delta time based on the average frametimes for the redisplay function.


### Linux FreeGLUT install
`sudo apt-get install freeglut3-dev`

### Windows FreeGLUT install
Download the respective prepackaged [Windows binary](https://www.transmissionzero.co.uk/software/freeglut-devel/) either for the MSVC or MinGW compilers, then place the x86 library files at the root of the project. For use with Visual C++, you can link the third-party lib and include folders in the project properties to circumvent placing each library file in the project folder.

---

![example](clock.gif)
