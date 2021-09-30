#define GLFW_INCLUDE_NONE
#include <glfw3.h>
#include <glad/glad.h>

#include<iostream>


//shader��Ϣ
struct ShaderInfo
{
    GLenum       type;      //shader�����ࣺ������ɫ����������ɫ����
    const char* filename;   //shader���ļ�
    GLuint       shader;    //shader������LoadShaders�����д���
};

//����ġ����֡�������ǰֻ��һ��Position��
enum Attrib_IDs { vPosition = 0 };
//������Ŀ
const GLuint  NumVertices = 6;
//�����������
GLuint  VertexArrayObject;

//��ȡshader�ļ���char������
static const GLchar* ReadShader(const char* filename)
{
    //��ȡ�ļ���
    FILE* infile;
    fopen_s(&infile, filename, "rb");

    if (!infile)
        return NULL;

    //���ļ�ָ��λ������Ϊ��ĩ�Ի��char��Ŀ
    fseek(infile, 0, SEEK_END);
    int len = ftell(infile);

    //�����ļ�ָ��Ϊ��ʼ
    fseek(infile, 0, SEEK_SET);

    //�����ڴ��Դ洢�ļ�����
    GLchar* source = new GLchar[len + 1];

    //��ȡ�ļ�����
    fread(source, 1, len, infile);
    source[len] = '\0';//�ļ�ĩ��Ϊ'\0'��־

    //�ر��ļ�
    fclose(infile);

    return const_cast<const GLchar*>(source);
}
//����shader����󷵻�һ��program����
GLuint LoadShaders(ShaderInfo* shaders)
{
    if (shaders == NULL)
        return 0;

    //����һ��program����
    GLuint program = glCreateProgram();

    //�������е�ShaderInfo
    for (ShaderInfo* entry = shaders; entry->type != GL_NONE; ++entry)
    {
        //����һ��shader���󲢽����ŵ�ShaderInfo��
        GLuint shader = glCreateShader(entry->type);
        entry->shader = shader;

        //��ȡshader�ļ���char������
        const GLchar* source = ReadShader(entry->filename);
        //ʧ����ɾ��shader���󲢷���
        if (source == NULL)
        {
            for (entry = shaders; entry->type != GL_NONE; ++entry)
            {
                glDeleteShader(entry->shader);
                entry->shader = 0;
            }
            return 0;
        }

        //��shaderԴ���봫�ݵ�shader������
        glShaderSource(shader, 1, &source, NULL);
        delete[] source;
        //����shader
        glCompileShader(shader);

        //��ѯ�Ƿ����ɹ�
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        //�������ʧ�������log
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

        //��shader�������ӵ�program������
        glAttachShader(program, shader);
    }

    //��program��������
    glLinkProgram(program);

    //��ѯ�Ƿ�����ʧ��
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    //�������ʧ�������log
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
        //ɾ�����е�shader
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

    //shader��Ϣ��
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "triangles.vert" },
        { GL_FRAGMENT_SHADER, "triangles.frag" },
        { GL_NONE, NULL }   //GL_NONE��ʾ��β
    };
    //��ȡshader��
    GLuint program = LoadShaders(shaders);
    //��shader��
    glUseProgram(program);


    //----------------------------------------------------------------------------------------------------
    //���㻺��
    //----------------------------------------------------------------------------------------------------

    //�������ݣ�
    GLfloat  vertices[NumVertices][2] =
    {
        { -0.90f, -0.90f }, {  0.85f, -0.90f }, { -0.90f,  0.85f },  // ��һ��������
        {  0.90f, -0.85f }, {  0.90f,  0.90f }, { -0.85f,  0.90f }   // �ڶ���������
    };

    //���������������
    glGenVertexArrays(1, &VertexArrayObject);
    //�趨�����������Ϊ����ǰ��
    glBindVertexArray(VertexArrayObject);

    //����Ļ���
    GLuint  VertexBuffer;
    //��������
    glCreateBuffers(1, &VertexBuffer);
    //�趨����Ϊ����ǰ��
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    //ָ������Ķ�������
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);

    //ָ���Ķ���ġ����֡�������ǰֻ��һ��Position��
    glVertexAttribPointer(Attrib_IDs::vPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(Attrib_IDs::vPosition);
}

void display(void)
{
    //������ɫ
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glClearBufferfv(GL_COLOR, 0, black);

    //�󶨶�������
    glBindVertexArray(VertexArrayObject);

    //Draw Call��
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

int main(void)
{
    //�����ȳ�ʼ���ÿ⣬Ȼ�����ʹ�ô����GLFW�������ɹ���ʼ����GLFW_TRUE�����ء������������GLFW_FALSE�򷵻ء�
    if (!glfwInit())
        return -1;

    //�������ڣ�OpenGL�������ƺ�Ҳһ�������ˣ�
    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    //ʹ��GLAD������OpenGL�ĺ�����ַ
    gladLoadGL();

    //��ʼ�������㻺���shader��
    init();

    //ѭ��ֱ���û��رմ���
    while (!glfwWindowShouldClose(window))
    {
        //��ʾ������ draw call��
        display();

        //����ǰ�󻺳�
        glfwSwapBuffers(window);

        //��ѯ�������¼�
        glfwPollEvents();
    }

    //ʹ��GLFW��ɲ�����ͨ������Ӧ�ó����˳�֮ǰ����Ҫ��ֹGLFW
    glfwTerminate();

    return 0;
}