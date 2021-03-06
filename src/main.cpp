#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../classes/Shader.h"
#include "../classes/Texture2D.h"
#include "../classes/Camera.h"
#include "../classes/Model.h"
#include "../classes/ChessFigure.h"
#include "../classes/Skybox.h"
#include "../classes/lights.h"
#include "../classes/materials.h"
#include "../classes/Scene.h"
#include "../classes/RawMesh.h"

void framebuffer_size_cb(GLFWwindow *window, int width, int height);
void key_cb(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(1.75f, 3.0f, 7.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -30.0f);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

ChessFigure *chessBoard[8][8];
ChessFigure *currentlyActive = nullptr;
std::pair<int, int> currentlyActiveRealPos;
std::pair<int, int> boardCursor = std::make_pair(6, 1);

void createChessBoard(Model *pawn, Model *rook, Model *knight, Model *bishop, Model *queen, Model *king);
void drawChessBoard(Shader &shader, MaterialColor &white, MaterialColor &black);
void destroyChessBoard();

int main() {
    glfwInit();

    // OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Chess Scene", nullptr, nullptr);
    if(window == nullptr)
    {
        std::cerr << "Window creation failed" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_cb);
    glfwSetKeyCallback(window, key_cb);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "GLAD initialization failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    Shader boardShader("../resources/shaders/board_vertex_shader.vs", "../resources/shaders/board_fragment_shader.fs");
    Shader lightcubeShader("../resources/shaders/lightcube_vertex_shader.vs", "../resources/shaders/lightcube_fragment_shader.fs");
    Shader modelShader("../resources/shaders/chess_piece_vertex_shader.vs", "../resources/shaders/chess_piece_fragment_shader.fs");
    Shader skyboxShader("../resources/shaders/skybox.vs", "../resources/shaders/skybox.fs");

    Texture2D checkerDifTex("../resources/textures/chess_board_diffuse.jpg", DIFFUSE, GL_REPEAT, GL_LINEAR);
    Texture2D checkerSpecTex("../resources/textures/chess_board_specular.jpg", SPECULAR, GL_REPEAT, GL_LINEAR);

    MaterialTexture boardMaterial(256.0f, checkerDifTex, checkerSpecTex);
    MaterialColor figureMaterialWhite(256.0f,
                                      glm::vec3(1.0f, 1.0f, 1.0f),
                                      glm::vec3(1.0f, 1.0f, 1.0f),
                                      glm::vec3(1.0f, 1.0f, 1.0f));
    MaterialColor figureMaterialBlack(256.0f, glm::vec3(0.1f, 0.1f, 0.1f),
                                      glm::vec3(0.15f, 0.15f, 0.15f),
                                      glm::vec3(1.0f, 1.0f, 1.0f));

    DirectionalLight directionalLight("directionalLight",
                                      glm::vec3(0.1, 0.1, 0.1),
                                      glm::vec3(0.3f, 0.3f, 0.3f),
                                      glm::vec3(1.0f, 1.0f, 1.0f),
                                      glm::vec3(3.0f, -3.0f, 3.0f));

    PointLight pointLight("pointLight",
                          glm::vec3(0.2, 0.2, 0.2),
                          glm::vec3(0.6f, 0.6f, 0.6f),
                          glm::vec3(1.0f, 1.0f, 1.0f),
                          glm::vec3(1.75f, 3.0f, 1.75f),
                          1.0f, 0.12f, 0.082f);

    SpotLight spotLight("spotLight",
                        glm::vec3(0.1, 0.1, 0.1),
                        glm::vec3(1.0f, 1.0f, 1.0f),
                        glm::vec3(1.0f, 1.0f, 1.0f),
                        glm::vec3(1.5f, 2.0f, 0.5f),
                        glm::vec3(0.0f, -1.0f, 0.0f),
                        7.5f,1.0f, 0.09f, 0.032f);

    Model pawn("../resources/models/chess/pawn/pawn.obj");
    Model rook("../resources/models/chess/rook/rook.obj");
    Model knight("../resources/models/chess/knight/knight.obj");
    Model bishop("../resources/models/chess/bishop/bishop.obj");
    Model queen("../resources/models/chess/queen/queen.obj");
    Model king("../resources/models/chess/king/king.obj");

    std::vector<std::string> skyboxFaces = {
            "../resources/skybox/right.jpg",
            "../resources/skybox/left.jpg",
            "../resources/skybox/top.jpg",
            "../resources/skybox/bottom.jpg",
            "../resources/skybox/front.jpg",
            "../resources/skybox/back.jpg",
    };
    Skybox skybox(skyboxFaces);
    skyboxShader.use();
    skyboxShader.setUniform1i("skybox", 0);

    createChessBoard(&pawn, &rook, &knight, &bishop, &queen, &king);

    Scene scene(camera);
    scene.addLight(&directionalLight, &boardShader);
    scene.addLight(&directionalLight, &modelShader);
    scene.addLight(&pointLight, &boardShader);
    scene.addLight(&pointLight, &modelShader);
    scene.addLight(&spotLight, &boardShader);
    scene.addLight(&spotLight, &modelShader);

    float boardVertices[] = {
            // Coords           Normals           Texture
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 4.0f, 0.0f, // bottom-right
            -0.5f, 0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 4.0f, // top-left
            0.5f, 0.5f, 0.0f,   0.0f, 0.0f, 1.0f, 4.0f, 4.0f  // top-right
    };

    unsigned boardIndices[] = {
            0, 1, 2,
            1, 2, 3
    };

    float cubeVertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    RawMesh brd(boardVertices, 4, sizeof(boardVertices), boardIndices, 6, boardMaterial);
    RawMesh cub(cubeVertices, 36, sizeof(cubeVertices), figureMaterialWhite);

    glm::mat4 boardTransform = glm::mat4(1.0);
    boardTransform = glm::translate(boardTransform, glm::vec3(1.75f, 0.0f, 1.75f));
    boardTransform = glm::rotate(boardTransform, (float)glm::radians(270.0), glm::vec3(1.0f, 0.0f, 0.0f));
    boardTransform = glm::rotate(boardTransform, (float)glm::radians(90.0), glm::vec3(0.0f, 0.0f, 1.0f));
    boardTransform = glm::scale(boardTransform, glm::vec3(4.0f, 4.0f, 4.0f));

    glm::mat4 cubeTransform = glm::mat4(1.0);

    scene.addRawMesh(&brd, &boardShader, &boardTransform);
    scene.addRawMesh(&cub, &lightcubeShader, &cubeTransform);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0);
        glm::mat4 view = glm::mat4(1.0);
        glm::mat4 projection = glm::mat4(1.0);

        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        float lightSpeedReduction = 5;
        cubeTransform = glm::mat4(1.0);
        cubeTransform = glm::translate(cubeTransform, glm::vec3(1.75f + 3.0 * cos(glfwGetTime() / lightSpeedReduction), 3.0f, 1.75f + 3.0 * sin(glfwGetTime() / lightSpeedReduction))); // m * T
        cubeTransform = glm::rotate(cubeTransform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); // m * T * R
        cubeTransform = glm::scale(cubeTransform, glm::vec3(0.2f, 0.2f, 0.2f)); // m * T * R * S

        pointLight.setPosition(glm::vec3(1.75 + 3.0f * cos(glfwGetTime() / lightSpeedReduction), 3.0f, 1.75 + 3.0f * sin(glfwGetTime() / lightSpeedReduction)));

        // Light up the currently selected field
        spotLight.setPosition(glm::vec3(boardCursor.second * 0.5f, 2.0f, boardCursor.first * 0.5f));
        spotLight.setDiffuse(glm::vec3((sin(glfwGetTime()) + 1) / 2, 0.5, 0.1));

        scene.render();

        modelShader.use();
        modelShader.setUniformMatrix4fv("view", view);
        modelShader.setUniformMatrix4fv("projection", projection);
        modelShader.setUniform3fv("viewPosition", camera.Position);
        drawChessBoard(modelShader, figureMaterialWhite, figureMaterialBlack);

        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setUniformMatrix4fv("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setUniformMatrix4fv("projection", projection);
        skybox.draw();
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
    }

    skybox.del();

    checkerDifTex.del();
    checkerSpecTex.del();

    boardShader.del();
    lightcubeShader.del();
    modelShader.del();
    skyboxShader.del();

    destroyChessBoard();

    glfwTerminate();

    return 0;
}

void framebuffer_size_cb(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void key_cb(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        int i = boardCursor.second;
        int j = boardCursor.first;
        if(boardCursor.first > 0)
        {
            boardCursor.first--;
            if (currentlyActive != nullptr)
                currentlyActive->position = std::make_pair(i, j - 1);
        }
    }
    if(key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        int i = boardCursor.second;
        int j = boardCursor.first;
        if(boardCursor.second > 0)
        {
            boardCursor.second--;
            if (currentlyActive != nullptr)
                currentlyActive->position = std::make_pair(i - 1, j);
        }
    }
    if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        int i = boardCursor.second;
        int j = boardCursor.first;
        if(boardCursor.second < 7)
        {
            boardCursor.second++;
            if (currentlyActive != nullptr)
                currentlyActive->position = std::make_pair(i + 1, j);
        }
    }
    if(key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        int i = boardCursor.second;
        int j = boardCursor.first;
        if(boardCursor.first < 7)
        {
            boardCursor.first++;
            if (currentlyActive != nullptr)
                currentlyActive->position = std::make_pair(i, j + 1);
        }
    }
    if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        int i = boardCursor.second;
        int j = boardCursor.first;
        // If there's an active chess figure and the square isn't occupied, place the chess figure on it
        if(currentlyActive != nullptr && chessBoard[i][j] == nullptr)
        {
            chessBoard[currentlyActiveRealPos.first][currentlyActiveRealPos.second] = nullptr;
            chessBoard[i][j] = currentlyActive;
            currentlyActive->figure_status = INACTIVE;
            currentlyActive = nullptr;
        }
        // If we're returning the active chess figure to its original square, just drop it
        else if(currentlyActive != nullptr && i == currentlyActiveRealPos.first && j == currentlyActiveRealPos.second)
        {
            currentlyActive->figure_status = INACTIVE;
            currentlyActive = nullptr;
        }
        // If we don't have an active chess figure and there is a figure on the selected square, pick it up
        else if(currentlyActive == nullptr && chessBoard[i][j] != nullptr)
        {
            currentlyActive = chessBoard[i][j];
            currentlyActive->figure_status = ACTIVE;
            currentlyActiveRealPos = std::make_pair(i, j);
        }
        // If we can capture the figure, delete it and move the active figure to its spot
        else if(currentlyActive != nullptr && chessBoard[i][j] != nullptr && currentlyActive->figure_color != chessBoard[i][j]->figure_color)
        {
            chessBoard[currentlyActiveRealPos.first][currentlyActiveRealPos.second] = nullptr;
            delete chessBoard[i][j];
            chessBoard[i][j] = currentlyActive;
            currentlyActive->figure_status = INACTIVE;
            currentlyActive = nullptr;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void createChessBoard(Model *pawn, Model *rook, Model *knight, Model *bishop, Model *queen, Model *king) {
    for(int i = 0; i < 8; i++)
        chessBoard[i][1] = new ChessFigure(pawn, std::make_pair(i, 1), PAWN, BLACK);
    chessBoard[0][0] = new ChessFigure(rook, std::make_pair(0, 0), ROOK, BLACK);
    chessBoard[7][0] = new ChessFigure(rook, std::make_pair(7, 0), ROOK, BLACK);
    chessBoard[1][0] = new ChessFigure(knight, std::make_pair(1, 0), KNIGHT, BLACK);
    chessBoard[6][0] = new ChessFigure(knight, std::make_pair(6, 0), KNIGHT, BLACK);
    chessBoard[2][0] = new ChessFigure(bishop, std::make_pair(2, 0), BISHOP, BLACK);
    chessBoard[5][0] = new ChessFigure(bishop, std::make_pair(5, 0), BISHOP, BLACK);
    chessBoard[3][0] = new ChessFigure(queen, std::make_pair(3, 0), QUEEN, BLACK);
    chessBoard[4][0] = new ChessFigure(king, std::make_pair(4, 0), KING, BLACK);

    for(int i = 0; i < 8; i++)
        chessBoard[i][6] = new ChessFigure(pawn, std::make_pair(i, 6), PAWN, WHITE);
    chessBoard[0][7] = new ChessFigure(rook, std::make_pair(0, 7), ROOK, WHITE);
    chessBoard[7][7] = new ChessFigure(rook, std::make_pair(7, 7), ROOK, WHITE);
    chessBoard[1][7] = new ChessFigure(knight, std::make_pair(1, 7), KNIGHT, WHITE);
    chessBoard[6][7] = new ChessFigure(knight, std::make_pair(6, 7), KNIGHT, WHITE);
    chessBoard[2][7] = new ChessFigure(bishop, std::make_pair(2, 7), BISHOP, WHITE);
    chessBoard[5][7] = new ChessFigure(bishop, std::make_pair(5, 7), BISHOP, WHITE);
    chessBoard[3][7] = new ChessFigure(queen, std::make_pair(3, 7), QUEEN, WHITE);
    chessBoard[4][7] = new ChessFigure(king, std::make_pair(4, 7), KING, WHITE);
}

void drawChessBoard(Shader &shader, MaterialColor &white, MaterialColor &black) {
    for(int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (chessBoard[i][j] != nullptr)
                chessBoard[i][j]->draw(shader, white, black);
        }
    }
}

void destroyChessBoard() {
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            if(chessBoard[i][j] != nullptr)
            {
                delete chessBoard[i][j];
                chessBoard[i][j] = nullptr;
            }
        }
    }
}