# very-own-path-tracer
Path tracer built from scratch in C++.

This path tracer was built using [smallpt](http://www.kevinbeason.com/smallpt/) by Kevin Beason and the [explanation slides](https://drive.google.com/file/d/0B8g97JkuSSBwUENiWTJXeGtTOHFmSm51UC01YWtCZw/view) by David Cline. This is very primitive path tracer with support to only Spheres and explicit lighting.

Learn more on my [blog post](https://medium.com/suriya-d-murthy/building-your-very-own-path-tracer-from-scratch-c-4d9d3ce5fea4) in medium, explaining the theory and working of a path tracer.

## Renders from the Path Tracer - 
4samples | 100samples
:-------------------------:|:-------------------------:
![4 samples render of smallpt](https://github.com/codesavory/very-own-path-tracer/blob/master/images/david%20cline/4samples_david_cline.jpg "4 samples")  |  ![100 samples render of smallpt](https://github.com/codesavory/very-own-path-tracer/blob/master/images/david%20cline/100samples_david_cline.jpg "100 samples")

500samples | 5000samples
:-------------------------:|:-------------------------:
![500 samples render of smallpt](https://github.com/codesavory/very-own-path-tracer/blob/master/images/david%20cline/500samples_david_cline.jpg "500 samples") |  ![5000 samples render of smallpt](https://github.com/codesavory/very-own-path-tracer/blob/master/images/david%20cline/5000samples_david_cline.jpg "5000 samples")

## Future works - 
1. **Importance Sampling** to probabilistically select a new ray to converge correctly to outgoing luminance on the surface point.
2. **Bidirectional Path Tracing** that combines two distinctive approaches to sample the integral in the same algorithm to obtain faster convergence of the integral.
3. **Volumetric Path Tracing** that extends the path tracing method with the effect to light scattering.
4. **Metropolis Light Transport** a method of perturbing previously found paths in order to increase performance.
