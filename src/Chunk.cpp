#include "Chunk.hpp"

namespace Cubex {
    Chunk::Chunk()
    {
		m_blocks = new int[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
		if(!m_blocks) {
			throw std::string("failed to allocate world size");
		}
		
		std::memset(m_blocks, 0, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(int));
		
		/*
		m_blocks_side = new int[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
		if(!m_blocks_side) {
			throw std::string("failed to allocate world size");
		}
		
		std::memset(m_blocks_side, 0, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(int));
		*/
		
		//m_boundingBoxMin = glm::vec3(0, 0, 0);
		//m_boundingBoxMax = glm::vec3(16, 0, 16);
	}
	
    Chunk::~Chunk()
    {		
		if(m_blocks != nullptr) {
			delete [] m_blocks;
		}
		
		glDeleteBuffers(4, vbo);
		glDeleteBuffers(1, &ebo);
		glDeleteVertexArrays(1, &vao);
	}
	
	int& Chunk::operator()(const int x, const int y, const int z)
	{
		//int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
		int index = x + CHUNK_SIZE * y + CHUNK_SIZE * CHUNK_SIZE * z;
		if(index > CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) {
			throw std::string("out of index");
		}
		
		return m_blocks[index];
	}
	
	void Chunk::CreateMesh(int chunkID, bool optimize)
	{
		vertices.clear();
		indices.clear();
		colors.clear();
		normals.clear();
		texcoords.clear();
		
		int chunkPos = chunkID * CHUNK_SIZE;
		
		using milli = std::chrono::milliseconds;
		auto start = std::chrono::high_resolution_clock::now();
		
		bool displayInnerFaces = optimize;
		bool displayChunkFaces = true;
		
		for (int x = 0; x < CHUNK_SIZE; x++) {
			for (int y = 0; y < CHUNK_SIZE; y++) {
				for (int z = 0; z < CHUNK_SIZE; z++) {
					if((*this)(x, y, z) == 0) {
						continue;
					}
					
					bool Xneg = true;
					bool Xpos = true;
					bool Yneg = true;
					bool Ypos = true;
					bool Zneg = true;
					bool Zpos = true;
					
					if(!displayInnerFaces) {
						Xneg = displayChunkFaces;
						if(x > 0)
							Xneg = ((*this)(x-1, y, z) > 0) ? false : true;

						Xpos = displayChunkFaces;
						if(x < CHUNK_SIZE - 1)
							Xpos = ((*this)(x+1, y, z) > 0) ? false : true;
							
						Yneg = displayChunkFaces;
						if(y > 0)
							Yneg = ((*this)(x, y-1, z) > 0) ? false : true;
							
						Ypos = displayChunkFaces;
						if(y < CHUNK_SIZE - 1)
							Ypos = ((*this)(x, y+1, z) > 0) ? false : true;

						Zneg = displayChunkFaces;
						if(z > 0)
							Zneg = ((*this)(x, y, z-1) > 0) ? false : true;

						Zpos = displayChunkFaces;
						if(z < CHUNK_SIZE - 1)
							Zpos = ((*this)(x, y, z+1) > 0) ? false : true;
					}
					
					this->CreateCube(x, y, z + chunkPos, Xneg, Xpos, Yneg, Ypos, Zneg, Zpos);
				}
			}
		}
		
		auto finish = std::chrono::high_resolution_clock::now();
		std::cout << "CreateMesh() took "
				  << std::chrono::duration_cast<milli>(finish - start).count()
				  << " milliseconds\n";
		
		if(lastVertex > 0) {
			this->initBuffers();
			lastVertex = 0;
		}
	}
	
	/* Greedy meshing base on: https://0fps.net/2012/06/30/meshing-in-a-minecraft-game/
	 * https://github.com/mikolalysenko/mikolalysenko.github.com/blob/gh-pages/MinecraftMeshes/js/greedy.js
	 * https://github.com/roboleary/GreedyMesh/blob/master/src/mygame/Main.java
	 * https://www.giawa.com/journal-entry-6-greedy-mesh-optimization/
	 */
	void Chunk::CreateGreedyMesh()
	{
		using milli = std::chrono::milliseconds;
		auto start = std::chrono::high_resolution_clock::now();
		
		int Xneg = 0;
		int Xpos = 1;
		int Yneg = 2;
		int Ypos = 3;
		int Zneg = 4;
		int Zpos = 5;
		
		int i, j, k, l, w, h, u, v, n, side = 0;
            
		int x[3] = {0,0,0};
		int q[3] = {0,0,0};
		int mask[CHUNK_SIZE * CHUNK_SIZE];
    
		for (bool backFace = true, b = false; b != backFace; backFace = backFace && b, b = !b) { 
			for (int d = 0; d < 3; d++)
			{
				u = (d + 1) % 3;
				v = (d + 2) % 3;
				
				x[0] = 0;
				x[1] = 0;
				x[2] = 0;

				q[0] = 0;
				q[1] = 0;
				q[2] = 0;
				q[d] = 1;
			
				if (d == 0)      { side = backFace ? Ypos : Yneg; }
				else if (d == 1) { side = backFace ? Zpos : Zneg; }
				else if (d == 2) { side = backFace ? Xpos : Xneg; }   
	 
				for (x[d] = -1; x[d] < CHUNK_SIZE; )
				{
					// Compute the mask
					n = 0;
					for (x[v] = 0; x[v] < CHUNK_SIZE; ++x[v])
					{
						for (x[u] = 0; x[u] < CHUNK_SIZE; ++x[u])
						{
							int voxelFace = (x[d] >= 0 ) ? (*this)(x[0], x[1], x[2]) : 0;
							int voxelFace1 = (x[d] < CHUNK_SIZE - 1) ? (*this)(x[0] + q[0], x[1] + q[1], x[2] + q[2]) : 0;
							
							/*
							if(voxelFace != 0) {
								getBlockSide(x[0], x[1], x[2]) = side;
							}
							if(voxelFace1 != 0) {
								getBlockSide(x[0] + q[0], x[1] + q[1], x[2] + q[2]) = side;
							}
							*/
													
							int voxelFaceSide = side;
							int voxelFaceSide1 = side;
							
							
							mask[n++] = ((voxelFace != 0 && voxelFace1 != 0 && voxelFace == voxelFace1)) 
											? 0 
											: backFace ? voxelFace1 : voxelFace;
							
							/*
							mask[n++] = (0 <= x[d] ? ((*this)(x[0], x[1], x[2]) > 0) : false) 
										!= (x[d] < CHUNK_SIZE - 1 ? ((*this)(x[0] + q[0], x[1] + q[1], x[2] + q[2]) > 0) : false);
							*/
						}
					}
	 
					// Increment x[d]
					++x[d];
	 
					// Generate mesh for mask using lexicographic ordering
					n = 0;
					for (j = 0; j < CHUNK_SIZE; ++j)
					{
						for (i = 0; i < CHUNK_SIZE; )
						{
							if (mask[n] != 0)
							{
								// Compute width
								//for (w = 1; i + w < CHUNK_SIZE && mask[n + w]; ++w) ;
								for (w = 1; i + w < CHUNK_SIZE && mask[n + w] != 0 && mask[n + w] == mask[n]; ++w) ;
	 
								// Compute height (this is slightly awkward
								bool done = false;
								for (h = 1; j + h < CHUNK_SIZE; ++h)
								{
									for (k = 0; k < w; ++k)
									{
										//if (!mask[n + k + h * CHUNK_SIZE])
										if(mask[n + k + h * CHUNK_SIZE] == 0 || mask[n + k + h * CHUNK_SIZE] != mask[n])
										{
											done = true;
											break;
										}
									}
									if (done) break;
								}
								
								if(mask[n] > 0) {
									// Add quad
									x[u] = i; x[v] = j;
									int du[3] = {0,0,0};
									int dv[3] = {0,0,0};
									du[u] = w;
									dv[v] = h;
									
									this->addFace(glm::vec3(x[0], x[1], x[2]),
												  glm::vec3(x[0] + du[0], x[1] + du[1], x[2] + du[2]),
												  glm::vec3(x[0] + du[0] + dv[0], x[1] + du[1] + dv[1], x[2] + du[2] + dv[2]),
												  glm::vec3(x[0] + dv[0], x[1] + dv[1], x[2] + dv[2]),
												  mask[n],
												  backFace);
								}
								
								// Zero-out mask
								for (l = 0; l < h; ++l)
								{
									for (k = 0; k < w; ++k)
									{
										mask[n + k + l * CHUNK_SIZE] = 0;
									}
								}
	 
								// Increment counters and continue
								i += w; n += w;
							}
							else
							{
								++i; ++n;
							}
						}
					}
				}
			}
		}
		
		auto finish = std::chrono::high_resolution_clock::now();
		std::cout << "CreateGreedyMesh() took "
				  << std::chrono::duration_cast<milli>(finish - start).count()
				  << " milliseconds\n";
        
		this->initBuffers();
	}
	
	void Chunk::addFace(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, int n, bool backFace)
	{
		if(backFace) {
			indices.push_back(glm::vec3(lastVertex+2,lastVertex+0,lastVertex+1));
			indices.push_back(glm::vec3(lastVertex+1,lastVertex+3,lastVertex+2));
		} else {
			indices.push_back(glm::vec3(lastVertex+2,lastVertex+3,lastVertex+1));
		    indices.push_back(glm::vec3(lastVertex+1,lastVertex+0,lastVertex+2));
		}
		
		if(n == 1) {
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
		} else {
			colors.push_back(glm::vec3(0.0, 0.0, 1.0));
			colors.push_back(glm::vec3(0.0, 0.0, 1.0));
			colors.push_back(glm::vec3(0.0, 0.0, 1.0));
			colors.push_back(glm::vec3(0.0, 0.0, 1.0));
		}
		
		vertices.push_back(a);
		vertices.push_back(d);
		vertices.push_back(b);
		vertices.push_back(c);
		
		glm::vec3 normal = glm::cross((c-d), (a-d));
		if(backFace) {
			normals.push_back(normal);
			normals.push_back(normal);
			normals.push_back(normal);
			normals.push_back(normal);
		} else {
			normals.push_back(-normal);
			normals.push_back(-normal);
			normals.push_back(-normal);
			normals.push_back(-normal);
		}
		
		/*
		normals.push_back(glm::vec3(0.0, 0.0, 1.0));
		normals.push_back(glm::vec3(0.0, 0.0, 1.0));
		normals.push_back(glm::vec3(0.0, 0.0, 1.0));
		normals.push_back(glm::vec3(0.0, 0.0, 1.0));
		
		colors.push_back(glm::vec3(1.0, 0.0, 0.0));
		colors.push_back(glm::vec3(1.0, 0.0, 0.0));
		colors.push_back(glm::vec3(1.0, 0.0, 0.0));
		colors.push_back(glm::vec3(1.0, 0.0, 0.0));
		
		
		
		indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 1, lastVertex + 2));
		indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 2, lastVertex + 3));
		*/
		
		lastVertex += 4;
	}
	
	/*
	int& Chunk::getBlockSide(const int x, const int y, const int z)
	{
		int index = x + CHUNK_SIZE * y + CHUNK_SIZE * CHUNK_SIZE * z;
		if(index > CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) {
			throw std::string("out of index");
		}
		
		return m_blocks_side[index];
	}
	*/
	
	void Chunk::CreateCube(int x, int y, int z, bool Xneg, bool Xpos, bool Yneg, bool Ypos, bool Zneg, bool Zpos)
	{
		//x = x * 2 * BLOCK_SIZE;
		//y = y * 2 * BLOCK_SIZE;
		//z = z * 2 * BLOCK_SIZE;
		
		/*
		glm::vec3 v1(x-BLOCK_SIZE, y-BLOCK_SIZE, z+BLOCK_SIZE);
		glm::vec3 v2(x+BLOCK_SIZE, y-BLOCK_SIZE, z+BLOCK_SIZE);
		glm::vec3 v3(x+BLOCK_SIZE, y+BLOCK_SIZE, z+BLOCK_SIZE);
		glm::vec3 v4(x-BLOCK_SIZE, y+BLOCK_SIZE, z+BLOCK_SIZE);
		
		glm::vec3 v5(x+BLOCK_SIZE, y-BLOCK_SIZE, z-BLOCK_SIZE);
		glm::vec3 v6(x-BLOCK_SIZE, y-BLOCK_SIZE, z-BLOCK_SIZE);
		glm::vec3 v7(x-BLOCK_SIZE, y+BLOCK_SIZE, z-BLOCK_SIZE);
		glm::vec3 v8(x+BLOCK_SIZE, y+BLOCK_SIZE, z-BLOCK_SIZE);
		*/

		// Front z+1
		if(Zpos) {
			normals.push_back(glm::vec3(0.0, 0.0, 1.0));
			normals.push_back(glm::vec3(0.0, 0.0, 1.0));
			normals.push_back(glm::vec3(0.0, 0.0, 1.0));
			normals.push_back(glm::vec3(0.0, 0.0, 1.0));
			
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			
			/*
			vertices.push_back(v1);
			vertices.push_back(v2);
			vertices.push_back(v3);
			vertices.push_back(v4);
			*/
			
			vertices.push_back(glm::vec3(x + 0, y + 0, z + 1));
			vertices.push_back(glm::vec3(x + 1, y + 0, z + 1));
			vertices.push_back(glm::vec3(x + 1, y + 1, z + 1));
			vertices.push_back(glm::vec3(x + 0, y + 1, z + 1));
			
			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 1, lastVertex + 2));
			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 2, lastVertex + 3));
			
			lastVertex += 4;
		}

		// Back z-1
		if(Zneg) {
			normals.push_back(glm::vec3(0.0, 0.0, -1.0));
			normals.push_back(glm::vec3(0.0, 0.0, -1.0));
			normals.push_back(glm::vec3(0.0, 0.0, -1.0));
			normals.push_back(glm::vec3(0.0, 0.0, -1.0));
			
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			
			/*
			vertices.push_back(v5);
			vertices.push_back(v6);
			vertices.push_back(v7);
			vertices.push_back(v8);
			*/
			
			vertices.push_back(glm::vec3(x + 0, y + 1, z + 0));
			vertices.push_back(glm::vec3(x + 1, y + 1, z + 0));
			vertices.push_back(glm::vec3(x + 1, y + 0, z + 0));
			vertices.push_back(glm::vec3(x + 0, y + 0, z + 0));
			
			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 1, lastVertex + 2));
			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 2, lastVertex + 3));
			
			lastVertex += 4;
		}
		
		// Right x+1
		if(Xpos) {
			normals.push_back(glm::vec3(1.0, 0.0, 0.0));
			normals.push_back(glm::vec3(1.0, 0.0, 0.0));
			normals.push_back(glm::vec3(1.0, 0.0, 0.0));
			normals.push_back(glm::vec3(1.0, 0.0, 0.0));
			
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			
			/*
			vertices.push_back(v2);
			vertices.push_back(v5);
			vertices.push_back(v8);
			vertices.push_back(v3);
			*/
			
			vertices.push_back(glm::vec3(x + 1, y + 0, z + 0));
			vertices.push_back(glm::vec3(x + 1, y + 1, z + 0));
			vertices.push_back(glm::vec3(x + 1, y + 1, z + 1));
			vertices.push_back(glm::vec3(x + 1, y + 0, z + 1));
			
			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 1, lastVertex + 2));
			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 2, lastVertex + 3));
			
			lastVertex += 4;
		}
		
		// left x-1
		if(Xneg) {
			normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
			normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
			normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
			normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
			
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));

			/*
			vertices.push_back(v6);
			vertices.push_back(v1);
			vertices.push_back(v4);
			vertices.push_back(v7);
			*/
			
			vertices.push_back(glm::vec3(x + 0, y + 0, z + 1));
			vertices.push_back(glm::vec3(x + 0, y + 1, z + 1));
			vertices.push_back(glm::vec3(x + 0, y + 1, z + 0));
			vertices.push_back(glm::vec3(x + 0, y + 0, z + 0));

			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 1, lastVertex + 2));
			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 2, lastVertex + 3));
			
			lastVertex += 4;
		}
		
		// Top y+1
		if(Ypos) {
			normals.push_back(glm::vec3(0.0, 1.0, 0.0));
			normals.push_back(glm::vec3(0.0, 1.0, 0.0));
			normals.push_back(glm::vec3(0.0, 1.0, 0.0));
			normals.push_back(glm::vec3(0.0, 1.0, 0.0));
			
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			
			/*
			vertices.push_back(v4);
			vertices.push_back(v3);
			vertices.push_back(v8);
			vertices.push_back(v7);
			*/
			
			vertices.push_back(glm::vec3(x + 0, y + 1, z + 1));
			vertices.push_back(glm::vec3(x + 1, y + 1, z + 1));
			vertices.push_back(glm::vec3(x + 1, y + 1, z + 0));
			vertices.push_back(glm::vec3(x + 0, y + 1, z + 0));
			
			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 1, lastVertex + 2));
			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 2, lastVertex + 3));
			
			lastVertex += 4;
		}
		
		// Bottom y-1
		if(Yneg) {
			normals.push_back(glm::vec3(0.0, -1.0, 0.0));
			normals.push_back(glm::vec3(0.0, -1.0, 0.0));
			normals.push_back(glm::vec3(0.0, -1.0, 0.0));
			normals.push_back(glm::vec3(0.0, -1.0, 0.0));
			
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			colors.push_back(glm::vec3(1.0, 0.0, 0.0));
			
			/*
			vertices.push_back(v6);
			vertices.push_back(v5);
			vertices.push_back(v2);
			vertices.push_back(v1);
			*/
			
			vertices.push_back(glm::vec3(x + 0, y + 0, z + 0));
			vertices.push_back(glm::vec3(x + 1, y + 0, z + 0));
			vertices.push_back(glm::vec3(x + 1, y + 0, z + 1));
			vertices.push_back(glm::vec3(x + 0, y + 0, z + 1));

			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 1, lastVertex + 2));
			indices.push_back(glm::vec3(lastVertex + 0, lastVertex + 2, lastVertex + 3));
			
			lastVertex += 4;
		}
	}
	
	void Chunk::initBuffers()
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbo);
		glGenBuffers(1, &ebo);
		glBindVertexArray(0);
		
		glBindVertexArray(vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);    
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * colors.size(), &colors[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);    
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), &normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);    
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * texcoords.size(), &texcoords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);    
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3) * indices.size(), &indices[0], GL_STATIC_DRAW);
		
		glBindVertexArray(0);
	}
	
	void Chunk::DrawChunk(glm::vec3 color, GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection)
	{
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
			
		glUseProgram(shader);
		glBindVertexArray(vao);
		
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		
		glm::vec3 lightPos(0.0f, 200.0f, 0.0f);
		
		glUniform3f(glGetUniformLocation(shader, "lightColor"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(shader, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		
		glUniform3f(glGetUniformLocation(shader, "customColor"), color.r, color.g, color.b);
		
		glDrawElements(GL_TRIANGLES, indices.size() * 3, GL_UNSIGNED_INT, 0);
		
		glBindVertexArray(0);
		glUseProgram(0);
		
		glDisable(GL_DEPTH_TEST);
		//glDisable(GL_CULL_FACE);
	}
	
	glm::vec3 Chunk::getBoundingBoxMin() const
	{
		return m_boundingBoxMin;
	}

	glm::vec3 Chunk::getBoundingBoxMax() const
	{
		return m_boundingBoxMax;
	}
	
	int Chunk::GetVertexCount() const
	{
		return vertices.size();
		//or return lastVertex;
	}
	
	glm::vec3 Chunk::getAABBCubeMin(const int x, const int y, const int z)
	{
		if((*this)(x, y, z) > 0) {
			return glm::vec3(x, y, z);
		}
		
		return glm::vec3(0);
	}
	
	glm::vec3 Chunk::getAABBCubeMax(const int x, const int y, const int z)
	{
		if((*this)(x, y, z) > 0) {
			return glm::vec3(x + 1, y + 1, z + 1);
		}
		
		return glm::vec3(0);
	}
}

