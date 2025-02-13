#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raylib.h"
#include "raymath.h"

#define MAX_PARTICLES 1024
#define MAX_TRAIL 100
#define G 6.67430e-3f
#define SOFTENING 5.0f

typedef struct Particle {
    Vector3 pos;
    Vector3 vel;
    float mass;
    Vector3 trail[MAX_TRAIL];
    int trailCount;
} Particle;

Particle particles[MAX_PARTICLES];
int particleCount = 0;

void AddParticle(Vector3 pos, Vector3 vel, float mass) {
    if (particleCount < MAX_PARTICLES) {
        particles[particleCount].pos = pos;
        particles[particleCount].vel = vel;
        particles[particleCount].mass = mass;
        particles[particleCount].trailCount = 0;
        particleCount++;
    }
}

float ComputeCurvature(float x, float z) {
    float curvature = 0.0f;
    for (int i = 0; i < particleCount; i++) {
        float dx = x - particles[i].pos.x;
        float dz = z - particles[i].pos.z;
        float r = sqrtf(dx * dx + dz * dz + SOFTENING);
        curvature += 1.0f / r;
    }
    return curvature;
}

int main(int argc, char *argv[]) {
    int initialCount = 3;
    float mass = 250000000.0f;
    char *arrangement = "circle";
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

    int screenWidth = 800, screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "3D N-Body Simulation");
    SetTargetFPS(60);

    float simSize = 400.0f;

    Camera3D camera = {0};
    camera.position = (Vector3){ simSize, simSize, simSize };
    camera.target = (Vector3){ 0, 0, 0 };
    camera.up = (Vector3){ 0, 1, 0 };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    if (strcmp(arrangement, "circle") == 0) {
        Vector3 center = { 0, 0, 0 };
        float radius = simSize / 3.0f;
        float speed = 50.0f;
        for (int i = 0; i < initialCount; i++) {
            float theta = acosf(1 - 2 * ((float)i + 0.5f) / initialCount);
            float phi = PI * (1 + sqrtf(5)) * i;
            Vector3 pos = {
                center.x + radius * sinf(theta) * cosf(phi),
                center.y + radius * sinf(theta) * sinf(phi),
                center.z + radius * cosf(theta)
            };
            Vector3 radial = { pos.x - center.x, pos.y - center.y, pos.z - center.z };
            Vector3 up = { 0, 1, 0 };
            if (fabsf(Vector3DotProduct(radial, up)) > 0.99f) up = (Vector3){ 1, 0, 0 };
            Vector3 tangent = Vector3Normalize(Vector3CrossProduct(radial, up));
            Vector3 vel = { tangent.x * speed, tangent.y * speed, tangent.z * speed };
            AddParticle(pos, vel, mass);
        }
    } else {
        for (int i = 0; i < initialCount; i++) {
            Vector3 pos = {
                (float)GetRandomValue(-simSize/2, simSize/2),
                (float)GetRandomValue(-simSize/2, simSize/2),
                (float)GetRandomValue(-simSize/2, simSize/2)
            };
            Vector3 vel = {
                (float)GetRandomValue(-50, 50),
                (float)GetRandomValue(-50, 50),
                (float)GetRandomValue(-50, 50)
            };
            AddParticle(pos, vel, mass);
        }
    }

    while (!WindowShouldClose()) {
        float dt = 1.0f / 60.0f;
        for (int i = 0; i < particleCount; i++) {
            Vector3 acceleration = { 0, 0, 0 };
            for (int j = 0; j < particleCount; j++) {
                if (i == j) continue;
                Vector3 diff = {
                    particles[j].pos.x - particles[i].pos.x,
                    particles[j].pos.y - particles[i].pos.y,
                    particles[j].pos.z - particles[i].pos.z
                };
                float distSqr = diff.x*diff.x + diff.y*diff.y + diff.z*diff.z + SOFTENING;
                float dist = sqrtf(distSqr);
                float force = G * particles[j].mass / distSqr;
                acceleration.x += (diff.x / dist) * force;
                acceleration.y += (diff.y / dist) * force;
                acceleration.z += (diff.z / dist) * force;
            }
            particles[i].vel.x += acceleration.x * dt;
            particles[i].vel.y += acceleration.y * dt;
            particles[i].vel.z += acceleration.z * dt;
        }

        for (int i = 0; i < particleCount; i++) {
            particles[i].pos.x += particles[i].vel.x * dt;
            particles[i].pos.y += particles[i].vel.y * dt;
            particles[i].pos.z += particles[i].vel.z * dt;
            particles[i].trail[particles[i].trailCount % MAX_TRAIL] = particles[i].pos;
            particles[i].trailCount++;
        }

#ifdef BOX
        for (int i = 0; i < particleCount; i++) {
            if (particles[i].pos.x >= simSize/2 || particles[i].pos.x <= -simSize/2)
                particles[i].vel.x *= -0.9;
            if (particles[i].pos.y >= simSize/2 || particles[i].pos.y <= -simSize/2)
                particles[i].vel.y *= -0.9;
            if (particles[i].pos.z >= simSize/2 || particles[i].pos.z <= -simSize/2)
                particles[i].vel.z *= -0.9;
        }
#endif

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            Ray ray = GetMouseRay(mousePos, camera);
            float t = -ray.position.y / ray.direction.y;
            Vector3 newPos = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
            Vector3 vel = {
                (float)GetRandomValue(-50, 50),
                (float)GetRandomValue(-50, 50),
                (float)GetRandomValue(-50, 50)
            };
            AddParticle(newPos, vel, mass);
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

#ifdef GRID
        float gridSpacing = 10.0f;
        float sampleStep = 10.0f;
        float scale = 50.0f;
        for (float z = -simSize/2; z <= simSize/2; z += gridSpacing) {
            Vector3 prev = { -simSize/2, scale * ComputeCurvature(-simSize/2, z), z };
            for (float x = -simSize/2; x <= simSize/2; x += sampleStep) {
                float offset = scale * ComputeCurvature(x, z);
                Vector3 curr = { x, offset, z };
                DrawLine3D(prev, curr, LIGHTGRAY);
                prev = curr;
            }
        }
        for (float x = -simSize/2; x <= simSize/2; x += gridSpacing) {
            Vector3 prev = { x, scale * ComputeCurvature(x, -simSize/2), -simSize/2 };
            for (float z = -simSize/2; z <= simSize/2; z += sampleStep) {
                float offset = scale * ComputeCurvature(x, z);
                Vector3 curr = { x, offset, z };
                DrawLine3D(prev, curr, LIGHTGRAY);
                prev = curr;
            }
        }
#endif

        for (int i = 0; i < particleCount; i++) {
            int count = particles[i].trailCount < MAX_TRAIL ? particles[i].trailCount : MAX_TRAIL;
            if (count > 1) {
                int startIndex = (particles[i].trailCount >= MAX_TRAIL) ? (particles[i].trailCount % MAX_TRAIL) : 0;
                Vector3 prev = particles[i].trail[startIndex];
                for (int k = 1; k < count; k++) {
                    int index = (startIndex + k) % MAX_TRAIL;
                    DrawLine3D(prev, particles[i].trail[index], RED);
                    prev = particles[i].trail[index];
                }
            }
        }

        for (int i = 0; i < particleCount; i++) {
            DrawSphere(particles[i].pos, 2.0f, RED);
        }
        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
