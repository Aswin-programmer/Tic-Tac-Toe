#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
    }
    return shader;
}

GLuint createShaderProgram() {
    // Vertex shader
    const char* vertexShaderSource = R"glsl(
        #version 430 core
        layout (location = 0) in vec3 position;

        layout (location = 0) uniform mat4 projection;
        layout (location = 1) uniform mat4 view;
        layout (location = 2) uniform mat4 model;
        
        void main() {
            gl_Position = projection * view * model * vec4(position, 1.0);
        }
    )glsl";

    // Fragment shader
    const char* fragmentShaderSource = R"glsl(
        #version 430 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color
        }
    )glsl";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check linking errors
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader linking error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main() {
    const auto WINDOW_WIDTH = 480.f;
    const auto WINDOW_HEIGHT = 480.f;
    const auto tile_spacing = 1.1f;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tic-Tac-Toe", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Create shader program
    GLuint shaderProgram = createShaderProgram();

    // Vertex data
    std::vector<float> vertices = 
    {
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };
    std::vector<unsigned int> indices = 
    {
        0, 1, 2,
        2, 3, 0
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    std::vector<float> verticesO;
    std::vector<unsigned int> indicesO;

    Assimp::Importer importer;
    auto scene = importer.ReadFile("ASSETS/OBJ_MODELS/o.obj", 0);
    
    auto mesh = scene->mMeshes[0];

    for (auto i = 0; i < mesh->mNumVertices; i++)
    {
        auto vertex = mesh->mVertices[i];
        verticesO.push_back(vertex.x);
        verticesO.push_back(vertex.y);
        verticesO.push_back(vertex.z);
    }

    for (auto i = 0; i < mesh->mNumFaces; i++)
    {
        auto& face = mesh->mFaces[i];

        indicesO.push_back(face.mIndices[0]);
        indicesO.push_back(face.mIndices[1]);
        indicesO.push_back(face.mIndices[2]);
    }

    GLuint vaoO{}, vboO{}, eboO{};

    glGenVertexArrays(1, &vaoO);
    glBindVertexArray(vaoO);

    glGenBuffers(1, &vboO);
    glBindBuffer(GL_ARRAY_BUFFER, vboO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesO.size(), verticesO.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &eboO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesO.size(), indicesO.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 0.1f, 100.f);
    glm::mat4 view = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -3.f));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vaoO);

        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));

        for (auto row = 0; row < 3; row++)
        {
            for (auto col = 0; col < 3; col++)
            {
                auto x = -tile_spacing + col * tile_spacing;
                auto y = -tile_spacing + row * tile_spacing;

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, 0.0f));
                glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(model));

                //glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

                glDrawElements(GL_TRIANGLES, indicesO.size(), GL_UNSIGNED_INT, 0);
            }
        }

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}