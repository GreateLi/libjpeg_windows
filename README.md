# libjpeg_windows
libjpeg_windows
因为要跨平台，图片压缩，所以选择了小库libjpeg；
一、下载源代码

http://jpegclub.org/reference/wp-content/uploads/2019/06/jpegsr09d.zip

前往 官网下载jpegsr09d源代码

你也可以选择其它版本，API可能会有区别

https://jpegclub.org/reference/reference-sources/
二、 编译

使用VS工具 的本地工具：

打开工具后，输入命令窗口：

cd 到解压 的目录：

nmake -f makefile.vc setup-v16 因为（9d中办有v16,虽然我的vc 是14）

可能会提示 win32.mak 这个文件找不到，原因是 makefile.vc 第一行，写的是当前目录，改成具体你的绝对目录就可以。

如果这个文件真找不到，你需要去网上下载一下。每个人的目录可能稍有差异，有的在v7.0A

include <C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include\win32.mak>

再次输入 nmake -f makefile.vc setup-v16，成功，生成 vc 工程  .sln.

用vs 打开即可编译。

注意：设置 项目编译方式(MT/MD)，以及使用的WindowsSDK等信息

否则可能报 error LNK2001: 无法解析的外部符号
--------------------- 
作者：恋恋西风 
来源：CSDN 
原文：https://blog.csdn.net/q610098308/article/details/93894948 
版权声明：本文为博主原创文章，转载请附上博文链接！
