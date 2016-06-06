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

/* Block types (stored as int in m_blocks)
 * 0 = empty (air)
 * 1 = dirt
 * 2 = grass
 * ....
*/

namespace Cubex {
	class Chunk {
		public:
			Chunk();
			~Chunk();
			
			int& operator()(const int x, const int y, const int z);
			
			//rebuild()
			
			void CreateMesh(int chunkID, bool optimize);
			void CreateGreedyMesh();
			void CreateCube(int x, int y, int z, bool Xneg, bool Xpos, bool Yneg, bool Ypos, bool Zneg, bool Zpos);
			
			void addFace(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, int n, bool backFace);
			void DrawChunk(glm::vec3 color, GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
			
			int GetVertexCount() const;
			
			//int& getBlockSide(const int x, const int y, const int z);
						
			static const int CHUNK_SIZE = 16;
			//static const int BLOCK_SIZE = 1;
			
			int* m_blocks;
			//int* m_blocks_side;
			
			glm::vec3 getBoundingBoxMin() const;
			glm::vec3 getBoundingBoxMax() const;
			
			glm::vec3 m_boundingBoxMin;
			glm::vec3 m_boundingBoxMax;
			
			glm::vec3 getAABBCubeMin(const int x, const int y, const int z);
			glm::vec3 getAABBCubeMax(const int x, const int y, const int z);
									
		private:
			void initBuffers();
			
			int lastVertex = 0;
			
			GLuint vao, vbo[4], ebo;
			std::vector<glm::vec3> vertices;
			std::vector<glm::ivec3> indices;
			std::vector<glm::vec3> colors;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec2> texcoords;
	};
}
