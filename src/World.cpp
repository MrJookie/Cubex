#include "World.hpp"

namespace Cubex {
    World::World()
    {
		m_worldSize = 8;
		m_chunkSize = 8;
		
		this->allocateWorld();
	}
	
	World::World(const int worldSize, const int chunkSize)
    {
		m_worldSize = worldSize;
		m_chunkSize = chunkSize;
		
		this->allocateWorld();
	}
	
    World::~World()
    {
		if(m_blocks != nullptr) {
			delete [] m_blocks;
		}
		
		for(int i = 0; i < chunks.size(); ++i) {
			glDeleteBuffers(4, chunks[i].vbo);
			glDeleteBuffers(1, &chunks[i].ebo);
			glDeleteVertexArrays(1, &chunks[i].vao);
		}
	}
	
	void World::allocateWorld()
	{
		m_blocks = new int[m_worldSize * m_worldSize * m_worldSize];
		if(!m_blocks) {
			throw std::string("failed to allocate world size");
		}
		
		std::memset(m_blocks, 0, m_worldSize * m_worldSize * m_worldSize * sizeof(int));
	}
    
    void World::CreateChunk(const int StartX, const int StartY, const int StartZ)
	{
		Chunk tempChunk;
		tempChunk.chunkID = chunks.size();
		tempChunk.startX = StartX;
		tempChunk.startY = StartY;
		tempChunk.startZ = StartZ;
		
		glGenVertexArrays(1, &tempChunk.vao);
		glBindVertexArray(tempChunk.vao);
		
		glGenBuffers(4, tempChunk.vbo);
		glGenBuffers(1, &tempChunk.ebo);
		
		glBindVertexArray(0);
		
		//add delete buffers on chunk unload
		
		bool optimizeAdjacent = true;
		int iVertex = 0;
		int Block;
		int Block1;
		
		int DefaultBlock = 1;
		int SX = 0;
		int SY = 0;
		int SZ = 0;
		int MaxSize = m_worldSize;
	 
		for (int z = StartZ; z < m_chunkSize + StartZ; ++z)
		{
			for (int y = StartY; y < 256; ++y)
			{
				for (int x = StartX; x < m_chunkSize + StartX; ++x)
				{
					Block = GetBlock(x,y,z);
					if (Block == 0) continue;
					
						//x-1
					if(optimizeAdjacent) {
						Block1 = DefaultBlock;
						if (x > SX) Block1 = GetBlock(x-1,y,z);
					} else {
						Block1 = 0;
						if (x > StartX) Block1 = GetBlock(x-1,y,z);
					}
	 
					if (Block1 == 0)
					{
						//add faces to face_vbo1;
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 0, z + 1));
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 1, z + 1));
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 1, z + 0));
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 0, z + 0));
						
						tempChunk.indices.push_back(glm::vec3(iVertex + 0, iVertex + 1, iVertex + 2));
						tempChunk.indices.push_back(glm::vec3(iVertex + 2, iVertex + 3, iVertex + 0));
						
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						
						tempChunk.normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
						
						tempChunk.texcoords.push_back(glm::vec2(0.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 0.0));
						tempChunk.texcoords.push_back(glm::vec2(0.0, 0.0));
	 
						iVertex += 4;
					}
	 
						//x+1
					if(optimizeAdjacent) {
						Block1 = DefaultBlock;
						if (x < SX + MaxSize - 1) Block1 = GetBlock(x+1,y,z);
					} else {
						Block1 = 0;
						if (x < StartX + m_chunkSize - 1) Block1 = GetBlock(x+1,y,z);
					}
	 
					if (Block1 == 0)
					{
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 0, z + 0));
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 1, z + 0));
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 1, z + 1));
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 0, z + 1));
						
						tempChunk.indices.push_back(glm::vec3(iVertex + 0, iVertex + 1, iVertex + 2));
						tempChunk.indices.push_back(glm::vec3(iVertex + 2, iVertex + 3, iVertex + 0));
						
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						
						tempChunk.normals.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(1.0, 0.0, 0.0));
						
						tempChunk.texcoords.push_back(glm::vec2(0.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 0.0));
						tempChunk.texcoords.push_back(glm::vec2(0.0, 0.0));
	 
						iVertex += 4;
					}
	 
						//y-1
					if(optimizeAdjacent) {
						Block1 = DefaultBlock;
						if (y > SY) Block1 = GetBlock(x,y-1,z);
					} else {
						Block1 = 0;
						if (y > StartY) Block1 = GetBlock(x,y-1,z);
					}
	 
					if (Block1 == 0)
					{
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 0, z + 0));
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 0, z + 0));
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 0, z + 1));
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 0, z + 1));
						
						tempChunk.indices.push_back(glm::vec3(iVertex + 0, iVertex + 1, iVertex + 2));
						tempChunk.indices.push_back(glm::vec3(iVertex + 2, iVertex + 3, iVertex + 0));
						
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						
						tempChunk.normals.push_back(glm::vec3(0.0, -1.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(0.0, -1.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(0.0, -1.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(0.0, -1.0, 0.0));
						
						tempChunk.texcoords.push_back(glm::vec2(0.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 0.0));
						tempChunk.texcoords.push_back(glm::vec2(0.0, 0.0));
	 
						iVertex += 4;
					}
	 
	 
						//y+1
					if(optimizeAdjacent) {
						Block1 = DefaultBlock;
						if (y < SY + MaxSize - 1) Block1 = GetBlock(x,y+1,z);
					} else {
						Block1 = 0;
						if (y < StartY + m_chunkSize - 1) Block1 = GetBlock(x,y+1,z);
					}
	 
					if (Block1 == 0)
					{
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 1, z + 1));
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 1, z + 1));
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 1, z + 0));
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 1, z + 0));
						
						tempChunk.indices.push_back(glm::vec3(iVertex + 0, iVertex + 1, iVertex + 2));
						tempChunk.indices.push_back(glm::vec3(iVertex + 2, iVertex + 3, iVertex + 0));
						
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						
						tempChunk.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
						tempChunk.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
						
						tempChunk.texcoords.push_back(glm::vec2(0.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 0.0));
						tempChunk.texcoords.push_back(glm::vec2(0.0, 0.0));
	 
						iVertex += 4;
					}
	 
						//z-1
					if(optimizeAdjacent) {
						Block1 = DefaultBlock;
						if (z > SZ) Block1 = GetBlock(x,y,z-1);
					} else {
						Block1 = 0;
						if (z > StartZ) Block1 = GetBlock(x,y,z-1);
					}
	 
					if (Block1 == 0)
					{
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 1, z + 0));
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 1, z + 0));
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 0, z + 0));
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 0, z + 0));
						
						tempChunk.indices.push_back(glm::vec3(iVertex + 0, iVertex + 1, iVertex + 2));
						tempChunk.indices.push_back(glm::vec3(iVertex + 2, iVertex + 3, iVertex + 0));
						
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						
						tempChunk.normals.push_back(glm::vec3(0.0, 0.0, -1.0));
						tempChunk.normals.push_back(glm::vec3(0.0, 0.0, -1.0));
						tempChunk.normals.push_back(glm::vec3(0.0, 0.0, -1.0));
						tempChunk.normals.push_back(glm::vec3(0.0, 0.0, -1.0));
						
						tempChunk.texcoords.push_back(glm::vec2(0.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 0.0));
						tempChunk.texcoords.push_back(glm::vec2(0.0, 0.0));
	 
						iVertex += 4;
					}
					
					
						//z+1
					if(optimizeAdjacent) {
						Block1 = DefaultBlock;
						if (z < SZ + MaxSize - 1) Block1 = GetBlock(x,y,z+1);
					} else {
						Block1 = 0;
						if (z < StartZ + m_chunkSize - 1) Block1 = GetBlock(x,y,z+1);
					}
	 
					if (Block1 == 0)
					{
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 0, z + 1));
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 0, z + 1));
						tempChunk.vertices.push_back(glm::vec3(x + 1, y + 1, z + 1));
						tempChunk.vertices.push_back(glm::vec3(x + 0, y + 1, z + 1));
						
						tempChunk.indices.push_back(glm::vec3(iVertex + 0, iVertex + 1, iVertex + 2));
						tempChunk.indices.push_back(glm::vec3(iVertex + 2, iVertex + 3, iVertex + 0));
						
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						tempChunk.colors.push_back(glm::vec3(1.0, 0.0, 0.0));
						
						tempChunk.normals.push_back(glm::vec3(0.0, 0.0, 1.0));
						tempChunk.normals.push_back(glm::vec3(0.0, 0.0, 1.0));
						tempChunk.normals.push_back(glm::vec3(0.0, 0.0, 1.0));
						tempChunk.normals.push_back(glm::vec3(0.0, 0.0, 1.0));
						
						tempChunk.texcoords.push_back(glm::vec2(0.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 1.0));
						tempChunk.texcoords.push_back(glm::vec2(1.0, 0.0));
						tempChunk.texcoords.push_back(glm::vec2(0.0, 0.0));
	 
						iVertex += 4;
					}
				}
			}
		}
		
		chunks.push_back(tempChunk);
		
		this->initChunk(chunks[chunks.size()-1]);
	}
	
	void World::BuildWorld()
	{
		for (int z = 0; z < m_worldSize; z += m_chunkSize)
		{
			//for (int y = 0; y < m_worldSize; y += m_chunkSize)
			//{
				for (int x = 0; x < m_worldSize; x += m_chunkSize)
				{
					this->CreateChunk(x,0,z);
				}
			//}
		}
		
		int numVerts = 0;
		int numIndices = 0;
		
		for(int i = 0; i < chunks.size(); ++i) {
			numVerts += chunks[i].vertices.size();
			numIndices += chunks[i].indices.size();
		}
		
		std::cout << "verts: " << numVerts << std::endl;
		std::cout << "tris: " << numIndices << std::endl;
	}
	
	int& World::GetBlock(const int x, const int y, const int z) 
	{
		int index = x + y * m_worldSize + z * m_worldSize * m_worldSize;
		if(index > m_worldSize * m_worldSize * m_worldSize) {
			throw std::string("out of index");
		}
		
		return m_blocks[index];
	}
	
	void World::DrawWorld(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection)
	{
		for(int i = 0; i < chunks.size(); ++i) {
			this->drawChunk(glm::vec3(1.0, 1.0, 1.0), chunks[i], shader, model, view, projection);
		}
	}
	
	void World::initChunk(Chunk& chunk)
	{
		glBindVertexArray(chunk.vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * chunk.vertices.size(), &chunk.vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);    
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * chunk.colors.size(), &chunk.colors[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);    
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * chunk.normals.size(), &chunk.normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);    
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * chunk.texcoords.size(), &chunk.texcoords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);    
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3) * chunk.indices.size(), &chunk.indices[0], GL_STATIC_DRAW);
		
		glBindVertexArray(0);
	}
	
	void World::drawChunk(glm::vec3 color, Chunk& chunk, GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection)
	{	
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
			
		glUseProgram(shader);
		glBindVertexArray(chunk.vao);
		
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		
		glm::vec3 lightPos(0.0f, 200.0f, 0.0f);
		
		glUniform3f(glGetUniformLocation(shader, "lightColor"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(shader, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		
		glUniform3f(glGetUniformLocation(shader, "customColor"), color.r, color.g, color.b);
		
		
		/*
		if(chunk.chunkID == 0) {
			glUniform3f(glGetUniformLocation(shader, "customColor"), 0.0f, 0.0f, 1.0f);
		} else {
			glUniform3f(glGetUniformLocation(shader, "customColor"), 0.0f, 1.0f, 0.0f);
		}
		*/
		
		/*
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		
		SDL_Surface *textureSurface = IMG_Load("block.png");
		if(!textureSurface) {
			std::cout << "couldnt load texture" << std::endl;
			return;
		}
		
		GLint colorMode;
		if(textureSurface->format->BytesPerPixel == 4) {
			if(textureSurface->format->Rmask == 0x000000ff) {
				colorMode = GL_RGBA;
			}
			else {
				colorMode = GL_BGRA;
			}
		}
		else if(textureSurface->format->BytesPerPixel == 3) {
			if(textureSurface->format->Rmask == 0x000000ff) {
				colorMode = GL_RGB;
			}
			else {
				colorMode = GL_BGR;
			}
		}
		else {
			 throw std::string("Image is not truecolor!");
		}
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSurface->w, textureSurface->h, 0, colorMode, GL_UNSIGNED_BYTE, textureSurface->pixels);
		
		glUniform1i(glGetUniformLocation(shaders[0]->GetShader(), "mytexture"), 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glDeleteTextures(1, &tex);
		
		SDL_FreeSurface(textureSurface);
		*/
		
		glDrawElements(GL_TRIANGLES, chunk.indices.size() * 3, GL_UNSIGNED_INT, 0);
		
		glBindVertexArray(0);
		glUseProgram(0);
		
		glDisable(GL_DEPTH_TEST);
		//glDisable(GL_CULL_FACE);
	}
}

