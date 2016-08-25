#pragma once

#include <map>

#include "graphics_threaded.h"
#include <SDL.h>
#include <GL/glew.h>

#if !SDL_VERSION_ATLEAST(2,0,0)
typedef void * SDL_GLContext;
typedef void * SDL_Window;
static inline SDL_GLContext SDL_GL_CreateContext(SDL_Window *window) { return 1; }
static inline int SDL_GL_MakeCurrent(SDL_Window * window, SDL_GLContext context) { return 0; }
static inline void SDL_GL_SwapWindow(SDL_Window * window) { SDL_GL_SwapBuffers(); };
#endif

// Use SDL semaphores on Mac OS X, because (unnamed) posix semaphores are not available
#if defined(CONF_PLATFORM_MACOSX)
	class semaphore
	{
		SDL_sem *sem;
	public:
		semaphore() { sem = SDL_CreateSemaphore(0); }
		~semaphore() { SDL_DestroySemaphore(sem); }
		void wait() { SDL_SemWait(sem); }
		void signal() { SDL_SemPost(sem); }
	};

	#include <objc/objc-runtime.h>
	
	class CAutoreleasePool
	{
	private:
		id m_Pool;

	public:
		CAutoreleasePool()
		{
			Class NSAutoreleasePoolClass = (Class) objc_getClass("NSAutoreleasePool");
			m_Pool = class_createInstance(NSAutoreleasePoolClass, 0);
			SEL selector = sel_registerName("init");
			objc_msgSend(m_Pool, selector);
		}

		~CAutoreleasePool()
		{
			SEL selector = sel_registerName("drain");
			objc_msgSend(m_Pool, selector);
		}
	};	
#endif

// basic threaded backend, abstract, missing init and shutdown functions
class CGraphicsBackend_Threaded : public IGraphicsBackend
{
public:
	// constructed on the main thread, the rest of the functions is run on the render thread
	class ICommandProcessor
	{
	public:
		virtual ~ICommandProcessor() {}
		virtual void RunBuffer(CCommandBuffer *pBuffer) = 0;
	};

	CGraphicsBackend_Threaded();

	virtual void RunBuffer(CCommandBuffer *pBuffer);
	virtual bool IsIdle() const;
	virtual void WaitForIdle();
		
protected:
	void StartProcessor(ICommandProcessor *pProcessor);
	void StopProcessor();

private:
	ICommandProcessor *m_pProcessor;
	CCommandBuffer * volatile m_pBuffer;
	volatile bool m_Shutdown;
	semaphore m_Activity;
	semaphore m_BufferDone;
	void *m_pThread;

	static void ThreadFunc(void *pUser);
};

// takes care of implementation independent operations
class CCommandProcessorFragment_General
{
	void Cmd_Nop();
	void Cmd_Signal(const CCommandBuffer::SCommand_Signal *pCommand);
public:
	bool RunCommand(const CCommandBuffer::SCommand * pBaseCommand);
};


// takes care of opengl related rendering
class CCommandProcessorFragment_OpenGL
{
	struct CTexture
	{
		GLuint m_Tex;
		int m_MemSize;
	};
	CTexture m_aTextures[CCommandBuffer::MAX_TEXTURES];
	volatile int *m_pTextureMemoryUsage;

	GLuint m_PixelTexture;
	
	GLuint textureBuffer[NUM_RENDERBUFFERS];
	GLuint renderedTexture[NUM_RENDERBUFFERS];

	class CShader
	{
		struct CUniformLocation
		{
			GLint value;
			CUniformLocation() { value = -2; } // need this to initialize new keys in our map correctly
		};

		GLuint m_Program;
		std::map<const GLcharARB*, CUniformLocation> m_aUniformLocationCache;

	public:
		GLuint Handle() const { return m_Program; }
		GLint getUniformLocation(const char *pName);

		GLuint operator=(GLuint Program) { return (m_Program = Program); } // to easily assign a linked shader program
	};
	CShader m_aShader[NUM_SHADERS];

public:
	enum
	{
		CMD_INIT = CCommandBuffer::CMDGROUP_PLATFORM_OPENGL,
	};

	struct SCommand_Init : public CCommandBuffer::SCommand
	{
		SCommand_Init() : SCommand(CMD_INIT) {}
		volatile int *m_pTextureMemoryUsage;
	};

private:
	static int TexFormatToOpenGLFormat(int TexFormat);
	static unsigned char Sample(int w, int h, const unsigned char *pData, int u, int v, int Offset, int ScaleW, int ScaleH, int Bpp);
	static void *Rescale(int Width, int Height, int NewWidth, int NewHeight, int Format, const unsigned char *pData);

	void SetState(const CCommandBuffer::SState &State);

	void Cmd_Init(const SCommand_Init *pCommand);
	void Cmd_Texture_Update(const CCommandBuffer::SCommand_Texture_Update *pCommand);
	void Cmd_Texture_Destroy(const CCommandBuffer::SCommand_Texture_Destroy *pCommand);
	void Cmd_Texture_Create(const CCommandBuffer::SCommand_Texture_Create *pCommand);
	void Cmd_CreateTextureBuffer(const CCommandBuffer::SCommand_CreateTextureBuffer *pCommand);
	void Cmd_LoadShaders(const CCommandBuffer::SCommand_LoadShaders *pCommand);
	void Cmd_ShaderBegin(const CCommandBuffer::SCommand_ShaderBegin *pCommand);
	void Cmd_ShaderEnd(const CCommandBuffer::SCommand_ShaderEnd *pCommand);
	void Cmd_Clear(const CCommandBuffer::SCommand_Clear *pCommand);
	void Cmd_ClearBufferTexture(const CCommandBuffer::SCommand_ClearBufferTexture *pCommand);
	void Cmd_Render(const CCommandBuffer::SCommand_Render *pCommand);
	void Cmd_Screenshot(const CCommandBuffer::SCommand_Screenshot *pCommand);

public:
	CCommandProcessorFragment_OpenGL();

	bool RunCommand(const CCommandBuffer::SCommand * pBaseCommand);
};

// takes care of sdl related commands
class CCommandProcessorFragment_SDL
{
	// SDL stuff
	SDL_GLContext m_GLContext;
	SDL_Window * m_pWindow;
public:
	enum
	{
		CMD_INIT = CCommandBuffer::CMDGROUP_PLATFORM_SDL,
		CMD_SHUTDOWN,
	};

	struct SCommand_Init : public CCommandBuffer::SCommand
	{
		SCommand_Init() : SCommand(CMD_INIT) {}
		SDL_GLContext m_GLContext;
		SDL_Window * m_pWindow;
	};

	struct SCommand_Shutdown : public CCommandBuffer::SCommand
	{
		SCommand_Shutdown() : SCommand(CMD_SHUTDOWN) {}
	};

private:
	void Cmd_Init(const SCommand_Init *pCommand);
	void Cmd_Shutdown(const SCommand_Shutdown *pCommand);
	void Cmd_Swap(const CCommandBuffer::SCommand_Swap *pCommand);
	void Cmd_VideoModes(const CCommandBuffer::SCommand_VideoModes *pCommand);
public:
	CCommandProcessorFragment_SDL();

	bool RunCommand(const CCommandBuffer::SCommand *pBaseCommand);
};

// command processor impelementation, uses the fragments to combine into one processor
class CCommandProcessor_SDL_OpenGL : public CGraphicsBackend_Threaded::ICommandProcessor
{
 	CCommandProcessorFragment_OpenGL m_OpenGL;
 	CCommandProcessorFragment_SDL m_SDL;
 	CCommandProcessorFragment_General m_General;
 public:
	virtual void RunBuffer(CCommandBuffer *pBuffer);
};

// graphics backend implemented with SDL and OpenGL
class CGraphicsBackend_SDL_OpenGL : public CGraphicsBackend_Threaded
{
    SDL_Window *m_pWindow;
    SDL_GLContext m_GLContext;
	
	ICommandProcessor *m_pProcessor;
	volatile int m_TextureMemoryUsage;
public:
	virtual int Init(const char *pName, int *Width, int *Height, int Screen, int FsaaSamples, int Flags, int *pDesktopWidth, int *pDesktopHeight);
	virtual int Shutdown();

	virtual int MemoryUsage() const;

	virtual void Minimize();
	virtual void Maximize();
	virtual void GrabWindow(bool);
	virtual void WarpMouse(int x, int y);
	virtual int WindowActive();
	virtual int WindowOpen();
	virtual int GetNumScreens();
};
