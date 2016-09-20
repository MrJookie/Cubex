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
    //int CHUNK_SIZE = 16;
    
    cx::World world;
    
    std::vector<cx::Chunk*> chunks;

    module::Perlin noise;
    noise.SetOctaveCount(1);
    noise.SetFrequency(1.0);
    noise.SetPersistence(0.1);
    noise.SetSeed(0);
    
    for(int l = 0; l < 1; l++) {
		cx::Chunk* chunk = new cx::Chunk;

		int chunkHeight = 0;
		for (int z = 0; z < chunk->CHUNK_SIZE; ++z) {
			for (int x = 0; x < chunk->CHUNK_SIZE; ++x) {
				int height = (0.5 + 0.5 * noise.GetValue (x / (float)chunk->CHUNK_SIZE, z / (float)chunk->CHUNK_SIZE, 0.0) ) * chunk->CHUNK_SIZE;
				
				if(height > chunkHeight) {
					chunkHeight = height;
				}
				
				for (int y = 0; y < height; ++y) {
					(*chunk)(x, y, z) = 1;
				}
			}
		}
		
		chunk->m_boundingBoxMin = glm::vec3(0, 0, 0);
		chunk->m_boundingBoxMax = glm::vec3(chunk->CHUNK_SIZE, chunkHeight, chunk->CHUNK_SIZE);
		
		/*
		(*chunk)(0, 0, 0) = 2;
		(*chunk)(1, 0, 0) = 2;
		(*chunk)(2, 0, 0) = 2;
		
		(*chunk)(4, 0, 0) = 2;
		*/
		
		if(l == 0)
		chunk->CreateGreedyMesh();
		else if(l == 1)
		chunk->CreateMesh(l, false);
		else
		chunk->CreateMesh(l, true);
		
		std::cout << "verts: " << chunk->GetVertexCount() << std::endl;
		
		chunks.push_back(chunk);
	}
	
	std::vector<glm::vec3> selectedCube;
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
    
    int ax = 0;
    
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
                    
                    /*
                    case SDLK_r:
                    {
						(*chunks[0])(ax++, 0, 0) = 0;
						chunks[0]->CreateMesh(0, true);
					}
					break;
					*/
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
						
						(*chunks[0])(selectedCube[0].x + x, selectedCube[0].y + y, selectedCube[0].z + z) = 1;
												
						std::cout << "X: " << selectedCube[0].x << 
									 " Y: " << selectedCube[0].y << 
									 " Z: " << selectedCube[0].z << 
									 " face: " << selectedFace << std::endl;
						
						chunks[0]->CreateGreedyMesh();
					}
				}
				
				if(e.button.button == SDL_BUTTON_RIGHT) {
					if(!selectedCube.empty()) {
						(*chunks[0])(selectedCube[0].x, selectedCube[0].y, selectedCube[0].z) = 0;
												
						std::cout << "X: " << selectedCube[0].x << 
									 " Y: " << selectedCube[0].y << 
									 " Z: " << selectedCube[0].z << 
									 " face: " << selectedFace << std::endl;
						
						chunks[0]->CreateGreedyMesh();
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
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));

        camera.ExtractFrustumPlanes(view, projection);
        
        //std::cout << "AABB intersects: " << camera.AABBIntersectsFrustum(chunk.m_boundingBoxMin, chunk.m_boundingBoxMax) << std::endl;
        
        selectedCube.clear();
        
        for(int i = 0; i < chunks.size(); ++i) {
			//if(camera.AABBIntersectsFrustum(chunks[i]->m_boundingBoxMin, chunks[i]->m_boundingBoxMax)) {
				chunks[i]->DrawChunk(glm::vec3(1.0, 0.0, 0.0), shaders[0]->GetShader(), model, view, projection);
			//}
			
			if(toggleMouseRelative) {
				glm::vec4 viewport = glm::vec4(0.0f, 0.0f, this->getSizeX(), this->getSizeY());
				glm::vec3 v0 = glm::unProject(glm::vec3(this->getSizeX()/2.0f, this->getSizeY()/2.0f, 0.0f), view, projection, viewport);
				glm::vec3 v1 = glm::unProject(glm::vec3(this->getSizeX()/2.0f, this->getSizeY() - this->getSizeY()/2.0f, 1.0f), view, projection, viewport);
				glm::vec3 dir = v1 - v0; 
				
				if(RayAABBIntersect(v0, dir, chunks[i]->getBoundingBoxMin(), chunks[i]->getBoundingBoxMax())) {
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
					
					/*
					//AABB of chunk
					glm::vec3 m_boundingBoxVertices[8];
					m_boundingBoxVertices[0] = glm::vec3(chunks[i]->getBoundingBoxMin().x, chunks[i]->getBoundingBoxMin().y, chunks[i]->getBoundingBoxMin().z);
					m_boundingBoxVertices[1] = glm::vec3(chunks[i]->getBoundingBoxMin().x, chunks[i]->getBoundingBoxMin().y, chunks[i]->getBoundingBoxMax().z);
					m_boundingBoxVertices[2] = glm::vec3(chunks[i]->getBoundingBoxMin().x, chunks[i]->getBoundingBoxMax().y, chunks[i]->getBoundingBoxMin().z);
					m_boundingBoxVertices[3] = glm::vec3(chunks[i]->getBoundingBoxMin().x, chunks[i]->getBoundingBoxMax().y, chunks[i]->getBoundingBoxMax().z);
					m_boundingBoxVertices[4] = glm::vec3(chunks[i]->getBoundingBoxMax().x, chunks[i]->getBoundingBoxMin().y, chunks[i]->getBoundingBoxMin().z);
					m_boundingBoxVertices[5] = glm::vec3(chunks[i]->getBoundingBoxMax().x, chunks[i]->getBoundingBoxMin().y, chunks[i]->getBoundingBoxMax().z);
					m_boundingBoxVertices[6] = glm::vec3(chunks[i]->getBoundingBoxMax().x, chunks[i]->getBoundingBoxMax().y, chunks[i]->getBoundingBoxMin().z);
					m_boundingBoxVertices[7] = glm::vec3(chunks[i]->getBoundingBoxMax().x, chunks[i]->getBoundingBoxMax().y, chunks[i]->getBoundingBoxMax().z);
					*/
										
					for(int x = 0; x < chunks[i]->CHUNK_SIZE; ++x) {
						for(int y = 0; y < chunks[i]->CHUNK_SIZE; ++y) {
							for(int z = 0; z < chunks[i]->CHUNK_SIZE; ++z) {
								//skip testing empty air block
								if((*chunks[i])(x,y,z) == 0) {
									continue;
								}
								
								glm::vec3 cube = glm::vec3(x,y,z);
								
								glm::vec3 AABBmin = chunks[i]->getAABBCubeMin(cube.x, cube.y, cube.z);
								glm::vec3 AABBmax = chunks[i]->getAABBCubeMax(cube.x, cube.y, cube.z);
								
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
						glm::vec3 AABBmin = chunks[i]->getAABBCubeMin(selectedCube[0].x, selectedCube[0].y, selectedCube[0].z);
						glm::vec3 AABBmax = chunks[i]->getAABBCubeMax(selectedCube[0].x, selectedCube[0].y, selectedCube[0].z);
						
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
								
						//draw cube bbox
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

