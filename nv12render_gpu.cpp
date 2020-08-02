/*
 * @Author: your name
 * @Date: 2020-08-02 11:10:34
 * @LastEditTime: 2020-08-02 23:46:45
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \vs_code\Nv12Render_Gpu\nv12render_gpu.cpp
 */ 
#include <cuda_runtime.h>
#include <cudaGL.h>
#include "nv12render_gpu.h"

Nv12Render_Gpu::~Nv12Render_Gpu()
{
    vbo.destroy();
    glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);
    glDeleteBuffers(sizeof(tex_buffers)/sizeof(GLuint), tex_buffers);
}

void Nv12Render_Gpu::initialize(const int width, const int height, bool horizontal = false, bool vertical = false)
{
    initializeOpenGLFunctions();
    const char *vsrc =
            "attribute vec4 vertexIn; \
             attribute vec4 textureIn; \
             varying vec4 textureOut;  \
             void main(void)           \
             {                         \
                 gl_Position = vertexIn; \
                 textureOut = textureIn; \
             }";

    const char *fsrc =
            "varying mediump vec4 textureOut;\n"
            "uniform sampler2D textureY;\n"
            "uniform sampler2D textureUV;\n"
            "void main(void)\n"
            "{\n"
            "vec3 yuv; \n"
            "vec3 rgb; \n"
            "yuv.x = texture2D(textureY, textureOut.st).r - 0.0625; \n"
            "yuv.y = texture2D(textureUV, textureOut.st).r - 0.5; \n"
            "yuv.z = texture2D(textureUV, textureOut.st).g - 0.5; \n"
            "rgb = mat3( 1,       1,         1, \n"
                        "0,       -0.39465,  2.03211, \n"
                        "1.13983, -0.58060,  0) * yuv; \n"
            "gl_FragColor = vec4(rgb, 1); \n"
            "}\n";

    program.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,vsrc);
    program.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,fsrc);
    program.link();

    if(horizontal){
        if(vertical){
            GLfloat points[]{
                -1.0f, 1.0f,
                 1.0f, 1.0f,
                 1.0f, -1.0f,
                -1.0f, -1.0f,

                1.0f,1.0f,
                0.0f,1.0f,
                0.0f,0.0f,
                1.0f,0.0f
            };

            vbo.create();
            vbo.bind();
            vbo.allocate(points,sizeof(points));
        }else{
            GLfloat points[]{
                -1.0f, 1.0f,
                 1.0f, 1.0f,
                 1.0f, -1.0f,
                -1.0f, -1.0f,

                1.0f,0.0f,
                0.0f,0.0f,
                0.0f,1.0f,
                1.0f,1.0f
            };

            vbo.create();
            vbo.bind();
            vbo.allocate(points,sizeof(points));
        }
    }else{
        if(vertical){
            GLfloat points[]{
                -1.0f, 1.0f,
                 1.0f, 1.0f,
                 1.0f, -1.0f,
                -1.0f, -1.0f,

                0.0f,1.0f,
                1.0f,1.0f,
                1.0f,0.0f,
                0.0f,0.0f
            };

            vbo.create();
            vbo.bind();
            vbo.allocate(points,sizeof(points));
        }else{
            GLfloat points[]{
                -1.0f, 1.0f,
                 1.0f, 1.0f,
                 1.0f, -1.0f,
                -1.0f, -1.0f,

                0.0f,0.0f,
                1.0f,0.0f,
                1.0f,1.0f,
                0.0f,1.0f
            };

            vbo.create();
            vbo.bind();
            vbo.allocate(points,sizeof(points));
        }
    }

    GLuint id[2];
    glGenTextures(2,id);
    idY = id[0];
    idUV = id[1];
    std::copy(std::begin(id),std::end(id),std::begin(textures));
    
    glBindTexture(GL_TEXTURE_2D,idY);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width,height,0,GL_RED,GL_UNSIGNED_BYTE,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glBindTexture(GL_TEXTURE_2D,idUV);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RG,width >> 1,height >> 1,0,GL_RG,GL_UNSIGNED_BYTE,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glGenBuffers(2, tex_buffers);
    ybuffer_id = tex_buffers[0];
    uvbuffer_id = tex_buffers[1];

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, ybuffer_id);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, width * height * sizeof(char), nullptr, GL_STREAM_DRAW_ARB);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, uvbuffer_id);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, width * height* sizeof(char) / 2, nullptr, GL_STREAM_DRAW_ARB);
    // cudaGraphicsGLRegisterImage(&cuda_tex_resource, tex_id, GL_TEXTURE_2D, cudaGraphicsMapFlagsWriteDiscard);
    cuGraphicsGLRegisterBuffer(cuda_tex_resource, 2, CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD);
}

void Nv12Render_Gpu::render(unsigned char* nv12_dPtr,int w,int h)
{
    if(!nv12_dPtr)
    {
        return;
    }

    // cudaGraphicsMapResources(1,&cuda_tex_resource,0);
    cuGraphicsMapResources(2, cuda_tex_resource, 0);
    CUdeviceptr dpBackBuffers[2];
    size_t buffer_size[2];
    cuGraphicsResourceGetMappedPointer(dpBackBuffers, buffer_size, cuda_tex_resource);
    CUdeviceptr d_ybuffer = dpBackBuffers[ybuffer_id];
    size_t d_y_size = buffer_size[ybuffer_id];
    CUdeviceptr d_uvbuffer = dpBackBuffers[uvbuffer_id];
    size_t d_uv_size = buffer_size[uvbuffer_id];

    CUDA_MEMCPY2D m = { 0 };
    m.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    m.srcDevice = nv12_dPtr;
    m.srcPitch = w;
    m.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    m.dstDevice = d_ybuffer;
    m.dstPitch = d_y_size / h;
    m.WidthInBytes = w;
    m.Height = h;
    cuMemcpy2DAsync(&m, 0);

    m.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    m.srcDevice = nv12_dPtr + w * h;
    m.srcPitch = w;
    m.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    m.dstDevice = d_uvbuffer;
    m.dstPitch = d_uv_size / (h>>1);
    m.WidthInBytes = w;
    m.Height = (h>>1);
    cuMemcpy2DAsync(&m, 0);

    cuGraphicsUnmapResources(2, cuda_tex_resource, 0);
    cuGraphicsUnregisterResource(cuda_tex_resource);

    program.bind();
    vbo.bind();
    program.enableAttributeArray("vertexIn");
    program.enableAttributeArray("textureIn");
    program.setAttributeBuffer("vertexIn",GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
    program.setAttributeBuffer("textureIn",GL_FLOAT,2 * 4 * sizeof(GLfloat),2,2*sizeof(GLfloat));
    
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, ybuffer_id);
    glBindTexture(GL_TEXTURE_2D,idY);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, uvbuffer_id);
    glBindTexture(GL_TEXTURE_2D,idUV);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w >> 1,h >> 1, GL_RG, GL_UNSIGNED_BYTE, nullptr);

    program.setUniformValue("textureY",1);
    program.setUniformValue("textureUV",0);
    glDrawArrays(GL_QUADS,0,4);
    program.disableAttributeArray("vertexIn");
    program.disableAttributeArray("textureIn");
    vbo.release();
    program.release();
}