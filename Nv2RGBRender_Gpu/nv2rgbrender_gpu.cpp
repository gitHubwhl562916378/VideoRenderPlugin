#include <QMutex>
#include "ColorSpace.h"
#include "nv2rgbrender_gpu.h"

inline bool check(int e, int iLine, const char *szFile)
{
	if (e != 0)
	{
		qDebug() << "General error " << e << " at line " << iLine << " in file " << szFile;
		return false;
	}
	return true;
}

#define ck(call) check(call, __LINE__, __FILE__)

Nv2RGBRender_Gpu::Nv2RGBRender_Gpu(CUcontext ctx) : context(ctx)
{
	qDebug() << "Nv2RGBRender_Gpu::Nv2RGBRender_Gpu context: " << reinterpret_cast<unsigned long long>(context);
	if (!context)
	{
		ck(cuInit(0));
		CUdevice cuDevice;
		ck(cuDeviceGet(&cuDevice, 0));
		char szDeviceName[80];
		ck(cuDeviceGetName(szDeviceName, sizeof(szDeviceName), cuDevice));
		qDebug() << "GPU in use: " << szDeviceName;
		ck(cuCtxCreate(&context, CU_CTX_SCHED_BLOCKING_SYNC, cuDevice));
		need_destroy_ = true;
	}
}

Nv2RGBRender_Gpu::~Nv2RGBRender_Gpu()
{
	qDebug() << "Nv2RGBRender_Gpu::~Nv2RGBRender_Gpu() in";
	ck(cuGraphicsUnregisterResource(cuda_tex_resource));
	if (need_destroy_)
	{
		ck(cuCtxDestroy(context));
		qDebug() << "Nv2RGBRender_Gpu::~Nv2RGBRender_Gpu() context destroy" << reinterpret_cast<unsigned long long>(context);
	}
	else
	{
		qDebug() << "Nv2RGBRender_Gpu::~Nv2RGBRender_Gpu() context from out";
	}

	if (d_nv12_ptr)
	{
		ck(cuMemFree(d_nv12_ptr));
	}
	vbo.destroy();
	glDeleteTextures(1, &texId);
	glDeleteBuffers(1, &tex_bufferId);
	qDebug() << "Nv2RGBRender_Gpu::~Nv2RGBRender_Gpu() out";
}

Q_GLOBAL_STATIC(QMutex, initMutex)
void Nv2RGBRender_Gpu::initialize(const int width, const int height, const bool horizontal, const bool vertical)
{
	initializeOpenGLFunctions();

	QMutexLocker initLock(initMutex());
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
		"varying vec4 textureOut;\n"
		"uniform sampler2D uTexture;\n"
		"void main(void)\n"
		"{\n"
		"gl_FragColor = texture2D(uTexture, textureOut.st); \n"
		"}\n";

	program.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
	program.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
	program.link();

	if (horizontal)
	{
		if (vertical)
		{
			GLfloat points[]{
				-1.0f, 1.0f,
				1.0f, 1.0f,
				1.0f, -1.0f,
				-1.0f, -1.0f,

				1.0f, 1.0f,
				0.0f, 1.0f,
				0.0f, 0.0f,
				1.0f, 0.0f };

			vbo.create();
			vbo.bind();
			vbo.allocate(points, sizeof(points));
		}
		else
		{
			GLfloat points[]{
				-1.0f, 1.0f,
				1.0f, 1.0f,
				1.0f, -1.0f,
				-1.0f, -1.0f,

				1.0f, 0.0f,
				0.0f, 0.0f,
				0.0f, 1.0f,
				1.0f, 1.0f };

			vbo.create();
			vbo.bind();
			vbo.allocate(points, sizeof(points));
		}
	}
	else
	{
		if (vertical)
		{
			GLfloat points[]{
				-1.0f, 1.0f,
				1.0f, 1.0f,
				1.0f, -1.0f,
				-1.0f, -1.0f,

				0.0f, 1.0f,
				1.0f, 1.0f,
				1.0f, 0.0f,
				0.0f, 0.0f };

			vbo.create();
			vbo.bind();
			vbo.allocate(points, sizeof(points));
		}
		else
		{
			GLfloat points[]{
				-1.0f, 1.0f,
				1.0f, 1.0f,
				1.0f, -1.0f,
				-1.0f, -1.0f,

				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,
				0.0f, 1.0f };

			vbo.create();
			vbo.bind();
			vbo.allocate(points, sizeof(points));
		}
	}

	glGenTextures(1, &texId);

	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr); //前一个RGBA是显示所需的格式，经测试RGBA可以，其它未知;后一个需要通道数相同的数据格式，此例是BGRA
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenBuffers(1, &tex_bufferId);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, tex_bufferId);
	glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, width * height * 4 * sizeof(char), nullptr, GL_STREAM_DRAW_ARB);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	glDisable(GL_DEPTH_TEST); //�򿪻��ڴ��ڴ�С�仯ʱ��������opengl������ʵ������Ҳ����Ҫ�򿪡�

	ck(cuCtxSetCurrent(context));
	ck(cuGraphicsGLRegisterBuffer(&cuda_tex_resource, tex_bufferId, CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD));
}

void Nv2RGBRender_Gpu::render(unsigned char *nv12_dPtr, const int width, const int height)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //�򿪻��ں�����������Ƶ֮������
	if (!nv12_dPtr)
	{
		return;
	}

	ck(cuCtxSetCurrent(context));
	CUdeviceptr d_tex_buffer;
	size_t d_tex_size;
	ck(cuGraphicsMapResources(1, &cuda_tex_resource, 0));
	ck(cuGraphicsResourceGetMappedPointer(&d_tex_buffer, &d_tex_size, cuda_tex_resource));
	Nv12ToColor32<BGRA32>(reinterpret_cast<uint8_t *>(nv12_dPtr), width, reinterpret_cast<uint8_t *>(d_tex_buffer), d_tex_size / height, width, height, 5);
	ck(cuGraphicsUnmapResources(1, &cuda_tex_resource, 0));
	ck(cuCtxSetCurrent(nullptr));

	program.bind();
	vbo.bind();
	program.enableAttributeArray("vertexIn");
	program.enableAttributeArray("textureIn");
	program.setAttributeBuffer("vertexIn", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
	program.setAttributeBuffer("textureIn", GL_FLOAT, 2 * 4 * sizeof(GLfloat), 2, 2 * sizeof(GLfloat));

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, tex_bufferId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	program.setUniformValue("uTexture", 0);
	glDrawArrays(GL_QUADS, 0, 4);
	program.disableAttributeArray("vertexIn");
	program.disableAttributeArray("textureIn");
	vbo.release();
	program.release();
}

void Nv2RGBRender_Gpu::render(unsigned char *planr[], int line_size[], const int width, const int height)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!planr)
	{
		return;
	}

	ck(cuCtxSetCurrent(context));
	if (!d_nv12_ptr)
	{
		ck(cuMemAlloc(&d_nv12_ptr, width * height * sizeof(uint8_t) * 3 / 2));
	}

	//拷贝y分量
	CUDA_MEMCPY2D m = { 0 };
	m.srcMemoryType = CU_MEMORYTYPE_DEVICE;
	m.srcDevice = reinterpret_cast<CUdeviceptr>(planr[0]);
	m.srcPitch = line_size[0];
	m.dstMemoryType = CU_MEMORYTYPE_DEVICE;
	m.dstDevice = d_nv12_ptr;
	m.dstPitch = width;
	m.WidthInBytes = width;
	m.Height = height;
	ck(cuMemcpy2DAsync(&m, 0));

	//拷贝uv分量
	m.srcMemoryType = CU_MEMORYTYPE_DEVICE;
	m.srcDevice = reinterpret_cast<CUdeviceptr>(planr[1]);
	m.srcPitch = line_size[1];
	m.dstMemoryType = CU_MEMORYTYPE_DEVICE;
	m.dstDevice = d_nv12_ptr + width * height;
	m.dstPitch = width;
	m.WidthInBytes = width;
	m.Height = height / 2;
	ck(cuMemcpy2DAsync(&m, 0));

	render(reinterpret_cast<unsigned char*>(d_nv12_ptr), width, height);
}

VideoRender *createRender(void *ctx)
{
	return new Nv2RGBRender_Gpu((CUcontext)ctx);
}