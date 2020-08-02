/*
 * @Author: your name
 * @Date: 2020-08-02 11:07:26
 * @LastEditTime: 2020-08-02 13:20:44
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \vs_code\Nv12Render_Gpu\videorender.h
 */ 
#ifndef VIDEORENDER_H
#define VIDEORENDER_H

class VideoRender
{
public:
    virtual ~VideoRender(){}
    virtual void initialize(const int width, const int height, bool horizontal = false, bool vertical = false) = 0;
    virtual void render(unsigned char*,int,int) = 0;
};

#endif // VIDEORENDER_H
