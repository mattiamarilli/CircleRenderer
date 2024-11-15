#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>
#include <omp.h>
#include <string>
#include <iomanip>
#include <sstream>

struct Circle {
    float x, y, z;
    float radius;
    int r, g, b, a;
};

// Comparator function to sort circles based on their z-coordinate (for depth sorting)
bool compareByZ(const Circle& a, const Circle& b) {
    return a.z > b.z;
}

// Function to check if a point (px, py) is inside a circle
bool isPointInCircle(const float px, const float py, const Circle& circle) {
    const float dx = px - circle.x;
    const float dy = py - circle.y;
    return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}

// Main rendering function for drawing the circles onto the image
void renderCircles(const std::vector<Circle>& circles, std::vector<std::vector<std::vector<int>>>& image, int width, int height, int numThreads) {
    // Create a copy of the circles and sort them by z-coordinate (in descending order)
    // Circles with a higher z value will be rendered last (closer to the viewer)
    std::vector<Circle> sortedCircles = circles;
    std::sort(sortedCircles.begin(), sortedCircles.end(), compareByZ);

    // Define the grid cell size (used to optimize circle intersection checks)
    const int gridSize = 100;

    // Create a 3D grid where each cell contains the circles that might intersect that cell
    // The grid divides the rendering area into smaller cells based on the grid size
    std::vector<std::vector<std::vector<Circle>>> grid(
        (height + gridSize - 1) / gridSize,
        std::vector<std::vector<Circle>>((width + gridSize - 1) / gridSize)
    );

    // Populate the grid with circles by checking which cells they intersect
    for (const auto& circle : sortedCircles) {
        // Calculate the minimum and maximum grid cell indices that the circle intersects in both the X and Y directions
        int minX = std::max(0, static_cast<int>((circle.x - circle.radius) / gridSize));
        int maxX = std::min(static_cast<int>(grid[0].size()) - 1, static_cast<int>((circle.x + circle.radius) / gridSize));
        int minY = std::max(0, static_cast<int>((circle.y - circle.radius) / gridSize));
        int maxY = std::min(static_cast<int>(grid.size()) - 1, static_cast<int>((circle.y + circle.radius) / gridSize));

        // Add the circle to all grid cells that it potentially intersects
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                grid[y][x].push_back(circle);
            }
        }
    }

    // Parallel rendering of the image
    // Each thread will process a portion of the image. We use "collapse(2)" to combine the two nested loops into one parallelized loop.
    #pragma omp parallel for collapse(2)  if(numThreads > 1)
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            // Initial color for the pixel (white background)
            int r = 255, g = 255, b = 255;

            // Determine the grid cell coordinates for the current pixel (i, j)
            int gridX = j / gridSize;
            int gridY = i / gridSize;

            // Check all circles that could potentially affect this pixel in the current grid cell
            for (const auto& circle : grid[gridY][gridX]) {
                if (isPointInCircle(j, i, circle)) {
                    // Calculate the alpha blending for the circle color based on its transparency (a)
                    float alpha = circle.a / 255.0f;
                    r = (int)((1 - alpha) * r + alpha * circle.r);
                    g = (int)((1 - alpha) * g + alpha * circle.g);
                    b = (int)((1 - alpha) * b + alpha * circle.b);
                }
            }

            // Assign the computed color to the image pixel (i, j)
            image[i][j][0] = r;
            image[i][j][1] = g;
            image[i][j][2] = b;
        }
    }
}

// Function to save the rendered image to a PPM file format
void saveImageToFile(const std::vector<std::vector<std::vector<int>>>& image, int width, int height) {
    const std::string filename = "./images/circles.ppm";

    std::ofstream out(filename);
    out << "P3\n" << width << " " << height << "\n255\n";
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            out << image[i][j][0] << " "
                << image[i][j][1] << " "
                << image[i][j][2] << " ";
        }
        out << "\n";
    }
    out.close();
}

// Function to generate a set of random circles for testing
std::vector<Circle> generateRandomCircles(const int numCircles, const int width, const int height) {
    std::vector<Circle> circles;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);

    std::uniform_real_distribution<float> xDist(0, width);
    std::uniform_real_distribution<float> yDist(0, height);
    std::uniform_real_distribution<float> zDist(0, 10);
    std::uniform_real_distribution<float> radiusDist(10, 50);
    std::uniform_int_distribution<int> colorDist(0, 255);

    for (int i = 0; i < numCircles; ++i) {
        Circle c;
        c.x = xDist(gen);
        c.y = yDist(gen);
        c.z = zDist(gen);
        c.radius = radiusDist(gen);
        c.r = colorDist(gen);
        c.g = colorDist(gen);
        c.b = colorDist(gen);
        c.a = colorDist(gen);
        circles.push_back(c);
    }
    return circles;
}

// Function to log the performance results of the rendering
void logExecutionDetails(const std::string& filename, const std::vector<std::tuple<int, int, double, double, double>>& results) {
    std::ofstream out(filename);
    out << "Circles | Threads | Render Duration (s) | Speedup | Efficiency\n";
    out << "-----------------------------------------------------------\n";
    for (const auto& result : results) {
        out << std::setw(7) << std::get<0>(result) << " | "
            << std::setw(7) << std::get<1>(result) << " | "
            << std::setw(18) << std::fixed << std::setprecision(4) << std::get<2>(result) << " | "
            << std::setw(7) << std::fixed << std::setprecision(2) << std::get<3>(result) << " | "
            << std::setw(9) << std::fixed << std::setprecision(2) << std::get<4>(result) << "\n";
    }
    out.close();
}

// Main function that sets up the experiment and measures performance
int main() {
    int width = 2000;
    int height = 2000;
    int numMeasurements = 1;

    std::vector<int> numCirclesList = {10, 100, 1000, 10000, 100000};
    std::vector<int> numThreadsList = {1, 2, 4, 8, 16};
    std::vector<std::tuple<int, int, double, double, double>> results;

    for (int numCircles : numCirclesList) {
        auto circles = generateRandomCircles(numCircles, width, height);
        double baseDuration = 0.0;

        for (int numThreads : numThreadsList) {
            omp_set_num_threads(numThreads);

            double totalDuration = 0.0;
            for (int measurement = 0; measurement < numMeasurements; ++measurement) {
                std::vector<std::vector<std::vector<int>>> image(height, std::vector<std::vector<int>>(width, {255, 255, 255}));

                double start = omp_get_wtime();
                renderCircles(circles, image, width, height, numThreads);
                double end = omp_get_wtime();

                totalDuration += (end - start);
                saveImageToFile(image, width, height);
            }

            double avgRenderDuration = totalDuration / numMeasurements;

            if (numThreads == 1) {
                baseDuration = avgRenderDuration;
            }
            double speedup = baseDuration / avgRenderDuration;
            double efficiency = speedup / numThreads;

            std::cout << "Circles: " << numCircles << ", Threads: " << numThreads
                      << ", Avg Duration: " << avgRenderDuration << "s, Speedup: " << speedup
                      << ", Efficiency: " << efficiency << std::endl;

            results.push_back({numCircles, numThreads, avgRenderDuration, speedup, efficiency});
        }
    }

    //Log
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);
    std::ostringstream timestamp;
    timestamp << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    std::string filePath = "./performancelog/performance_log_" + timestamp.str() + ".txt";
    logExecutionDetails(filePath, results);

    return 0;
}
