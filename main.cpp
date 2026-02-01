#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader_m.h"
#include "camera.h"
#include "model.h"

using namespace std;

class Shader
{
public:
    // the program ID
    unsigned int ID;
  
    // constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    // use/activate the shader
    void use();
    // utility uniform functions
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
};

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
    public:
        // mesh data
        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture>      textures;

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

        void Draw(Shader &shader);
    private:
        //  render data
        unsigned int VAO, VBO, EBO;

        void setupMesh();
};  

class Model 
{
    public:
        Model(char *path)
        {
            loadModel(path);
        }
        void ObjToRender();
        void Draw(Shader &shader);
    private:
        // model data
        std::vector<Mesh> meshes;
        std::string directory;

        void loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene);
        int TextureFromFile(const char *path, const std::string &directory);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        vector<Texture> textures_loaded; 
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, 
                                             std::string typeName);
};

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

int Model::TextureFromFile(const char *path, const std::string &directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;

}

vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture texture;
        texture.id = TextureFromFile(str.C_Str(), directory);
        texture.type = typeName;
        texture.path = std::string(str.C_Str());
        textures.push_back(texture);
    }
    return textures;
} 

void Model::loadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);	
	
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}  

void Model::processNode(aiNode *node, const aiScene *scene)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(processMesh(mesh, scene));			
    }
    
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;
        
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        
        if(mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);        
    }
    
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }
    
    return Mesh(vertices, indices, textures);
}



Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    setupMesh();
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Shader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
void Shader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
} 

void Mesh::Draw(Shader &shader) 
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    for(unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures[i].type;
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++);

        shader.setInt(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}  


void Model::Draw(Shader &shader)
{
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}  

Shader::Shader(const char* vertexSource, const char* fragmentSource)
{
    // 1. compile vertex shader
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, nullptr);
    glCompileShader(vertex);

    int success;
    char infoLog[512];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // 2. compile fragment shader
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, nullptr);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // 3. link program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // 4. delete shaders after linking
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() 
{ 
    glUseProgram(ID);
}

int renderViewport(GLFWwindow* userInterface, unsigned int renderedWidth, unsigned int renderedHeight) {
    renderCircle(30, std::vector<float> {0.0f, 0.0f, 0.0f}, 0.1, renderedWidth, renderedHeight, false);

    Shader shader(vertexShaderSource, fragmentShaderSource);
    shader.use();

    Model cubeModel((char*)"/home/legion/Documents/vscode/mein engine/uploads_files_2787791_Mercedes+Benz+GLS+580.obj");

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
        
        cubeModel.Draw(shader);

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
