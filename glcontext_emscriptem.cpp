#include <Python.h>
#include <cstring>
#include <unordered_map>

#include <GLES3/gl32.h>
#include <emscripten.h>

#if defined(__x86_64__) || defined(_WIN64)
typedef long long int sizeiptr;
#else
typedef int sizeiptr;
#endif

struct ImageFormat {
    unsigned format;
    unsigned type;
};

static std::unordered_map<unsigned, unsigned> texture_targets;
static std::unordered_map<unsigned, unsigned> targetbindings;
static std::unordered_map<unsigned, ImageFormat> internalformats;
static unsigned vertex_array_temp_buffer;
static sizeiptr vertex_array_temp_offset;
static int vertex_array_temp_stride;

static void glMultiDrawArraysIndirect(unsigned mode, const void * indirect, int drawcount, int stride) {
}

static void glMultiDrawElementsIndirect(unsigned mode, unsigned type, const void * indirect, int drawcount, int stride) {
}

static void glBindBuffersRange(unsigned target, unsigned first, int count, const unsigned * buffers, const sizeiptr * offsets, const sizeiptr * sizes) {
    for (int i = 0; i < count; ++i) {
        glBindBufferRange(target, first + i, buffers[i], offsets[i], sizes[i]);
    }
}

static void glBindTextures(unsigned first, int count, const unsigned * textures) {
    int active_texture = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
    for (int i = 0; i < count; ++i) {
        glActiveTexture(GL_TEXTURE0 + first + i);
        unsigned target = texture_targets[textures[i]];
        glBindTexture(target, textures[i]);
    }
    glActiveTexture(active_texture);
}

static void glBindSamplers(unsigned first, int count, const unsigned * samplers) {
    for (int i = 0; i < count; ++i) {
        glBindSampler(first + i, samplers[i]);
    }
}

static void glBindImageTextures(unsigned first, int count, const unsigned * textures) {
}

static void glCreateBuffers(int n, unsigned * buffers) {
    int array_buffer_binding = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
    glGenBuffers(n, buffers);
    for (int i = 0; i < n; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
    }
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);
}

static void glNamedBufferStorage(unsigned buffer, sizeiptr size, const void * data, unsigned flags) {
    int array_buffer_binding = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);
}

static void glNamedBufferSubData(unsigned buffer, sizeiptr offset, sizeiptr size, const void * data) {
    int array_buffer_binding = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);
}

static void * glMapNamedBufferRange(unsigned buffer, sizeiptr offset, sizeiptr length, unsigned access) {
    int array_buffer_binding = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    void * result = glMapBufferRange(GL_ARRAY_BUFFER, offset, length, access);
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);
    return result;
}

static unsigned char glUnmapNamedBuffer(unsigned buffer) {
    int array_buffer_binding = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    unsigned char result = glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);
    return result;
}

static void glGetNamedBufferSubData(unsigned buffer, sizeiptr offset, sizeiptr size, void * data) {
    // int array_buffer_binding = 0;
    // glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
    // glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // glGetBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    // glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);
}

static void glCreateFramebuffers(int n, unsigned * framebuffers) {
    int read_framebuffer_binding = 0;
    int draw_framebuffer_binding = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_framebuffer_binding);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer_binding);
    glGenFramebuffers(n, framebuffers);
    for (int i = 0; i < n; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, read_framebuffer_binding);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer_binding);
}

static void glNamedFramebufferRenderbuffer(unsigned framebuffer, unsigned attachment, unsigned renderbuffertarget, unsigned renderbuffer) {
    int read_framebuffer_binding = 0;
    int draw_framebuffer_binding = 0;
    int renderbuffer_binding = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_framebuffer_binding);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer_binding);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbuffer_binding);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, renderbuffertarget, renderbuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, read_framebuffer_binding);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer_binding);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_binding);
}

static void glNamedFramebufferParameteri(unsigned framebuffer, unsigned pname, int param) {
}

static void glNamedFramebufferTexture(unsigned framebuffer, unsigned attachment, unsigned texture, int level) {
    int read_framebuffer_binding = 0;
    int draw_framebuffer_binding = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_framebuffer_binding);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer_binding);
    unsigned target = texture_targets[texture];
    glBindTexture(target, texture);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, texture, level);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, read_framebuffer_binding);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer_binding);
}

static void glNamedFramebufferTextureLayer(unsigned framebuffer, unsigned attachment, unsigned texture, int level, int layer) {
    int read_framebuffer_binding = 0;
    int draw_framebuffer_binding = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_framebuffer_binding);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer_binding);
    unsigned target = texture_targets[texture];
    glBindTexture(target, texture);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, texture, level, layer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, read_framebuffer_binding);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer_binding);
}

static void glNamedFramebufferDrawBuffers(unsigned framebuffer, int n, const unsigned * bufs) {
    int read_framebuffer_binding = 0;
    int draw_framebuffer_binding = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_framebuffer_binding);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer_binding);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glDrawBuffers(n, bufs);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, read_framebuffer_binding);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer_binding);
}

static void glNamedFramebufferReadBuffer(unsigned framebuffer, unsigned src) {
    int read_framebuffer_binding = 0;
    int draw_framebuffer_binding = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_framebuffer_binding);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer_binding);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glReadBuffer(src);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, read_framebuffer_binding);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer_binding);
}

static void glBlitNamedFramebuffer(unsigned readFramebuffer, unsigned drawFramebuffer, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, unsigned mask, unsigned filter) {
    int read_framebuffer_binding = 0;
    int draw_framebuffer_binding = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_framebuffer_binding);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer_binding);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);
    glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, read_framebuffer_binding);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer_binding);
}

static void glCreateRenderbuffers(int n, unsigned * renderbuffers) {
    int renderbuffer_binding = 0;
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbuffer_binding);
    glGenRenderbuffers(n, renderbuffers);
    for (int i = 0; i < n; ++i) {
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers[i]);
    }
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_binding);
}

static void glNamedRenderbufferStorageMultisample(unsigned renderbuffer, int samples, unsigned internalformat, int width, int height) {
    int renderbuffer_binding = 0;
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbuffer_binding);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalformat, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_binding);
}

static void glCreateTextures(unsigned target, int n, unsigned * textures) {
    int texture_binding = 0;
    glGetIntegerv(targetbindings[target], &texture_binding);
    glGenTextures(n, textures);
    for (int i = 0; i < n; ++i) {
        glBindTexture(target, textures[i]);
        texture_targets[textures[i]] = target;
    }
    glBindTexture(target, texture_binding);
}

static void glTextureStorage2D(unsigned texture, int levels, unsigned internalformat, int width, int height) {
    unsigned target = texture_targets[texture];
    unsigned format = internalformats[internalformat].format;
    unsigned type = internalformats[internalformat].type;
    int texture_binding = 0;
    glGetIntegerv(targetbindings[target], &texture_binding);
    glBindTexture(target, texture);
    for (int level = 0; level < levels; ++level) {
        glTexImage2D(target, level, internalformat, width, height, 0, format, type, NULL);
        width = width > 2 ? width >> 1 : 1;
        height = height > 2 ? height >> 1 : 1;
    }
    glBindTexture(target, texture_binding);
}

static void glTextureStorage3D(unsigned texture, int levels, unsigned internalformat, int width, int height, int depth) {
    unsigned target = texture_targets[texture];
    unsigned format = internalformats[internalformat].format;
    unsigned type = internalformats[internalformat].type;
    int texture_binding = 0;
    glGetIntegerv(targetbindings[target], &texture_binding);
    glBindTexture(target, texture);
    for (int level = 0; level < levels; ++level) {
        glTexImage3D(target, level, internalformat, width, height, depth, 0, format, type, NULL);
        width = width > 2 ? width >> 1 : 1;
        height = height > 2 ? height >> 1 : 1;
        if (target != GL_TEXTURE_2D_ARRAY && target != GL_TEXTURE_CUBE_MAP_ARRAY) {
            depth = depth > 2 ? depth >> 1 : 1;
        }
    }
    glBindTexture(target, texture_binding);
}

static void glTextureSubImage2D(unsigned texture, int level, int xoffset, int yoffset, int width, int height, unsigned format, unsigned type, const void * pixels) {
    unsigned target = texture_targets[texture];
    int texture_binding = 0;
    glGetIntegerv(targetbindings[target], &texture_binding);
    glBindTexture(target, texture);
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    glBindTexture(target, texture_binding);
}

static void glTextureSubImage3D(unsigned texture, int level, int xoffset, int yoffset, int zoffset, int width, int height, int depth, unsigned format, unsigned type, const void * pixels) {
    unsigned target = texture_targets[texture];
    int texture_binding = 0;
    glGetIntegerv(targetbindings[target], &texture_binding);
    glBindTexture(target, texture);
    glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    glBindTexture(target, texture_binding);
}

static void glTextureParameteri(unsigned texture, unsigned pname, int param) {
    unsigned target = texture_targets[texture];
    int texture_binding = 0;
    glGetIntegerv(targetbindings[target], &texture_binding);
    glBindTexture(target, texture);
    glTexParameteri(target, pname, param);
    glBindTexture(target, texture_binding);
}

static void glGenerateTextureMipmap(unsigned texture) {
    unsigned target = texture_targets[texture];
    int texture_binding = 0;
    glGetIntegerv(targetbindings[target], &texture_binding);
    glBindTexture(target, texture);
    glGenerateMipmap(target);
    glBindTexture(target, texture_binding);
}

static void glCreateVertexArrays(int n, unsigned * arrays) {
    int vertex_array_binding = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertex_array_binding);
    glGenVertexArrays(n, arrays);
    for (int i = 0; i < n; ++i) {
        glBindVertexArray(arrays[i]);
    }
    glBindVertexArray(vertex_array_binding);
}

static void glEnableVertexArrayAttrib(unsigned vaobj, unsigned index) {
    int vertex_array_binding = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertex_array_binding);
    glBindVertexArray(vaobj);
    glEnableVertexAttribArray(index);
    glBindVertexArray(vertex_array_binding);
}

static void glVertexArrayElementBuffer(unsigned vaobj, unsigned buffer) {
    int vertex_array_binding = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertex_array_binding);
    glBindVertexArray(vaobj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBindVertexArray(vertex_array_binding);
}

static void glVertexArrayVertexBuffer(unsigned vaobj, unsigned bindingindex, unsigned buffer, sizeiptr offset, int stride) {
    vertex_array_temp_buffer = buffer;
    vertex_array_temp_offset = offset;
    vertex_array_temp_stride = stride;
}

static void glVertexArrayAttribFormat(unsigned vaobj, unsigned attribindex, int size, unsigned type, unsigned char normalized, unsigned relativeoffset) {
    int vertex_array_binding = 0;
    int array_buffer_binding = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertex_array_binding);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
    glBindVertexArray(vaobj);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_array_temp_buffer);
    glVertexAttribPointer(attribindex, size, type, normalized, vertex_array_temp_stride, (void *)vertex_array_temp_offset);
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);
    glBindVertexArray(vertex_array_binding);
}

static void glVertexArrayAttribIFormat(unsigned vaobj, unsigned attribindex, int size, unsigned type, unsigned relativeoffset) {
    int vertex_array_binding = 0;
    int array_buffer_binding = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertex_array_binding);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
    glBindVertexArray(vaobj);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_array_temp_buffer);
    glVertexAttribIPointer(attribindex, size, type, vertex_array_temp_stride, (void *)vertex_array_temp_offset);
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);
    glBindVertexArray(vertex_array_binding);
}

static void glVertexArrayBindingDivisor(unsigned vaobj, unsigned bindingindex, unsigned divisor) {
    int vertex_array_binding = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertex_array_binding);
    glBindVertexArray(vaobj);
    glVertexAttribDivisor(bindingindex, divisor);
    glBindVertexArray(vertex_array_binding);
}

static void glCreateSamplers(int n, unsigned * samplers) {
    int active_texture = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
    glActiveTexture(GL_TEXTURE0);
    int sampler_binding = 0;
    glGetIntegerv(GL_SAMPLER_BINDING, &sampler_binding);
    glGenSamplers(n, samplers);
    for (int i = 0; i < n; ++i) {
        glBindSampler(0, samplers[i]);
    }
    glBindSampler(0, sampler_binding);
    glActiveTexture(active_texture);
}

static void * resolve_opengl_function(const char * name) {
    if (!strcmp(name, "glCullFace")) return (void *)glCullFace;
    if (!strcmp(name, "glDepthMask")) return (void *)glDepthMask;
    if (!strcmp(name, "glDisable")) return (void *)glDisable;
    if (!strcmp(name, "glEnable")) return (void *)glEnable;
    if (!strcmp(name, "glFlush")) return (void *)glFlush;
    if (!strcmp(name, "glDepthFunc")) return (void *)glDepthFunc;
    if (!strcmp(name, "glReadPixels")) return (void *)glReadPixels;
    if (!strcmp(name, "glGetError")) return (void *)glGetError;
    if (!strcmp(name, "glGetIntegerv")) return (void *)glGetIntegerv;
    if (!strcmp(name, "glGetString")) return (void *)glGetString;
    if (!strcmp(name, "glViewport")) return (void *)glViewport;
    if (!strcmp(name, "glDeleteTextures")) return (void *)glDeleteTextures;
    if (!strcmp(name, "glGenQueries")) return (void *)glGenQueries;
    if (!strcmp(name, "glDeleteQueries")) return (void *)glDeleteQueries;
    if (!strcmp(name, "glBeginQuery")) return (void *)glBeginQuery;
    if (!strcmp(name, "glEndQuery")) return (void *)glEndQuery;
    if (!strcmp(name, "glGetQueryObjectuiv")) return (void *)glGetQueryObjectuiv;
    if (!strcmp(name, "glBindBuffer")) return (void *)glBindBuffer;
    if (!strcmp(name, "glDeleteBuffers")) return (void *)glDeleteBuffers;
    if (!strcmp(name, "glStencilOpSeparate")) return (void *)glStencilOpSeparate;
    if (!strcmp(name, "glStencilFuncSeparate")) return (void *)glStencilFuncSeparate;
    if (!strcmp(name, "glStencilMaskSeparate")) return (void *)glStencilMaskSeparate;
    if (!strcmp(name, "glAttachShader")) return (void *)glAttachShader;
    if (!strcmp(name, "glCompileShader")) return (void *)glCompileShader;
    if (!strcmp(name, "glCreateProgram")) return (void *)glCreateProgram;
    if (!strcmp(name, "glCreateShader")) return (void *)glCreateShader;
    if (!strcmp(name, "glDeleteProgram")) return (void *)glDeleteProgram;
    if (!strcmp(name, "glDeleteShader")) return (void *)glDeleteShader;
    if (!strcmp(name, "glGetProgramiv")) return (void *)glGetProgramiv;
    if (!strcmp(name, "glGetProgramInfoLog")) return (void *)glGetProgramInfoLog;
    if (!strcmp(name, "glGetShaderiv")) return (void *)glGetShaderiv;
    if (!strcmp(name, "glGetShaderInfoLog")) return (void *)glGetShaderInfoLog;
    if (!strcmp(name, "glGetUniformiv")) return (void *)glGetUniformiv;
    if (!strcmp(name, "glLinkProgram")) return (void *)glLinkProgram;
    if (!strcmp(name, "glShaderSource")) return (void *)glShaderSource;
    if (!strcmp(name, "glUseProgram")) return (void *)glUseProgram;
    if (!strcmp(name, "glEnablei")) return (void *)glEnablei;
    if (!strcmp(name, "glDisablei")) return (void *)glDisablei;
    if (!strcmp(name, "glClearBufferiv")) return (void *)glClearBufferiv;
    if (!strcmp(name, "glClearBufferuiv")) return (void *)glClearBufferuiv;
    if (!strcmp(name, "glClearBufferfv")) return (void *)glClearBufferfv;
    if (!strcmp(name, "glClearBufferfi")) return (void *)glClearBufferfi;
    if (!strcmp(name, "glDeleteRenderbuffers")) return (void *)glDeleteRenderbuffers;
    if (!strcmp(name, "glBindFramebuffer")) return (void *)glBindFramebuffer;
    if (!strcmp(name, "glDeleteFramebuffers")) return (void *)glDeleteFramebuffers;
    if (!strcmp(name, "glBlitFramebuffer")) return (void *)glBlitFramebuffer;
    if (!strcmp(name, "glBindVertexArray")) return (void *)glBindVertexArray;
    if (!strcmp(name, "glDeleteVertexArrays")) return (void *)glDeleteVertexArrays;
    if (!strcmp(name, "glDrawArraysInstanced")) return (void *)glDrawArraysInstanced;
    if (!strcmp(name, "glDrawElementsInstanced")) return (void *)glDrawElementsInstanced;
    if (!strcmp(name, "glFenceSync")) return (void *)glFenceSync;
    if (!strcmp(name, "glDeleteSync")) return (void *)glDeleteSync;
    if (!strcmp(name, "glClientWaitSync")) return (void *)glClientWaitSync;
    if (!strcmp(name, "glDeleteSamplers")) return (void *)glDeleteSamplers;
    if (!strcmp(name, "glSamplerParameteri")) return (void *)glSamplerParameteri;
    if (!strcmp(name, "glSamplerParameterf")) return (void *)glSamplerParameterf;
    if (!strcmp(name, "glBlendEquationSeparatei")) return (void *)glBlendEquationSeparatei;
    if (!strcmp(name, "glBlendFunci")) return (void *)glBlendFunci;
    if (!strcmp(name, "glBlendFuncSeparatei")) return (void *)glBlendFuncSeparatei;
    if (!strcmp(name, "glProgramUniform1i")) return (void *)glProgramUniform1i;
    if (!strcmp(name, "glProgramUniform1iv")) return (void *)glProgramUniform1iv;
    if (!strcmp(name, "glProgramUniform1fv")) return (void *)glProgramUniform1fv;
    if (!strcmp(name, "glProgramUniform1uiv")) return (void *)glProgramUniform1uiv;
    if (!strcmp(name, "glProgramUniform2iv")) return (void *)glProgramUniform2iv;
    if (!strcmp(name, "glProgramUniform2fv")) return (void *)glProgramUniform2fv;
    if (!strcmp(name, "glProgramUniform2uiv")) return (void *)glProgramUniform2uiv;
    if (!strcmp(name, "glProgramUniform3iv")) return (void *)glProgramUniform3iv;
    if (!strcmp(name, "glProgramUniform3fv")) return (void *)glProgramUniform3fv;
    if (!strcmp(name, "glProgramUniform3uiv")) return (void *)glProgramUniform3uiv;
    if (!strcmp(name, "glProgramUniform4iv")) return (void *)glProgramUniform4iv;
    if (!strcmp(name, "glProgramUniform4fv")) return (void *)glProgramUniform4fv;
    if (!strcmp(name, "glProgramUniform4uiv")) return (void *)glProgramUniform4uiv;
    if (!strcmp(name, "glProgramUniformMatrix2fv")) return (void *)glProgramUniformMatrix2fv;
    if (!strcmp(name, "glProgramUniformMatrix3fv")) return (void *)glProgramUniformMatrix3fv;
    if (!strcmp(name, "glProgramUniformMatrix4fv")) return (void *)glProgramUniformMatrix4fv;
    if (!strcmp(name, "glProgramUniformMatrix2x3fv")) return (void *)glProgramUniformMatrix2x3fv;
    if (!strcmp(name, "glProgramUniformMatrix3x2fv")) return (void *)glProgramUniformMatrix3x2fv;
    if (!strcmp(name, "glProgramUniformMatrix2x4fv")) return (void *)glProgramUniformMatrix2x4fv;
    if (!strcmp(name, "glProgramUniformMatrix4x2fv")) return (void *)glProgramUniformMatrix4x2fv;
    if (!strcmp(name, "glProgramUniformMatrix3x4fv")) return (void *)glProgramUniformMatrix3x4fv;
    if (!strcmp(name, "glProgramUniformMatrix4x3fv")) return (void *)glProgramUniformMatrix4x3fv;
    if (!strcmp(name, "glMemoryBarrier")) return (void *)glMemoryBarrier;
    if (!strcmp(name, "glDispatchCompute")) return (void *)glDispatchCompute;
    if (!strcmp(name, "glMultiDrawArraysIndirect")) return (void *)glMultiDrawArraysIndirect;
    if (!strcmp(name, "glMultiDrawElementsIndirect")) return (void *)glMultiDrawElementsIndirect;
    if (!strcmp(name, "glGetProgramInterfaceiv")) return (void *)glGetProgramInterfaceiv;
    if (!strcmp(name, "glGetProgramResourceName")) return (void *)glGetProgramResourceName;
    if (!strcmp(name, "glGetProgramResourceiv")) return (void *)glGetProgramResourceiv;
    if (!strcmp(name, "glBindBuffersRange")) return (void *)glBindBuffersRange;
    if (!strcmp(name, "glBindTextures")) return (void *)glBindTextures;
    if (!strcmp(name, "glBindSamplers")) return (void *)glBindSamplers;
    if (!strcmp(name, "glBindImageTextures")) return (void *)glBindImageTextures;
    if (!strcmp(name, "glCreateBuffers")) return (void *)glCreateBuffers;
    if (!strcmp(name, "glNamedBufferStorage")) return (void *)glNamedBufferStorage;
    if (!strcmp(name, "glNamedBufferSubData")) return (void *)glNamedBufferSubData;
    if (!strcmp(name, "glMapNamedBufferRange")) return (void *)glMapNamedBufferRange;
    if (!strcmp(name, "glUnmapNamedBuffer")) return (void *)glUnmapNamedBuffer;
    if (!strcmp(name, "glGetNamedBufferSubData")) return (void *)glGetNamedBufferSubData;
    if (!strcmp(name, "glCreateFramebuffers")) return (void *)glCreateFramebuffers;
    if (!strcmp(name, "glNamedFramebufferRenderbuffer")) return (void *)glNamedFramebufferRenderbuffer;
    if (!strcmp(name, "glNamedFramebufferParameteri")) return (void *)glNamedFramebufferParameteri;
    if (!strcmp(name, "glNamedFramebufferTexture")) return (void *)glNamedFramebufferTexture;
    if (!strcmp(name, "glNamedFramebufferTextureLayer")) return (void *)glNamedFramebufferTextureLayer;
    if (!strcmp(name, "glNamedFramebufferDrawBuffers")) return (void *)glNamedFramebufferDrawBuffers;
    if (!strcmp(name, "glNamedFramebufferReadBuffer")) return (void *)glNamedFramebufferReadBuffer;
    if (!strcmp(name, "glCreateRenderbuffers")) return (void *)glCreateRenderbuffers;
    if (!strcmp(name, "glNamedRenderbufferStorageMultisample")) return (void *)glNamedRenderbufferStorageMultisample;
    if (!strcmp(name, "glCreateTextures")) return (void *)glCreateTextures;
    if (!strcmp(name, "glTextureStorage2D")) return (void *)glTextureStorage2D;
    if (!strcmp(name, "glTextureStorage3D")) return (void *)glTextureStorage3D;
    if (!strcmp(name, "glTextureSubImage2D")) return (void *)glTextureSubImage2D;
    if (!strcmp(name, "glTextureSubImage3D")) return (void *)glTextureSubImage3D;
    if (!strcmp(name, "glTextureParameteri")) return (void *)glTextureParameteri;
    if (!strcmp(name, "glGenerateTextureMipmap")) return (void *)glGenerateTextureMipmap;
    if (!strcmp(name, "glCreateVertexArrays")) return (void *)glCreateVertexArrays;
    if (!strcmp(name, "glEnableVertexArrayAttrib")) return (void *)glEnableVertexArrayAttrib;
    if (!strcmp(name, "glVertexArrayElementBuffer")) return (void *)glVertexArrayElementBuffer;
    if (!strcmp(name, "glVertexArrayVertexBuffer")) return (void *)glVertexArrayVertexBuffer;
    if (!strcmp(name, "glVertexArrayAttribFormat")) return (void *)glVertexArrayAttribFormat;
    if (!strcmp(name, "glVertexArrayAttribIFormat")) return (void *)glVertexArrayAttribIFormat;
    if (!strcmp(name, "glVertexArrayBindingDivisor")) return (void *)glVertexArrayBindingDivisor;
    if (!strcmp(name, "glCreateSamplers")) return (void *)glCreateSamplers;
}

static PyObject * meth_load_opengl_function(PyObject * self, PyObject * args, PyObject * kwargs) {
    const char * keywords[] = {"name", NULL};

    const char * name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", (char **)keywords, &name)) {
        return NULL;
    }

    return PyLong_FromVoidPtr((void *)resolve_opengl_function(name));
}

static PyMethodDef module_methods[] = {
    {"load_opengl_function", (PyCFunction)meth_load_opengl_function, METH_VARARGS | METH_KEYWORDS, NULL},
    {},
};

static PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "glcontext_emscriptem", NULL, -1, module_methods};

extern "C" PyObject * PyInit_glcontext_emscriptem() {
    PyObject * module = PyModule_Create(&module_def);
    return module;
}
