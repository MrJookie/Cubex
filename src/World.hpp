#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>

namespace Cubex {
	class World {
		
		struct Chunk {
			int chunkID;
			int startX, startY, startZ;
			
			GLuint vao, vbo[4], ebo;
			std::vector<glm::vec3> vertices;
			std::vector<glm::ivec3> indices;
			std::vector<glm::vec3> colors;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec2> texcoords;
		};
		
		public:
			World();
			World(const int worldSize, const int chunkSize);
			~World();
			
			void CreateChunk(const int StartX, const int StartY, const int StartZ);
			
			void BuildWorld();
			void DrawWorld(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
			
			int& GetBlock(const int x, const int y, const int z);
			
			std::vector<Chunk> chunks;
			void drawChunk(glm::vec3 color, Chunk& chunk, GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
						
		private:
			void allocateWorld();
			
			void initChunk(Chunk& chunk);
			
			
			int m_worldSize;
			int m_chunkSize;
			
			int* m_blocks;
			
			
	};
}
