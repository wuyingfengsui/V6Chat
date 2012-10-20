#include "stdafx.h"
extern "C"
{
	#include "libjpeg\jpeglib.h"
}
#include "libpng\png.h"

#include "V6Util.h"
#include "V6ChatDef.h"

#include <vector>
#include <string>
#include <fstream>

#include <Shlwapi.h>
#include <Commdlg.h>

using namespace std;

wstring V6Util::Utf8ToUnicode(const char* buf)
{
	int   nwLen = ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
	if (nwLen == 0) return L"";

	vector<wchar_t> uBuf(nwLen);
	::MultiByteToWideChar(CP_UTF8, 0, buf, -1, &uBuf[0], nwLen);

	return &uBuf[0];
}

string V6Util::UnicodeToUtf8(const wchar_t* buf)
{
	int len = ::WideCharToMultiByte(CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL);
    if (len == 0) return "";

	vector<char> utf8(len);
    ::WideCharToMultiByte(CP_UTF8, 0, buf, -1, &utf8[0], len, NULL, NULL);

    return &utf8[0];
}

string  V6Util::UnicodeToAnsi(const wchar_t* buf)
{
	int len = ::WideCharToMultiByte(CP_ACP, 0, buf, -1, NULL, 0, NULL, NULL);
    if (len == 0) return "";

    std::vector<char> utf8(len);
    ::WideCharToMultiByte(CP_ACP, 0, buf, -1, &utf8[0], len, NULL, NULL);

    return &utf8[0];
}

wstring V6Util::AnsiToUnicode(const char* buf)
{
	int len = ::MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
    if (len == 0) return L"";

    std::vector<wchar_t> unicode(len);
    ::MultiByteToWideChar(CP_ACP, 0, buf, -1, &unicode[0], len);

    return &unicode[0];
}

std::string V6Util::AnsiToUtf8(const char *buf)
{
	int nlen = ::MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
	if (nlen == 0) return "";
	std::vector<wchar_t> unicode(nlen);
	::MultiByteToWideChar(CP_ACP, 0, buf, -1, &unicode[0], nlen);

	nlen = ::WideCharToMultiByte(CP_UTF8, 0, &unicode[0], -1, NULL, 0, NULL, NULL);
	std::vector<char> utf8(nlen);
	::WideCharToMultiByte(CP_UTF8, 0, &unicode[0], -1, &utf8[0], nlen, NULL, NULL);
	return &utf8[0];
}

std::string V6Util::Utf8ToAnsi(const char *buf)
{
	int   nLen = ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
	if (nLen == 0) return "";
	std::vector<wchar_t> uBuf(nLen);
	::MultiByteToWideChar(CP_UTF8, 0, buf, -1, &uBuf[0], nLen);

	nLen = ::WideCharToMultiByte(CP_ACP, 0, &uBuf[0], -1, NULL, 0, NULL, NULL);
	std::vector<char> utf8(nLen);
	::WideCharToMultiByte(CP_ACP, 0, &uBuf[0], -1, &utf8[0], nLen, NULL, NULL);
	return &utf8[0];
}

void V6Util::V6Log(const char * fmt, ...)
{
	static const int LOG_LEN = 256;
    char buffer[LOG_LEN] = "";

   va_list ap;
   va_start( ap, fmt );
   vsnprintf_s( buffer, LOG_LEN-1, fmt, ap );
   va_end(ap);

   SYSTEMTIME now_time;
   ::GetLocalTime(&now_time);

   char szModulePath[MAX_PATH];
   ::GetModuleFileNameA(NULL,szModulePath,MAX_PATH);
   ::PathAppendA(szModulePath, "..\\log.txt");
   FILE *m_pFile = NULL;
   fopen_s(&m_pFile, szModulePath, "a+");

   if ( NULL != m_pFile )
   {
	   fprintf(m_pFile, "%4d-%2d-%2d    %2d:%2d:%2d    %s\n", now_time.wYear,now_time.wMonth,now_time.wDay,now_time.wHour,now_time.wMinute,now_time.wSecond, buffer);
	   fclose(m_pFile);
   }
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

struct mem_encode
{
	char *buffer;
	size_t size;
};

void my_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	/* with libpng15 next line causes pointer deference error; use libpng12 */
	struct mem_encode* p=(struct mem_encode*)png_ptr->io_ptr;
	size_t nsize = p->size + length;

	/* allocate or grow buffer */
	if(p->buffer)
		p->buffer = (char*)realloc(p->buffer, nsize);
	else
		p->buffer = (char*)malloc(nsize);

	if(!p->buffer)
		png_error(png_ptr, "Write Error");

	/* copy new bytes to end of buffer */
	memcpy(p->buffer + p->size, data, length);
	p->size += length;
}

void *V6Util::GetImageHandle(const wchar_t* path)
{
	// 先尝试直接按png格式打开
	void* imgHandle = NULL;
	if(imgHandle = XL_LoadBitmapFromFile(path, 16))
		return imgHandle;

	// 失败的话尝试转码显示，暂时只考虑jpeg
	FILE *jpegfp;

	if ((jpegfp = fopen(UnicodeToAnsi(path).c_str(), "rb")) == NULL) {
		return NULL;
	}

	jpeg_decompress_struct cinfo;
	my_error_mgr jerr;
	JSAMPARRAY buffer;
	int row_stride;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	// 向libjpeg注册了一个my_error_exit回调函数，当发生错误的时候，该回调函数将会被调用。
	// 然后我们调用setjmp函数，设置一个返回点。
	// 这样我们在回调函数处理完错误信息之后，就可以调用longjmp函数返回到这里，在这个返回点进行资源的释放（非常重要，否则将会内存泄漏）
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/
		jpeg_destroy_decompress(&cinfo);
		fclose(jpegfp);
		return NULL;
	}
	
	jpeg_create_decompress(&cinfo);

	/* 提供一个文件作为输入数据的接口 */
	jpeg_stdio_src(&cinfo, jpegfp);

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	
	row_stride = cinfo.output_width * cinfo.output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	// 创建png头
	png_structp png_ptr;
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		jpeg_destroy_decompress(&cinfo);
		fclose(jpegfp);
		return NULL;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
		jpeg_destroy_decompress(&cinfo);
		fclose(jpegfp);
		return NULL;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem writing the file */
		png_destroy_write_struct(&png_ptr, &info_ptr);
		jpeg_destroy_decompress(&cinfo);
		fclose(jpegfp);
		return NULL;
	}

	png_set_IHDR(png_ptr,
                 info_ptr,
                 cinfo.output_width, 
				 cinfo.output_height,
                 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

	// 读取解码数据
	png_byte **row_pointers = (png_byte**)png_malloc(png_ptr, cinfo.output_height * sizeof(png_byte *));;

	int i = 0;
	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		/* Assume put_scanline_someplace wants a pointer and sample count. */
		// put_scanline_someplace(buffer[0], row_stride);
		png_byte* row = (png_byte*)png_malloc(png_ptr, row_stride);
		memcpy(row, buffer[0], row_stride);
		row_pointers[i++] = row;
	}

	struct mem_encode state;
	/* initialise - put this before png_write_png() call */
	state.buffer = NULL;
	state.size = 0;

	/* if my_png_flush() is not needed, change the arg to NULL */
	png_set_write_fn(png_ptr, &state, my_png_write_data, NULL);
	png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	void* pnghandle = XL_LoadBitmapFromMemory(state.buffer, state.size, 16);

	if(state.buffer)
		free(state.buffer);
	for (unsigned int y = 0; y < cinfo.output_height; ++y) {
        png_free(png_ptr, row_pointers[y]);
    }
	png_free(png_ptr, row_pointers);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(jpegfp);

	return pnghandle;
}

void *V6Util::GetUserDataImageHandle(const wchar_t *path)
{
	WCHAR wszModulePath[MAX_PATH];
    GetModuleFileNameW(NULL,wszModulePath,MAX_PATH);
	PathAppend(wszModulePath, USERDATA_RELATIVE_PATH);
	PathAppend(wszModulePath, path);

	if(void* h = GetImageHandle(wszModulePath))
		return h;

	wstring pt = wszModulePath;
	if(wstring::npos == pt.find(L".png"))
		pt += USERHEAD_EXT;
	return XL_LoadBitmapFromFile(pt.c_str(), 16);
}

bool V6Util::SelectFile(const wchar_t* fileType, const wchar_t* lpstrTitle, std::wstring &path)
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH]={0};
  
    ZeroMemory(&ofn,sizeof(ofn)); 
  
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = fileType;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = lpstrTitle;
    ofn.Flags = OFN_EXPLORER;
  
    if (GetOpenFileName(&ofn))
	{
		path = szFile;
		return true;
	}
	else
		return false;
}