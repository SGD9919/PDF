# PDF
Qt PDF PDFium QGraphicsView

基于QPDFium 利用Qt widget实现的简单款PDF阅读器，由于修改了PDFium的源代码来适配了一些编译器，所以没有使用submodules的方式；
工程管理采用的pri，方便qmake集成，CMAKE可以去下载QPDFium然后使用本项目的thirdparty目录替编译成qt lib 使用；
已测试平台Ubuntu Win10 Win11 Android Arm-Linux(PDF基础库编译通过)
更多高级功能请自行研究开发

参考URL:
QPDFium: https://github.com/paulovap/qtpdfium.git
PDFium: https://github.com/snowpong/pdfium.git (使用这个库可以在MinGW编译器下使用)
