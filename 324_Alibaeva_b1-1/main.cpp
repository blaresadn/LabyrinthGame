#include "common.h"
#include "Image.h"
#include "Player.h"
#include "labyrinth.h"

#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <unistd.h>

constexpr GLsizei WINDOW_WIDTH = 1024, WINDOW_HEIGHT = 1024;

struct InputState
{
    bool keys[1024]{}; //массив состояний кнопок - нажата/не нажата
    GLfloat lastX = 400, lastY = 300; //исходное положение мыши
    bool firstMouse = true;
    bool captureMouse         = true;  // Мышка захвачена нашим приложением или нет?
    bool capturedMouseJustNow = false;
} static Input;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_1:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case GLFW_KEY_2:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
        default:
            if (action == GLFW_PRESS)
                Input.keys[key] = true;
            else if (action == GLFW_RELEASE)
                Input.keys[key] = false;
    }
}

void processPlayerMovement(Player &player)
{
    if (Input.keys[GLFW_KEY_W])
        player.ProcessInput(MovementDir::UP);
    else if (Input.keys[GLFW_KEY_S])
        player.ProcessInput(MovementDir::DOWN);
    if (Input.keys[GLFW_KEY_A])
        player.ProcessInput(MovementDir::LEFT);
    else if (Input.keys[GLFW_KEY_D])
        player.ProcessInput(MovementDir::RIGHT);
}

void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        Input.captureMouse = !Input.captureMouse;

    if (Input.captureMouse)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        Input.capturedMouseJustNow = true;
    }
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    if (Input.firstMouse)
    {
        Input.lastX = float(xpos);
        Input.lastY = float(ypos);
        Input.firstMouse = false;
    }

    GLfloat xoffset = float(xpos) - Input.lastX;
    GLfloat yoffset = Input.lastY - float(ypos);

    Input.lastX = float(xpos);
    Input.lastY = float(ypos);
}


void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    // ...
}


int initGL()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    std::cout << "Controls: "<< std::endl;
    std::cout << "press right mouse button to capture/release mouse cursor  "<< std::endl;
    std::cout << "W, A, S, D - movement  "<< std::endl;
    std::cout << "press ESC to exit" << std::endl;

    return 0;
}

int main(int argc, char** argv)
{
    if(!glfwInit())
        return -1;

//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow*  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "task1 base project", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback        (window, OnKeyboardPressed);
    glfwSetCursorPosCallback  (window, OnMouseMove);
    glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
    glfwSetScrollCallback     (window, OnMouseScroll);

    if(initGL() != 0)
        return -1;

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    Point starting_pos{.x = 0, .y = 32 * 15};
    Player player{starting_pos};

    Image screenBuffer(WINDOW_WIDTH, WINDOW_HEIGHT, 4);
    //Image tile("resources/player/right.png");
    Image Win("resources/win.jpg");
    Image GameOver("resources/game_over.jpg");

    // прорисовываем первую комнату
    Labyrinth Map;
    Pixel **map_ = Map.GetMap();
    for (int y = 0; y < WINDOW_HEIGHT; y++) {
        for (int x = 0; x < WINDOW_WIDTH; x++) {
            screenBuffer.PutPixel(x, y, map_[x][y]);
        }
    }

    // прорисовываем жизни персонажа
    Image heart = Image("resources/heart.png");
    for (int i = 0; i < 3; i++) {
        for (int y = WINDOW_HEIGHT - 1; y > WINDOW_HEIGHT - heart.Height(); y--) {
            for (int x = WINDOW_WIDTH - heart.Width() * (i + 1); x < WINDOW_WIDTH; x++) {
                screenBuffer.PutPixel(x, y, player.blend(map_[x][y], heart.GetPixel(x % heart.Width(), heart.Height() - y % heart.Height() - 1)));
            }
        }
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);  GL_CHECK_ERRORS;
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); GL_CHECK_ERRORS;

    int end = 0; // код, который возвращает метод Draw (говорит о том, нужно ли делать переход)
    bool mode = false; // флаг, который меняется в зависимости от того, нужно комнату освещать или затемнять
    bool done = false; // флаг для завершения игры
    //game loop
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwPollEvents();

        processPlayerMovement(player);

        if (!done) {
            end = player.Draw(screenBuffer, lastFrame);
        }

        // затемняем картинку
        if (end != 0 && !done && !mode) {
            for (float k = 1.; k >= 0.; k -= 0.05) {
                for (int y = 0; y < WINDOW_HEIGHT; y++) {
                    for (int x = 0; x < WINDOW_WIDTH; x++) {
                        Pixel p = screenBuffer.GetPixel(x, y);
                        p.r *= k;
                        p.g *= k;
                        p.b *= k;
                        p.a *= k;
                        screenBuffer.PutPixel(x, y, p);
                    }
                }
                glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenBuffer.Data());
                GL_CHECK_ERRORS;
                glfwSwapBuffers(window);
            }
            mode = true;
            // осветляем новую комнату
        } else if (end == 0 && mode) {
            Image tmp(WINDOW_WIDTH, WINDOW_HEIGHT, 4);
            for (int y = 0; y < WINDOW_HEIGHT; y++) {
                for (int x = 0; x < WINDOW_WIDTH; x++) {
                    tmp.PutPixel(x, y, screenBuffer.GetPixel(x, y));
                }
            }
            for (float k = 0.; k <= 1.; k += 0.05) {
                for (int y = 0; y < WINDOW_HEIGHT; y++) {
                    for (int x = 0; x < WINDOW_WIDTH; x++) {
                        Pixel p = tmp.GetPixel(x, y);
                        p.r *= k;
                        p.g *= k;
                        p.b *= k;
                        p.a *= k;
                        screenBuffer.PutPixel(x, y, p);
                    }
                }
                glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenBuffer.Data());
                GL_CHECK_ERRORS;
                glfwSwapBuffers(window);
            }
            mode = false;
        }

        if (!done && (end == 2 || end == 3)) {
            // сообщение о победе
            if (end == 2) {
                for (float k = 0.; k <= 1.; k += 0.05) {
                    for (int y = 0; y < WINDOW_HEIGHT; y++) {
                        for (int x = 0; x < WINDOW_WIDTH; x++) {
                            Pixel p = Win.GetPixel(x, y);
                            p.r *= k;
                            p.g *= k;
                            p.b *= k;
                            p.a *= k;
                            screenBuffer.PutPixel(x, WINDOW_HEIGHT - y - 1, p);
                        }
                    }
                    glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenBuffer.Data());
                    GL_CHECK_ERRORS;
                    glfwSwapBuffers(window);
                }
                // сообщение о проигрыше
            } else if (end == 3) {
                for (float k = 0.; k <= 1.; k += 0.05) {
                    for (int y = 0; y < WINDOW_HEIGHT; y++) {
                        for (int x = 0; x < WINDOW_WIDTH; x++) {
                            Pixel p = GameOver.GetPixel(x, y);
                            p.r *= k;
                            p.g *= k;
                            p.b *= k;
                            p.a *= k;
                            screenBuffer.PutPixel(x, WINDOW_HEIGHT - y - 1, p);
                        }
                    }
                    glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenBuffer.Data());
                    GL_CHECK_ERRORS;
                    glfwSwapBuffers(window);
                }
            }
            done = true;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;
        glDrawPixels (WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenBuffer.Data()); GL_CHECK_ERRORS;

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
