#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;
bool temp = false;
int random;
float setTime = 0;

// camera
Camera camera(glm::vec3(-10.0f, 8.0f, 10.0f));
float lastX = SCR_WIDTH / 1.0f;
float lastY = SCR_HEIGHT / 1.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float currentTime;
float lastTime;

// boxPos Init
float shakeAmount = 17.0f;
float shakeSpeed = 7.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // glfw cursor creation
    int width, height, nrChannel;
    unsigned char* cursorData = stbi_load("resources/textures/click3.png", &width, &height, &nrChannel, 0);
    if (nrChannel != 4) {
        cout << "nrChannel: " << nrChannel << endl;
        cout << "Failed to load cursor image" << endl;
        stbi_image_free(cursorData);
        return -1;
    }
    const GLFWimage cursorImage = { width, height, cursorData };
    GLFWcursor* cursor = glfwCreateCursor(&cursorImage, 0, 0);

    glfwMakeContextCurrent(window);
    glfwSetCursor(window, cursor);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    // stbi_set_flip_vertically_on_load(true);/

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // combile shaders
    Shader backModelShader("shader/model_loading.vs", "shader/model_loading.fs");
    Shader boxShader("shader/box.vs", "shader/box.fs");
    Shader texShader("shader/texture.vs", "shader/texture.fs");
    Shader geoShader("shader/geometry_shader.vs", "shader/geometry_shader.fs", "shader/geometry_shader.gs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    // VAO, VBO setting
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    // load Model
    Model backgroundModel("resources/background/sky1.obj");
    Model boxModel("resources/background/giftBox.obj");

    // load textures
    // -------------
    unsigned int ggamza1 = loadTexture("resources/textures/plz.png");
    unsigned int ggamza2 = loadTexture("resources/textures/love_ggamza.png");
    unsigned int ggamza3 = loadTexture("resources/textures/ming.png");
    unsigned int ggamza4 = loadTexture("resources/textures/sneeze.png");
    unsigned int ggamza5 = loadTexture("resources/textures/trance.png");
    unsigned int ggamza6 = loadTexture("resources/textures/present.png");


    // random seeds
    srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed

    // configure depth map FBO
    // -----------------------
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // texShader configuration
    // --------------------
    texShader.use();
    texShader.setInt("texture1", 0);

    // boxShader configuration
    boxShader.use();
    boxShader.setInt("texture_diffuse1", 0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // lighting info
    glm::vec3 ligthPos(-30.0f, 5.0f, 0.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.25f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // set backModelShader
        backModelShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        backModelShader.setMat4("projection", projection);
        backModelShader.setMat4("view", view);

        // render the loaded background model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
        backModelShader.setMat4("model", model);
        backgroundModel.Draw(backModelShader);

        // click on/off
        if (temp == false) {
            // set boxShader
            boxShader.use();
            boxShader.setVec3("light.direction", 0.6f, -0.1f, 0.0f);
            boxShader.setVec3("viewPos", camera.Position);
        
            // light properties
            glm::vec3 lightColor(0.8f, 0.8f, 0.8f);
            glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f);
            glm::vec3 ambientColor = diffuseColor * glm::vec3(0.8f);
            boxShader.setVec3("light.ambient", ambientColor);
            boxShader.setVec3("light.diffuse", diffuseColor);
            boxShader.setVec3("light.specular", 0.6f, 0.6f, 0.6f);

            // setting shininess
            boxShader.setFloat("shininess", 8.0f);

            // render the loaded box model
            float angle = sin(glfwGetTime() * shakeSpeed) * shakeAmount;
            glm::mat4 model1 = glm::mat4(1.0f);
            model1 = glm::translate(model1, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
            model1 = glm::rotate(model1, glm::radians(angle), glm::vec3(1.0f, 1.0f, 1.0f));
            model1 = glm::scale(model1, glm::vec3(1.5f, 1.5f, 1.5f));	// it's a bit too big for our scene, so scale it down
            boxShader.setMat4("model", model1);
            boxShader.setMat4("projection", projection);
            boxShader.setMat4("view", view);
            boxModel.Draw(boxShader);
        }
        else {
            glm::mat4 model3 = glm::mat4(1.0f);
            model3 = glm::translate(model3, glm::vec3(0.0f));
            model3 = glm::scale(model, glm::vec3(10.5f, 10.5f, 10.5f));
            geoShader.use();
            geoShader.setMat4("projection", projection);
            geoShader.setMat4("view", view);
            geoShader.setMat4("model", model3);

            // add time component to geometry shader in the form of a uniform
            geoShader.setFloat("time", static_cast<float>(glfwGetTime()));

            boxModel.Draw(geoShader);

            // setting ggmaza pop
            // set texShader
            texShader.use();
            glBindVertexArray(VAO);
            glActiveTexture(GL_TEXTURE0);
            switch (random)
            {
            case 1:
                glBindTexture(GL_TEXTURE_2D, ggamza1);
                break;
            case 2:
                glBindTexture(GL_TEXTURE_2D, ggamza2);
                break;
            case 3:
                glBindTexture(GL_TEXTURE_2D, ggamza3);
                break;
            case 4:
                glBindTexture(GL_TEXTURE_2D, ggamza4);
                break;
            case 5:
                glBindTexture(GL_TEXTURE_2D, ggamza5);
                break;
            case 6:
                glBindTexture(GL_TEXTURE_2D, ggamza6);
            default:
                break;

            // set particle          
            }

            // render the load texture
            glm::mat4 model2 = glm::mat4(1.0f);
            model2 = glm::translate(model2, glm::vec3(-2.0f, 0.0f, -4.0f)); // translate it down so it's at the center of the scene
            model2 = glm::rotate(model2, glm::radians(-55.0f), glm::vec3(0.5f, 1.0f, 0.1f));
            model2 = glm::scale(model2, glm::vec3(6.0f, 6.0f, 6.0f));	// it's a bit too big for our scene, so scale it down
            texShader.setMat4("model", model2);
            texShader.setMat4("projection", projection);
            texShader.setMat4("view", view);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cursor 해제
    glfwDestroyCursor(cursor);
    
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
	return 0;
}



// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    // ESC press: exit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // WSAD press: movement
    float cameraSpeed = static_cast<float>(0.3 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, cameraSpeed);

    // SPECE: cursor on/off
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPosCallback(window, NULL);
    }
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
    }

    // MOUSE LEFT CLICK: 랜덤뽑기
    if (glfwGetKey(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        glfwSetMouseButtonCallback(window, mouse_button_callback);
    }

        
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // 클릭 시 랜덤값 갱신
        random = rand() % 5 + 1;

        // 클릭 시 explore 시작점 원점으로.. ㅠㅠ
        glfwSetTime(0);

        //temp값 전환
        temp = (temp == false) ? true : false;
    }
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)  
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        cout << "nrComponent: " << nrComponents << endl;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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
