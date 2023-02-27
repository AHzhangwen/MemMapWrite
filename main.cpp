
/*
ʹ���ڴ�ӳ���ļ��Ĳ��裺
1���������һ���ļ��ں˶��󣬸ö����ʶ��������Ҫ�����ڴ�ӳ���ļ����Ǹ������ļ���
CreateFile();
2������һ���ļ�ӳ���ں˶���������ϵͳ�ļ��Ĵ�С�Լ����Ǵ�����η����ļ���
CreateFileMapping();
3������ϵͳ���ļ�ӳ�����Ĳ��ֻ�ȫ��ӳ�䵽���̵ĵ�ַ�ռ��С�
MapViewOfFile();
�����ڴ�ӳ���ļ�֮�󣬱���ִ������������������
1������ϵͳ�ӽ��̵�ַ�ռ���ȡ�����ļ�ӳ���ں˶����ӳ�䡣
UnmapViewOfFile();
2���ر��ļ�ӳ���ں˶���
CloseHandle();
3���ر��ļ��ں˶���
CloseHandle();
*/


#include <windows.h>
#include <string>

int MemMapAppFile(std::string fPath,void *data, size_t length)
{
    //���������򿪣��������򴴽���·�����Ϸ�ʧ�ܣ�
    HANDLE hFile = CreateFileA(fPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

    //���ļ���С
    LARGE_INTEGER  oldSize;
    bool ret = GetFileSizeEx(hFile, &oldSize);
    if (!ret)
    {
        return GetLastError();
    }

    //д����С
    LARGE_INTEGER newSize;
    newSize.QuadPart = oldSize.QuadPart + length;

    //������С����
    SYSTEM_INFO SysInfo;  
    DWORD dwSysGran;      
    GetSystemInfo(&SysInfo);
    dwSysGran = SysInfo.dwAllocationGranularity;

    //Ҫӳ��Ĵ�С,������С���ȣ��ٷ��ĵ�û���Ҫ��
    LARGE_INTEGER fileSize;
    fileSize.QuadPart = newSize.QuadPart;// (newSize.QuadPart / dwSysGran + 1)* (dwSysGran);
    HANDLE hMapFile;      // handle for the file's memory-mapped region�ļ��ڴ�ӳ������ľ��
    hMapFile = CreateFileMappingA(hFile,          // current file handle��ǰ�ļ����
        NULL,           // default securityĬ�ϰ�ȫ��
        PAGE_READWRITE, // read/write permission��/дȨ��
        fileSize.HighPart,   // size of mapping object, highӳ�����Ĵ�С����
        fileSize.LowPart,  // size of mapping object, lowӳ�����Ĵ�С����
        NULL);          // name of mapping objectӳ����������

    if (hMapFile == NULL)
    {
        return GetLastError();
    }

    //ӳ����ͼ��С������ͼ�ڿ������ڴ�һ������,������С���ȣ��ٷ��ĵ�û���Ҫ��
    LARGE_INTEGER viewSize;
    viewSize .QuadPart = (oldSize.QuadPart / dwSysGran) * (dwSysGran); //length;
    LPVOID lpMapAddress;  // pointer to the base address of theָ��洢��ӳ������
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

    int startOffset = oldSize.QuadPart % dwSysGran;//�Ǹ���������
    memcpy((char*)lpMapAddress + startOffset, data, length);

    BOOL bFlag;           // a result holder���������
    // Close the file mapping object and the open file�ر��ļ�ӳ�����ʹ򿪵��ļ�
    bFlag = UnmapViewOfFile(lpMapAddress);
    bFlag = CloseHandle(hMapFile); // close the file mapping object�ر��ļ�ӳ�����
    if (!bFlag)
    {
        return GetLastError();
    }

    bFlag = CloseHandle(hFile);   // close the file itself�ر��ļ�����
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

