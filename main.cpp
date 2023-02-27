
/*
使用内存映射文件的步骤：
1、创建或打开一个文件内核对象，该对象标识了我们想要用作内存映射文件的那个磁盘文件。
CreateFile();
2、创建一个文件映射内核对象，来告诉系统文件的大小以及我们打算如何访问文件。
CreateFileMapping();
3、告诉系统把文件映射对象的部分或全部映射到进程的地址空间中。
MapViewOfFile();
用完内存映射文件之后，必须执行以下三步进行清理。
1、告诉系统从进程地址空间中取消对文件映射内核对象的映射。
UnmapViewOfFile();
2、关闭文件映射内核对象。
CloseHandle();
3、关闭文件内核对象。
CloseHandle();
*/


#include <windows.h>
#include <string>

int MemMapAppFile(std::string fPath,void *data, size_t length)
{
    //如果存在则打开，不存在则创建（路径不合法失败）
    HANDLE hFile = CreateFileA(fPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

    //旧文件大小
    LARGE_INTEGER  oldSize;
    bool ret = GetFileSizeEx(hFile, &oldSize);
    if (!ret)
    {
        return GetLastError();
    }

    //写入后大小
    LARGE_INTEGER newSize;
    newSize.QuadPart = oldSize.QuadPart + length;

    //磁盘最小粒度
    SYSTEM_INFO SysInfo;  
    DWORD dwSysGran;      
    GetSystemInfo(&SysInfo);
    dwSysGran = SysInfo.dwAllocationGranularity;

    //要映射的大小,对齐最小粒度（官方文档没这个要求）
    LARGE_INTEGER fileSize;
    fileSize.QuadPart = newSize.QuadPart;// (newSize.QuadPart / dwSysGran + 1)* (dwSysGran);
    HANDLE hMapFile;      // handle for the file's memory-mapped region文件内存映射区域的句柄
    hMapFile = CreateFileMappingA(hFile,          // current file handle当前文件句柄
        NULL,           // default security默认安全性
        PAGE_READWRITE, // read/write permission读/写权限
        fileSize.HighPart,   // size of mapping object, high映射对象的大小，高
        fileSize.LowPart,  // size of mapping object, low映射对象的大小，低
        NULL);          // name of mapping object映射对象的名称

    if (hMapFile == NULL)
    {
        return GetLastError();
    }

    //映射试图大小，在试图内可以像内存一样操作,对齐最小粒度（官方文档没这个要求）
    LARGE_INTEGER viewSize;
    viewSize .QuadPart = (oldSize.QuadPart / dwSysGran) * (dwSysGran); //length;
    LPVOID lpMapAddress;  // pointer to the base address of the指向存储器映射区域
    lpMapAddress = MapViewOfFileEx(hMapFile,            // handle to
        FILE_MAP_ALL_ACCESS, // read/write
        viewSize.HighPart,                   // high-order 32
        viewSize.LowPart,      // low-order 32
        0, // number of bytes
        NULL);     

    if (lpMapAddress == NULL)
    {
        return GetLastError();
    }

    int startOffset = oldSize.QuadPart % dwSysGran;//非负数，由于
    memcpy((char*)lpMapAddress + startOffset, data, length);

    BOOL bFlag;           // a result holder结果持有者
    // Close the file mapping object and the open file关闭文件映射对象和打开的文件
    bFlag = UnmapViewOfFile(lpMapAddress);
    bFlag = CloseHandle(hMapFile); // close the file mapping object关闭文件映射对象
    if (!bFlag)
    {
        return GetLastError();
    }

    bFlag = CloseHandle(hFile);   // close the file itself关闭文件本身
    if (!bFlag)
    {
        return GetLastError();
    }
    return 0;
}

int main(void)
{
    size_t max = 1024ll*1024*1024+512;
    char* a = new char[max] {};
    a[0] = '0';
    a[max - 1] = '1';
    int ret = 0;
    for(int i=0;i<100;i++)
    ret = MemMapAppFile(R"(C:\testMwmMap.txt)", a, max);
}

