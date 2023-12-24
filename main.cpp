/*	A raycaster implementation similar to the classic DOOM & Wolfenstein 3D */

#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>

#include <cmath>
#include <limits>
#include <utility>
#include <vector>

const double PI = 3.14159265358979323846;
const double PI2 = PI / 2;
const double PI3 = 3 * PI / 2;

const size_t HEIGHT = 640;
const size_t WIDTH = 1290;
const size_t BLOCKSIZE = 64;
const size_t NUM_RAYS = 60;
const size_t MAP_WIDTH = 10;
const float DEGREE = PI / 180;

struct Position {
    float x;
    float y;

    Position(float x, float y) : x(x), y(y) {}
};

// sin(angle) = changeInY
// cos(angle) = changeInX
struct Player {
    Position pos;
    float angle;

    Player(float x, float y, float angle = 0) : pos(x, y), angle(angle) {}
};

// Global player var
Player player = Player(70, 70);

float getEuclideanDist(float x1, float y1, float x2, float y2) {
    return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void drawPlayer() {
    glColor3f(1, 1, 0);
    glPointSize(10);
    glBegin(GL_POINTS);
    glVertex2i(player.pos.x, player.pos.y);
    glEnd();

    // Draw direction faced
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2i(player.pos.x, player.pos.y);
    glVertex2i(player.pos.x + std::cos(player.angle) * 25,
               player.pos.y + std::sin(player.angle) * 25);
    glEnd();
}

std::vector<std::vector<int>> map = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 0, 0, 1}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 1}, {1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 1}, {1, 0, 0, 0, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

void drawMap() {
    int xoffset, yoffset;
    for (size_t i = 0; i < map.size(); ++i) {
        for (size_t j = 0; j < map[0].size(); ++j) {
            if (map[i][j] == 1) {
                glColor3f(1, 1, 1);
            } else {
                glColor3f(0, 0, 0);
            }

            xoffset = j * BLOCKSIZE;
            yoffset = i * BLOCKSIZE;
            glBegin(GL_QUADS);

            // need counter clockwise ordering OpenGL convention
            glVertex2i(xoffset + 1, yoffset + 1);
            glVertex2i(xoffset + 1, yoffset + BLOCKSIZE - 1);
            glVertex2i(xoffset + BLOCKSIZE - 1, yoffset + BLOCKSIZE - 1);
            glVertex2i(xoffset + BLOCKSIZE - 1, yoffset + 1);
            glEnd();
        }
    }
}

void raycast() {
    float rayX, rayY, rayAngle, rayLength;
    float xoffset, yoffset;
    size_t mapX, mapY;

    rayAngle = player.angle - 30 * DEGREE;

    for (size_t i = 0; i < NUM_RAYS; ++i) {
        if (rayAngle < 0) {
            rayAngle += 2 * PI;
        } else if (rayAngle > 2 * PI) {
            rayAngle -= 2 * PI;
        }

        float horizontalRayDist = std::numeric_limits<float>::max();

        // initialized for edge case when no hit to wall
        float horX = player.pos.x;
        float horY = player.pos.y;

        size_t iteration = 0;
        float aTan = -1 / std::tan(rayAngle);

        if (rayAngle > PI) {
            rayY = (((int)player.pos.y >> 6) << 6) - 0.0001;
            rayX = (player.pos.y - rayY) * aTan + player.pos.x;
            yoffset = -64;
            xoffset = -yoffset * aTan;
        } else if (rayAngle < PI) {
            rayY = (((int)player.pos.y >> 6) << 6) + 64;
            rayX = (player.pos.y - rayY) * aTan + player.pos.x;
            yoffset = 64;
            xoffset = -yoffset * aTan;
        } else if (rayAngle == 0 || rayAngle == PI) {
            rayX = player.pos.x;
            rayY = player.pos.y;
            iteration = 8;
        }

        while (iteration < MAP_WIDTH) {
            // division by 64
            mapX = (int)(rayX) >> 6;
            mapY = (int)(rayY) >> 6;

            if (mapX >= 0 && mapY >= 0 && mapX < map[0].size() && mapY < map.size() &&
                map[mapY][mapX] == 1) {
                iteration = MAP_WIDTH;
                horX = rayX;
                horY = rayY;
                horizontalRayDist =
                    getEuclideanDist(player.pos.x, player.pos.y, horX, horY);
            } else {
                rayX += xoffset;
                rayY += yoffset;
                ++iteration;
            }
        }

        iteration = 0;
        float verticalRayDist = std::numeric_limits<float>::max();

        // initialized for edge case when no hit to wall
        float verX = player.pos.x;
        float verY = player.pos.y;

        float nTan = -std::tan(rayAngle);
        if (rayAngle > PI2 && rayAngle < PI3) {
            rayX = (((int)player.pos.x >> 6) << 6) - 0.0001;
            rayY = (player.pos.x - rayX) * nTan + player.pos.y;
            xoffset = -64;
            yoffset = -xoffset * nTan;
        } else if (rayAngle < PI2 || rayAngle > PI3) {
            rayX = (((int)player.pos.x >> 6) << 6) + 64;
            rayY = (player.pos.x - rayX) * nTan + player.pos.y;
            xoffset = 64;
            yoffset = -xoffset * nTan;
        } else if (rayAngle == 0 || rayAngle == PI) {
            rayX = player.pos.x;
            rayY = player.pos.y;
            iteration = 8;
        }

        while (iteration < MAP_WIDTH) {
            // division by 64
            mapX = (int)(rayX) >> 6;
            mapY = (int)(rayY) >> 6;

            if (mapX >= 0 && mapY >= 0 && mapX < map[0].size() && mapY < map.size() &&
                map[mapY][mapX] == 1) {
                iteration = MAP_WIDTH;
                verX = rayX;
                verY = rayY;
                verticalRayDist =
                    getEuclideanDist(player.pos.x, player.pos.y, verX, verY);
            } else {
                rayX += xoffset;
                rayY += yoffset;
                ++iteration;
            }
        }

        // Different lighting for shade
        if (verticalRayDist < horizontalRayDist) {
            rayX = verX;
            rayY = verY;
            rayLength = verticalRayDist;
            glColor3f(0.9, 0, 0);
        } else {
            rayX = horX;
            rayY = horY;
            rayLength = horizontalRayDist;
            glColor3f(0.7, 0, 0);
        }

        glLineWidth(1);
        glBegin(GL_LINES);
        glVertex2i(player.pos.x, player.pos.y);
        glVertex2i(rayX, rayY);
        glEnd();

        // 3D Scene
        // Cosine adjustment!
        float correctionAngle = player.angle - rayAngle;
        if (correctionAngle < 0) {
            correctionAngle += 2 * PI;
        } else if (correctionAngle > 2 * PI) {
            correctionAngle -= 2 * PI;
        }

        rayLength = rayLength * std::cos(correctionAngle);

        float lineHeight = (BLOCKSIZE * 480) / rayLength;
        if (lineHeight > 480) {
            lineHeight = 480;
        }

        float sliceWidth = 640 / (float)NUM_RAYS;
        float sliceX = 650 + i * sliceWidth;

        glLineWidth(sliceWidth);
        glBegin(GL_LINES);
        glVertex2i(sliceX, HEIGHT / 2 - lineHeight / 2);
        glVertex2i(sliceX, HEIGHT / 2 + lineHeight / 2);
        glEnd();

        rayAngle += DEGREE;
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawMap();
    drawPlayer();
    raycast();
    glutSwapBuffers();
}

void buttons(unsigned char key, int x, int y) {
    float newX;
    float newY;
    if (key == 'a') {
        player.angle -= 0.1;
        if (player.angle < 0) {
            player.angle += 2 * PI;
        }
    }
    if (key == 'd') {
        player.angle += 0.1;
        if (player.angle > 2 * PI) {
            player.angle -= 2 * PI;
        }
    }
    if (key == 'w') {
        newX = player.pos.x + std::cos(player.angle) * 5;
        newY = player.pos.y + std::sin(player.angle) * 5;

        if (map[(int)newY >> 6][(int)newX >> 6] != 1) {
            player.pos.x = newX;
            player.pos.y = newY;
        }
    }
    if (key == 's') {
        newX = player.pos.x - std::cos(player.angle) * 5;
        newY = player.pos.y - std::sin(player.angle) * 5;

        if (map[(int)newY >> 6][(int)newX >> 6] != 1) {
            player.pos.x = newX;
            player.pos.y = newY;
        }
    }

    glutPostRedisplay();
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Raycaster");

    glClearColor(0.3, 0.3, 0.3, 0);
    gluOrtho2D(0, WIDTH, HEIGHT, 0);

    glutDisplayFunc(display);
    glutKeyboardFunc(buttons);
    glutMainLoop();

    return 0;
}
