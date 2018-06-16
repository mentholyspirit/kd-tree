----------------------------------------------------------------------------------
Raytracer with kd-tree space partitioning
----------------------------------------------------------------------------------
The program uses a kd-tree built with Surface Area Heuristics, using an algorithm, 

that has a time complexity of O(N log N), the theoretical lower bound (because that's the lower bound of sorting).

It is based on this paper: http://www.eng.utah.edu/~cs6965/papers/kdtree.pdf
----------------------------------------------------------------------------------
There are no micro optimizations in this program, just keeping an eye on cache misses and -O3.

You will need OpenCV2: 

`brew install opencv@2`

Use 'make build' to build the program.

Usage kd_tree_raytracer <ply_model_path>
  Optional Parameters:
    --no-kdtree Raytrace without kd-Tree
    --interactive Interactive windowed mode

The program renders the highest resolution happy buddha model at 640x480 resolution at 6 seconds on a 2,3 GHz Intel Core i7. 


Here's a capture of the interactive mode (happy buddha, 1M triangles, 640x480 - 0.1s for actual rendering after kd-tree creation):

![Alt Text](https://media.giphy.com/media/1fhIuQTUSA3Z8W0iuw/giphy.gif)