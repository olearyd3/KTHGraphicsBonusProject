#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderSphere();

// width and height of screen
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera variables and mouse bool
Camera camera(glm::vec3(0.0f, 3.0f, 5.0f));
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
bool firstMouse = true;

// timing for updating scene
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// to rotate the spheres when keys are pressed
float sphereRotator = 0.0f;
float brickSphereRotator = 0.0f;
float phongSphereRotator = 0.0f;
float phongSphere2Rotator = 0.0f;

// switch from Phong to Blinn-Phong
bool blinn = false;
bool blinnPressed = false;

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Cook Torrance Spheres", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders for Cook-Torrance and Phong
    Shader shader("cookTorrance.vs", "cookTorrance.fs");
    Shader phongShader("phongShader.vs", "phongShader.fs");

    // Load in the chair model
    Model chairModel("chair/source/stul/stul.obj");

    // floor - draw as two triangles and define vertices normals and texcoords
    float planeVertices[] = {
        // positions            // normals         // texcoords
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
         10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
    };
    // back wall
    float plane2Vertices[] = {
        // positions            // normals         // texcoords
         10.0f, -0.5f,  -10.0f,  0.0f, 0.0f, 1.0f,  10.0f,  0.0f,
        -10.0f, -0.5f,  -10.0f,  0.0f, 0.0f, 1.0f,   0.0f,  0.0f,
        -10.0f, 9.5f, -10.0f,  0.0f, 0.0f, 1.0f,   0.0f, 10.0f,

         10.0f, -0.5f,  -10.0f,  0.0f, 0.0f, 1.0f,  10.0f,  0.0f,
        -10.0f, 9.5f, -10.0f,  0.0f, 0.0f, 1.0f,   0.0f, 10.0f,
         10.0f, 9.5f, -10.0f,  0.0f, 0.0f, 1.0f,  10.0f, 10.0f
    };
    // left wall
    float plane3Vertices[] = {
        // positions            // normals         // texcoords
        -10.0f, -0.5f, 10.0f,  1.0f, 0.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  1.0f, 0.0f, 0.0f,   0.0f,  0.0f,
        -10.0f, 9.5f, -10.0f,  1.0f, 0.0f, 0.0f,   0.0f, 10.0f,

        -10.0f, -0.5f, 10.0f,  1.0f, 0.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, 9.5f, -10.0f,  1.0f, 0.0f, 0.0f,   0.0f, 10.0f,
        -10.0f, 9.5f, 10.0f,  1.0f, 0.0f, 0.0f,  10.0f, 10.0f
    };
    // right wall
    float plane4Vertices[] = {
        // positions            // normals         // texcoords
        10.0f, -0.5f, 10.0f,  -1.0f, 0.0f, 0.0f,  10.0f,  0.0f,
        10.0f, -0.5f, -10.0f,  -1.0f, 0.0f, 0.0f,   0.0f,  0.0f,
        10.0f, 9.5f, -10.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 10.0f,

        10.0f, -0.5f, 10.0f,  -1.0f, 0.0f, 0.0f,  10.0f,  0.0f,
        10.0f, 9.5f, -10.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 10.0f,
        10.0f, 9.5f, 10.0f,  -1.0f, 0.0f, 0.0f,  10.0f, 10.0f
    };
    // ceiling
    float plane5Vertices[] = {
        // positions            // normals         // texcoords
         10.0f, 9.5f,  10.0f,  0.0f, -1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, 9.5f,  10.0f,  0.0f, -1.0f, 0.0f,   0.0f,  0.0f,
        -10.0f, 9.5f, -10.0f,  0.0f, -1.0f, 0.0f,   0.0f, 10.0f,

         10.0f, 9.5f,  10.0f,  0.0f, -1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, 9.5f, -10.0f,  0.0f, -1.0f, 0.0f,   0.0f, 10.0f,
         10.0f, 9.5f, -10.0f,  0.0f, -1.0f, 0.0f,  10.0f, 10.0f
    };
    // behind me wall
    float plane6Vertices[] = {
        // positions            // normals         // texcoords
         10.0f, -0.5f,  10.0f,  0.0f, 0.0f, -1.0f,  10.0f,  0.0f,
        -10.0f, -0.5f,  10.0f,  0.0f, 0.0f, -1.0f,   0.0f,  0.0f,
        -10.0f, 9.5f, 10.0f,  0.0f, 0.0f, -1.0f,   0.0f, 10.0f,

         10.0f, -0.5f,  10.0f,  0.0f, 0.0f, -1.0f,  10.0f,  0.0f,
        -10.0f, 9.5f, 10.0f,  0.0f, 0.0f, -1.0f,   0.0f, 10.0f,
         10.0f, 9.5f, 10.0f,  0.0f, 0.0f, -1.0f,  10.0f, 10.0f
    };

    // floor VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // back wall VAO
    unsigned int plane2VAO, plane2VBO;
    glGenVertexArrays(1, &plane2VAO);
    glGenBuffers(1, &plane2VBO);
    glBindVertexArray(plane2VAO);
    glBindBuffer(GL_ARRAY_BUFFER, plane2VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane2Vertices), plane2Vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // left wall VAO
    unsigned int plane3VAO, plane3VBO;
    glGenVertexArrays(1, &plane3VAO);
    glGenBuffers(1, &plane3VBO);
    glBindVertexArray(plane3VAO);
    glBindBuffer(GL_ARRAY_BUFFER, plane3VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane3Vertices), plane3Vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // right wall VAO
    unsigned int plane4VAO, plane4VBO;
    glGenVertexArrays(1, &plane4VAO);
    glGenBuffers(1, &plane4VBO);
    glBindVertexArray(plane4VAO);
    glBindBuffer(GL_ARRAY_BUFFER, plane4VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane4Vertices), plane4Vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // ceiling VAO
    unsigned int plane5VAO, plane5VBO;
    glGenVertexArrays(1, &plane5VAO);
    glGenBuffers(1, &plane5VBO);
    glBindVertexArray(plane5VAO);
    glBindBuffer(GL_ARRAY_BUFFER, plane5VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane5Vertices), plane5Vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // back wall VAO
    unsigned int plane6VAO, plane6VBO;
    glGenVertexArrays(1, &plane6VAO);
    glGenBuffers(1, &plane6VBO);
    glBindVertexArray(plane6VAO);
    glBindBuffer(GL_ARRAY_BUFFER, plane6VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane6Vertices), plane6Vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // use the cook torrance shader and define each of the maps as locations
    shader.use();
    shader.setInt("albedoMap", 0);
    shader.setInt("normalMap", 1);
    shader.setInt("metallicMap", 2);
    shader.setInt("roughnessMap", 3);
    shader.setInt("aoMap", 4);

    // load PBR material textures
    // celtic gold
    unsigned int goldAlbedo    = loadTexture("ornate-celtic-gold-bl/ornate-celtic-gold-albedo.png");
    unsigned int goldNormal    = loadTexture("ornate-celtic-gold-bl/ornate-celtic-gold-normal-ogl.png");
    unsigned int goldMetallic  = loadTexture("ornate-celtic-gold-bl/ornate-celtic-gold-metallic.png");
    unsigned int goldRoughness = loadTexture("ornate-celtic-gold-bl/ornate-celtic-gold-roughness.png");
    unsigned int goldAO        = loadTexture("ornate-celtic-gold-bl/ornate-celtic-gold-ao.png");

    // floor and walls
    unsigned int floorAlbedo    = loadTexture("hardwood-brown-planks-bl/hardwood-brown-planks-albedo.png");
    unsigned int floorNormal    = loadTexture("hardwood-brown-planks-bl/hardwood-brown-planks-normal-ogl.png");
    unsigned int floorMetallic  = loadTexture("hardwood-brown-planks-bl/hardwood-brown-planks-metallic.png");
    unsigned int floorRoughness = loadTexture("hardwood-brown-planks-bl/hardwood-brown-planks-roughness.png");
    unsigned int floorAO        = loadTexture("hardwood-brown-planks-bl/hardwood-brown-planks-ao.png");

    // ceiling
    unsigned int ceilingAlbedo    = loadTexture("sprayed-wall-texture1-bl/sprayed-wall-texture1_albedo.png");
    unsigned int ceilingNormal    = loadTexture("sprayed-wall-texture1-bl/sprayed-wall-texture1_normal-ogl.png");
    unsigned int ceilingMetallic  = loadTexture("sprayed-wall-texture1-bl/sprayed-wall-texture1_metallic.png");
    unsigned int ceilingRoughness = loadTexture("sprayed-wall-texture1-bl/sprayed-wall-texture1_roughness.png");
    unsigned int ceilingAO        = loadTexture("sprayed-wall-texture1-bl/sprayed-wall-texture1_ao.png");

    // bricks - CookTorrance
    unsigned int bricksAlbedo    = loadTexture("castle-bricks/castle_brick_wall_29_16_diffuse.jpg");
    unsigned int bricksNormal    = loadTexture("castle-bricks/castle_brick_wall_29_16_normal.jpg");
    unsigned int bricksMetallic  = loadTexture("castle-bricks/castle_brick_wall_29_16_metalness.jpg");
    unsigned int bricksRoughness = loadTexture("castle-bricks/castle_brick_wall_29_16_roughness.jpg");
    unsigned int bricksAO        = loadTexture("castle-bricks/castle_brick_wall_29_16_ao.jpg");

    // chair 
    unsigned int chairAlbedo    = loadTexture("chair/source/stul/Albedo.png");
    unsigned int chairNormal    = loadTexture("chair/source/stul/Normal.png");
    unsigned int chairMetallic  = loadTexture("hardwood-brown-planks-bl/hardwood-brown-planks-metallic.png");
    unsigned int chairRoughness = loadTexture("chair/source/stul/Specular.png");
    unsigned int chairAO        = loadTexture("chair/source/stul/AO.png");

    // change to Phong for the concrete ball and brick ball
    phongShader.use();
    phongShader.setInt("brickTexture", 0);

    // bricks
    unsigned int concretePhongAlbedo = loadTexture("PolishedConcrete01_MR_4K/PolishedConcrete01_4K_BaseColor.png");
    unsigned int bricksPhongAlbedo = loadTexture("castle-bricks/castle_brick_wall_29_16_diffuse.jpg");

    // light positions and colour (all the same colour
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f, 6.0f, 7.5f),
        glm::vec3(-7.5f, 6.0f, 0.0f),
        glm::vec3(7.5f, 6.0f, 0.0f),
        glm::vec3(0.0f, 6.0f, -7.5f)
    };
    glm::vec3 lightColors[] = {
        glm::vec3(150.0f, 150.0f, 150.0f),
    };

    // set the projection matrix in the CT shader
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    shader.use();
    shader.setMat4("projection", projection);


    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // render
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // render lights for each position
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i) {
            shader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
            shader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[0]);
        }
        
        // using CT shader
        shader.use();
        // set the view, camPos
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);
        shader.setVec3("camPos", camera.Position);

        // set the model to the floor
        glm::mat4 floorModel = glm::mat4(1.0f);
        shader.setMat4("model", floorModel);
        
        // floor
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorAlbedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, floorMetallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, floorRoughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, floorAO);   
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // back wall
        glBindVertexArray(plane2VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorAlbedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, floorMetallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, floorRoughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, floorAO);   
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // left wall
        glBindVertexArray(plane3VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorAlbedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, floorMetallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, floorRoughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, floorAO);   
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // right wall
        glBindVertexArray(plane4VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorAlbedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, floorMetallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, floorRoughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, floorAO);   
        glDrawArrays(GL_TRIANGLES, 0, 6);

         // behind user wall
        glBindVertexArray(plane6VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorAlbedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, floorMetallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, floorRoughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, floorAO);   
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // ceiling
        glBindVertexArray(plane5VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ceilingAlbedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ceilingNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ceilingMetallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ceilingRoughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, ceilingAO);   
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // next model
        // apply celtic gold textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, goldAlbedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, goldNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, goldMetallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, goldRoughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, goldAO);

        // draw the celtic gold sphere
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0, 0.5, 0.0));
        model = glm::rotate(model, glm::radians(sphereRotator), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", model);
        renderSphere();

        // draw a celtic gold chair
        glm::mat4 goldChair = glm::mat4(1.0f);
        goldChair = glm::translate(goldChair, glm::vec3(-7.0, -0.5, -8.0));
        goldChair = glm::scale(goldChair, glm::vec3(0.05, 0.05, 0.05));
        goldChair = glm::rotate(goldChair, 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        goldChair = glm::translate(goldChair, glm::vec3(0.0, 0.0, 0.0));
        shader.setMat4("model", goldChair);
        chairModel.Draw(shader);

        // apply brick textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bricksAlbedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bricksNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, bricksMetallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, bricksRoughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, bricksAO); 

        // draw the brick sphere - CT
        glm::mat4 brickModelCT = glm::mat4(1.0f);
        brickModelCT = glm::translate(brickModelCT, glm::vec3(0.0, 0.5, 0.0));
        brickModelCT = glm::rotate(brickModelCT, glm::radians(brickSphereRotator), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", brickModelCT);
        renderSphere();

        // draw a brick chair
        glm::mat4 brickChair = glm::mat4(1.0f);
        brickChair = glm::translate(brickChair, glm::vec3(-4.5, -0.5, -8.0));
        brickChair = glm::scale(brickChair, glm::vec3(0.05, 0.05, 0.05));
        brickChair = glm::rotate(brickChair, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        brickChair = glm::translate(brickChair, glm::vec3(0.0, 0.0, 0.0));
        shader.setMat4("model", brickChair);
        chairModel.Draw(shader);

        // apply chair textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, chairAlbedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, chairNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, chairMetallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, chairRoughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, chairAO); 

        // draw the chair
        glm::mat4 chairMod = glm::mat4(1.0f);
        chairMod = glm::translate(chairMod, glm::vec3(-2.0, -0.5, -8.0));
        chairMod = glm::scale(chairMod, glm::vec3(0.05, 0.05, 0.05));
        chairMod = glm::rotate(chairMod, glm::radians(70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        chairMod = glm::translate(chairMod, glm::vec3(0.0, 0.0, 0.0));
        shader.setMat4("model", chairMod);
        chairModel.Draw(shader);

        // Start Phong now
        phongShader.use();
        phongShader.setMat4("projection", projection);
        phongShader.setMat4("view", view);
        phongShader.setVec3("viewPos", camera.Position);
        // for each light pass it
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i) {
            phongShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
        }
        // pass the boolean for whether to use blinn or BP
        phongShader.setInt("blinn", blinn);

        // set the texture and draw the concrete
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, concretePhongAlbedo);
        glm::mat4 concreteModel = glm::mat4(1.0f);
        concreteModel = glm::translate(concreteModel, glm::vec3(-6.0, 0.5, 0.0));
        concreteModel = glm::rotate(concreteModel, glm::radians(phongSphere2Rotator), glm::vec3(0.0f, 1.0f, 0.0f));
        phongShader.setMat4("model", concreteModel);
        // set shininess, diffuse and specular values for the material
        phongShader.setFloat("shininess", 32.0f);
        phongShader.setVec3("materialDiffuse", glm::vec3(0.8f));
        phongShader.setVec3("materialSpecular", glm::vec3(0.3f));
        renderSphere();

        // draw a brick sphere with Phong also
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bricksPhongAlbedo);
        glm::mat4 bricksPhongModel = glm::mat4(1.0f);
        bricksPhongModel = glm::translate(bricksPhongModel, glm::vec3(-3.0, 0.5, 0.0));
        bricksPhongModel = glm::rotate(bricksPhongModel, glm::radians(phongSphereRotator), glm::vec3(0.0f, 1.0f, 0.0f));
        phongShader.setMat4("model", bricksPhongModel);
        // set shininess, diffuse and specular values for the material
        phongShader.setFloat("shininess",4.0f);
        phongShader.setVec3("materialDiffuse", glm::vec3(0.3f));
        phongShader.setVec3("materialSpecular", glm::vec3(0.1f));
        renderSphere();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        shader.use();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// key presses
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.Position = camera.Position + glm::vec3(0.0f, 0.05f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.Position = camera.Position - glm::vec3(0.0f, 0.05f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        sphereRotator = sphereRotator+1;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        brickSphereRotator = brickSphereRotator+1;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        phongSphereRotator = phongSphereRotator+1;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        phongSphere2Rotator = phongSphere2Rotator+1;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !blinnPressed) {
        blinn = !blinn;
        blinnPressed = true;
    }
    if(glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
        blinnPressed = false;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {

    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    // reversed since y-coordinates go from bottom to top in the window
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// renders and creates a sphere
unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere() {
    if (sphereVAO == 0) {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
            // even rows: y == 0, y == 2; and so on
            if (!oddRow) {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else {
                for (int x = X_SEGMENTS; x >= 0; --x) {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i) {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0) {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0) {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

// function to load textures
unsigned int loadTexture(char const * path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
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
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}