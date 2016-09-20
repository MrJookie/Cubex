#include "World.hpp"

namespace Cubex {
    World::World()
    {
		/*
		Chunk chunk;
		chunk(0, 0, 0) = 1;
		
		chunks.push_back(chunk);
		
		chunks[0].CreateMesh();
		*/
	}
	
    World::~World()
    {
		for(int i = 0; i < chunks.size(); ++i) {
			//delete chunks[i];
		}
	}
}

