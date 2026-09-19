#if defined(__linux__)
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
static bool g_attempted;

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

void gpu_gl_import_pose_fd(int fd, uint64_t size, const uint8_t* uuid)
{
	if (g_attempted || fd < 0 || size == 0)
	{
		return;
	}
    g_attempted = true;
    GLint extension_count = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extension_count);
    bool has_fd = false, has_storage = false;
    for (GLint i = 0; i < extension_count; ++i)
    {
        const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i);
        if (!extension) continue;
        has_fd |= strcmp(extension, "GL_EXT_memory_object_fd") == 0;
        has_storage |= strcmp(extension, "GL_EXT_memory_object") == 0;
    }
    if (!has_fd || !has_storage)
    {
        fprintf(stderr, "GPU pose import unavailable; using current CPU pose mirrors\n");
        return;
    }
    // Extensions alone do not make cross-adapter external-memory imports safe.
    typedef void (*GetUuidFn)(GLenum, GLuint, GLubyte*);
    GetUuidFn get_uuid = (GetUuidFn)proc("glGetUnsignedBytei_vEXT");
    GLint device_count = 0;
    glGetIntegerv(GL_NUM_DEVICE_UUIDS_EXT, &device_count);
    bool same_device = false;
    const uint8_t zero_uuid[16] = {0};
    if (get_uuid && uuid && memcmp(uuid, zero_uuid, sizeof(zero_uuid)) != 0)
    {
        for (GLint i = 0; i < device_count; ++i)
        {
            GLubyte gl_uuid[16] = {0};
            get_uuid(GL_DEVICE_UUID_EXT, (GLuint)i, gl_uuid);
            same_device |= memcmp(gl_uuid, uuid, sizeof(gl_uuid)) == 0;
        }
    }
    if (!same_device)
    {
        fprintf(stderr, "GPU pose import skipped: different or unknown GPU UUID; using current CPU pose mirrors\n");
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
    // glGenBuffers reserves a name; bind once to create the object before DSA.
    // Strict drivers reject NamedBufferStorageMemEXT on an uncreated name.
    GLint previous = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previous);
    glGenBuffers(1, &g_buf);
    glBindBuffer(GL_ARRAY_BUFFER, g_buf);
    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)previous);
	NamedBufferStorageMemEXT(g_buf, (GLsizeiptr)size, g_mem, 0);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		fprintf(stderr, "GPU pose import: GL error 0x%x; using current CPU pose mirrors\n", (unsigned)err);
		return;
	}
	g_ok = true;
	fprintf(stderr, "GPU pose import: GL buffer %u (%llu bytes)\n", g_buf, (unsigned long long)size);
}

#else
#include <stdbool.h>
#include <stdint.h>
bool gpu_gl_poses_imported(void) { return false; }
unsigned int gpu_gl_pose_buffer(void) { return 0; }
void gpu_gl_import_pose_fd(int fd, uint64_t size, const uint8_t* uuid) { (void)fd; (void)size; (void)uuid; }
#endif
