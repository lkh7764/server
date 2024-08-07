#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <vector>
#include <sstream>

using namespace std;

#define FILE_PATH   "C:\\Windows\\Temp\\"



// ex1: GetQueuedCompletionStaus
#define BUFF_SIZE   65536       // 64KB
#define READ_KEY    1
#define WROTE_KEY   2
#define FILE_MAX    10
#define THRPOOL_MAX 4

struct COPY_CHUNCK : OVERLAPPED
{
    // COPY_CHUNCK: 파일 복사 동작에서 필요한 정보

    HANDLE  _hfSrc;             // 복사할 파일(SOURCE) 핸들 
    HANDLE  _hfDst;             // 복사할 위치의 파일(TARGET) 핸들

    BYTE    _arBuff[BUFF_SIZE]; // 복사를 위한 버퍼 필드


    COPY_CHUNCK(HANDLE hfSrc, HANDLE hfDst) {
        memset(this, 0, sizeof(*this));
        _hfSrc = hfSrc, _hfDst = hfDst;
    }
};
typedef COPY_CHUNCK* PCOPY_CHUNCK;

struct COPY_ENV
{
    // COPY_ENV:메인 함수와 스레드 엔트리 함수에서 공유할 정보

    HANDLE  _hIocp;             // IOCP 객체 핸들
    LONG    _nCpCnt;            // 복사할 전체 파일의 개수
    HANDLE  _hevEnd;            // 복사 완료 통지를 할 수동 리셋 이벤트

    // hw
    HANDLE  _hevRead;           // 읽기 완료 시 비신호상태가 되는 수동 리셋 이벤트
    //      읽기 완료하면 비신호 상태를 만들어
    //      출력이 중첩되지 않게 함
};
typedef COPY_ENV* PCOPY_ENV;

DWORD WINAPI IOCPCopyProc(PVOID pParam)
{
    PCOPY_ENV   pEnv = (PCOPY_ENV)pParam;
    DWORD       dwThrID = GetCurrentThreadId();

    while (true) {
        DWORD           dwErrCode = 0;
        PCOPY_CHUNCK    pcc = NULL;
        DWORD           dwTrBytes = 0;
        ULONG_PTR       ulKey;

        BOOL bIsOK = GetQueuedCompletionStatus(pEnv->_hIocp, &dwTrBytes, &ulKey, (LPOVERLAPPED*)&pcc, INFINITE);

        cout << "[THR ACTIVE] Thr " << dwThrID << endl;

        if (!bIsOK) {
            if (pcc == NULL)
                break;

            dwErrCode = GetLastError();
            goto $LABEL_CLOSE;
        }

        if (ulKey == READ_KEY) {
            printf(" => Thr %d Read bytes : %d\n", dwThrID, pcc->Offset);
            bIsOK = WriteFile(pcc->_hfDst, pcc->_arBuff, dwTrBytes, NULL, pcc);
        }

        else {
            pcc->Offset += dwTrBytes;

            printf(" <= Thr %d Wrote bytes : %d\n", dwThrID, pcc->Offset);
            bIsOK = ReadFile(pcc->_hfSrc, pcc->_arBuff, BUFF_SIZE, NULL, pcc);
        }

        if (!bIsOK) {
            dwErrCode = GetLastError();
            if (dwErrCode != ERROR_IO_PENDING)
                goto $LABEL_CLOSE;
        }

        continue;

    $LABEL_CLOSE:
        if (dwErrCode == ERROR_HANDLE_EOF)
            printf(" ****** Thr %d copy successfully completed...\n", dwThrID);
        else
            printf(" ###### Thr %d copy failed, code: %d\n", dwThrID, dwErrCode);

        CloseHandle(pcc->_hfSrc);
        CloseHandle(pcc->_hfDst);

        if (InterlockedDecrement(&pEnv->_nCpCnt) == 0) {
            SetEvent(pEnv->_hevEnd);
            cout << "[COMPLETE] ALL FILE COPIED" << endl;
        }

        return 0;
    }
}

void ex1(int argc, _TCHAR* argv[])
{
    /// <summary>
    ///     IOCP를 이용하여 다중 파일을 동시에 복사하는 예제
    ///         1. 실행 인자로 전체 경로를 담은 파일 리스트를 받아서 여러 개의 파일을 동시에 복사
    ///                 - 파일은 10개까지 받을 수 있음
    ///                 - 두 개의 스레드가 스레드풀을 구성
    ///         2. 복사되는 파일은 원본 파일과 동일한 경로에 "파일명.copied" 형태로 복사
    /// </summary>

    // 10개의 원소를 가지는 COPY_CHUNCK 구조체 포인터 배열 선언&초기화
    PCOPY_CHUNCK arChunck[FILE_MAX];
    memset(arChunck, 0, sizeof(PCOPY_CHUNCK) * FILE_MAX);

    // 스레드 엔트리 함수에 넘겨줄 COPY_ENV 구조체 생성 후 필드 구성
    COPY_ENV env;
    env._nCpCnt = 0;
    env._hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 2);  // 최대 2개의 스레드가 동시 실행 가능
    env._hevEnd = CreateEvent(NULL, TRUE, FALSE, NULL);

    // 복사할 파일 개수를 최대 10개로 제한
    if (argc > FILE_MAX + 1) argc = FILE_MAX + 1;
    env._nCpCnt = argc;

    LONG    lChnCnt = env._nCpCnt;
    DWORD   dwThrID = 0;
    HANDLE  harWorks[THRPOOL_MAX];
    for (int i = 0; i < THRPOOL_MAX; i++) {
        harWorks[i] = CreateThread(NULL, 0, IOCPCopyProc, &env, 0, &dwThrID);
        cout << "[ThrCreate Complete] ThrID: " << dwThrID << endl;
    }

    for (int i = 1; i < argc; i++) {
        // 패러미터로 넘겨받은 파일 주소 저장
        TCHAR* pszSrcFile = argv[i];

        // 해당 위치의 원본 파일 열기
        HANDLE  hSrcFile = CreateFile(
            pszSrcFile, GENERIC_READ, 0, NULL,
            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL
        );

        // 오류 제어
        if (hSrcFile == INVALID_HANDLE_VALUE) {
            cout << "[[ERROR]] " << pszSrcFile << " open failed, code: " << GetLastError() << endl;
            return;
        }


        // 원본 파일과 같은 위치에 복사 파일(+.copied) 생성
        TCHAR szDstFile[MAX_PATH];
        _tcscpy(szDstFile, pszSrcFile);
        _tcscat(szDstFile, _T("_copied.txt"));
        HANDLE hDstFile = CreateFile(
            szDstFile, GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL
        );

        // 오류제어
        if (hDstFile == INVALID_HANDLE_VALUE) {
            cout << "[[ERROR]] " << szDstFile << " open failed, code: " << GetLastError() << endl;
            return;
        }


        // 생성한 각 장치 핸들을 env의 IOCP 객체에 연결
        CreateIoCompletionPort(hSrcFile, env._hIocp, READ_KEY, 0);
        CreateIoCompletionPort(hDstFile, env._hIocp, WROTE_KEY, 0);

        // 해당 정보를 가진 새로운 COPY_CHUNCK 구조체를 생성하여 arChunck 배열에 추가 & _nCpCnt(복사할 파일 개수) 증가
        PCOPY_CHUNCK pcc = new COPY_CHUNCK(hSrcFile, hDstFile);
        arChunck[i - 1] = pcc;
        // env._nCpCnt++;
    }

    for (int i = 0; i < lChnCnt; i++) {
        PCOPY_CHUNCK pcc = arChunck[i];

        BOOL bIsOK = ReadFile(pcc->_hfSrc, pcc->_arBuff, BUFF_SIZE, NULL, pcc);
        if (!bIsOK) {
            DWORD dwErrCode = GetLastError();
            if (dwErrCode != ERROR_IO_PENDING)
                break;
        }
    }

    WaitForSingleObject(env._hevEnd, INFINITE);

    CloseHandle(env._hIocp);

    WaitForMultipleObjects(THRPOOL_MAX, harWorks, TRUE, INFINITE);

    for (int i = 0; i < lChnCnt; i++) {
        PCOPY_CHUNCK pcc = arChunck[i];
        delete pcc;
    }
    for (int i = 0; i < THRPOOL_MAX; i++)
        CloseHandle(harWorks[i]);
    CloseHandle(env._hevEnd);

    cout << "-- PROGRAM COLSE --" << endl;
}



// ex2: PostQueuedCompletionStatus
#define BEGIN_KEY   0

DWORD WINAPI IOCPCopyProc_2(PVOID pParam)
{
    PCOPY_ENV   pEnv = (PCOPY_ENV)pParam;
    DWORD       dwThrID = GetCurrentThreadId();

    while (true) {
        DWORD           dwErrCode = 0;
        PCOPY_CHUNCK    pcc = NULL;
        DWORD           dwTrBytes = 0;
        ULONG_PTR       ulKey;

        BOOL bIsOK = GetQueuedCompletionStatus(pEnv->_hIocp, &dwTrBytes, &ulKey, (LPOVERLAPPED*)&pcc, INFINITE);

        cout << "[THR ACTIVE] Thr " << dwThrID << endl;

        if (!bIsOK) {
            if (pcc == NULL)
                break;

            dwErrCode = GetLastError();
            goto $LABEL_CLOSE;
        }

        if (ulKey == READ_KEY) {
            printf(" => Thr %d Read bytes : %d\n", dwThrID, pcc->Offset);
            bIsOK = WriteFile(pcc->_hfDst, pcc->_arBuff, dwTrBytes, NULL, pcc);
        }

        else {
            if (ulKey == BEGIN_KEY) {
                pcc->Offset += dwTrBytes;
                printf(" => Thr %d Wrote bytes : %d\n", dwThrID, pcc->Offset);
            }
            bIsOK = ReadFile(pcc->_hfSrc, pcc->_arBuff, BUFF_SIZE, NULL, pcc);
        }

        if (!bIsOK) {
            dwErrCode = GetLastError();
            if (dwErrCode != ERROR_IO_PENDING)
                goto $LABEL_CLOSE;
        }

        continue;

    $LABEL_CLOSE:
        if (dwErrCode == ERROR_HANDLE_EOF) {
            printf(" ****** Thr %d copy successfully completed...\n", dwThrID);
            //pEnv->_nCpCnt -= 1;
        }
        else
            printf(" ###### Thr %d copy failed, code: %d\n", dwThrID, dwErrCode);

        CloseHandle(pcc->_hfSrc);
        CloseHandle(pcc->_hfDst);

        if (InterlockedDecrement(&pEnv->_nCpCnt) == 0) {
            SetEvent(pEnv->_hevEnd);
            cout << "[COMPLETE] ALL FILE COPIED" << endl;
        }

        return 0;
    }
}

void ex2(int argc, _TCHAR* argv[])
{
    // 10개의 원소를 가지는 COPY_CHUNCK 구조체 포인터 배열 선언&초기화
    PCOPY_CHUNCK arChunck[10];
    memset(arChunck, 0, sizeof(PCOPY_CHUNCK) * FILE_MAX);

    // 스레드 엔트리 함수에 넘겨줄 COPY_ENV 구조체 생성 후 필드 구성
    COPY_ENV env;
    env._nCpCnt = 0;
    env._hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 2);  // 최대 2개의 스레드가 동시 실행 가능
    env._hevEnd = CreateEvent(NULL, TRUE, FALSE, NULL);

    // 복사할 파일 개수를 최대 10개로 제한
    if (argc > FILE_MAX + 1) argc = FILE_MAX + 1;

    for (int i = 1; i < argc; i++) {
        // 패러미터로 넘겨받은 파일 주소 저장
        TCHAR* pszSrcFile = argv[i];

        // 해당 위치의 원본 파일 열기
        HANDLE  hSrcFile = CreateFile(
            pszSrcFile, GENERIC_READ, 0, NULL,
            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL
        );

        // 오류 제어
        if (hSrcFile == INVALID_HANDLE_VALUE) {
            cout << "[[ERROR]] " << pszSrcFile << " open failed, code: " << GetLastError() << endl;
            return;
        }


        // 원본 파일과 같은 위치에 복사 파일(+.copied) 생성
        TCHAR szDstFile[MAX_PATH];
        _tcscpy(szDstFile, pszSrcFile);
        _tcscat(szDstFile, _T("_copied.txt"));
        HANDLE hDstFile = CreateFile(
            szDstFile, GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL
        );

        // 오류제어
        if (hDstFile == INVALID_HANDLE_VALUE) {
            cout << "[[ERROR]] " << szDstFile << " open failed, code: " << GetLastError() << endl;
            return;
        }


        // 생성한 각 장치 핸들을 env의 IOCP 객체에 연결
        CreateIoCompletionPort(hSrcFile, env._hIocp, READ_KEY, 0);
        CreateIoCompletionPort(hDstFile, env._hIocp, WROTE_KEY, 0);

        // 해당 정보를 가진 새로운 COPY_CHUNCK 구조체를 생성하여 arChunck 배열에 추가 & _nCpCnt(복사할 파일 개수) 증가
        PCOPY_CHUNCK pcc = new COPY_CHUNCK(hSrcFile, hDstFile);
        arChunck[i - 1] = pcc;
        env._nCpCnt++;

        cout << "[OPEN FILE] file num: " << env._nCpCnt << endl;
    }

    LONG    lChnCnt = env._nCpCnt;
    DWORD   dwThrID = 0;
    HANDLE  harWorks[THRPOOL_MAX];
    for (int i = 0; i < THRPOOL_MAX; i++) {
        harWorks[i] = CreateThread(NULL, 0, IOCPCopyProc_2, &env, 0, &dwThrID);
        cout << "[ThrCreate Complete] ThrID: " << dwThrID << endl;
    }

    for (int i = 0; i < lChnCnt; i++) {
        PCOPY_CHUNCK pcc = arChunck[i];
        PostQueuedCompletionStatus(env._hIocp, 0, BEGIN_KEY, pcc);
    }

    WaitForSingleObject(env._hevEnd, INFINITE);

    cout << "-- PROGRAM COLSE --" << endl;

    CloseHandle(env._hIocp);

    WaitForMultipleObjects(THRPOOL_MAX, harWorks, TRUE, INFINITE);

    for (int i = 0; i < lChnCnt; i++) {
        PCOPY_CHUNCK pcc = arChunck[i];
        delete pcc;
    }
    for (int i = 0; i < THRPOOL_MAX; i++)
        CloseHandle(harWorks[i]);
    CloseHandle(env._hevEnd);
}



// hw
#define FILE_NUM    4

void printNums(const vector<int>& nums, HANDLE& hevRead)
{
    printf("DATA: ");
    for (const int& num : nums)
        printf("%d, ", num);
    printf("\n");

    SetEvent(hevRead);
}

bool readAndPrintData(PCOPY_CHUNCK& pcc, HANDLE& hevRead)
{
    //cout << "readAndPrintData" << endl;

    // readData
    BOOL bIsOK = ReadFile(pcc->_hfSrc, pcc->_arBuff, BUFF_SIZE, NULL, pcc);

    // 읽기가 정상적으로 완료되면 printData진행
    if (!bIsOK && GetLastError() == ERROR_IO_PENDING) {
        //cout << "readAndPrintData TRUE" << endl;

        //ResetEvent(hevRead);

        // BYTE to char
        char cBuff[BUFF_SIZE + 1];
        memcpy(cBuff, pcc->_arBuff, BUFF_SIZE);

        // char to sstream & int vector 생성
        string str(cBuff, sizeof(cBuff));
        stringstream str2(str);
        vector<int> nums; int num;
        while (str2 >> num)
            nums.push_back(num);

        // printData 전에 출력이 중첩되지 않도록 _hevRead를 비신호상태로 설정
        printNums(nums, hevRead);

        //SetEvent(hevRead);
    }

    return bIsOK;
}

void printData(PCOPY_CHUNCK& pcc, HANDLE& hevRead)
{
    // BYTE to char
    char cBuff[BUFF_SIZE + 1];
    memcpy(cBuff, pcc->_arBuff, BUFF_SIZE);

    // char to sstream & int vector 생성
    string str(cBuff, sizeof(cBuff));
    stringstream str2(str);
    vector<int> nums; int num;
    while (str2 >> num)
        nums.push_back(num);

    WaitForSingleObject(hevRead, INFINITE);

    // printData 전에 출력이 중첩되지 않도록 _hevRead를 비신호상태로 설정
    ResetEvent(hevRead);

    printNums(nums, hevRead);
}

DWORD WINAPI IOCPCopyProc_hw(PVOID pParam)
{
    PCOPY_ENV   pEnv = (PCOPY_ENV)pParam;
    DWORD       dwThrID = GetCurrentThreadId();

    while (true) {
        DWORD           dwErrCode = 0;
        PCOPY_CHUNCK    pcc = NULL;
        DWORD           dwTrBytes = 0;
        ULONG_PTR       ulKey;

        BOOL bIsOK = GetQueuedCompletionStatus(pEnv->_hIocp, &dwTrBytes, &ulKey, (LPOVERLAPPED*)&pcc, INFINITE);

        cout << "[THR ACTIVE] Thr " << dwThrID << endl;

        if (!bIsOK) {
            if (pcc == NULL)
                break;

            dwErrCode = GetLastError();
            goto $LABEL_CLOSE;
        }

        if (ulKey == READ_KEY) {
            printf(" => Thr %d Read bytes : %d\n", dwThrID, pcc->Offset);

            printData(pcc, pEnv->_hevRead);
            WaitForSingleObject(pEnv->_hevRead, INFINITE);

            bIsOK = WriteFile(pcc->_hfDst, pcc->_arBuff, dwTrBytes, NULL, pcc);
        }

        else {
            pcc->Offset += dwTrBytes;

            WaitForSingleObject(pEnv->_hevRead, INFINITE);
            printf(" <= Thr %d Wrote bytes : %d\n", dwThrID, pcc->Offset);
            //bIsOK = readAndPrintData(pcc, pEnv->_hevRead);
            bIsOK = ReadFile(pcc->_hfSrc, pcc->_arBuff, BUFF_SIZE, NULL, pcc);
        }

        if (!bIsOK) {
            dwErrCode = GetLastError();
            if (dwErrCode != ERROR_IO_PENDING)
                goto $LABEL_CLOSE;
        }

        continue;

    $LABEL_CLOSE:
        if (dwErrCode == ERROR_HANDLE_EOF)
            printf(" ****** Thr %d copy successfully completed...\n", dwThrID);
        else
            printf(" ###### Thr %d copy failed, code: %d\n", dwThrID, dwErrCode);

        CloseHandle(pcc->_hfSrc);
        CloseHandle(pcc->_hfDst);

        if (InterlockedDecrement(&pEnv->_nCpCnt) == 0) {
            SetEvent(pEnv->_hevEnd);
            cout << "[COMPLETE] ALL FILE COPIED" << endl;
        }

        return 0;
    }
}

void hw(int argc, _TCHAR* argv[])
{
    // FILE_NUM개의 원소를 가지는 COPY_CHUNCK 구조체 포인터 배열 선언&초기화
    PCOPY_CHUNCK arChunck[FILE_NUM];
    memset(arChunck, 0, sizeof(PCOPY_CHUNCK) * FILE_NUM);

    // 스레드 엔트리 함수에 넘겨줄 COPY_ENV 구조체 생성 후 필드 구성
    COPY_ENV env;
    env._nCpCnt = 0;
    env._hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 2);  // 최대 2개의 스레드가 동시 실행 가능
    env._hevEnd = CreateEvent(NULL, TRUE, FALSE, NULL);
    env._hevEnd = CreateEvent(NULL, TRUE, TRUE, NULL);

    // THRPOOL_MAX 개수만큼 스레드를 생성하여 스레드풀 구성
    DWORD   dwThrID = 0;
    HANDLE  harWorks[THRPOOL_MAX];
    for (int i = 0; i < THRPOOL_MAX; i++) {
        //harWorks[i] = CreateThread(NULL, 0, IOCPCopyProc, &env, 0, &dwThrID);
        harWorks[i] = CreateThread(NULL, 0, IOCPCopyProc_hw, &env, 0, &dwThrID);
        cout << "[ThrCreate Complete] ThrID: " << dwThrID << endl;
    }

    // 파일 복사 과정에서 필요한 정보들을 COPY_CHUNCK 구조체로 만들어 arChunck에 저장
    for (int i = 1; i < argc; i++) {
        // 패러미터로 넘겨받은 파일 주소 저장
        TCHAR* pszSrcFile = argv[i];

        // 해당 위치의 원본 파일 열기
        HANDLE  hSrcFile = CreateFile(
            pszSrcFile, GENERIC_READ, 0, NULL,
            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL
        );

        // 오류 제어
        if (hSrcFile == INVALID_HANDLE_VALUE) {
            cout << "[[ERROR]] " << pszSrcFile << " open failed, code: " << GetLastError() << endl;
            return;
        }


        // 원본 파일과 같은 위치에 복사 파일(+.copied) 생성
        TCHAR szDstFile[MAX_PATH];
        _tcscpy(szDstFile, pszSrcFile);
        _tcscat(szDstFile, _T("_copied.txt"));
        HANDLE hDstFile = CreateFile(
            szDstFile, GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL
        );

        // 오류제어
        if (hDstFile == INVALID_HANDLE_VALUE) {
            cout << "[[ERROR]] " << szDstFile << " open failed, code: " << GetLastError() << endl;
            return;
        }


        // 생성한 각 장치 핸들을 env의 IOCP 객체에 연결
        CreateIoCompletionPort(hSrcFile, env._hIocp, READ_KEY, 0);
        CreateIoCompletionPort(hDstFile, env._hIocp, WROTE_KEY, 0);

        // 해당 정보를 가진 새로운 COPY_CHUNCK 구조체를 생성하여 arChunck 배열에 추가 & _nCpCnt(복사할 파일 개수) 증가
        PCOPY_CHUNCK pcc = new COPY_CHUNCK(hSrcFile, hDstFile);
        arChunck[i - 1] = pcc;
        env._nCpCnt++;
    }

    LONG    lChnCnt = env._nCpCnt;
    for (int i = 0; i < lChnCnt; i++) {
        PCOPY_CHUNCK pcc = arChunck[i];

        //BOOL bIsOK = readAndPrintData(pcc, env._hevRead);
        BOOL bIsOK = ReadFile(pcc->_hfSrc, pcc->_arBuff, BUFF_SIZE, NULL, pcc);
        if (!bIsOK) {
            DWORD dwErrCode = GetLastError();
            if (dwErrCode != ERROR_IO_PENDING)
                break;
        }
    }

    WaitForSingleObject(env._hevRead, INFINITE);

    WaitForSingleObject(env._hevEnd, INFINITE);

    CloseHandle(env._hIocp);

    WaitForMultipleObjects(THRPOOL_MAX, harWorks, TRUE, INFINITE);

    for (int i = 0; i < lChnCnt; i++) {
        PCOPY_CHUNCK pcc = arChunck[i];
        delete pcc;
    }
    for (int i = 0; i < THRPOOL_MAX; i++)
        CloseHandle(harWorks[i]);
    CloseHandle(env._hevEnd);
}




void _tmain(int argc, _TCHAR* argv[])
{
    if (argc < 2) {
        cout << "Uasge : MultiCopyIOCP SourceFile1 SourceFile2 SourceFile3 ..."
            << endl;
        return;
    }

    //ex1(argc, argv);
    //ex2(argc, argv);
    hw(argc, argv);

    return;
}
