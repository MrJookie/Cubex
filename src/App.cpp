#include "App.hpp"

App::App()
{
    m_sizeX = 800;
    m_sizeY = 600;
    
    m_ticks_previous = SDL_GetTicks();
    m_ticks_current = 0;
    m_frames_current = 0;
    m_frames_elapsed = 0;
    
    m_delta_time = 0;
    m_ticks_then = 0;

    m_chrono_start = std::chrono::high_resolution_clock::now();
    m_chrono_elapsed = 0;
    
    this->init();
}

App::~App() {}

void App::init()
{
	SDL_Window* window;
    SDL_GLContext glContext;
 
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        throw std::string("Failed to initialize SDL: ") + SDL_GetError();
    }

    window = SDL_CreateWindow("Cubex", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_sizeX, m_sizeY, SDL_WINDOW_OPENGL);
    if(window == nullptr) {
        throw std::string("Failed to create window: ") + SDL_GetError();
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    
    glContext = SDL_GL_CreateContext(window);
    if(glContext == nullptr) {
        throw std::string("Failed to create GLContext: ") + SDL_GetError();
    }

    SDL_GL_SetSwapInterval(0);
    SDL_GL_MakeCurrent(window, glContext);
    
    glewExperimental = GL_TRUE; 
    glewInit();
    
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    printf("GLSL:  %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
    //SDL_SetRelativeMouseMode(SDL_TRUE);
    //SDL_SetWindowGrab(window, SDL_TRUE);
    
    cx::Shader shader1("Assets/shader1.vs", "Assets/shader1.fs");
    shaders.push_back(&shader1);
    
    cx::Shader shader2("Assets/frustum.vs", "Assets/frustum.fs");
    shaders.push_back(&shader2);
    
    cx::Camera camera(glm::vec3(10.0f, 10.0f, 50.0f));
    
    //int WORLD_SIZE = 16;
    int CHUNK_SIZE = 32;
    
    cx::World world;
    
    //std::vector<cx::Chunk*> chunks;
    
    int chunksOnX = 5;
    int chunksOnY = 5;
    int chunksOnZ = 8;
	cx::Chunk* chunks[chunksOnX][chunksOnY][chunksOnZ];

    module::Perlin noise;
    noise.SetOctaveCount(2);
    noise.SetFrequency(0.2);
    noise.SetPersistence(4);
    noise.SetLacunarity(0.2);
    noise.SetSeed(0);
    
    int verts = 0;
	
	auto start = std::chrono::high_resolution_clock::now();
	//int it = 0;
	for(int chunkX = 0; chunkX < chunksOnX; chunkX++) {
		for(int chunkY = 0; chunkY < chunksOnY; chunkY++) {
			for(int chunkZ = 0; chunkZ < chunksOnZ; chunkZ++) {
				cx::Chunk* chunk = new cx::Chunk;
				
				for(int x = 0; x < CHUNK_SIZE; x++) {
					for(int y = 0; y < CHUNK_SIZE; y++) {
						for(int z = 0; z < CHUNK_SIZE; z++) {
							double height = noise.GetValue(x + chunkX * CHUNK_SIZE, z + chunkY * CHUNK_SIZE, y + chunkZ * CHUNK_SIZE);
							
							if(height > 0) {
								(*chunk)(x, y, z) = 1;
							} else {
								(*chunk)(x, y, z) = 0;
							}
						}
					}
				}
				
				chunk->m_pos = glm::vec3(chunkX * chunk->CHUNK_SIZE, chunkZ * chunk->CHUNK_SIZE, chunkY * chunk->CHUNK_SIZE);
				
				chunk->m_boundingBoxMin = glm::vec3(0, 0, 0);
				chunk->m_boundingBoxMax = glm::vec3(chunk->CHUNK_SIZE, chunk->CHUNK_SIZE, chunk->CHUNK_SIZE);
				
				//std::cout << chunk->m_boundingBoxMin.x << " - " << chunk->m_boundingBoxMin.y << " - " << chunk->m_boundingBoxMin.z << std::endl;
				//std::cout << chunk->m_boundingBoxMax.x << " - " << chunk->m_boundingBoxMax.y << " - " << chunk->m_boundingBoxMax.z << std::endl;
				//if(it++ == 2) return;
				
				chunk->CreateGreedyMesh();
				//chunk->CreateMesh(false);
				std::cout << "verts: " << chunk->GetVertexCount() << std::endl;

				verts += chunk->GetVertexCount();
				
				chunks[chunkX][chunkY][chunkZ] = chunk;
			}
		}
	}
	
	std::cout << "total verts: " << verts << std::endl;
	
	auto finish = std::chrono::high_resolution_clock::now();
	std::cout << "Generating chunks() took "
			  << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count()
			  << " milliseconds\n";
	
	std::vector<glm::vec3> selectedCube;
	std::vector<cx::Chunk*> selectedChunk;
	
	int selectedFace;
	
    bool toggleMouseRelative = true;
    bool toggleFullscreen = true;
    bool toggleWireframe = true;
    
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
    SDL_SetWindowGrab(window, SDL_TRUE);
    
    if(toggleMouseRelative) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }

    int skipMouseResolution = 0;
    
    bool running = true;

    while(running)
    {
		this->loop();
		
        SDL_Event e;

        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT) {
                running = false;
            }
            else if(e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym)
                {
					case SDLK_f:
                    {       
                        if(toggleFullscreen) {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                            
                            int w, h;
                            SDL_GetWindowSize(window, &w, &h);
                            m_sizeX = w;
                            m_sizeY = h;
                            
                            toggleFullscreen = false;
                        }
                        else {
                            SDL_SetWindowFullscreen(window, 0);
                            //SDL_SetWindowDisplayMode(window, 0);

                            int w, h;
                            SDL_GetWindowSize(window, &w, &h); //?
                            
                            m_sizeX = 800;
                            m_sizeY = 600;
                            
                            toggleFullscreen = true;
                        }
                        
                        skipMouseResolution = 2;     
                    }
                    break;
                    
					case SDLK_ESCAPE:
						running = false;
					break;
					
					case SDLK_e:
                    {
                        if(toggleWireframe) {
                            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                            toggleWireframe = false;
                        }
                        else {
                            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                            toggleWireframe = true;
                        }
                    }
                    break;
                }
            } else if(e.type == SDL_MOUSEBUTTONDOWN) {
				if(e.button.button == SDL_BUTTON_LEFT) {
					if(!selectedCube.empty()) {
						int x = 0;
						int y = 0;
						int z = 0;
						
						switch(selectedFace) {
							case 0:
								//-z
								z = -1;
							break;
								
							case 1:
								//+z
								z = 1;
							break;
							
							case 2:
								//-y
								y = -1;
							break;
								
							case 3:
								//+y
								y = 1;
							break;
							
							case 4:
								//-x
								x = -1;
							break;
							
							case 5:
								//+x
								x = 1;
							break;
						}
						
						(*selectedChunk[0])(selectedCube[0].x + x, selectedCube[0].y + y, selectedCube[0].z + z) = 1;
												
						std::cout << "X: " << selectedCube[0].x << 
									 " Y: " << selectedCube[0].y << 
									 " Z: " << selectedCube[0].z << 
									 " face: " << selectedFace << std::endl;
						
						selectedChunk[0]->CreateGreedyMesh();
					}
				}
				
				if(e.button.button == SDL_BUTTON_RIGHT) {
					if(!selectedCube.empty()) {
						(*selectedChunk[0])(selectedCube[0].x, selectedCube[0].y, selectedCube[0].z) = 0;
												
						std::cout << "X: " << selectedCube[0].x << 
									 " Y: " << selectedCube[0].y << 
									 " Z: " << selectedCube[0].z << 
									 " face: " << selectedFace << std::endl;
						
						selectedChunk[0]->CreateGreedyMesh();
					}
				}
            } else if(e.type == SDL_MOUSEBUTTONUP) {
				if(e.button.button == SDL_BUTTON_LEFT) {

				}
				
				if(e.button.button == SDL_BUTTON_RIGHT) {

				}
            }
        }
        
        glViewport(0, 0, m_sizeX, m_sizeY);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        SDL_PumpEvents();
        const Uint8* state = SDL_GetKeyboardState(NULL);
        
        if(state[SDL_SCANCODE_W]) {
            camera.ProcessKeyboard(cx::Camera::MoveDirection::FORWARD, getDeltaTime() * 2.0);
        }
        
        if(state[SDL_SCANCODE_S]) {
            camera.ProcessKeyboard(cx::Camera::MoveDirection::BACKWARD, getDeltaTime() * 2.0);
        }
        
        if(state[SDL_SCANCODE_A]) {
            camera.ProcessKeyboard(cx::Camera::MoveDirection::LEFT, getDeltaTime() * 2.0);
        }

        if(state[SDL_SCANCODE_D]) {
            camera.ProcessKeyboard(cx::Camera::MoveDirection::RIGHT, getDeltaTime() * 2.0);
        }
        
        int xpos;
        int ypos;
        SDL_GetRelativeMouseState(&xpos, &ypos);
        
        if(skipMouseResolution > 0 && (xpos != 0 || ypos != 0)) {
            skipMouseResolution--; 
        }
        else {
            if(toggleMouseRelative) {
				camera.ProcessMouseMovement(xpos, ypos);
            }
            else {
                //int xpos;
                //int ypos;
                SDL_GetMouseState(&xpos, &ypos);
                
                //std::cout << xpos << " " << ypos << std::endl;
            }
        }
        
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()), this->getSizeX()/(float)this->getSizeY(), 0.1f, 1000.0f);

        camera.ExtractFrustumPlanes(view, projection);
        
        selectedCube.clear();
        selectedChunk.clear();
        
        for(int chunkX = 0; chunkX < chunksOnX; ++chunkX) {
			for(int chunkY = 0; chunkY < chunksOnY; ++chunkY) {
				for(int chunkZ = 0; chunkZ < chunksOnZ; ++chunkZ) {
					//draw chunk
					glm::mat4 model = glm::translate(glm::mat4(1.0f), chunks[chunkX][chunkY][chunkZ]->m_pos);
					chunks[chunkX][chunkY][chunkZ]->DrawChunk(glm::vec3(1.0, 0.0, 0.0), shaders[0]->GetShader(), model, view, projection);
					
					//select nearest chunk
					glm::vec3 chunkBoudingBoxMin = chunks[chunkX][chunkY][chunkZ]->m_pos;
					glm::vec3 chunkBoudingBoxMax = chunks[chunkX][chunkY][chunkZ]->m_pos + chunks[chunkX][chunkY][chunkZ]->getBoundingBoxMax();
					
					glm::vec4 viewport = glm::vec4(0.0f, 0.0f, this->getSizeX(), this->getSizeY());
					glm::vec3 v0 = glm::unProject(glm::vec3(this->getSizeX()/2.0f, this->getSizeY()/2.0f, 0.0f), view, projection, viewport);
					glm::vec3 v1 = glm::unProject(glm::vec3(this->getSizeX()/2.0f, this->getSizeY() - this->getSizeY()/2.0f, 1.0f), view, projection, viewport);
					glm::vec3 dir = v1 - v0; 

					if(RayAABBIntersect(v0, dir, chunkBoudingBoxMin, chunkBoudingBoxMax)) {
						if(selectedChunk.size() < 1) {
							selectedChunk.push_back(chunks[chunkX][chunkY][chunkZ]);
						} else {
							if(glm::distance(chunkBoudingBoxMin, camera.GetPosition()) < glm::distance(selectedChunk[0]->m_pos, camera.GetPosition())) {
								selectedChunk[0] = chunks[chunkX][chunkY][chunkZ];
							}
						}
					}
				}
			}
		}
		
		if(!selectedChunk.empty() && selectedChunk[0] != nullptr) {
			glm::vec3 chunkBoudingBoxMin = selectedChunk[0]->m_pos;
			glm::vec3 chunkBoudingBoxMax = selectedChunk[0]->m_pos + selectedChunk[0]->getBoundingBoxMax();
			
			glm::mat4 model = glm::translate(glm::mat4(1.0f), selectedChunk[0]->m_pos);
			
			glm::vec4 viewport = glm::vec4(0.0f, 0.0f, this->getSizeX(), this->getSizeY());
			glm::vec3 v0 = glm::unProject(glm::vec3(this->getSizeX()/2.0f, this->getSizeY()/2.0f, 0.0f), view, projection, viewport);
			glm::vec3 v1 = glm::unProject(glm::vec3(this->getSizeX()/2.0f, this->getSizeY() - this->getSizeY()/2.0f, 1.0f), view, projection, viewport);
			glm::vec3 dir = v1 - v0; 
			
			if(RayAABBIntersect(v0, dir, chunkBoudingBoxMin, chunkBoudingBoxMax)) {
			//if(true) {
				GLubyte indices[] = {0, 1, 1, 5, 5, 4, 4, 0,
									 2, 3, 3, 7, 7, 6, 6, 2,
									 0, 2, 1, 3, 5, 7, 4, 6,
				};
				
				GLfloat colors[] = {1.0, 1.0, 1.0,
									1.0, 1.0, 1.0,
									1.0, 1.0, 1.0,
									1.0, 1.0, 1.0,
									1.0, 1.0, 1.0,
									1.0, 1.0, 1.0,
									1.0, 1.0, 1.0,
									1.0, 1.0, 1.0,
				};
				
				GLfloat colors2[] = {0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,
				};
				
				//AABB of chunk
				
				glm::vec3 BBOXmin = selectedChunk[0]->getBoundingBoxMin();
				glm::vec3 BBOXmax = selectedChunk[0]->getBoundingBoxMax();
				
				glm::vec3 m_boundingBoxVertices[8];
				m_boundingBoxVertices[0] = glm::vec3(BBOXmin.x, BBOXmin.y, BBOXmin.z);
				m_boundingBoxVertices[1] = glm::vec3(BBOXmin.x, BBOXmin.y, BBOXmax.z);
				m_boundingBoxVertices[2] = glm::vec3(BBOXmin.x, BBOXmax.y, BBOXmin.z);
				m_boundingBoxVertices[3] = glm::vec3(BBOXmin.x, BBOXmax.y, BBOXmax.z);
				m_boundingBoxVertices[4] = glm::vec3(BBOXmax.x, BBOXmin.y, BBOXmin.z);
				m_boundingBoxVertices[5] = glm::vec3(BBOXmax.x, BBOXmin.y, BBOXmax.z);
				m_boundingBoxVertices[6] = glm::vec3(BBOXmax.x, BBOXmax.y, BBOXmin.z);
				m_boundingBoxVertices[7] = glm::vec3(BBOXmax.x, BBOXmax.y, BBOXmax.z);
				
				std::vector<glm::vec3> bbox;
				bbox.push_back(m_boundingBoxVertices[0]);
				bbox.push_back(m_boundingBoxVertices[1]);
				bbox.push_back(m_boundingBoxVertices[2]);
				bbox.push_back(m_boundingBoxVertices[3]);
				bbox.push_back(m_boundingBoxVertices[4]);
				bbox.push_back(m_boundingBoxVertices[5]);
				bbox.push_back(m_boundingBoxVertices[6]);
				bbox.push_back(m_boundingBoxVertices[7]);
				
				
				glUseProgram(shaders[0]->GetShader());
				
				glEnable(GL_PROGRAM_POINT_SIZE);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				
				GLuint vao_0, vbo_0, vbo_1;
				
				glGenVertexArrays(1, &vao_0);
				glGenBuffers(1, &vbo_0);
				glGenBuffers(1, &vbo_1);
				
				glBindVertexArray(vao_0);
				
				glBindBuffer(GL_ARRAY_BUFFER, vbo_0);
				glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * bbox.size(), &bbox[0], GL_DYNAMIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
				
				glBindBuffer(GL_ARRAY_BUFFER, vbo_1);
				glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
				
				glUniformMatrix4fv(glGetUniformLocation(shaders[0]->GetShader(), "model"), 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(glGetUniformLocation(shaders[0]->GetShader(), "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(shaders[0]->GetShader(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				
				//glDrawArrays(GL_POINTS, 0, 8);
				glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, indices);
			
				glBindVertexArray(0);
				
				glDeleteVertexArrays(1, &vao_0);
				glDeleteBuffers(1, &vbo_0);
				glDeleteBuffers(1, &vbo_1);
				
				glDisable(GL_PROGRAM_POINT_SIZE);
				glDisable(GL_BLEND);
				
				glUseProgram(0);
			
				
				for(int x = 0; x < selectedChunk[0]->CHUNK_SIZE; ++x) {
					for(int y = 0; y < selectedChunk[0]->CHUNK_SIZE; ++y) {
						for(int z = 0; z < selectedChunk[0]->CHUNK_SIZE; ++z) {
							//skip testing empty air block
							if((*selectedChunk[0])(x,y,z) == 0) {
								continue;
							}
							
							glm::vec3 cube = glm::vec3(x,y,z);
							
							glm::vec3 AABBmin = selectedChunk[0]->getAABBCubeMin(cube.x, cube.y, cube.z) + selectedChunk[0]->m_pos;
							glm::vec3 AABBmax = selectedChunk[0]->getAABBCubeMax(cube.x, cube.y, cube.z) + selectedChunk[0]->m_pos;
							
							if(RayAABBIntersect(v0, dir, AABBmin, AABBmax)) {
								//select within range 10
								//if(glm::length(camera.GetPosition() - cube) < 10) {
									if(selectedCube.size() < 1) {
										selectedCube.push_back(cube);
									} else {
										//replace closer cube by the current cube to draw, glm::distance is basically length(p0-p1)
										if(glm::distance(cube, camera.GetPosition()) < glm::distance(selectedCube[0], camera.GetPosition())) {
											selectedCube[0] = cube;
										}
									}
								//}
							}
						}
					}
				}

				if(selectedCube.size() > 0) {
					glm::vec3 AABBmin = selectedChunk[0]->getAABBCubeMin(selectedCube[0].x, selectedCube[0].y, selectedCube[0].z);
					glm::vec3 AABBmax = selectedChunk[0]->getAABBCubeMax(selectedCube[0].x, selectedCube[0].y, selectedCube[0].z);
					
					//draw cube bbox
					glm::vec3 boundingBoxVertices[8];
					boundingBoxVertices[0] = glm::vec3(AABBmin.x, AABBmin.y, AABBmin.z);
					boundingBoxVertices[1] = glm::vec3(AABBmin.x, AABBmin.y, AABBmax.z);
					boundingBoxVertices[2] = glm::vec3(AABBmin.x, AABBmax.y, AABBmin.z);
					boundingBoxVertices[3] = glm::vec3(AABBmin.x, AABBmax.y, AABBmax.z);
					boundingBoxVertices[4] = glm::vec3(AABBmax.x, AABBmin.y, AABBmin.z);
					boundingBoxVertices[5] = glm::vec3(AABBmax.x, AABBmin.y, AABBmax.z);
					boundingBoxVertices[6] = glm::vec3(AABBmax.x, AABBmax.y, AABBmin.z);
					boundingBoxVertices[7] = glm::vec3(AABBmax.x, AABBmax.y, AABBmax.z);
					
					glUseProgram(shaders[1]->GetShader());
					
					GLuint vao_0, vbo_0, vbo_1;
					
					glGenVertexArrays(1, &vao_0);
					glGenBuffers(1, &vbo_0);
					glGenBuffers(1, &vbo_1);
					
					glBindVertexArray(vao_0);
					
					glBindBuffer(GL_ARRAY_BUFFER, vbo_0);
					glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 8, &boundingBoxVertices[0], GL_DYNAMIC_DRAW);
					glEnableVertexAttribArray(0);
					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
					
					glBindBuffer(GL_ARRAY_BUFFER, vbo_1);
					glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);
					glEnableVertexAttribArray(1);
					glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
					
					glUniformMatrix4fv(glGetUniformLocation(shaders[1]->GetShader(), "model"), 1, GL_FALSE, glm::value_ptr(model));
					glUniformMatrix4fv(glGetUniformLocation(shaders[1]->GetShader(), "view"), 1, GL_FALSE, glm::value_ptr(view));
					glUniformMatrix4fv(glGetUniformLocation(shaders[1]->GetShader(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
					
					glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, indices);
				
					glBindVertexArray(0);
					
					glDeleteVertexArrays(1, &vao_0);
					glDeleteBuffers(1, &vbo_0);
					glDeleteBuffers(1, &vbo_1);
										
					glUseProgram(0);
					
					//draw face
					
					AABBmin += selectedChunk[0]->m_pos; //ofset (cuz before AABBmin is offsetted by model matrix)
					AABBmax += selectedChunk[0]->m_pos;
					
					double x[] = { AABBmin.x, AABBmax.x, AABBmin.x, AABBmax.x, AABBmin.x, AABBmax.x, AABBmin.x, AABBmax.x, AABBmin.x, AABBmin.x, AABBmax.x, AABBmax.x };
					double y[] = { AABBmin.y, AABBmax.y, AABBmin.y, AABBmax.y, AABBmin.y, AABBmin.y, AABBmax.y, AABBmax.y, AABBmin.y, AABBmax.y, AABBmin.y, AABBmax.y };
					double z[] = { AABBmin.z, AABBmin.z, AABBmax.z, AABBmax.z, AABBmin.z, AABBmax.z, AABBmin.z, AABBmax.z, AABBmin.z, AABBmax.z, AABBmin.z, AABBmax.z };
					
					selectedFace = -1;
					double distance = 100;
					
					glm::vec3 AABBFaceMin;
					glm::vec3 AABBFaceMax;
					
					for (int i = 0; i < 6; i++)
					{
						glm::vec3 min(x[i * 2], y[i * 2], z[i * 2]);
						glm::vec3 max(x[i * 2 + 1], y[i * 2 + 1], z[i * 2 + 1]);
						
						glm::vec3 center(min.x == max.x ? min.x : min.x + (max.x-min.x) / 2, min.y == max.y ? min.y : min.y + (max.y-min.y) / 2, min.z == max.z ? min.z : min.z + (max.z-min.z) / 2);
						
						double d = glm::distance(center, camera.GetPosition());
						if(RayAABBIntersect(v0, dir, min, max) && d < distance) {
							selectedFace = i;
							distance = d;
							
							AABBFaceMin = min;
							AABBFaceMax = max;
						}
					}
					
					//std::cout << selectedFace << std::endl;
					
					AABBmin = AABBFaceMin;
					AABBmax = AABBFaceMax;
					
					AABBmin -= selectedChunk[0]->m_pos; //ofset (cuz before AABBmin is offsetted by model matrix)
					AABBmax -= selectedChunk[0]->m_pos;
							
					//draw face bbox
					boundingBoxVertices[0] = glm::vec3(AABBmin.x, AABBmin.y, AABBmin.z);
					boundingBoxVertices[1] = glm::vec3(AABBmin.x, AABBmin.y, AABBmax.z);
					boundingBoxVertices[2] = glm::vec3(AABBmin.x, AABBmax.y, AABBmin.z);
					boundingBoxVertices[3] = glm::vec3(AABBmin.x, AABBmax.y, AABBmax.z);
					boundingBoxVertices[4] = glm::vec3(AABBmax.x, AABBmin.y, AABBmin.z);
					boundingBoxVertices[5] = glm::vec3(AABBmax.x, AABBmin.y, AABBmax.z);
					boundingBoxVertices[6] = glm::vec3(AABBmax.x, AABBmax.y, AABBmin.z);
					boundingBoxVertices[7] = glm::vec3(AABBmax.x, AABBmax.y, AABBmax.z);
					
					glUseProgram(shaders[1]->GetShader());
					
					glGenVertexArrays(1, &vao_0);
					glGenBuffers(1, &vbo_0);
					glGenBuffers(1, &vbo_1);
					
					glBindVertexArray(vao_0);
					
					glBindBuffer(GL_ARRAY_BUFFER, vbo_0);
					glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 8, &boundingBoxVertices[0], GL_DYNAMIC_DRAW);
					glEnableVertexAttribArray(0);
					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
					
					glBindBuffer(GL_ARRAY_BUFFER, vbo_1);
					glBufferData(GL_ARRAY_BUFFER, sizeof(colors2), colors2, GL_DYNAMIC_DRAW);
					glEnableVertexAttribArray(1);
					glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
					
					glUniformMatrix4fv(glGetUniformLocation(shaders[1]->GetShader(), "model"), 1, GL_FALSE, glm::value_ptr(model));
					glUniformMatrix4fv(glGetUniformLocation(shaders[1]->GetShader(), "view"), 1, GL_FALSE, glm::value_ptr(view));
					glUniformMatrix4fv(glGetUniformLocation(shaders[1]->GetShader(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
					
					glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, indices);
				
					glBindVertexArray(0);
					
					glDeleteVertexArrays(1, &vao_0);
					glDeleteBuffers(1, &vbo_0);
					glDeleteBuffers(1, &vbo_1);
										
					glUseProgram(0);
				}
			}
		}
	
        SDL_GL_SwapWindow(window);
        
        this->showFPS();
        
        //SDL_Delay(16);
    }
    
    SDL_DestroyWindow(window);
    
    SDL_Quit();
}

void App::loop()
{
    // fps
    m_frames_elapsed++;
    m_ticks_current = SDL_GetTicks();
    
    // delta time
    m_delta_time = (m_ticks_current - m_ticks_then) / 1000.0f;
    m_ticks_then = m_ticks_current;

    // time elapsed
    auto m_chrono_now = std::chrono::high_resolution_clock::now();
    m_chrono_elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(m_chrono_now - m_chrono_start).count();
}

void App::showFPS()
{
    if(m_ticks_previous < m_ticks_current - 1000) {
        m_ticks_previous = SDL_GetTicks();
        m_frames_current = m_frames_elapsed;
        m_frames_elapsed = 0;
        
        if(m_frames_current < 1) {
            m_frames_current = 1;
        }
        
        std::cout << "FPS: " << m_frames_current << std::endl;
    }
}

bool App::RayAABBIntersect(const glm::vec3& rayOrig, const glm::vec3& rayDir, const glm::vec3& min, const glm::vec3& max)
{ 
    float tmin = (min.x - rayOrig.x) / rayDir.x; 
    float tmax = (max.x - rayOrig.x) / rayDir.x; 
 
    if (tmin > tmax) std::swap(tmin, tmax); 
 
    float tymin = (min.y - rayOrig.y) / rayDir.y; 
    float tymax = (max.y - rayOrig.y) / rayDir.y; 
 
    if (tymin > tymax) std::swap(tymin, tymax); 
 
    if ((tmin > tymax) || (tymin > tmax)) 
        return false; 
 
    if (tymin > tmin) 
        tmin = tymin; 
 
    if (tymax < tmax) 
        tmax = tymax; 
 
    float tzmin = (min.z - rayOrig.z) / rayDir.z; 
    float tzmax = (max.z - rayOrig.z) / rayDir.z; 
 
    if (tzmin > tzmax) std::swap(tzmin, tzmax); 
 
    if ((tmin > tzmax) || (tzmin > tmax)) 
        return false; 
 
    if (tzmin > tmin) 
        tmin = tzmin; 
 
    if (tzmax < tmax) 
        tmax = tzmax; 
 
    return true; 
} 

int App::getSizeX() const
{
    return m_sizeX;
}

int App::getSizeY() const
{
    return m_sizeY;
}

void App::setSizeX(int sizeX)
{
    m_sizeX = sizeX;
}

void App::setSizeY(int sizeY)
{
    m_sizeY = sizeY;
}

double App::getDeltaTime() const
{ 
    return m_delta_time;
}

double App::getTimeElapsed() const
{
    return m_chrono_elapsed;
}

