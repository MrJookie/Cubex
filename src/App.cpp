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
    
    cx::Camera camera(glm::vec3(0.0f, 150.0f, 50.0f));
    
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
		*/

		//chunk->CreateMesh(l);
		chunk->CreateGreedyMesh();
		
		chunks.push_back(chunk);
	}
	
    bool toggleMouseRelative = false;
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
                    
                    case SDLK_r:
                    {
						(*chunks[0])(ax++, 0, 0) = 0;
						chunks[0]->CreateMesh(0);
					}
					break;
                }
            }
            else if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
                SDL_ShowCursor(SDL_DISABLE);
                SDL_SetRelativeMouseMode(SDL_TRUE);
                toggleMouseRelative = true;

                skipMouseResolution = 2;
            }
            else if(e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT) {
                SDL_ShowCursor(SDL_ENABLE);
                SDL_SetRelativeMouseMode(SDL_FALSE);
                toggleMouseRelative = false;

                skipMouseResolution = 2;
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
                int xpos;
                int ypos;
                SDL_GetMouseState(&xpos, &ypos);
            }
        };
        
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()), m_sizeX/(float)m_sizeY, 0.1f, 1000.0f);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));

        camera.ExtractFrustumPlanes(view, projection);
        
        //std::cout << "AABB intersects: " << camera.AABBIntersectsFrustum(chunk.m_boundingBoxMin, chunk.m_boundingBoxMax) << std::endl;
        
        for(int i = 0; i < chunks.size(); ++i) {
			//if(camera.AABBIntersectsFrustum(chunks[i]->m_boundingBoxMin, chunks[i]->m_boundingBoxMax)) {
				chunks[i]->DrawChunk(glm::vec3(1.0, 0.0, 0.0), shaders[0]->GetShader(), model, view, projection);
			//}
			
			
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
        
			glm::vec3 m_boundingBoxVertices[8];
			m_boundingBoxVertices[0] = glm::vec3(chunks[i]->getBoundingBoxMin().x, chunks[i]->getBoundingBoxMin().y, chunks[i]->getBoundingBoxMin().z);
			m_boundingBoxVertices[1] = glm::vec3(chunks[i]->getBoundingBoxMin().x, chunks[i]->getBoundingBoxMin().y, chunks[i]->getBoundingBoxMax().z);
			m_boundingBoxVertices[2] = glm::vec3(chunks[i]->getBoundingBoxMin().x, chunks[i]->getBoundingBoxMax().y, chunks[i]->getBoundingBoxMin().z);
			m_boundingBoxVertices[3] = glm::vec3(chunks[i]->getBoundingBoxMin().x, chunks[i]->getBoundingBoxMax().y, chunks[i]->getBoundingBoxMax().z);
			m_boundingBoxVertices[4] = glm::vec3(chunks[i]->getBoundingBoxMax().x, chunks[i]->getBoundingBoxMin().y, chunks[i]->getBoundingBoxMin().z);
			m_boundingBoxVertices[5] = glm::vec3(chunks[i]->getBoundingBoxMax().x, chunks[i]->getBoundingBoxMin().y, chunks[i]->getBoundingBoxMax().z);
			m_boundingBoxVertices[6] = glm::vec3(chunks[i]->getBoundingBoxMax().x, chunks[i]->getBoundingBoxMax().y, chunks[i]->getBoundingBoxMin().z);
			m_boundingBoxVertices[7] = glm::vec3(chunks[i]->getBoundingBoxMax().x, chunks[i]->getBoundingBoxMax().y, chunks[i]->getBoundingBoxMax().z);
        
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

