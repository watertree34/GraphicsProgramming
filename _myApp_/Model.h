#ifndef MODEL_H
#define MODEL_H

#include <stdio.h>
#include <string>
#include <cstring>
#include <vector>

#include <sb6.h>
#include <vmath.h>

class Model{
public:
	std::vector<vmath::vec3> vPositions;
	std::vector<vmath::vec2> vTexCoords;
	std::vector<vmath::vec3> vNormals;
	std::vector<GLuint> vIndices;
	std::vector<vmath::vec3> vTangent;
	std::vector<vmath::vec3> vBitangent;
	


	GLuint diffuseMap, specularMap, normalMap;
	float shininess;
	vmath::vec3 defaultAmbient, defaultDiffuse, defaultSpecular;

private:
	GLuint VAO, VAO_Screen;
	GLuint VBO_positions, VBO_texCoords, VBO_normals, VBO_Tangent, VBO_Bitangent, VBO_Screen;
	GLuint EBO;


	bool useDiffuseMap, useSpecularMap, useNormalMap;


public:
	// 생성자
	Model() {
		shininess = 32.f;
		useDiffuseMap = false;
		useSpecularMap = false;
		useNormalMap = false;

		defaultAmbient = vmath::vec3(1.0f, 1.0f, 1.0f);
		defaultDiffuse = vmath::vec3(1.0f, 1.0f, 1.0f);
		defaultSpecular = vmath::vec3(0.0f, 0.0f, 0.0f);
	}
	// 소멸자
	~Model() {
		glDeleteTextures(1, &diffuseMap);
		glDeleteTextures(1, &specularMap);
		glDeleteTextures(1, &normalMap);
		glDeleteBuffers(1, &EBO);
		glDeleteBuffers(1, &VBO_positions);
		glDeleteBuffers(1, &VBO_texCoords);
		glDeleteBuffers(1, &VBO_normals);
		glDeleteBuffers(1, &VBO_Tangent);
		glDeleteBuffers(1, &VBO_Bitangent);
		glDeleteBuffers(1, &VBO_Screen);

		glDeleteVertexArrays(1, &VAO);
		glDeleteVertexArrays(1, &VAO_Screen);
	}

	
	void init() {
		glGenVertexArrays(1, &VAO);
		glGenVertexArrays(1, &VAO_Screen);
		glGenBuffers(1, &VBO_positions);
		glGenBuffers(1, &VBO_texCoords);
		glGenBuffers(1, &VBO_normals);
		glGenBuffers(1, &VBO_Tangent);
		glGenBuffers(1, &VBO_Bitangent);
		glGenBuffers(1, &VBO_Screen);

		glGenBuffers(1, &EBO);

		glGenTextures(1, &diffuseMap);
		glGenTextures(1, &specularMap);
		glGenTextures(1, &normalMap);
	}
	// 메쉬 설정-위치, 텍스쳐, tangent, bitangent, 노멀 값
	void setupMesh(int _numVertices, GLfloat *_vPositions, GLfloat *_vTexCoords = NULL, GLfloat* _vNormals = NULL) {
		// 1. copy data from arrays
		for (int i = 0; i < _numVertices; i++) {  //버텍스 position값
			vmath::vec3 position;
			position[0] = _vPositions[i * 3 + 0];
			position[1] = _vPositions[i * 3 + 1];
			position[2] = _vPositions[i * 3 + 2];
			this->vPositions.push_back(position);
		}

		if (_vTexCoords) {   // 버텍스 텍스쳐coords값
			for (int i = 0; i < _numVertices; i++) {
				vmath::vec2 texCoords;
				texCoords[0] = _vTexCoords[i * 2 + 0];
				texCoords[1] = _vTexCoords[i * 2 + 1];
				this->vTexCoords.push_back(texCoords);
			}
			for (int i = 0; i < _numVertices; i += 3)  // 노멀맵 적용을 위한 tangent, bitangent구하기
			{
				vmath::vec3 tangent;
				vmath::vec3 bitangent;
				vmath::vec3 v0;  // 각 버텍스 위치 저장
				v0[0] = _vPositions[i * 3 + 0];
				v0[1] = _vPositions[i * 3 + 1];
				v0[2] = _vPositions[i * 3 + 2];

				vmath::vec3 v1;
				v1[0] = _vPositions[(i + 1) * 3 + 0];
				v1[1] = _vPositions[(i + 1) * 3 + 1];
				v1[2] = _vPositions[(i + 1) * 3 + 2];

				vmath::vec3 v2;
				v2[0] = _vPositions[(i + 2) * 3 + 0];
				v2[1] = _vPositions[(i + 2) * 3 + 1];
				v2[2] = _vPositions[(i + 2) * 3 + 2];


				vmath::vec2 uv0;
				vmath::vec2 uv1;
				vmath::vec2 uv2;
				//각 버텍스 텍스쳐위치 저장
				uv0[0] = _vTexCoords[i * 2 + 0];
				uv0[1] = _vTexCoords[i * 2 + 1];

				uv1[0] = _vTexCoords[(i + 1) * 2 + 0];
				uv1[1] = _vTexCoords[(i + 1) * 2 + 1];

				uv2[0] = _vTexCoords[(i + 2) * 2 + 0];
				uv2[1] = _vTexCoords[(i + 2) * 2 + 1];

				//tangent,bitangent 구하기 공식
				vmath::vec3 edge0 = v1 - v0;
				vmath::vec3 edge1 = v2 - v0;
				vmath::vec2 deltaUV1 = uv1 - uv0;
				vmath::vec2 deltaUV2 = uv2 - uv0;

				float f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);
				tangent = f * (edge0 * deltaUV2[1] - edge1 * deltaUV1[1]);

				bitangent = f*(edge1 * deltaUV1[0] - edge0 * deltaUV2[0]);

				this->vTangent.push_back(tangent);
				this->vTangent.push_back(tangent);
				this->vTangent.push_back(tangent);
				this->vBitangent.push_back(bitangent);
				this->vBitangent.push_back(bitangent);
				this->vBitangent.push_back(bitangent);

			}
		}
		
		if (_vNormals) {		//버텍스 normal값
			for (int i = 0; i < _numVertices; i++) {
				vmath::vec3 normal;
				normal[0] = _vNormals[i * 3 + 0];
				normal[1] = _vNormals[i * 3 + 1];
				normal[2] = _vNormals[i * 3 + 2];
				this->vNormals.push_back(normal);
			}
		}



		// 2. create buffers
		prepareBuffers();

	}

	//EBO설정 함수
	void setupIndices(int _numIndices, GLuint *_indices) {
		for (int i = 0; i < _numIndices; i++) {
			vIndices.push_back(_indices[i]);
		}

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _numIndices * sizeof(int), &vIndices[0], GL_STATIC_DRAW);
		glBindVertexArray(0);
	}

	//diffuse맵 로드 확인 함수
	bool loadDiffuseMap(const char *_filepath) {
		if (loadTextureFile(diffuseMap, _filepath)) {
			useDiffuseMap = true;
			return true;
		}

		useDiffuseMap = false;
		return false;
	}
	//specular맵 로드 확인 함수
	bool loadSpecularMap(const char *_filepath) {
		if (loadTextureFile(specularMap, _filepath)) {
			useSpecularMap = true;
			return true;
		}

		useSpecularMap = false;
		return false;
	}
	//normal맵 로드 확인 함수
	bool loadNormalMap(const char *_filepath) {
		if (loadTextureFile(normalMap, _filepath)) {
			useNormalMap = true;
			return true;
		}

		useNormalMap = false;
		return false;
	}
	


	//draw함수-uniform으로 값들을 쉐이더로 넘겨줌
	void draw(GLuint _shaderID) {
		glUniform3fv(glGetUniformLocation(_shaderID, "material.defaultAmbient"), 1, defaultAmbient);
		glUniform3fv(glGetUniformLocation(_shaderID, "material.defaultDiffuse"), 1, defaultDiffuse);
		glUniform3fv(glGetUniformLocation(_shaderID, "material.defaultSpecular"), 1, defaultSpecular);
		glUniform1i(glGetUniformLocation(_shaderID, "material.useDiffuseMap"), (int)useDiffuseMap);
		glUniform1i(glGetUniformLocation(_shaderID, "material.useSpecularMap"), (int)useSpecularMap);
		glUniform1i(glGetUniformLocation(_shaderID, "useNormalMap"), (int)useNormalMap);

		glUniform1i(glGetUniformLocation(_shaderID, "useNormal"), (int)vNormals.size());
		glUniform1f(glGetUniformLocation(_shaderID, "material.shininess"), shininess);

		if (useDiffuseMap) {
			glUniform1i(glGetUniformLocation(_shaderID, "material.diffuse"), 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuseMap);
		}

		if (useSpecularMap) {
			glUniform1i(glGetUniformLocation(_shaderID, "material.specular"), 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, specularMap);
		}

		if (useNormalMap) {
			glUniform1i(glGetUniformLocation(_shaderID, "normalMap"), 2);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, normalMap);
		}


		glBindVertexArray(VAO);
		if (vIndices.empty())
			glDrawArrays(GL_TRIANGLES, 0, vPositions.size());
		else
			glDrawElements(GL_TRIANGLES, vIndices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}



	//obj파일 로드 함수
	bool loadOBJ(const char *filepath) {
		FILE *file = fopen(filepath, "r");
		if (file == NULL)
			return false;

		bool readVT = false, readVN = false;

		std::vector<vmath::vec3> tmpPositions;
		std::vector<vmath::vec2> tmpTexCoords;
		std::vector<vmath::vec3> tmpNormals;
		std::vector<GLuint> tmpVIndices, tmpVtIndices, tmpVnIndices;

		while (true) {
			char lineBuffer[512];

			// 처음 한 단어 읽어오기
			int res = fscanf(file, "%s", lineBuffer);
			// 파일 끝이면 읽기 종료
			if (res == EOF)
				break;

			// 버텍스 3차원 포지션 읽어오기
			if (strcmp(lineBuffer, "v") == 0) {
				vmath::vec3 v;
				fscanf(file, "%f %f %f\n", &v[0], &v[1], &v[2]);
				tmpPositions.push_back(v);
			}
			// 버텍스 텍스처 좌표 읽어오기
			else if (strcmp(lineBuffer, "vt") == 0) {
				vmath::vec2 vt;
				fscanf(file, "%f %f\n", &vt[0], &vt[1]);
				tmpTexCoords.push_back(vt);
				readVT = true;
			}
			// 버텍스 노말 읽어오기
			else if (strcmp(lineBuffer, "vn") == 0) {
				vmath::vec3 vn;
				fscanf(file, "%f %f %f\n", &vn[0], &vn[1], &vn[2]);
				tmpNormals.push_back(vn);
				readVN = true;
			}
			// 페이스 인덱스들 읽어오기
			else if (strcmp(lineBuffer, "f") == 0) {
				GLuint vIndex[3], vtIndex[3], vnIndex[3];

				// 포지션만 있는 경우
				if (!readVT && !readVN) {
					fscanf(file, "%d %d %d\n", &vIndex[0], &vIndex[1], &vIndex[2]);
				}
				// 포지션/텍스쳐좌표 있는 경우
				else if (readVT && !readVN) {
					fscanf(file, "%d/%d %d/%d %d/%d\n", &vIndex[0], &vtIndex[0], &vIndex[1], &vtIndex[1], &vIndex[2], &vtIndex[2]);
				}
				// 포지션/노멀 있는 경우
				else if (!readVT && readVN) {
					fscanf(file, "%d//%d %d//%d %d//%d\n", &vIndex[0], &vnIndex[0], &vIndex[1], &vnIndex[1], &vIndex[2], &vnIndex[2]);
				}
				// 포지션/텍스쳐좌표/노멀 있는 경우
				else if (readVT && readVN) {
					fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vIndex[0], &vtIndex[0], &vnIndex[0], &vIndex[1], &vtIndex[1], &vnIndex[1], &vIndex[2], &vtIndex[2], &vnIndex[2]);
				}

				for (int i = 0; i < 3; i++) {
					tmpVIndices.push_back(vIndex[i]);
					if (readVT)
						tmpVtIndices.push_back(vtIndex[i]);
					if (readVN)
						tmpVnIndices.push_back(vnIndex[i]);
				}
			}
			else {
				char trashBuffer[1000];
				fgets(trashBuffer, 1000, file);
			}
		}

		vPositions.clear();
		vTexCoords.clear();
		vNormals.clear();
		vIndices.clear();
		// 인덱스를 사용하지는 않고 그냥 중첩해서 정보를 저장 (OpenGL EBO는 단일 인덱스밖에 지원하지 않으므로...)
		for (int i = 0; i < tmpVIndices.size(); i++) {
			vmath::vec3 position = tmpPositions[tmpVIndices[i] - 1];
			vPositions.push_back(position);
		}
		for (int i = 0; i < tmpVtIndices.size(); i++) {
			vmath::vec2 texCoords = tmpTexCoords[tmpVtIndices[i] - 1];
			vTexCoords.push_back(texCoords);
		}
		for (int i = 0; i < tmpVnIndices.size(); i++) {
			vmath::vec3 normal = tmpPositions[tmpVnIndices[i] - 1];
			vNormals.push_back(normal);
		}

		//tangent, bitangent 넘겨주기
		for (int i = 0; i < tmpVtIndices.size(); i += 3)
		{
			vmath::vec3 tangent;
			vmath::vec3 bitangent;

			vmath::vec3 v0 = tmpPositions[tmpVIndices[i] - 1];
			vmath::vec3 v1 = tmpPositions[tmpVIndices[i + 1] - 1];
			vmath::vec3 v2 = tmpPositions[tmpVIndices[i + 2] - 1];

			vmath::vec2 uv0 = tmpTexCoords[tmpVtIndices[i] - 1];
			vmath::vec2 uv1 = tmpTexCoords[tmpVtIndices[i + 1] - 1];
			vmath::vec2 uv2 = tmpTexCoords[tmpVtIndices[i + 2] - 1];

			vmath::vec3 edge0 = v1 - v0;
			vmath::vec3 edge1 = v2 - v0;
			vmath::vec2 deltaUV1 = uv1 - uv0;
			vmath::vec2 deltaUV2 = uv2 - uv0;

			float f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);
			tangent = f * (edge0 * deltaUV2[1] - edge1 * deltaUV1[1]);
			bitangent = f*(edge1 * deltaUV1[0] - edge0 * deltaUV2[0]);

			vTangent.push_back(tangent);
			vTangent.push_back(tangent);
			vTangent.push_back(tangent);
			vBitangent.push_back(bitangent);
			vBitangent.push_back(bitangent);
			vBitangent.push_back(bitangent);
		}



		prepareBuffers();

		return true;
	}

private:
	void prepareBuffers() {
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
		glBufferData(GL_ARRAY_BUFFER, vPositions.size()*sizeof(vmath::vec3), &vPositions[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		if (!vTexCoords.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, VBO_texCoords);
			glBufferData(GL_ARRAY_BUFFER, vTexCoords.size()*sizeof(vmath::vec2), &vTexCoords[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		}

		if (!vNormals.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
			glBufferData(GL_ARRAY_BUFFER, vNormals.size()*sizeof(vmath::vec3), &vNormals[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		}
		// 노멀맵 Tangent(높이/밑면=기울기)=접선 공간-텍스쳐 좌표인 UV 좌표와 비교하여 U좌표와 일치하는 Vector를  Tangent, V 좌표와 일치하는 Vector를 BiTangent Vector
		if (!vTangent.empty()){
			glBindBuffer(GL_ARRAY_BUFFER, VBO_Tangent);
			glBufferData(GL_ARRAY_BUFFER, vTangent.size()*sizeof(vmath::vec3), &vTangent[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		}
		// 노멀맵 Bitangent-접선을 외적한 바이노멀 벡터를 축으로 하는 공간
		if (!vBitangent.empty()){
			glBindBuffer(GL_ARRAY_BUFFER, VBO_Bitangent);
			glBufferData(GL_ARRAY_BUFFER, vBitangent.size()*sizeof(vmath::vec3), &vBitangent[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	//vao, vbo bind하는 준비함수
	void prepareFrame(){
		glGenVertexArrays(1, &VAO_Screen);
		glGenBuffers(1, &VBO_Screen);
		glBindVertexArray(VAO_Screen);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_Screen);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//텍스쳐 파일 로드 함수
	bool loadTextureFile(GLuint textureID, const char* filepath) {
		int width, height, nrChannels;
		unsigned char *data = stbi_load(filepath, &width, &height, &nrChannels, 0);

		if (data) {
			GLenum format;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 3) // R, G, B (ex. jpg)
				format = GL_RGB;
			else if (nrChannels == 4) // PNG
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);

			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
			return true;
		}

		stbi_image_free(data);
		return false;
	}

};

#endif 