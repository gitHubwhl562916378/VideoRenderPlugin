/*
 * @Author: your name
 * @Date: 2020-08-02 11:07:26
 * @LastEditTime: 2020-08-15 14:44:17
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \vs_code\Nv12Render_Gpu\videorender.h
 */ 
#ifndef VIDEORENDER_H
#define VIDEORENDER_H

#ifdef linux
#  define VIDEORENDERSHARED_EXPORT __attribute__((visibility("default")))
#else
#  define VIDEORENDERSHARED_EXPORT __declspec(dllexport)
#endif

class VIDEORENDERSHARED_EXPORT VideoRender
{
public:
    virtual ~VideoRender(){}
    /**
     * @description: 初始化opengl上下文，编译链接shader;如果是GPU直接与OOPENGL对接数据，则会分配GPU内存或注册资�?
     * @param width 视频宽度
     * @param height 视频高度
     * @param horizontal 是否水平镜像
     * @param vertical 是否垂直镜像
     */
    virtual void initialize(const int width, const int height, const bool horizontal = false, const bool vertical = false) = 0;
    /**
     * @description: 渲染一帧数据，buffer需要为连续空间
     * @param buffer 内存地址
     * @param width 视频帧宽�?
     * @param height 视频帧高�?
     */
    virtual void render(unsigned char* buffer, const int width, const int height) = 0;
    /**
     * @description: 渲染一帧分离在多个planr的数�?
     * @param planr 多个平面地址的指针数组。按照默认格式排序，如YUV�?0(Y分量)�?1(U分量)�?2(V分量); NV12�?0(Y分量)�?1(UV分量)
     * @param line_size 二维图片的每行字节大小，也是GPU内存的nPitch
     * @param width 视频帧宽�?
     * @param height 视频帧高�?
     */
    virtual void render(unsigned char* planr[], int line_size[], const int width, const int height) = 0;
    /**
     * @description: �첽�������ݵ�����
     * @param buffer �����ڴ��ַ
     * @param width ��Ƶ֡����
     * @param height ��Ƶ֡�߶�
     */
    virtual void upLoad(unsigned char* buffer, const int width, const int height) = 0;
    /**
     * @description: �첽������һ����ɢ�ڶ��planr�����ݵ�����
     * @param planr ���ƽ���ַ��ָ�����顣����Ĭ�ϸ�ʽ������YUVΪ0(Y����)��1(U����)��2(V����); NV12Ϊ0(Y����)��1(UV����)
     * @param line_size ��άͼƬ��ÿ���ֽڴ�С��Ҳ��GPU�ڴ��nPitch
     * @param width ��Ƶ֡����
     * @param height ��Ƶ֡�߶�
     */
    virtual void upLoad(unsigned char* planr[], const int line_size[], const int width, const int height) = 0;
    /**
     * @description: �첽������������
     */
    virtual void draw() = 0;
};

extern "C"
{
    VIDEORENDERSHARED_EXPORT VideoRender* createRender(void *ctx);
}
#endif // VIDEORENDER_H
