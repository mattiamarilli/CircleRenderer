[![Università degli studi di Firenze](https://i.imgur.com/1NmBfH0.png)](https://ingegneria.unifi.it)
# Circle Rendering - Sequential vs Parallel

## Overview

This program simulates the rendering of circles in a 2D space, with a focus on two different approaches: a **sequential** version and a **parallel** version using OpenMP. The goal of the program is to render a collection of randomly generated circles on a grid (image) while considering their depth (z-coordinate). The circles are sorted based on their depth, with the ones closer to the viewer rendered last (overlapping others).

The two versions differ in how they handle the computational load:
1. The **sequential version** renders circles one pixel at a time.
2. The **parallel version** splits the work across multiple threads to speed up the rendering process.

### Key Concepts
- **Circle Rendering:** Circles are drawn based on their coordinates (x, y), size (radius), and color (RGBA). The program checks if each pixel in the image lies within a circle and updates the pixel color accordingly.
- **Sorting by Depth:** Circles are sorted by their z-coordinate to ensure that circles closer to the observer are rendered on top of those further away.
- **Alpha Blending:** When multiple circles overlap, their colors are blended based on their alpha (transparency) values.

---

## Sequential Program

### Functionality:
The sequential program renders the circles one by one and checks each pixel in the image individually to determine if it's inside any of the circles. Circles are sorted by their z-coordinate so that closer circles are drawn last, ensuring proper layering.

### Main Steps:
1. **Sorting Circles:** Circles are sorted based on their z-coordinate (depth) in descending order.
2. **Pixel Check:** For each pixel in the image, the program checks if it's inside any of the circles by calculating the distance from the pixel to the circle's center.
3. **Color Calculation:** If a pixel lies within a circle, the pixel color is adjusted using alpha blending based on the circle's transparency.
4. **Image Creation:** The image is represented as a 2D array, where each element stores the RGB color values of a pixel.
5. **PPM Output:** The image is saved in PPM format for easy visualization.

### Performance:
Since the program operates sequentially, it does not take advantage of multiple processors, and rendering time increases with the number of circles or image resolution.

---

## Parallel Program

### Functionality:
The parallel version of the program is optimized to take advantage of multiple threads using OpenMP. The primary difference is the use of parallelism for rendering pixels, which divides the work among multiple threads, significantly improving performance for larger datasets.

### Main Steps:
1. **Sorting Circles:** Similar to the sequential version, the circles are sorted by their z-coordinate.
2. **Grid Optimization:** A 2D grid is created to optimize which circles need to be checked for each pixel. This reduces the number of unnecessary distance checks, making the rendering process faster.
3. **Parallel Pixel Rendering:** Using OpenMP's `#pragma omp parallel for collapse(2)`, the program renders the image in parallel, splitting the work across multiple threads. Each thread processes a portion of the image.
4. **Alpha Blending:** As in the sequential version, each pixel's color is updated using alpha blending when it's inside a circle.
5. **PPM Output:** The image is saved in PPM format as in the sequential version.

### Performance:
The parallel version offers substantial speedup, especially when rendering a large number of circles. The speedup depends on the number of threads used and the number of circles to be rendered. However, performance is influenced by factors like memory access patterns and the number of threads (too many threads may cause overhead).

---

## Differences in Implementation

### Sequential:
- Processes each pixel sequentially.
- Simpler to implement, as no thread synchronization or parallelization techniques are involved.
- Higher execution time as it only uses a single thread.

### Parallel:
- Uses OpenMP for parallel execution, allowing the rendering task to be distributed across multiple threads.
- The rendering process is split into chunks that each thread can handle independently, improving performance with larger inputs.
- Involves creating an optimized 2D grid to minimize unnecessary circle checks for each pixel, enhancing efficiency.

### Rendering Difference:
- **Sequential Rendering:** Loops over every pixel in the image for every circle, checking if the pixel is inside the circle and adjusting the color accordingly.
- **Parallel Rendering:** Uses OpenMP to parallelize the outer loop over pixels, allowing multiple sections of the image to be processed concurrently. This drastically reduces the total rendering time when multiple threads are available.

---

## Performance Evaluation

### Metrics:
- **Render Duration (s):** The time taken to render the image.
- **Speedup:** The ratio of the render duration with one thread to the render duration with multiple threads.
- **Efficiency:** The ratio of speedup to the number of threads used, which measures how efficiently the threads are utilized.

### Logging:
The performance results, including render duration, speedup, and efficiency, are logged into a file (`performance_log.txt`) for analysis.

---

## Conclusion

This program demonstrates the difference between sequential and parallel computing for image rendering. While the sequential version is simple and works well for small datasets, the parallel version leverages multi-threading to handle larger datasets more efficiently. By splitting the workload across multiple processors, the parallel version can provide significant performance improvements.

---

## Example Usage

### Build and Run Sequential Version:
```bash
g++ -o sequential_render sequential.cpp -std=c++11
./sequential_render
```

### Build and Run Parallel Version:
```bash
g++ -o parallel_render parallel.cpp -std=c++11 -fopenmp
./parallel_render
```

After running either version, an image will be generated in the `./images/` directory, and performance results will be saved in `performance_log.txt`.

