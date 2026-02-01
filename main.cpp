#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "out vec3 FragPos;\n"
    "out vec3 Normal;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   FragPos = vec3(model * vec4(aPos, 1.0));\n"
    "   Normal = mat3(transpose(inverse(model))) * aNormal;\n"
    "   gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 FragPos;\n"
    "in vec3 Normal;\n"
    "uniform vec3 lightPos;\n"
    "uniform vec3 viewPos;\n"
    "uniform vec3 lightColor;\n"
    "uniform vec3 objectColor;\n"
    "void main()\n"
    "{\n"
    "float specularStrength = 0.5;\n"
    "float ambientStrength = 0.1;\n"
    "vec3 ambient = ambientStrength * lightColor;\n"
    "vec3 norm = normalize(Normal);\n"
    "vec3 lightDir = normalize(lightPos - FragPos);\n"
    "float diff = max(dot(norm, lightDir), 0.0);\n"
    "vec3 diffuse = diff * lightColor;\n"
    "vec3 viewDir = normalize(viewPos - FragPos);\n"
    "vec3 reflectDir = reflect(-lightDir, norm);\n"
    "float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
    "vec3 specular = specularStrength * spec * lightColor;\n"
    "vec3 result = (ambient + diffuse + specular) * objectColor;"
    "FragColor = vec4(result, 1.0);\n"
    "}\0";

unsigned int shaderProgram;
unsigned int VAO;
unsigned int VBO;

std::vector< std::vector<float> > verticesContainer;

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::mat4 view;

float lastFrame = 0.0f;
float deltaTime = 0.0f;

float yaw = -90.0f;
float pitch = 0.0f;

bool firstMouse = true;

glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor = glm::vec3(1.0f, 1.0f, 1.0f);

int initialize(std::vector<float>* verticesVector, unsigned int verticesBytes) {

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
    glBufferData(GL_ARRAY_BUFFER, verticesBytes, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return 0;
}

int renderObject(unsigned int shaderProgram, unsigned int VAO, size_t vectorSize) {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vectorSize / 6);

    return 0;
}

void circle2D(unsigned int renderedWidth, unsigned int renderedHeight, float x, float y, float z, float radius, float operation, unsigned int i, std::vector<float>& vertices) {
        float aspectRatio = (float)renderedWidth / (float)renderedHeight;
        float angle1 = i * operation;
        float angle2 = (i + 1) * operation;

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);

        vertices.push_back(x + cos(angle1) * radius);
        vertices.push_back(y + sin(angle1) * radius);
        vertices.push_back(z);

        vertices.push_back(x + cos(angle2) * radius);
        vertices.push_back(y + sin(angle2) * radius);
        vertices.push_back(z);
}

void circle3D(unsigned int renderedWidth, unsigned int renderedHeight, float x, float y, float z, float radius, float operation, unsigned int i, unsigned int j, std::vector<float>& vertices, unsigned int resolution) {

}

int renderCircle(unsigned int resolution, std::vector<float> originVertices, float radius, unsigned int renderedWidth, unsigned int renderedHeight, bool is3D) {

    std::vector<float> vertices;

    float x = originVertices[0];
    float y = originVertices[1];
    float z = originVertices[2];

    float operation = 2.0f * 3.1415926f / resolution;

    if (!is3D) {
        for (int i = 0; i < resolution; i++) {
            circle2D(renderedWidth, renderedHeight, x, y, z, radius, operation, i, vertices);
        }
    } else {
        for (int i = 0; i < resolution; i++) {
            for (int j = 0; j < resolution; j++) {
                circle3D(renderedWidth, renderedHeight, x, y, z, radius, operation, i, j, vertices, resolution);
            }
        }
    }

    verticesContainer.push_back(vertices);
    return 0;
}

struct objData {
    unsigned int VAO;
    unsigned int shaderProgram;
    size_t vectorSize;
};

float lastX = 800.0f, lastY = 600.0f;

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

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

}

void movementHandler(GLFWwindow* window) {
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraSpeed = cameraSpeed / 2.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
}

void lightingHandler(unsigned int shaderProgram) {
    // USES PHONG LIGHTING -- DIFFUSE + AMBIENT + SPECULAR COMBINATION
    unsigned int lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    unsigned int objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    glUniform3fv(lightColorLoc, 1, glm::value_ptr(::lightColor));
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(::objectColor));

    glm::vec4 FragColor;

    float ambientStrength = 0.1;
    glm::vec3 ambient = ambientStrength * lightColor;

    glm::vec3 result = ambient * objectColor;
    FragColor = glm::vec4(result, 1.0);
}

int renderViewport(GLFWwindow* userInterface, unsigned int renderedWidth, unsigned int renderedHeight) {
    std::vector<float> vertices {
    1.0f,  1.0f,  1.0f,    0.577f,  0.577f, -0.577f,
    1.0f, -1.0f, -1.0f,    0.577f,  0.577f, -0.577f,
    -1.0f,  1.0f, -1.0f,   0.577f,  0.577f, -0.577f,

    1.0f,  1.0f,  1.0f,    0.577f, -0.577f,  0.577f,
    -1.0f, -1.0f,  1.0f,   0.577f, -0.577f,  0.577f,
    1.0f, -1.0f, -1.0f,    0.577f, -0.577f,  0.577f,

    1.0f,  1.0f,  1.0f,   -0.577f,  0.577f,  0.577f,
    -1.0f,  1.0f, -1.0f,  -0.577f,  0.577f,  0.577f,
    -1.0f, -1.0f,  1.0f,  -0.577f,  0.577f,  0.577f,

    1.0f, -1.0f, -1.0f,   -0.577f, -0.577f, -0.577f,
    -1.0f, -1.0f,  1.0f,  -0.577f, -0.577f, -0.577f,
    -1.0f,  1.0f, -1.0f,  -0.577f, -0.577f, -0.577f,
};

    verticesContainer.push_back(vertices);

    renderCircle(30, std::vector<float> {0.0f, 0.0f, 0.0f}, 0.1, renderedWidth, renderedHeight, false);

    std::vector<objData> objsData;

    for (std::vector<float> v : verticesContainer) {
        initialize(&v, v.size() * sizeof(float));
        objData obj;
        obj.VAO = VAO;
        obj.shaderProgram = shaderProgram;
        obj.vectorSize = v.size();
        objsData.push_back(obj);

        lightingHandler(shaderProgram);

    }

    while (!glfwWindowShouldClose(userInterface)) {
        movementHandler(userInterface);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
        (float)renderedWidth / (float)renderedHeight, 0.1f, 100.0f);

        glm::vec3 lightPos = glm::vec3(3.0f, 3.0f, 3.0f);
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 objectColor = glm::vec3(1.0f, 1.0f, 1.0f);

        for (objData v : objsData) {
            glUseProgram(v.shaderProgram);
        
            unsigned int modelLoc = glGetUniformLocation(v.shaderProgram, "model");
            unsigned int viewLoc = glGetUniformLocation(v.shaderProgram, "view");
            unsigned int projLoc = glGetUniformLocation(v.shaderProgram, "projection");
        
            glm::mat4 model = glm::mat4(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
            unsigned int lightPosLoc = glGetUniformLocation(v.shaderProgram, "lightPos");
            unsigned int viewPosLoc = glGetUniformLocation(v.shaderProgram, "viewPos");
            unsigned int lightColorLoc = glGetUniformLocation(v.shaderProgram, "lightColor");
            unsigned int objectColorLoc = glGetUniformLocation(v.shaderProgram, "objectColor");
    
            glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
            glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
            glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
            glUniform3fv(objectColorLoc, 1, glm::value_ptr(objectColor));

            renderObject(v.shaderProgram, v.VAO, v.vectorSize);
        }


        glfwSwapBuffers(userInterface);
        glfwPollEvents();

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

    unsigned int renderedWidth = width;
    unsigned int renderedHeight = height;

    glViewport(0, 0, renderedWidth, renderedHeight); // PORTION OF SCREEN THAT RENDERING WORKS ACROSS, CURRENTLY USER RESOLUTION

    renderViewport(userInterface, renderedWidth, renderedHeight);

    return 0;
}

int main() {
    int callBack = interface();
    return 0;

}
