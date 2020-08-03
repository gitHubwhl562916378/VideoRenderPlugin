/*
 * @Author: whl
 * @Date: 2020-08-02 11:06:45
 * @LastEditTime: 2020-08-02 23:45:09
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \vs_code\Nv12Render_Gpu\nv12render_gpu.h
 */ 
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <cuda_gl_interop.h>
#include "videorender.h"

class Nv12Render_Gpu : public QOpenGLFunctions, public VideoRender
{
public:
    Nv12Render_Gpu();
    ~Nv12Render_Gpu();
    /**
     * @description: 初始化渲染器
     * @param {width} 视频宽度 
     * @param {height} 视频宽度 
     * @param {horizontal} 是否水平镜像 
     * @param {vertical} 是否垂直镜像 
     */
    void initialize(const int width, const int height, bool horizontal = false, bool vertical = false) override;
    /**
     * @description: 渲染一帧数据
     * @param {nv12_dPtr} 设备指针
     * @param {w} 帧宽度
     * @param {h} 帧高度
     */    
    void render(unsigned char* nv12_dPtr,int w,int h) override;

private:
    CUgraphicsResource cuda_tex_resource[2];
    QOpenGLShaderProgram program;
    GLuint idY,idUV, textures[2];
    GLuint ybuffer_id, uvbuffer_id, tex_buffers[2];
    QOpenGLBuffer vbo;
};
