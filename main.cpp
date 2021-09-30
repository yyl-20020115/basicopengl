#define GLFW_INCLUDE_NONE
#include <glfw3.h>
#include <glad/glad.h>

#include<iostream>


//shader信息
struct ShaderInfo
{
    GLenum       type;      //shader的种类：顶点着色器？像素着色器？
    const char* filename;   //shader的文件
    GLuint       shader;    //shader对象，在LoadShaders函数中创建
};

//顶点的“布局”：（当前只有一个Position）
enum Attrib_IDs { vPosition = 0 };
//顶点数目
const GLuint  NumVertices = 6;
//顶点数组对象
GLuint  VertexArrayObject;

//读取shader文件到char数组中
static const GLchar* ReadShader(const char* filename)
{
    //读取文件：
    FILE* infile;
    fopen_s(&infile, filename, "rb");

    if (!infile)
        return NULL;

    //将文件指针位置设置为最末以获得char数目
    fseek(infile, 0, SEEK_END);
    int len = ftell(infile);

    //设置文件指针为开始
    fseek(infile, 0, SEEK_SET);

    //申请内存以存储文件内容
    GLchar* source = new GLchar[len + 1];

    //读取文件内容
    fread(source, 1, len, infile);
    source[len] = '\0';//文件末置为'\0'标志

    //关闭文件
    fclose(infile);

    return const_cast<const GLchar*>(source);
}
//编译shader们最后返回一个program对象
GLuint LoadShaders(ShaderInfo* shaders)
{
    if (shaders == NULL)
        return 0;

    //创建一个program对象
    GLuint program = glCreateProgram();

    //遍历所有的ShaderInfo
    for (ShaderInfo* entry = shaders; entry->type != GL_NONE; ++entry)
    {
        //创建一个shader对象并将它放到ShaderInfo中
        GLuint shader = glCreateShader(entry->type);
        entry->shader = shader;

        //读取shader文件到char数组中
        const GLchar* source = ReadShader(entry->filename);
        //失败则删除shader对象并返回
        if (source == NULL)
        {
            for (entry = shaders; entry->type != GL_NONE; ++entry)
            {
                glDeleteShader(entry->shader);
                entry->shader = 0;
            }
            return 0;
        }

        //将shader源代码传递到shader对象中
        glShaderSource(shader, 1, &source, NULL);
        delete[] source;
        //编译shader
        glCompileShader(shader);

        //查询是否编译成功
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        //如果编译失败则输出log
        if (!compiled)
        {
#ifdef _DEBUG
            GLsizei len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            GLchar* log = new GLchar[len + 1];
            glGetShaderInfoLog(shader, len, &len, log);
            std::cerr << "Shader compilation failed: " << log << std::endl;
            delete[] log;
#endif /* DEBUG */
            return 0;
        }

        //将shader对象连接到program对象上
        glAttachShader(program, shader);
    }

    //将program对象连接
    glLinkProgram(program);

    //查询是否连接失败
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    //如果连接失败则输出log
    if (!linked)
    {
#ifdef _DEBUG
        GLsizei len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        GLchar* log = new GLchar[len + 1];
        glGetProgramInfoLog(program, len, &len, log);
        std::cerr << "Shader linking failed: " << log << std::endl;
        delete[] log;
#endif /* DEBUG */
        //删除所有的shader
        for (ShaderInfo* entry = shaders; entry->type != GL_NONE; ++entry)
        {
            glDeleteShader(entry->shader);
            entry->shader = 0;
        }
        return 0;
    }

    return program;
}

void init(void)
{
    //----------------------------------------------------------------------------------------------------
    //Shader
    //----------------------------------------------------------------------------------------------------

    //shader信息：
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "triangles.vert" },
        { GL_FRAGMENT_SHADER, "triangles.frag" },
        { GL_NONE, NULL }   //GL_NONE表示结尾
    };
    //读取shader：
    GLuint program = LoadShaders(shaders);
    //绑定shader：
    glUseProgram(program);


    //----------------------------------------------------------------------------------------------------
    //顶点缓冲
    //----------------------------------------------------------------------------------------------------

    //顶点数据：
    GLfloat  vertices[NumVertices][2] =
    {
        { -0.90f, -0.90f }, {  0.85f, -0.90f }, { -0.90f,  0.85f },  // 第一个三角形
        {  0.90f, -0.85f }, {  0.90f,  0.90f }, { -0.85f,  0.90f }   // 第二个三角形
    };

    //创建顶点数组对象
    glGenVertexArrays(1, &VertexArrayObject);
    //设定顶点数组对象为“当前”
    glBindVertexArray(VertexArrayObject);

    //顶点的缓冲
    GLuint  VertexBuffer;
    //创建缓冲
    glCreateBuffers(1, &VertexBuffer);
    //设定缓冲为“当前”
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    //指定缓冲的顶点数据
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);

    //指定的顶点的“布局”：（当前只有一个Position）
    glVertexAttribPointer(Attrib_IDs::vPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(Attrib_IDs::vPosition);
}

void display(void)
{
    //清理颜色
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glClearBufferfv(GL_COLOR, 0, black);

    //绑定顶点数据
    glBindVertexArray(VertexArrayObject);

    //Draw Call：
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

int main(void)
{
    //必须先初始化该库，然后才能使用大多数GLFW函数。成功初始化后，GLFW_TRUE将返回。如果发生错误，GLFW_FALSE则返回。
    if (!glfwInit())
        return -1;

    //创建窗口（OpenGL上下文似乎也一并创建了）
    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    //使用GLAD来加载OpenGL的函数地址
    gladLoadGL();

    //初始化（顶点缓冲和shader）
    init();

    //循环直到用户关闭窗口
    while (!glfwWindowShouldClose(window))
    {
        //显示（调用 draw call）
        display();

        //交换前后缓冲
        glfwSwapBuffers(window);

        //轮询并处理事件
        glfwPollEvents();
    }

    //使用GLFW完成操作后，通常是在应用程序退出之前，需要终止GLFW
    glfwTerminate();

    return 0;
}