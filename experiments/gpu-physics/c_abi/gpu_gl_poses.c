#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#ifndef GL_HANDLE_TYPE_OPAQUE_FD_EXT
#define GL_HANDLE_TYPE_OPAQUE_FD_EXT 0x9586
#endif

typedef void (*PFNGLCREATEMEMORYOBJECTSEXTPROC)(GLsizei, GLuint*);
typedef void (*PFNGLIMPORTMEMORYFDEXTPROC)(GLuint, GLuint64, GLenum, GLint);
typedef void (*PFNGLNAMEDBUFFERSTORAGEMEMEXTPROC)(GLuint, GLsizeiptr, GLuint, GLuint64);

static GLuint g_mem;
static GLuint g_buf;
static bool g_ok;

static void* proc(const char* name)
{
	return (void*)glXGetProcAddressARB((const GLubyte*)name);
}

bool gpu_gl_poses_imported(void)
{
	return g_ok;
}

GLuint gpu_gl_pose_buffer(void)
{
	return g_ok ? g_buf : 0;
}

void gpu_gl_import_pose_fd(int fd, uint64_t size)
{
	if (g_ok || fd < 0 || size == 0)
	{
		return;
	}
	int owned = dup(fd);
	if (owned < 0)
	{
		perror("dup pose fd");
		return;
	}
	PFNGLCREATEMEMORYOBJECTSEXTPROC CreateMemoryObjectsEXT =
		(PFNGLCREATEMEMORYOBJECTSEXTPROC)proc("glCreateMemoryObjectsEXT");
	PFNGLIMPORTMEMORYFDEXTPROC ImportMemoryFdEXT = (PFNGLIMPORTMEMORYFDEXTPROC)proc("glImportMemoryFdEXT");
	PFNGLNAMEDBUFFERSTORAGEMEMEXTPROC NamedBufferStorageMemEXT =
		(PFNGLNAMEDBUFFERSTORAGEMEMEXTPROC)proc("glNamedBufferStorageMemEXT");
	if (CreateMemoryObjectsEXT == NULL || ImportMemoryFdEXT == NULL || NamedBufferStorageMemEXT == NULL)
	{
		fprintf(stderr, "GPU pose import: GL_EXT_memory_object_fd missing\n");
		close(owned);
		return;
	}
	CreateMemoryObjectsEXT(1, &g_mem);
	ImportMemoryFdEXT(g_mem, (GLuint64)size, GL_HANDLE_TYPE_OPAQUE_FD_EXT, owned);
	glGenBuffers(1, &g_buf);
	NamedBufferStorageMemEXT(g_buf, (GLsizeiptr)size, g_mem, 0);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		fprintf(stderr, "GPU pose import: GL error 0x%x\n", (unsigned)err);
		return;
	}
	g_ok = true;
	fprintf(stderr, "GPU pose import: GL buffer %u (%llu bytes)\n", g_buf, (unsigned long long)size);
}
