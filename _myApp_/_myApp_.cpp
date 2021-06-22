// sb6.h ��� ������ ���Խ�Ų��.
#include <sb6.h>
#include <vmath.h>
#include <shader.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Model.h"

// sb6::application�� ��ӹ޴´�.
class my_application : public sb6::application
{
public:
	// ���̴� �������Ѵ�.
	// basic multiple lighting shader
	GLuint compile_shader(void)
	{
		// ���ؽ� ���̴��� �����ϰ� �������Ѵ�.
		GLuint vertex_shader = sb6::shader::load("../../src/_myApp_/multiple_lights_vs.glsl", GL_VERTEX_SHADER);

		// �����׸�Ʈ ���̴��� �����ϰ� �������Ѵ�.
		GLuint fragment_shader = sb6::shader::load("../../src/_myApp_/multiple_lights_fs.glsl", GL_FRAGMENT_SHADER);

		// ���α׷��� �����ϰ� ���̴��� Attach��Ű�� ��ũ�Ѵ�.
		GLuint program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

		// ���� ���α׷��� ���̴��� �����ϹǷ� ���̴��� �����Ѵ�.
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return program;
	}
	//postProcessing ���α׷�
	GLuint postProcessing(void)
	{
		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint program;

		// ���ؽ� ���̴� �ҽ� �ڵ�
		static const GLchar * vertex_shader_source[] =
		{
			"#version 430 core											\n"
			"															\n"
			"layout(location = 0) in vec2 pos;							\n"
			"layout(location = 1) in vec2 texCoord;						\n"

			"															\n"
			"out vec2 vsTexCoord;										\n"
			"															\n"
			"void main(void)											\n"
			"{															\n"
			"	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);				\n"
			"															\n"
			"	vsTexCoord = texCoord;									\n"
			"}															\n"
		};

		// �����׸�Ʈ ���̴� �ҽ� �ڵ�
		static const GLchar * fragment_shader_source[] =
		{
			"#version 430 core								\n"
			"												\n"
			"in vec2 vsTexCoord;							\n"
			"uniform sampler2D screenTexture;				\n"
			"												\n"
			"out vec4 fragColor;							\n"
			"												\n"
			"float offsetX = 1.0f / float(textureSize(screenTexture, 0).x);	\n"
			"float offsetY = 1.0f / float(textureSize(screenTexture, 0).y);	\n"

			"void main(void)								\n"
			"{												\n"
			"	vec2 offsets[9] = vec2[](					\n"
			"	vec2(-offsetX, offsetY), // top-left		\n"
			"	vec2(0.0f, offsetY), // top-center			\n"
			"	vec2(offsetX, offsetY), // top-right		\n"
			"	vec2(-offsetX, 0.0f), // center-left		\n"
			"	vec2(0.0f, 0.0f), // center-center			\n"
			"	vec2(offsetX, 0.0f), // center-right		\n"
			"	vec2(-offsetX, -offsetY), // bottom-left	\n"
			"	vec2(0.0f, -offsetY), // bottom-center		\n"
			"	vec2(offsetX, -offsetY) // bottom-right 	\n"
			"	);											\n"
			"	float kernel[9] = float[](					\n"   // ���� 
			"		2, 2, 2,								\n"
			"		2, -13, 2,								\n"
			"		2, 2, 2									\n"
			"		);										\n"
			"	vec3 sampleTex[9];							\n"
			"	for (int i = 0; i < 9; i++) {				\n"
			"		sampleTex[i] = vec3(texture(screenTexture, vsTexCoord.st + offsets[i]));	\n"
			"	}																				\n"
			"	vec3 color = vec3(0.0);	\n"
			"	for (int i = 0; i < 9; i++)	\n"
			"		color += sampleTex[i] * kernel[i];	\n"
			"	fragColor = vec4(color, 1.0);	\n"
			//"	fragColor = texture(screenTexture, vsTexCoord);"
			"}												\n"
		};

		// ���ؽ� ���̴��� �����ϰ� �������Ѵ�.
		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		// �����׸�Ʈ ���̴��� �����ϰ� �������Ѵ�.
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		// ���α׷��� �����ϰ� ���̴��� Attach��Ű�� ��ũ�Ѵ�.
		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

		// ���� ���α׷��� ���̴��� �����ϹǷ� ���̴��� �����Ѵ�.
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return program;
	}
	// ���ø����̼� �ʱ�ȭ �����Ѵ�.
	virtual void startup()
	{
		// Ŭ���� ���� ���� �ʱ�ȭ
		initValues();

		// ���̴� ���α׷� ������ �� ����
		shader_program = compile_shader();  //������Ʈ ���α׷�
		shader_program_screen = postProcessing();  // post processing ���α׷�
		// VAO, VBO, EBO, texture ����
		glGenVertexArrays(1, &VAO_screen);
		glGenBuffers(1, &VBO_screen);

		stbi_set_flip_vertically_on_load(true);

		// ù ��° ��ü ���� :������ OBJ �𵨵�  --------------------------------------------------
		objModel[0].init();
		objModel[0].loadOBJ("../../src/_myApp_/tree.obj");   // ����
		objModel[0].loadDiffuseMap("../../src/_myApp_/Vegetation_Bark_Maple_1_baseColor.jpeg");

		objModel[1].init();                                 // �̲���Ʋ
		objModel[1].loadOBJ("../../src/_myApp_/slide.obj");
		objModel[1].loadDiffuseMap("../../src/_myApp_/slide.png");

		objModel[2].init();                                 /// ��ġ����
		objModel[2].loadOBJ("../../src/_myApp_/bench.obj");
		objModel[2].loadDiffuseMap("../../src/_myApp_/benchColor.jpeg");
		objModel[2].loadNormalMap("../../src/_myApp_/bench_normal.png");

		// �� ��° ��ü ���� : �Ƕ�̵� --------------------------------------------------
		// �Ƕ�̵� ������ ��ġ�� �÷�, �ؽ�ó ��ǥ�� �����Ѵ�.
		GLfloat pyramid_vertices[] = {
			1.0f, 0.0f, -1.0f,    // ���� ���
			-1.0f, 0.0f, -1.0f,   // ���� ���
			-1.0f, 0.0f, 1.0f,    // ���� �ϴ�
			1.0f, 0.0f, 1.0f,     // ���� �ϴ�
			0.0f, 1.0f, 0.0f,      // ��� ������
			0.0f, -1.0f, 0.0f,      // �ϴ� ������
		};

		// �ﰢ������ �׸� �ε����� �����Ѵ�.
		GLuint pyramid_indices[] = {
			4, 0, 1,
			4, 1, 2,
			4, 2, 3,
			4, 3, 0,

			5, 1, 0,
			5, 2, 1,
			5, 3, 2,
			5, 0, 3,
		};

		pyramidModel.init();
		pyramidModel.setupMesh(6, pyramid_vertices);
		pyramidModel.setupIndices(24, pyramid_indices);

		// ����° ��ü����-�� ���̽�----------------------------
		buildingModel[0].init();
		buildingModel[0].loadOBJ("../../src/_myApp_/base.obj");
		buildingModel[0].loadDiffuseMap("../../src/_myApp_/base.png");
		buildingModel[0].loadNormalMap("../../src/_myApp_/base_normal.png");


		// ����° ��ü����-�� ����
		buildingModel[1].init();
		buildingModel[1].loadOBJ("../../src/_myApp_/roof.obj");
		buildingModel[1].loadDiffuseMap("../../src/_myApp_/roof2.jpg");
		buildingModel[1].loadNormalMap("../../src/_myApp_/roof2_normal.png");

		// ����° ��ü����-�� ��
		buildingModel[2].init();
		buildingModel[2].loadOBJ("../../src/_myApp_/wall.obj");
		buildingModel[2].loadDiffuseMap("../../src/_myApp_/wall.png");

		// ����° ��ü����-�� ����
		buildingModel[3].init();
		buildingModel[3].loadOBJ("../../src/_myApp_/wood.obj");
		buildingModel[3].loadDiffuseMap("../../src/_myApp_/wood.jpeg");
		buildingModel[3].loadNormalMap("../../src/_myApp_/wood_normal.png");

		///�׹�° ��ü����----�ٴ�---------------------------------
		
		//��ġ
		GLfloat floor_vertices[] = {
			1.0f, 0.0f, -1.0f,   // ���� ���
			-1.0f, 0.0f, -1.0f,   // ���� ���
			-1.0f, 0.0f, 1.0f,    // ���� �ϴ�
			1.0f, 0.0f, 1.0f   // ���� �ϴ�
		};
		//�ؽ���
		GLfloat floor_texVertices[] = {
			10.0f, 10.0f,  // ���� ���
			0.0f, 10.0f,  // ���� ���
			0.0f, 0.0f,   // ���� �ϴ�
			10.0f, 0.0f   // ���� �ϴ�
		};
		//���
		GLfloat floor_normVertices[] = {
			0.0f, 1.0f, 0.0f, // ���� ���
			0.0f, 1.0f, 0.0f,  // ���� ���
			0.0f, 1.0f, 0.0f,   // ���� �ϴ�
			0.0f, 1.0f, 0.0f   // ���� �ϴ�
		};

		// �ﰢ������ �׸� �ε����� ����
		GLuint floor_indices[] = {
			0, 1, 2,	// ù��° �ﰢ��
			0, 2, 3		// �ι�° �ﰢ��
		};
		groundModel.init();
		groundModel.setupMesh(4, floor_vertices, floor_texVertices, floor_normVertices);
		groundModel.setupIndices(6, floor_indices);
		groundModel.loadDiffuseMap("../../src/_myApp_/grass.jpg");
		


		glEnable(GL_MULTISAMPLE);


		/////////////////Post-processing/////////////
		glBindVertexArray(VAO_screen);

		// Post-processing�� ��ũ�� ����
		GLfloat screenVertices[] = {
			//NDC xy��ǥ //�ؽ�ó UV
			-1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f
		};
		// VBO�� �����Ͽ� vertices ������ ����
		glBindBuffer(GL_ARRAY_BUFFER, VBO_screen);
		glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), screenVertices, GL_STATIC_DRAW);

		// VBO�� ����� �� ���ؽ� �Ӽ����� ����
		//NDC xy��ǥ �Ӽ� (location = 0)
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// �ؽ�ó UV�Ӽ� (location = 1)
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// VBO �� ���ؽ� �Ӽ��� �� ������ VBO�� VAO�� unbind�Ѵ�.
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		//////////////FBO/////////////////////////////////


		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		// color buffer �ؽ�ó ���� �� ����

		glGenTextures(1, &FBO_texture);
		glBindTexture(GL_TEXTURE_2D, FBO_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, info.windowWidth, info.windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBO_texture, 0);


		// depth&stencil buffer�� ���� Render Buffer Object ���� �� ����

		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, info.windowWidth, info.windowHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			glfwTerminate();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// ���ø����̼� ���� �� ȣ��ȴ�.
	virtual void shutdown()
	{
		glDeleteProgram(shader_program);
		delete[] genModel;

		glDeleteTextures(1, &FBO_texture);
		glDeleteFramebuffers(1, &FBO);
		glDeleteRenderbuffers(1, &RBO);
		glDeleteBuffers(1, &VBO_screen);
		glDeleteVertexArrays(1, &VAO_screen);
		glDeleteProgram(shader_program_screen);
	}

	// ������ virtual �Լ��� �ۼ��ؼ� �������̵��Ѵ�.
	virtual void render(double currentTime)
	{
		if (filterMode){  // ��������
			////////////////// FBO ���ε�
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);
			// FBO�� ����� ���۵��� ���� �����, ���� �׽��� Ȱ��ȭ
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			// Do 1st Rendering
			// ���ݺ��� �������� ��������� �츮�� ���� �����ӹ��ۿ� ����
		}

		if (pause) {  // �Ͻ�����
			previousTime = currentTime;
			return;
		}

		animationTime += currentTime - previousTime;
		previousTime = currentTime;

		const GLfloat color[] = { 0.0f, (float)sin(animationTime / 2) * 0.5f + 0.5f, (float)sin(animationTime / 2) * 0.5f + 0.5f, 1.0f };  // ���� �� ��� ǥ��

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		glClearBufferfv(GL_COLOR, 0, color);
		// ī�޶� ��Ʈ���� ���


		float distance = 5.f;

		vmath::vec3 eye((float)sin(camXangle)*distance + camXpos, (float)sin(camYangle)*distance + camYpos, (float)cos(camYangle + camXangle)*distance);     // ���콺 ��Ŭ�� �巡��-ī�޶� ȸ��, �� Ŭ��-�̵�
		vmath::vec3 center(camXpos, camYpos, 0.0);  //�� Ŭ��-�̵�
		vmath::vec3 up(0.0, 1.0, 0.0);
		vmath::mat4 lookAt = vmath::lookat(eye, center, up);
		vmath::mat4 projM = vmath::perspective(fov, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);

		// ������ ���� ---------------------------------------
		float r = 5.0f;
		vmath::vec3 pointLightPos[] = { vmath::vec3(-1.0f, -0.2f, 0.2f),  // �� �� �Һ�
			vmath::vec3(0.0f, (float)sin(animationTime*0.5f)*r, (float)cos(animationTime*0.5f)*r) };   // �� �Һ�
		vmath::vec3 lightColor(1.0f, 1.0f, 1.0f);
		vmath::vec3 viewPos = eye;

		// �� �׸��� ---------------------------------------
		glUseProgram(shader_program);

		//uniform���� ����, ī�޶� ���� �Ѱ���
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, projM);
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, lookAt);

		glUniform3fv(glGetUniformLocation(shader_program, "viewPos"), 1, viewPos);

		glUniform3f(glGetUniformLocation(shader_program, "dirLight.direction"), -1.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(shader_program, "dirLight.ambient"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(shader_program, "dirLight.diffuse"), 0.4f, 0.4f, 0.4f);
		glUniform3f(glGetUniformLocation(shader_program, "dirLight.specular"), 0.5f, 0.5f, 0.5f);

		glUniform3fv(glGetUniformLocation(shader_program, "pointLights[0].position"), 1, pointLightPos[0]);
		glUniform3f(glGetUniformLocation(shader_program, "pointLights[0].ambient"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(shader_program, "pointLights[0].diffuse"), 0.8f, 0.8f, 0.8f);
		glUniform3f(glGetUniformLocation(shader_program, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(shader_program, "pointLights[0].c1"), 0.09f);
		glUniform1f(glGetUniformLocation(shader_program, "pointLights[0].c2"), 0.032f);
		glUniform3fv(glGetUniformLocation(shader_program, "pointLights[1].position"), 1, pointLightPos[1]);
		glUniform3f(glGetUniformLocation(shader_program, "pointLights[1].ambient"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(shader_program, "pointLights[1].diffuse"), 0.8f, 0.8f, 0.8f);
		glUniform3f(glGetUniformLocation(shader_program, "pointLights[1].specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(shader_program, "pointLights[1].c1"), 0.09f);
		glUniform1f(glGetUniformLocation(shader_program, "pointLights[1].c2"), 0.032f);

		glUniform3fv(glGetUniformLocation(shader_program, "spotLight.position"), 1, eye);
		glUniform3fv(glGetUniformLocation(shader_program, "spotLight.direction"), 1, center - eye);
		glUniform1f(glGetUniformLocation(shader_program, "spotLight.cutOff"), (float)cos(vmath::radians(22.5)));
		glUniform1f(glGetUniformLocation(shader_program, "spotLight.outerCutOff"), (float)cos(vmath::radians(25.5)));
		glUniform1f(glGetUniformLocation(shader_program, "spotLight.c1"), 0.09f);
		glUniform1f(glGetUniformLocation(shader_program, "spotLight.c2"), 0.032f);
		glUniform3f(glGetUniformLocation(shader_program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);

		if (onSpotLight)  // ������ �Ѱ� ����
		{
			glUniform3f(glGetUniformLocation(shader_program, "spotLight.diffuse"), 1.0f, 1.0f, 1.0f);
			glUniform3f(glGetUniformLocation(shader_program, "spotLight.specular"), 1.0f, 1.0f, 1.0f);
		}
		else{
			glUniform3f(glGetUniformLocation(shader_program, "spotLight.diffuse"), 0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(shader_program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
		}


		/*if (lineMode)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/


		vmath::mat4 model = vmath::translate(genPosition) *
			vmath::rotate(objYangle, 0.0f, 1.0f, 0.0f) *
			vmath::scale(0.01f);

		//t������ �� ������Ʈ �����
		if (makeObj && nowModel <= 10)
		{
			objPosition[nowModel - 1] = vmath::vec3(objXpos, -1.0f, objZpos);  // wasd�� �̵�
			genModel[nowModel - 1] = objModel[nowObj];
			objYangles[nowModel - 1] = objYangle;  // ��Ŭ�� ��ü ȸ��
		}

		for (int i = 0; i < nowModel; i++)
		{
			model = vmath::translate(objPosition[i]) *
				vmath::rotate(objYangles[i], 0.0f, 1.0f, 0.0f) *
				vmath::scale(0.01f);
			glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, model);
			genModel[i].draw(shader_program);
		}

		// �� �׸���
		if (drawHouse) {


			model = vmath::translate(vmath::vec3(-1, -1, -0.5)) *
				vmath::rotate(0.0f, 0.0f, 1.0f, 0.0f) *
				vmath::scale(0.1f);

			glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, model);
			buildingModel[0].draw(shader_program);
			buildingModel[1].draw(shader_program);
			buildingModel[2].draw(shader_program);
			buildingModel[3].draw(shader_program);

		}

		// ���� �Ƕ�̵� �׸��� ---------------------------------------
		if (drawLight) {
			for (int i = 0; i < 2; i++)
			{
				float scaleFactor = 0.05f;
				vmath::mat4 transform = vmath::translate(pointLightPos[i])*
					vmath::scale(scaleFactor, scaleFactor, scaleFactor);
				glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, transform);

				pyramidModel.draw(shader_program);
			}
		}

		// �ٴ� �׸���
		model = vmath::translate(vmath::vec3(-1, -1, -0.5)) *
			vmath::rotate(0.0f, 0.0f, 1.0f, 0.0f) *
			vmath::scale(10.0f);
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, model);
		groundModel.draw(shader_program);


		if (filterMode){
			///////////////////////�����ӹ��� �ؽ�������/////////////////////////////
			// �⺻ Framebuffer�� �ǵ�����
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			// ���۵��� �� �����
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			//// FBO Texture�� ��ũ�� ���̴� ���α׷��� ����   //// POST PROCESSING
			glUseProgram(shader_program_screen);
			glUniform1i(glGetUniformLocation(shader_program_screen, "screenTexture"), 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, FBO_texture);

			glBindVertexArray(VAO_screen);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

	}

	//ȭ�� Ű������ ũ�� ����
	virtual void onResize(int w, int h)
	{
		sb6::application::onResize(w, h);

		if (glViewport != NULL)
			glViewport(0, 0, info.windowWidth, info.windowHeight);
	}
	//��Ƽ �ٸ����
	virtual void init()
	{
		sb6::application::init();

		info.samples = 8;
		info.flags.debug = 1;
	}

	//Ű���� �̺�Ʈ
	virtual void onKey(int key, int action)
	{
		if (action == GLFW_PRESS) {
			switch (key) {
			case ' ': // �����̽��� - �Ͻ�����
				pause = !pause;
				break;

			case 'H':  // �� �׸���
				drawHouse = !drawHouse;
				break;
			case 'I':  // ���� ������Ʈ �׸���
				drawLight = !drawLight;
				break;
			case 'M':   // ����Ʈ ���μ��� ���
				filterMode = !filterMode;
				break;
			case 'R':   // �ʱ�ȭ
				initValues();
				break;
			case 'L':   // ������ �Ѱ� ����
				onSpotLight = !onSpotLight;
				break;
			default:
				break;
			case 'A':     //  �����Ǵ� ������Ʈ ��ġ �ű��
				if (makeObj)
					objXpos--;
				break;
			case 'D':
				if (makeObj)
					objXpos++;
				break;
			case 'W':
				if (makeObj)
					objZpos--;
				break;
			case 'S':
				if (makeObj)
					objZpos++;
				break;
			case'T':   // �� ������Ʈ ����
				makeObj = !makeObj;
				if (makeObj)
				{
					nowModel++;
					objXpos = 0.0f;
					objZpos = 0.0f;
					objYangle = 0.0f;
				}
				break;

			case '1':  // ������Ʈ ���� ���� 1-���� 2-�̲���Ʋ 3-��ġ
				if (makeObj)
					nowObj = 0;
				break;
			case '2':
				if (makeObj)
					nowObj = 1;
				break;
			case '3':
				if (makeObj)
					nowObj = 2;
				break;
			};
		}


	}

	//���콺 �̺�Ʈ
	virtual void onMouseButton(int button, int action)
	{
		if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT || GLFW_MOUSE_BUTTON_MIDDLE) && action == GLFW_PRESS)
		{
			mouseDown = true;

			int x, y;
			if (button == GLFW_MOUSE_BUTTON_LEFT)
				mouseBTN = 0;
			else if (button == GLFW_MOUSE_BUTTON_RIGHT)
				mouseBTN = 1;
			else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
				mouseBTN = 2;
			getMousePosition(x, y);
			mousePosition = vmath::vec2(float(x), float(y));
		}
		else
		{
			mouseDown = false;
		}


	}

	virtual void onMouseMove(int x, int y)
	{
		if (mouseDown)
		{
			if (mouseBTN == 0)  // ��Ŭ�� �巡��-ī�޶� ȸ��
			{
				camXangle += float(x - mousePosition[0]) / (info.windowWidth / 2);
				camYangle += float(y - mousePosition[1]) / info.windowHeight;
				
			}
			else if (mouseBTN == 1)  // ��Ŭ�� �巡��-������Ʈ ���������� �� ������Ʈ ȸ��
			{ 
				objYangle += float(x - mousePosition[0]) / (info.windowWidth /50);
			}
			else if (mouseBTN == 2)  // �� Ŭ�� �巡�� - ī�޶� �̵�
			{
				camXpos += float(x - mousePosition[0]) / (info.windowWidth / 2);
				camYpos += float(y - mousePosition[1]) / (info.windowHeight / 2);
			}

			mousePosition = vmath::vec2(float(x), float(y));
		}
	}

	//���콺 �ٷ� ī�޶� Ȯ�� ���
#define MAX_FOV 120.f
#define MIN_FOV 10.f
	virtual void onMouseWheel(int pos)
	{
		int deltaPos = pos - wheelPos;
		if (deltaPos > 0)
			fov = vmath::min(MAX_FOV, fov + 1.0f);
		else
			fov = vmath::max(MIN_FOV, fov - 1.0f);

		wheelPos = pos;
	}

	//�� �ʱ�ȭ
	void initValues()
	{
		drawHouse = true;   //�� �Ѱ� ����
		drawLight = true;   // ����Ʈ ������Ʈ
		onSpotLight = false;  // ������ 
		makeObj = false;  // ������Ʈ ���� ����
		nowObj = 0;     //������ �� ����
		pause = false;
		animationTime = 0;
		previousTime = 0;
		filterMode = false;  // ���� ����

		mouseDown = false;  // ���콺 Ŭ��
		wheelPos = 0;  // ���콺 ��
		fov = 50.f;  // ī�޶� �þ߰�

		nowModel = 0;  // genModel �ε���

		genPosition = vmath::vec3(1.0f, -1.0f, 0.0f);  // �ʱ� ��ġ

		objYangle = 0.0f;  // ������ ��ü y�� ȸ����
		camXangle = 0.0f;  // ī�޶� ȸ�� ��
		camYangle = 0.0f; 
		camXpos = 0.0f;   // ī�޶� ��ġ
		camYpos = 0.0f;
		objXpos = 0.0f;   // wdsa�� ������ ������Ʈ �̵���ġ
		objZpos = 0.0f;

		mouseBTN = 0;  // Ŭ���� ���콺 ��ư ����
	}


private:
	GLuint shader_program;


	Model objModel[3], pyramidModel, buildingModel[4], groundModel;
	Model *genModel = new Model[10];  // �������� ���� �𵨵�
	int nowModel;
	int nowObj;

	vmath::vec3 *objPosition = new vmath::vec3[10];// �������� ���� �𵨵� ��ġ
	vmath::vec3 genPosition;

	float *objYangles = new float[10];// �������� ���� �𵨵� ȸ��

	float objYangle, camXangle, camYangle, camXpos, camYpos;
	float objXpos, objZpos;

	bool drawHouse, drawLight, onSpotLight, makeObj;
	bool filterMode;
	bool pause;
	double previousTime;
	double animationTime;

	vmath::vec2 mousePosition;
	bool mouseDown;
	int wheelPos, mouseBTN;

	float fov;
	GLuint shader_program_screen;  // post processing
	GLuint VAO_screen;    // post processing
	GLuint VBO_screen;    // post processing
	GLuint FBO;
	GLuint FBO_texture;
	GLuint RBO;
	
};

// DECLARE_MAIN�� �ϳ����� �ν��Ͻ�
DECLARE_MAIN(my_application)