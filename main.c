#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raylib.h"

#define MAX_PARTICLES 1024
#define MAX_TRAIL 100
#define G 6.67430e-3f
#define SOFTENING 5.0f

typedef struct Particle {
    Vector2 pos;
    Vector2 vel;
    float mass;
    Vector2 trail[MAX_TRAIL];
    int trailCount;
} Particle;

Particle particles[MAX_PARTICLES];
int particleCount = 0;

void AddParticle(Vector2 pos, Vector2 vel, float mass) {
    if (particleCount < MAX_PARTICLES) {
        particles[particleCount].pos = pos;
        particles[particleCount].vel = vel;
        particles[particleCount].mass = mass;
        particles[particleCount].trailCount = 0;
        particleCount++;
    }
}

float ComputeCurvature(float x, float y) {
    float curvature = 0.0f;
    for (int i = 0; i < particleCount; i++) {
        float dx = x - particles[i].pos.x;
        float dy = y - particles[i].pos.y;
        float r = sqrtf(dx * dx + dy * dy + SOFTENING);
        curvature += 1.0f / r;
    }
    return curvature;
}

int main(int argc, char *argv[]) {
    int initialCount = 50;
    float mass = 250000000.0f;
    char *arrangement = "random";
    if (argc >= 2) {
        initialCount = atoi(argv[1]);
        if (initialCount < 1) initialCount = 50;
        if (initialCount > MAX_PARTICLES) initialCount = MAX_PARTICLES;
    }
    if (argc >= 3) {
        arrangement = argv[2];
    }
    if (argc >= 4) {
        mass = atof(argv[3]);
    }

    int screenWidth = 800;
    int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "N-Body Simulation");
    SetTargetFPS(60);

    if (strcmp(arrangement, "circle") == 0) {
        Vector2 center = { screenWidth / 2.0f, screenHeight / 2.0f };
        float radius = (screenWidth < screenHeight ? screenWidth : screenHeight) / 3.0f;
        for (int i = 0; i < initialCount; i++) {
            float angle = (2.0f * PI * i) / initialCount;
            Vector2 pos = { center.x + cosf(angle) * radius, center.y + sinf(angle) * radius };
            float speed = 50.0f;
            Vector2 vel = { -sinf(angle) * speed, cosf(angle) * speed };
            AddParticle(pos, vel, mass);
        }
    } else {
        for (int i = 0; i < initialCount; i++) {
            Vector2 pos = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
            Vector2 vel = { (float)GetRandomValue(-50, 50), (float)GetRandomValue(-50, 50) };
            AddParticle(pos, vel, mass);
        }
    }

    while (!WindowShouldClose()) {
        float dt = 1.0f / 60.0f;
        for (int i = 0; i < particleCount; i++) {
            Vector2 acceleration = { 0, 0 };
            for (int j = 0; j < particleCount; j++) {
                if (i == j) continue;
                Vector2 diff = { particles[j].pos.x - particles[i].pos.x,
                                 particles[j].pos.y - particles[i].pos.y };
                float distSqr = diff.x * diff.x + diff.y * diff.y + SOFTENING;
                float dist = sqrtf(distSqr);
                float force = G * particles[j].mass / distSqr;
                acceleration.x += (diff.x / dist) * force;
                acceleration.y += (diff.y / dist) * force;
            }
            particles[i].vel.x += acceleration.x * dt;
            particles[i].vel.y += acceleration.y * dt;
        }

        for (int i = 0; i < particleCount; i++) {
            particles[i].pos.x += particles[i].vel.x * dt;
            particles[i].pos.y += particles[i].vel.y * dt;
            particles[i].trail[particles[i].trailCount % MAX_TRAIL] = particles[i].pos;
            particles[i].trailCount++;
        }

#ifdef BOX
        for (int i = 0; i < particleCount; i++) {
            if (particles[i].pos.x >= screenWidth || particles[i].pos.x <= 0) {
                particles[i].vel.x *= -0.9;
            }
            if (particles[i].pos.y >= screenHeight || particles[i].pos.y <= 0) {
                particles[i].vel.y *= -0.9;
            }
        }
#endif // BOX

        // add a new particle on mouse click
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            Vector2 vel = { (float)GetRandomValue(-50, 50), (float)GetRandomValue(-50, 50) };
            AddParticle(mousePos, vel, mass);
        }
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        BeginDrawing();
        ClearBackground(WHITE);

#ifdef GRID        
        float gridSpacing = 20.0f;
        float sampleStep = 10.0f;
        float scale = 50.0f;  // adjust to control curvature exaggeration

        for (float y = 0; y <= screenHeight; y += gridSpacing) {
            Vector2 prev = { 0, y + scale * ComputeCurvature(0, y) };
            for (float x = sampleStep; x <= screenWidth; x += sampleStep) {
                float offset = scale * ComputeCurvature(x, y);
                Vector2 curr = { x, y + offset };
                DrawLineV(prev, curr, LIGHTGRAY);
                prev = curr;
            }
        }
        
        for (float x = 0; x <= screenWidth; x += gridSpacing) {
            Vector2 prev = { x + scale * ComputeCurvature(x, 0), 0 };
            for (float y = sampleStep; y <= screenHeight; y += sampleStep) {
                float offset = scale * ComputeCurvature(x, y);
                Vector2 curr = { x + offset, y };
                DrawLineV(prev, curr, LIGHTGRAY);
                prev = curr;
            }
        }
#endif // GRID

        for (int i = 0; i < particleCount; i++) {
            int count = particles[i].trailCount < MAX_TRAIL ? particles[i].trailCount : MAX_TRAIL;
            if (count > 1) {
                int startIndex = (particles[i].trailCount >= MAX_TRAIL) ? (particles[i].trailCount % MAX_TRAIL) : 0;
                Vector2 prev = particles[i].trail[startIndex];
                for (int k = 1; k < count; k++) {
                    int index = (startIndex + k) % MAX_TRAIL;
                    DrawLineV(prev, particles[i].trail[index], RED);
                    prev = particles[i].trail[index];
                }
            }
        }

        for (int i = 0; i < particleCount; i++) {
            DrawCircleV(particles[i].pos, 3, RED);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
