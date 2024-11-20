#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <random>

// Structure to represent a Circle with 3D coordinates (x, y, z), radius, and RGBA color values
struct Circle {
    float x, y, z;  // 3D Coordinates of the circle's center
    float radius;   // Radius of the circle
    int r, g, b, a; // RGBA values for the circle's color (Red, Green, Blue, Alpha for transparency)
};

// Comparator function to sort circles by their Z value (depth)
bool compareByZ(const Circle& a, const Circle& b) {
    return a.z > b.z;  // Sort in descending order, so that closer circles come first
}

// Function to check if a point (px, py) is inside a given circle
bool isPointInCircle(int px, int py, const Circle& circle) {
    // Calculate the squared distance from the point to the circle's center
    int dx = px - circle.x;
    int dy = py - circle.y;
    // Check if the point is inside the circle using the equation of a circle: (dx^2 + dy^2 <= radius^2)
    return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}

// Main rendering function for drawing the circles onto the image
void renderCirclesGrid(const std::vector<Circle>& circles, std::vector<std::vector<std::vector<int>>>& image, int width, int height, int numThreads) {
    // Create a copy of the circles and sort them by z-coordinate (in descending order)
    // Circles with a higher z value will be rendered last (closer to the viewer)
    std::vector<Circle> sortedCircles = circles;
    std::sort(sortedCircles.begin(), sortedCircles.end(), compareByZ);

    // The grid size will be a fraction of the smallest image dimension (width or height)
    const int gridSize = std::max(1, std::min(width, height) / 10);

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

// Function to render circles on an image and save it as a PPM file
void renderCirclesNaive(const std::vector<Circle>& circles, int width, int height, int numCircles) {
    // Dynamically generate the filename based on the number of circles and image dimensions
    std::string filename = "./images/" + std::to_string(numCircles) + "circles_" + std::to_string(width) + "Wx" + std::to_string(height) + "H.ppm";

    // Create and open the file to write the PPM image data
    std::ofstream out(filename);
    out << "P3\n" << width << " " << height << "\n255\n";  // PPM header: format, width, height, max color value (255)

    // Initialize the image with a white background
    std::vector<std::vector<std::vector<int>>> image(height, std::vector<std::vector<int>>(width, {255, 255, 255}));

    // Sort the circles by their Z value (depth) so that closer circles are rendered on top
    std::vector<Circle> sortedCircles = circles;
    std::sort(sortedCircles.begin(), sortedCircles.end(), compareByZ);

    // Loop over each circle to render it on the image
    for (const auto& circle : sortedCircles) {
        // Loop over all pixels in the image
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                // Check if the current pixel (j, i) is inside the current circle
                if (isPointInCircle(j, i, circle)) {
                    // Retrieve the current pixel color (RGB) from the image
                    int r = image[i][j][0];
                    int g = image[i][j][1];
                    int b = image[i][j][2];

                    // Blend the circle's color with the pixel's color using the alpha transparency value
                    float alpha = circle.a / 255.0f;  // Convert alpha to a float between 0 and 1
                    r = (int)((1 - alpha) * r + alpha * circle.r); // Blend red channel
                    g = (int)((1 - alpha) * g + alpha * circle.g); // Blend green channel
                    b = (int)((1 - alpha) * b + alpha * circle.b); // Blend blue channel

                    // Update the pixel color in the image
                    image[i][j][0] = r;
                    image[i][j][1] = g;
                    image[i][j][2] = b;
                }
            }
        }
    }

    // Write the pixel data to the PPM file
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            out << image[i][j][0] << " "  // Red component
                << image[i][j][1] << " "  // Green component
                << image[i][j][2] << " "; // Blue component
        }
        out << "\n";  // New line after each row of pixels
    }

    // Close the output file after writing the image
    out.close();
    std::cout << "Image saved as " << filename << std::endl;  // Notify the user that the image was saved
}

// Function to generate random circles with random properties
std::vector<Circle> generateRandomCircles(int numCircles, int width, int height) {
    std::vector<Circle> circles;
    std::random_device rd;
    std::mt19937 gen(rd());  // Mersenne Twister random number generator
    std::uniform_real_distribution<> dis(0, 1);  // Uniform distribution between 0 and 1 for randomness

    // Define random distributions for circle properties
    std::uniform_real_distribution<float> xDist(0, width);     // X coordinate between 0 and width
    std::uniform_real_distribution<float> yDist(0, height);    // Y coordinate between 0 and height
    std::uniform_real_distribution<float> zDist(0, 10);        // Random depth (Z coordinate) between 0 and 10
    std::uniform_real_distribution<float> radiusDist(10, 50);   // Random radius between 10 and 50
    std::uniform_int_distribution<int> colorDist(0, 255);       // Random color components (RGBA) between 0 and 255

    // Generate random circles
    for (int i = 0; i < numCircles; ++i) {
        Circle c;
        c.x = xDist(gen);     // Random X coordinate
        c.y = yDist(gen);     // Random Y coordinate
        c.z = zDist(gen);     // Random Z coordinate (depth)
        c.radius = radiusDist(gen); // Random radius
        c.r = colorDist(gen);  // Random red color component
        c.g = colorDist(gen);  // Random green color component
        c.b = colorDist(gen);  // Random blue color component
        c.a = colorDist(gen);  // Random alpha (transparency)
        circles.push_back(c);  // Add the generated circle to the list
    }
    return circles;  // Return the list of random circles
}

int main() {
    // Define the image dimensions
    int width = 1000;
    int height = 1000;

    // Define the number of circles to generate
    int numCircles = 200;

    // Generate random circles
    auto circles = generateRandomCircles(numCircles, width, height);

    // Render the circles and save the resulting image
    renderCirclesNaive(circles, width, height, numCircles);

    std::cout << "Execution time for rendering " << numCircles << " circles: " << std::endl;

    return 0;  // Return success
}
