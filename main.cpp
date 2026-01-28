#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"

    "void main()\n"
    "{\n"
    "FragColor = vec4(0.1, 0.1, 0.1, 0.1);\n"
    "}\0";

unsigned int shaderProgram;
unsigned int VAO;
unsigned int VBO;

std::vector< std::vector<float> > verticesContainer;

std::vector<float> vertices {
    -0.9f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f,  0.5f, 0.0f
};

std::vector<float> vertices2 {
    -0.8f, -0.1f, 0.3f,
    0.21f, -0.12f, 0.5f,
    0.0f, 0.0f, 0.0f
};

int initialize(std::vector<float>* verticesVector, unsigned int verticeBytes) {

    float* vertices = verticesVector->data();

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader); // COMPILE THE VERTEX SHADER

    int  success; // CHECK FOR SUCCESS
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader); // COMPILE THE FRAGMENT SHADER

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glUseProgram(shaderProgram); // LINK SHADERS TO THE PROGRAM

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader); // DELETE UNNECESSARY SHADER OBJECTS


    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verticeBytes, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return 0;
}

int renderObject(unsigned int shaderProgram, unsigned int VAO) {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    return 0;
}

struct objData {
    unsigned int VAO;
    unsigned int shaderProgram;
};

int renderViewport(GLFWwindow* userInterface) {
    verticesContainer.push_back(vertices);
    verticesContainer.push_back(vertices2);

    std::vector<objData> objsData;

    for (std::vector<float> v : verticesContainer) {
        initialize(&v, sizeof(v));
        objData obj;
        obj.VAO = VAO;
        obj.shaderProgram = shaderProgram;
        objsData.push_back(obj);
    }

    while (!glfwWindowShouldClose(userInterface)) {
        glClear(GL_COLOR_BUFFER_BIT);
        for (objData v : objsData) {
            renderObject(v.shaderProgram, v.VAO);
        }
        glfwSwapBuffers(userInterface);
        glfwPollEvents();

        if (glfwGetKey(userInterface, GLFW_KEY_F6) == GLFW_PRESS) {
        }

    }
    return 0;
}

int interface() {
    glfwInit();
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);

    int width = mode->width;
    int height = mode->height;

    GLFWwindow* userInterface = glfwCreateWindow(width, height, "engine", nullptr, nullptr);

    glfwMakeContextCurrent(userInterface);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    renderViewport(userInterface);

    return 0;
}

int main() {
    int callBack = interface();
    return 0;

}
