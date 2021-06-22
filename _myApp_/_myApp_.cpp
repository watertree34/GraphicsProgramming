// sb6.h 헤더 파일을 포함시킨다.
#include <sb6.h>
#include <vmath.h>
#include <shader.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Model.h"

// sb6::application을 상속받는다.
class my_application : public sb6::application
{
public:
	// 쉐이더 컴파일한다.
	// basic multiple lighting shader
	GLuint compile_shader(void)
	{
		// 버텍스 쉐이더를 생성하고 컴파일한다.
		GLuint vertex_shader = sb6::shader::load("../../src/_myApp_/multiple_lights_vs.glsl", GL_VERTEX_SHADER);

		// 프래그먼트 쉐이더를 생성하고 컴파일한다.
		GLuint fragment_shader = sb6::shader::load("../../src/_myApp_/multiple_lights_fs.glsl", GL_FRAGMENT_SHADER);

		// 프로그램을 생성하고 쉐이더를 Attach시키고 링크한다.
		GLuint program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

		// 이제 프로그램이 쉐이더를 소유하므로 쉐이더를 삭제한다.
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return program;
	}
	//postProcessing 프로그램
	GLuint postProcessing(void)
	{
		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint program;

		// 버텍스 쉐이더 소스 코드
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

		// 프래그먼트 쉐이더 소스 코드
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
			"	float kernel[9] = float[](					\n"   // 필터 
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

		// 버텍스 쉐이더를 생성하고 컴파일한다.
		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		// 프래그먼트 쉐이더를 생성하고 컴파일한다.
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		// 프로그램을 생성하고 쉐이더를 Attach시키고 링크한다.
		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

		// 이제 프로그램이 쉐이더를 소유하므로 쉐이더를 삭제한다.
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return program;
	}
	// 애플리케이션 초기화 수행한다.
	virtual void startup()
	{
		// 클래스 내부 변수 초기화
		initValues();

		// 쉐이더 프로그램 컴파일 및 연결
		shader_program = compile_shader();  //오브젝트 프로그램
		shader_program_screen = postProcessing();  // post processing 프로그램
		// VAO, VBO, EBO, texture 생성
		glGenVertexArrays(1, &VAO_screen);
		glGenBuffers(1, &VBO_screen);

		stbi_set_flip_vertically_on_load(true);

		// 첫 번째 객체 정의 :생성할 OBJ 모델들  --------------------------------------------------
		objModel[0].init();
		objModel[0].loadOBJ("../../src/_myApp_/tree.obj");   // 나무
		objModel[0].loadDiffuseMap("../../src/_myApp_/Vegetation_Bark_Maple_1_baseColor.jpeg");

		objModel[1].init();                                 // 미끄럼틀
		objModel[1].loadOBJ("../../src/_myApp_/slide.obj");
		objModel[1].loadDiffuseMap("../../src/_myApp_/slide.png");

		objModel[2].init();                                 /// 벤치의자
		objModel[2].loadOBJ("../../src/_myApp_/bench.obj");
		objModel[2].loadDiffuseMap("../../src/_myApp_/benchColor.jpeg");
		objModel[2].loadNormalMap("../../src/_myApp_/bench_normal.png");

		// 두 번째 객체 정의 : 피라미드 --------------------------------------------------
		// 피라미드 점들의 위치와 컬러, 텍스처 좌표를 정의한다.
		GLfloat pyramid_vertices[] = {
			1.0f, 0.0f, -1.0f,    // 우측 상단
			-1.0f, 0.0f, -1.0f,   // 좌측 상단
			-1.0f, 0.0f, 1.0f,    // 좌측 하단
			1.0f, 0.0f, 1.0f,     // 우측 하단
			0.0f, 1.0f, 0.0f,      // 상단 꼭지점
			0.0f, -1.0f, 0.0f,      // 하단 꼭지점
		};

		// 삼각형으로 그릴 인덱스를 정의한다.
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

		// 세번째 객체정의-집 베이스----------------------------
		buildingModel[0].init();
		buildingModel[0].loadOBJ("../../src/_myApp_/base.obj");
		buildingModel[0].loadDiffuseMap("../../src/_myApp_/base.png");
		buildingModel[0].loadNormalMap("../../src/_myApp_/base_normal.png");


		// 세번째 객체정의-집 지붕
		buildingModel[1].init();
		buildingModel[1].loadOBJ("../../src/_myApp_/roof.obj");
		buildingModel[1].loadDiffuseMap("../../src/_myApp_/roof2.jpg");
		buildingModel[1].loadNormalMap("../../src/_myApp_/roof2_normal.png");

		// 세번째 객체정의-집 벽
		buildingModel[2].init();
		buildingModel[2].loadOBJ("../../src/_myApp_/wall.obj");
		buildingModel[2].loadDiffuseMap("../../src/_myApp_/wall.png");

		// 세번째 객체정의-집 나무
		buildingModel[3].init();
		buildingModel[3].loadOBJ("../../src/_myApp_/wood.obj");
		buildingModel[3].loadDiffuseMap("../../src/_myApp_/wood.jpeg");
		buildingModel[3].loadNormalMap("../../src/_myApp_/wood_normal.png");

		///네번째 객체정의----바닥---------------------------------
		
		//위치
		GLfloat floor_vertices[] = {
			1.0f, 0.0f, -1.0f,   // 우측 상단
			-1.0f, 0.0f, -1.0f,   // 좌측 상단
			-1.0f, 0.0f, 1.0f,    // 좌측 하단
			1.0f, 0.0f, 1.0f   // 우측 하단
		};
		//텍스쳐
		GLfloat floor_texVertices[] = {
			10.0f, 10.0f,  // 우측 상단
			0.0f, 10.0f,  // 좌측 상단
			0.0f, 0.0f,   // 좌측 하단
			10.0f, 0.0f   // 우측 하단
		};
		//노멀
		GLfloat floor_normVertices[] = {
			0.0f, 1.0f, 0.0f, // 우측 상단
			0.0f, 1.0f, 0.0f,  // 좌측 상단
			0.0f, 1.0f, 0.0f,   // 좌측 하단
			0.0f, 1.0f, 0.0f   // 우측 하단
		};

		// 삼각형으로 그릴 인덱스를 정의
		GLuint floor_indices[] = {
			0, 1, 2,	// 첫번째 삼각형
			0, 2, 3		// 두번째 삼각형
		};
		groundModel.init();
		groundModel.setupMesh(4, floor_vertices, floor_texVertices, floor_normVertices);
		groundModel.setupIndices(6, floor_indices);
		groundModel.loadDiffuseMap("../../src/_myApp_/grass.jpg");
		


		glEnable(GL_MULTISAMPLE);


		/////////////////Post-processing/////////////
		glBindVertexArray(VAO_screen);

		// Post-processing용 스크린 정의
		GLfloat screenVertices[] = {
			//NDC xy좌표 //텍스처 UV
			-1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f
		};
		// VBO를 생성하여 vertices 값들을 복사
		glBindBuffer(GL_ARRAY_BUFFER, VBO_screen);
		glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), screenVertices, GL_STATIC_DRAW);

		// VBO를 나누어서 각 버텍스 속성으로 연결
		//NDC xy좌표 속성 (location = 0)
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// 텍스처 UV속성 (location = 1)
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// VBO 및 버텍스 속성을 다 했으니 VBO와 VAO를 unbind한다.
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		//////////////FBO/////////////////////////////////


		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		// color buffer 텍스처 생성 및 연결

		glGenTextures(1, &FBO_texture);
		glBindTexture(GL_TEXTURE_2D, FBO_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, info.windowWidth, info.windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBO_texture, 0);


		// depth&stencil buffer를 위한 Render Buffer Object 생성 및 연결

		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, info.windowWidth, info.windowHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			glfwTerminate();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// 애플리케이션 끝날 때 호출된다.
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

	// 렌더링 virtual 함수를 작성해서 오버라이딩한다.
	virtual void render(double currentTime)
	{
		if (filterMode){  // 필터적용
			////////////////// FBO 바인딩
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);
			// FBO에 연결된 버퍼들의 값을 지우고, 뎁스 테스팅 활성화
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			// Do 1st Rendering
			// 지금부터 렌더링한 결과물들은 우리가 만든 프레임버퍼에 저장
		}

		if (pause) {  // 일시정지
			previousTime = currentTime;
			return;
		}

		animationTime += currentTime - previousTime;
		previousTime = currentTime;

		const GLfloat color[] = { 0.0f, (float)sin(animationTime / 2) * 0.5f + 0.5f, (float)sin(animationTime / 2) * 0.5f + 0.5f, 1.0f };  // 낮과 밤 배경 표현

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		glClearBufferfv(GL_COLOR, 0, color);
		// 카메라 매트릭스 계산


		float distance = 5.f;

		vmath::vec3 eye((float)sin(camXangle)*distance + camXpos, (float)sin(camYangle)*distance + camYpos, (float)cos(camYangle + camXangle)*distance);     // 마우스 좌클릭 드래그-카메라 회전, 휠 클릭-이동
		vmath::vec3 center(camXpos, camYpos, 0.0);  //휠 클릭-이동
		vmath::vec3 up(0.0, 1.0, 0.0);
		vmath::mat4 lookAt = vmath::lookat(eye, center, up);
		vmath::mat4 projM = vmath::perspective(fov, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);

		// 라이팅 설정 ---------------------------------------
		float r = 5.0f;
		vmath::vec3 pointLightPos[] = { vmath::vec3(-1.0f, -0.2f, 0.2f),  // 집 안 불빛
			vmath::vec3(0.0f, (float)sin(animationTime*0.5f)*r, (float)cos(animationTime*0.5f)*r) };   // 해 불빛
		vmath::vec3 lightColor(1.0f, 1.0f, 1.0f);
		vmath::vec3 viewPos = eye;

		// 모델 그리기 ---------------------------------------
		glUseProgram(shader_program);

		//uniform으로 조명, 카메라 값을 넘겨줌
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

		if (onSpotLight)  // 손전등 켜고 끄기
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

		//t누르면 새 오브젝트 만들기
		if (makeObj && nowModel <= 10)
		{
			objPosition[nowModel - 1] = vmath::vec3(objXpos, -1.0f, objZpos);  // wasd로 이동
			genModel[nowModel - 1] = objModel[nowObj];
			objYangles[nowModel - 1] = objYangle;  // 우클릭 물체 회전
		}

		for (int i = 0; i < nowModel; i++)
		{
			model = vmath::translate(objPosition[i]) *
				vmath::rotate(objYangles[i], 0.0f, 1.0f, 0.0f) *
				vmath::scale(0.01f);
			glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, model);
			genModel[i].draw(shader_program);
		}

		// 집 그리기
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

		// 조명 피라미드 그리기 ---------------------------------------
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

		// 바닥 그리기
		model = vmath::translate(vmath::vec3(-1, -1, -0.5)) *
			vmath::rotate(0.0f, 0.0f, 1.0f, 0.0f) *
			vmath::scale(10.0f);
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, model);
		groundModel.draw(shader_program);


		if (filterMode){
			///////////////////////프레임버퍼 텍스쳐적용/////////////////////////////
			// 기본 Framebuffer로 되돌리기
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			// 버퍼들의 값 지우기
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			//// FBO Texture를 스크린 쉐이더 프로그램에 연결   //// POST PROCESSING
			glUseProgram(shader_program_screen);
			glUniform1i(glGetUniformLocation(shader_program_screen, "screenTexture"), 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, FBO_texture);

			glBindVertexArray(VAO_screen);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

	}

	//화면 키웠을때 크기 조절
	virtual void onResize(int w, int h)
	{
		sb6::application::onResize(w, h);

		if (glViewport != NULL)
			glViewport(0, 0, info.windowWidth, info.windowHeight);
	}
	//안티 앨리어싱
	virtual void init()
	{
		sb6::application::init();

		info.samples = 8;
		info.flags.debug = 1;
	}

	//키보드 이벤트
	virtual void onKey(int key, int action)
	{
		if (action == GLFW_PRESS) {
			switch (key) {
			case ' ': // 스페이스바 - 일시정지
				pause = !pause;
				break;

			case 'H':  // 집 그리기
				drawHouse = !drawHouse;
				break;
			case 'I':  // 조명 오브젝트 그리기
				drawLight = !drawLight;
				break;
			case 'M':   // 포스트 프로세싱 모드
				filterMode = !filterMode;
				break;
			case 'R':   // 초기화
				initValues();
				break;
			case 'L':   // 손전등 켜고 끄기
				onSpotLight = !onSpotLight;
				break;
			default:
				break;
			case 'A':     //  생성되는 오브젝트 위치 옮기기
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
			case'T':   // 새 오브젝트 생성
				makeObj = !makeObj;
				if (makeObj)
				{
					nowModel++;
					objXpos = 0.0f;
					objZpos = 0.0f;
					objYangle = 0.0f;
				}
				break;

			case '1':  // 오브젝트 종류 변경 1-나무 2-미끄럼틀 3-벤치
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

	//마우스 이벤트
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
			if (mouseBTN == 0)  // 좌클릭 드래그-카메라 회전
			{
				camXangle += float(x - mousePosition[0]) / (info.windowWidth / 2);
				camYangle += float(y - mousePosition[1]) / info.windowHeight;
				
			}
			else if (mouseBTN == 1)  // 우클릭 드래그-오브젝트 생성상태일 때 오브젝트 회전
			{ 
				objYangle += float(x - mousePosition[0]) / (info.windowWidth /50);
			}
			else if (mouseBTN == 2)  // 휠 클릭 드래그 - 카메라 이동
			{
				camXpos += float(x - mousePosition[0]) / (info.windowWidth / 2);
				camYpos += float(y - mousePosition[1]) / (info.windowHeight / 2);
			}

			mousePosition = vmath::vec2(float(x), float(y));
		}
	}

	//마우스 휠로 카메라 확대 축소
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

	//값 초기화
	void initValues()
	{
		drawHouse = true;   //집 켜고 끄기
		drawLight = true;   // 라이트 오브젝트
		onSpotLight = false;  // 손전등 
		makeObj = false;  // 오브젝트 생성 상태
		nowObj = 0;     //생성될 모델 종류
		pause = false;
		animationTime = 0;
		previousTime = 0;
		filterMode = false;  // 필터 적용

		mouseDown = false;  // 마우스 클릭
		wheelPos = 0;  // 마우스 휠
		fov = 50.f;  // 카메라 시야각

		nowModel = 0;  // genModel 인덱스

		genPosition = vmath::vec3(1.0f, -1.0f, 0.0f);  // 초기 위치

		objYangle = 0.0f;  // 생성될 물체 y축 회전값
		camXangle = 0.0f;  // 카메라 회전 각
		camYangle = 0.0f; 
		camXpos = 0.0f;   // 카메라 위치
		camYpos = 0.0f;
		objXpos = 0.0f;   // wdsa로 생성될 오브젝트 이동위치
		objZpos = 0.0f;

		mouseBTN = 0;  // 클릭될 마우스 버튼 종류
	}


private:
	GLuint shader_program;


	Model objModel[3], pyramidModel, buildingModel[4], groundModel;
	Model *genModel = new Model[10];  // 동적으로 만들 모델들
	int nowModel;
	int nowObj;

	vmath::vec3 *objPosition = new vmath::vec3[10];// 동적으로 만들 모델들 위치
	vmath::vec3 genPosition;

	float *objYangles = new float[10];// 동적으로 만들 모델들 회전

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

// DECLARE_MAIN의 하나뿐인 인스턴스
DECLARE_MAIN(my_application)