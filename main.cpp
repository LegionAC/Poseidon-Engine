#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
    "FragColor = vec4(50.0f, 10.0f, 20.0f, 1.0f);\n"
    "}\0";

float vertices[] {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
};

unsigned int shaderProgram;
unsigned int VAO;
unsigned int VBO;

int initialize() {
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return 0;
}

int renderObject() {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

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

    initialize();

    while (!glfwWindowShouldClose(userInterface)) {
        glClear(GL_COLOR_BUFFER_BIT);
        renderObject();
        glfwSwapBuffers(userInterface);
        glfwPollEvents();
    }

    return 0;
}

int main() {
    int callBack = interface();
    return 0;

}