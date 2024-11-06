#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <random>

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
bool isPointInCircle(int px, int py, const Circle& circle) {
    int dx = px - circle.x;
    int dy = py - circle.y;
    return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}

// Funzione per disegnare i cerchi su un'immagine
void renderCircles(const std::vector<Circle>& circles, int width, int height, int numCircles) {
    // Creazione del nome del file dinamicamente
    std::string filename = "./images/" + std::to_string(numCircles) + "circles_" + std::to_string(width) + "Wx" + std::to_string(height) + "H.ppm";

    // Creazione dell'immagine in formato PPM
    std::ofstream out(filename);
    out << "P3\n" << width << " " << height << "\n255\n";  // Header del formato PPM

    // Inizializza l'immagine con uno sfondo bianco
    std::vector<std::vector<std::vector<int>>> image(height, std::vector<std::vector<int>>(width, {255, 255, 255}));

    // Ordinamento dei cerchi per la profondità (Z)
    std::vector<Circle> sortedCircles = circles;
    std::sort(sortedCircles.begin(), sortedCircles.end(), compareByZ);

    // Disegnare i cerchi sull'immagine
    for (const auto& circle : sortedCircles) {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (isPointInCircle(j, i, circle)) {
                    // Gestione della trasparenza: modifichiamo il colore del pixel in base alla trasparenza
                    int r = image[i][j][0];
                    int g = image[i][j][1];
                    int b = image[i][j][2];

                    // Combinazione del colore del cerchio con quello già presente (trasparenza)
                    float alpha = circle.a / 255.0f;
                    r = (int)((1 - alpha) * r + alpha * circle.r);
                    g = (int)((1 - alpha) * g + alpha * circle.g);
                    b = (int)((1 - alpha) * b + alpha * circle.b);

                    // Aggiorna il pixel dell'immagine
                    image[i][j][0] = r;
                    image[i][j][1] = g;
                    image[i][j][2] = b;
                }
            }
        }
    }

    // Scrive i dati dell'immagine nel file PPM
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            out << image[i][j][0] << " "
                << image[i][j][1] << " "
                << image[i][j][2] << " ";
        }
        out << "\n";
    }

    out.close();
    std::cout << "Immagine salvata come " << filename << std::endl;
}

// Funzione per generare cerchi casuali
std::vector<Circle> generateRandomCircles(int numCircles, int width, int height) {
    std::vector<Circle> circles;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);  // Uniforme tra 0 e 1 per randomizzare

    std::uniform_real_distribution<float> xDist(0, width);
    std::uniform_real_distribution<float> yDist(0, height);
    std::uniform_real_distribution<float> zDist(0, 10);  // Profondità random tra 0 e 10
    std::uniform_real_distribution<float> radiusDist(10, 50);  // Raggio tra 10 e 50
    std::uniform_int_distribution<int> colorDist(0, 255);  // Colori casuali per RGBA

    for (int i = 0; i < numCircles; ++i) {
        Circle c;
        c.x = xDist(gen);
        c.y = yDist(gen);
        c.z = zDist(gen);
        c.radius = radiusDist(gen);
        c.r = colorDist(gen);
        c.g = colorDist(gen);
        c.b = colorDist(gen);
        c.a = colorDist(gen);  // Trasparenza random
        circles.push_back(c);
    }
    return circles;
}

int main() {
    // Dimensioni dell'immagine
    int width = 1000;
    int height = 1000;

    // Numero di cerchi da generare
    int numCircles = 200;  // Puoi modificare questo numero per testare diversi casi

    // Generazione dei cerchi casuali
    auto circles = generateRandomCircles(numCircles, width, height);

    // Misura del tempo di esecuzione
    auto start = std::chrono::high_resolution_clock::now();

    // Renderizzare i cerchi e salvare l'immagine
    renderCircles(circles, width, height, numCircles);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Tempo di esecuzione per " << numCircles << " cerchi: " << duration.count() << " secondi." << std::endl;

    return 0;
}
