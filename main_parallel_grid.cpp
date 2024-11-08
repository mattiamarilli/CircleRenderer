#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>
#include <omp.h>

struct Circle {
    float x, y, z;  // Coordinate 3D
    float radius;
    int r, g, b, a; // Colore con trasparenza (RGBA)
};

// Funzione di ordinamento per profondità (Z)
bool compareByZ(const Circle& a, const Circle& b) {
    return a.z > b.z;  // Ordinamento per profondità (distanza lungo l'asse Z)
}

// Funzione per calcolare la posizione di un pixel all'interno del cerchio
bool isPointInCircle(const float px, const float py, const Circle& circle) {
    const float dx = px - circle.x;
    const float dy = py - circle.y;
    return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}

// Funzione per disegnare i cerchi su un'immagine (rendering)
void renderCircles(const std::vector<Circle>& circles, std::vector<std::vector<std::vector<int>>>& image, int width, int height) {
    // Ordinamento dei cerchi per la profondità (Z)
    std::vector<Circle> sortedCircles = circles;
    std::sort(sortedCircles.begin(), sortedCircles.end(), compareByZ);

    // Creazione di una griglia per la partizione spaziale
    const int gridSize = 100;
    std::vector<std::vector<std::vector<Circle>>> grid((height + gridSize - 1) / gridSize, std::vector<std::vector<Circle>>((width + gridSize - 1) / gridSize));

    for (const auto& circle : sortedCircles) {
        int minX = std::max(0, static_cast<int>((circle.x - circle.radius) / gridSize));
        int maxX = std::min(static_cast<int>(grid[0].size()) - 1, static_cast<int>((circle.x + circle.radius) / gridSize));
        int minY = std::max(0, static_cast<int>((circle.y - circle.radius) / gridSize));
        int maxY = std::min(static_cast<int>(grid.size()) - 1, static_cast<int>((circle.y + circle.radius) / gridSize));
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                grid[y][x].push_back(circle);
            }
        }
    }

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int r = 255, g = 255, b = 255;  // Colore di sfondo bianco
            int gridX = j / gridSize;
            int gridY = i / gridSize;
            for (const auto& circle : grid[gridY][gridX]) {
                if (isPointInCircle(j, i, circle)) {
                    float alpha = circle.a / 255.0f;
                    r = (int)((1 - alpha) * r + alpha * circle.r);
                    g = (int)((1 - alpha) * g + alpha * circle.g);
                    b = (int)((1 - alpha) * b + alpha * circle.b);
                }
            }
            image[i][j][0] = r;
            image[i][j][1] = g;
            image[i][j][2] = b;
        }
    }
}

// Funzione per salvare l'immagine in un file PPM
void saveImageToFile(const std::vector<std::vector<std::vector<int>>>& image, int width, int height, int numCircles) {
    // Creazione del nome del file dinamicamente
    const std::string filename = "./images/" + std::to_string(numCircles) + "circles_" + std::to_string(width) + "Wx" + std::to_string(height) + "H.ppm";

    std::ofstream out(filename);
    out << "P3\n" << width << " " << height << "\n255\n";  // Header del formato PPM

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

// Funzione per generare cerchi casuali
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

int main() {
    int width = 2000;
    int height = 2000;
    int numCircles = 1000;

    auto circles = generateRandomCircles(numCircles, width, height);

    for (int numThreads = 1; numThreads <= 16; numThreads *= 2) {
        omp_set_num_threads(numThreads);

        // Inizializzazione dell'immagine con uno sfondo bianco
        std::vector<std::vector<std::vector<int>>> image(height, std::vector<std::vector<int>>(width, {255, 255, 255}));

        double render_start_time = omp_get_wtime();
        renderCircles(circles, image, width, height);
        double render_end_time = omp_get_wtime();

        double render_duration = render_end_time - render_start_time;
        std::cout << "Tempo di rendering con " << numThreads << " thread: " << render_duration << " secondi." << std::endl;

        // Salva l'immagine su file
        saveImageToFile(image, width, height, numCircles);
    }

    return 0;
}
