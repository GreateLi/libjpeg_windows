#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include <android/log.h>
#include <setjmp.h>
//#include "jpeg_mem.h"
extern "C" {
#include "jpeglib.h"
#include "cdjpeg.h"
}

#define LOG_TAG "jni"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

typedef uint8_t BYTE;

typedef struct my_error_mgr *my_error_ptr;
struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

METHODDEF(void)
my_error_exit(j_common_ptr cinfo) {
    my_error_ptr myerr = (my_error_ptr) cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    LOGE("jpeg_message_table[%d]:%s", myerr->pub.msg_code,
         myerr->pub.jpeg_message_table[myerr->pub.msg_code]);
    longjmp(myerr->setjmp_buffer, 1);
}

//unsigned char * jpeg_compress_scale(const unsigned char * inbuffer,
//                                    unsigned long insize)
//{
//    //变量定义
//    struct jpeg_compress_struct cinfo;
//    struct jpeg_error_mgr jerr;
//    FILE * outfile;
//    JSAMPROW row_pointer;
//    int row_stride;
//    //绑定标准错误处理结构
//    cinfo.err = jpeg_std_error(&jerr);
//    //初始化JPEG对象
//    jpeg_create_compress(&cinfo);
//    //指定目标图像文件
//    if ((outfile = fopen("dest.jpg", "wb")) == NULL)
//    {
//        return;
//    }
//    jpeg_stdio_dest(&cinfo, outfile);
//    //设定压缩参数
//    cinfo.image_width = image_width;
//    cinfo.image_height = image_height;
//    cinfo.input_components = 3;
//    cinfo.in_color_space = JCS_RGB;
//
//    jpeg_set_defaults(&cinfo);
//    //此处设压缩比为90%
//    jpeg_set_quality(&cinfo, 100, TRUE);
//    //开始压缩
//    jpeg_start_compress(&cinfo, TRUE);
//    //假设使用的是RGB图像
//    row_stride = image_width * 3;
//    //写入数据
//
//    while (cinfo.next_scanline < cinfo.image_height)
//    {
//        row_pointer = image_buffer[cinfo.next_scanline];
//
//        (void) jpeg_write_scanlines(&cinfo, &row_pointer, 1);
//    }
//
//    //压缩完毕
//    jpeg_finish_compress(&cinfo);
//    //释放资源
//    fclose(outfile);
//    jpeg_destroy_compress(&cinfo);
//
//}
unsigned char  * bmp2jpeg_compress(unsigned char *inbuf,unsigned long *outSize,
                       int quality,int WIDTH,int HEIGHT)
{
    unsigned char  *outbuf = nullptr;

    int jpegWidth =WIDTH;//jpeg的宽度;
    int jpegHeight =HEIGHT;//jpeg的高度;
//开始进行jpg的数据写入
    struct jpeg_compress_struct toWriteCinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;

    toWriteCinfo.err= jpeg_std_error(&jerr);
    jpeg_create_compress(&toWriteCinfo);
    //确定要用于输出压缩的jpeg的数据空间
    jpeg_mem_dest(&toWriteCinfo,&outbuf,outSize);

    toWriteCinfo.image_width = jpegWidth;
    toWriteCinfo.image_height = jpegHeight;

    //颜色通道数量
    int nComponent = 3;
    toWriteCinfo.input_components = nComponent;

    toWriteCinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&toWriteCinfo);
    //是否采用哈弗曼表数据计算 品质相差5-10倍
    toWriteCinfo.optimize_coding = TRUE;
    //设置质量
    jpeg_set_quality(&toWriteCinfo, quality,TRUE);

    //开始压缩，(是否写入全部像素)
    jpeg_start_compress(&toWriteCinfo,TRUE);
    //一行的RGB数量
    row_stride =jpegWidth *nComponent;//如果图片为RGB，这个值要*3.灰度图像不用。
    //一行一行遍历

    while(toWriteCinfo.next_scanline <toWriteCinfo.image_height)
    {
        //得到一行的首地址
        row_pointer[0] =& inbuf[toWriteCinfo.next_scanline * row_stride];
        //此方法会将jcs.next_scanline加1
        (void)jpeg_write_scanlines(&toWriteCinfo,row_pointer, 1);//row_pointer就是一行的首地址，1：写入的行数
    }
    jpeg_finish_compress(&toWriteCinfo);
    jpeg_destroy_compress(&toWriteCinfo);
    return  outbuf;
}

/**
 * byte数组转unsigned char * 的字符串
 */
unsigned char *jstrinToUnsignedChar(JNIEnv *env, jbyteArray barr) {
    unsigned char *rtn = NULL;
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, 0);
    if (alen > 0) {
        rtn = (unsigned char *) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}
 /**
 * 采用Libjpeg压缩
 * @param data
 * @param w
 * @param h
 * @param quality
 * @param outfilename
 * @param optimize
 * @return
 */
int generateJPEG(BYTE *data, int w, int h, int quality,
                 const char *outfilename, jboolean optimize) {

    //jpeg的结构体，保存的比如宽、高、位深、图片格式等信息
    struct jpeg_compress_struct jcs;
    //当读完整个文件的时候就会回调my_error_exit
    struct my_error_mgr jem;
    jcs.err = jpeg_std_error(&jem.pub);
    jem.pub.error_exit = my_error_exit;
    if (setjmp(jem.setjmp_buffer)) {
        return 0;
    }
    //初始化jsc结构体
    jpeg_create_compress(&jcs);
    //打开输出文件
    FILE* f = fopen(outfilename, "wb");
    if (f == NULL) {
        return 0;
    }
    //设置结构体的文件路径
    jpeg_stdio_dest(&jcs, f);
    jcs.image_width = w;//设置宽高
    jcs.image_height = h;
    //设置哈夫曼编码，TRUE=arithmetic coding, FALSE=Huffman
	if (optimize) {
        jcs.arith_code = FALSE;
	} else {
        jcs.arith_code = TRUE;
	}
    //颜色通道数量
    int nComponent = 3;
    jcs.input_components = nComponent;
    //设置结构体的颜色空间为RGB
    jcs.in_color_space = JCS_RGB;
    //全部设置默认参数
    jpeg_set_defaults(&jcs);
    //是否采用哈弗曼表数据计算 品质相差5-10倍
    jcs.optimize_coding = (boolean)optimize;
    //设置质量
    jpeg_set_quality(&jcs, quality, TRUE);
    //开始压缩，(是否写入全部像素)
    jpeg_start_compress(&jcs, TRUE);

    JSAMPROW row_pointer[1];
    int row_stride;
    //一行的RGB数量
    row_stride = jcs.image_width * nComponent;
    //一行一行遍历
    while (jcs.next_scanline < jcs.image_height) {
        //得到一行的首地址
        row_pointer[0] = &data[jcs.next_scanline * row_stride];
        //此方法会将jcs.next_scanline加1
        jpeg_write_scanlines(&jcs, row_pointer, 1);//row_pointer就是一行的首地址，1：写入的行数
    }
    jpeg_finish_compress(&jcs);
    jpeg_destroy_compress(&jcs);
    fclose(f);
    return 1;
}

/**
 * byte数组转C的字符串
 */
char *jstrinTostring(JNIEnv *env, jbyteArray barr) {
    char *rtn = NULL;
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, 0);
    if (alen > 0) {
        rtn = (char *) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_handsome_bitmapcompress_CompressUtils_compressBitmap(JNIEnv *env, jclass type,
                                                              jobject bitmap,
                                                              jint quality,
                                                              jbyteArray fileNameBytes_,
                                                              jboolean optimize) {
    //获取Bitmap信息
    AndroidBitmapInfo android_bitmap_info;
    AndroidBitmap_getInfo(env, bitmap, &android_bitmap_info);
    //获取bitmap的 宽，高，format
    int w = android_bitmap_info.width;
    int h = android_bitmap_info.height;
    int format = android_bitmap_info.format;

    if (format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        return -1;
    }

    //存储ARGB所有像素点
    BYTE *pixelsColor;
    //1、读取Bitmap所有像素信息
    AndroidBitmap_lockPixels(env, bitmap, (void **) &pixelsColor);
    //2、解析每个像素，去除A通量，取出RGB通量
    int i = 0, j = 0;
    BYTE a, r, g, b;
    //存储RGB所有像素点
    BYTE *data;
    data = (BYTE *) malloc(w * h * 3);
    //存储RGB首地址
    BYTE *tempData = data;

    int color;
    for (i = 0; i < h; ++i) {
        for (j = 0; j < w; ++j) {
            //将8位通道转成32位通道
            color = *((int *) pixelsColor);
            //取值
            a = ((color & 0xFF000000) >> 24);
            r = ((color & 0x00FF0000) >> 16);
            g = ((color & 0x0000FF00) >> 8);
            b = ((color & 0x000000FF));
            //赋值
            *data = b;
            *(data + 1) = g;
            *(data + 2) = r;
            //指针往后移
            data += 3;
            pixelsColor += 4;
        }
    }

    //3、读取像素点完毕
    AndroidBitmap_unlockPixels(env, bitmap);
    char *fileName = jstrinTostring(env, fileNameBytes_);
    //4、采用Libjpeg进行压缩
    int resultCode = generateJPEG(tempData, w, h, quality, fileName, optimize);
    if (resultCode == 0) {
        return 0;
    }
    return 1;
}
/*参数为：
 *返回图片的宽度(w_Dest),
 *返回图片的高度(h_Dest),
 *返回图片的位深(bit_depth),
 *源图片的RGB数据(src),
 *源图片的宽度(w_Src),
 *源图片的高度(h_Src)
 */
unsigned char* do_Stretch_Linear(int w_Dest,int h_Dest,int bit_depth,unsigned char *src,int w_Src,int h_Src)
{
    int sw = w_Src-1, sh = h_Src-1, dw = w_Dest-1, dh = h_Dest-1;
    int B, N, x, y;
    int nPixelSize = bit_depth/8;
    unsigned char *pLinePrev,*pLineNext;
    unsigned char *pDest = new unsigned char[w_Dest*h_Dest*bit_depth/8];
    unsigned char *tmp;
    unsigned char *pA,*pB,*pC,*pD;

    for(int i=0;i<=dh;++i)
    {
        tmp =pDest + i*w_Dest*nPixelSize;
        y = i*sh/dh;
        N = dh - i*sh%dh;
        pLinePrev = src + (y++)*w_Src*nPixelSize;
        //pLinePrev =(unsigned char *)aSrc->m_bitBuf+((y++)*aSrc->m_width*nPixelSize);
        pLineNext = (N==dh) ? pLinePrev : src+y*w_Src*nPixelSize;
        //pLineNext = ( N == dh ) ? pLinePrev : (unsigned char *)aSrc->m_bitBuf+(y*aSrc->m_width*nPixelSize);
        for(int j=0;j<=dw;++j)
        {
            x = j*sw/dw*nPixelSize;
            B = dw-j*sw%dw;
            pA = pLinePrev+x;
            pB = pA+nPixelSize;
            pC = pLineNext + x;
            pD = pC + nPixelSize;
            if(B == dw)
            {
                pB=pA;
                pD=pC;
            }

            for(int k=0;k<nPixelSize;++k)
            {
                *tmp++ = ( unsigned char )( int )(
                        ( B * N * ( *pA++ - *pB - *pC + *pD ) + dw * N * *pB++
                          + dh * B * *pC++ + ( dw * dh - dh * B - dw * N ) * *pD++
                          + dw * dh / 2 ) / ( dw * dh ) );
            }
        }
    }
    return pDest;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_handsome_bitmapcompress_CompressUtils_compressByLibjpeg(JNIEnv *env, jclass type,
                                                                 jbyteArray buffer, jint quality,jint scale,
                                                                 jint width, jint height) {

//    //获取Bitmap信息
//    AndroidBitmapInfo android_bitmap_info;
//    AndroidBitmap_getInfo(env, bitmap, &android_bitmap_info);
//    //获取bitmap的 宽，高，format
//    int w = android_bitmap_info.width;
//    int h = android_bitmap_info.height;
//    int format = android_bitmap_info.format;
//
//    if (format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
//        return nullptr;
//    }
//
//    //存储ARGB所有像素点
//    BYTE *pixelsColor;
//    //1、读取Bitmap所有像素信息
//    AndroidBitmap_lockPixels(env, bitmap, (void **) &pixelsColor);
//    //2、解析每个像素，去除A通量，取出RGB通量
//    int i = 0, j = 0;
//    BYTE a, r, g, b;
//    //存储RGB所有像素点
//    BYTE *data;
//    data = (BYTE *) malloc(w * h * 3);
//    //存储RGB首地址
//    BYTE *tempData = data;
//
//    int color;
//    for (i = 0; i < h; ++i) {
//        for (j = 0; j < w; ++j) {
//            //将8位通道转成32位通道
//            color = *((int *) pixelsColor);
//            //取值
//            a = ((color & 0xFF000000) >> 24);
//            r = ((color & 0x00FF0000) >> 16);
//            g = ((color & 0x0000FF00) >> 8);
//            b = ((color & 0x000000FF));
//            //赋值
//            *data = b;
//            *(data + 1) = g;
//            *(data + 2) = r;
//            //指针往后移
//            data += 3;
//            pixelsColor += 4;
//        }
//    }
//
//    //3、读取像素点完毕
//    AndroidBitmap_unlockPixels(env, bitmap);
    //缩放图片，缩放后的大小为(tb_w,tb_h)
    int tb_w = width , tb_h = height ;
    unsigned char * scale_buf = nullptr;
    unsigned char * img_buf = jstrinToUnsignedChar(env,buffer);
    if(scale>0)
    {
        tb_w = width/2, tb_h = height/2;
        scale_buf = do_Stretch_Linear(tb_w,tb_h,24,img_buf,width,height);
        if(img_buf!= nullptr)
        {
            free(img_buf);
            img_buf= nullptr;
        }
    } else
    {
        scale_buf = jstrinToUnsignedChar(env,buffer);
    }

    unsigned long outSize =0;
    unsigned char   *outbuf  =   bmp2jpeg_compress(scale_buf,&outSize,
                                                   quality,  tb_w, tb_h);

    if (scale_buf != nullptr) {
        free(scale_buf);
        img_buf = nullptr;
    }

    jbyte  *by = (jbyte*)outbuf;
    jbyteArray jarray = env->NewByteArray(outSize);
    env->SetByteArrayRegion(  jarray, 0, outSize, by);
    free(outbuf);
    return jarray;

}
