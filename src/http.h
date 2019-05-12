#ifndef HTTP_H
#define HTTP_H

#include <wininet.h>
#include <vector>
#include <thread>
#pragma comment( lib, "wininet" )

class WininetTimeout
{
public:
    WininetTimeout(HINTERNET handle, int timeout_ms)
    {
        exit_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        thread = new std::thread(&WininetTimeout::wait, this, handle, timeout_ms);
    }
    ~WininetTimeout()
    {
        SetEvent(exit_event);
        thread->join();
        delete thread;
    }
private:
    void wait(HINTERNET handle, int timeout_ms)
    {
        if (::WaitForSingleObject(exit_event, timeout_ms) == WAIT_TIMEOUT)
        {
            ::InternetCloseHandle(handle);
        }
    }
private:
    HANDLE exit_event;
    std::thread *thread;
};

bool HttpRequestWithCallback(std::vector<uint8_t> &buffer, const char *url, const char *post = "", int timeout_ms = 8000)
{
    bool result = false;
    //解析url
    char host[MAX_PATH + 1];
    char path[MAX_PATH + 1];

    URL_COMPONENTSA uc = { 0 };
    uc.dwStructSize = sizeof(uc);

    uc.lpszHostName = host;
    uc.dwHostNameLength = MAX_PATH;

    uc.lpszUrlPath = path;
    uc.dwUrlPathLength = MAX_PATH;

    ::InternetCrackUrlA(url, 0, ICU_ESCAPE, &uc);

    HINTERNET hInet = ::InternetOpenA("Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hInet)
    {
        WininetTimeout timeout(hInet, timeout_ms);
        HINTERNET hConn = ::InternetConnectA(hInet, host, uc.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
        if (hConn)
        {
            DWORD flag = INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD;
            if (uc.nScheme == INTERNET_SCHEME_HTTPS)
            {
                flag = flag | INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
            }
            HINTERNET hRes = ::HttpOpenRequestA(hConn, post[0] ? "POST" : "GET", path, 0, NULL, NULL, flag, 1);
            if (hRes)
            {
                // wchar_t heads[] = L"Cache-Control: no-cache\r\n";
                // HttpAddRequestHeadersW(hRes, heads, -1, HTTP_ADDREQ_FLAG_ADD);

                if (::HttpSendRequestA(hRes, NULL, 0, (LPVOID *)post, (DWORD)strlen(post)))
                {
                    DWORD http_code = 0;
                    DWORD code_size = sizeof(http_code);
                    ::HttpQueryInfo(hRes, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &http_code, &code_size, NULL);

                    DWORD dwTotal = 0;
                    DWORD dwTotalLen = sizeof(dwTotal);
                    ::HttpQueryInfo(hRes, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_CONTENT_LENGTH, &dwTotal, &dwTotalLen, NULL);

                    while (http_code == 200)
                    {
                        DWORD dwLength = 0;
                        DWORD dwByteRead = 0;

                        if (::InternetQueryDataAvailable(hRes, &dwLength, 0, 0) && dwLength)
                        {
                            size_t size = buffer.size();
                            buffer.resize(buffer.size() + dwLength);
                            if (::InternetReadFile(hRes, &buffer[size], dwLength, &dwByteRead))
                            {
                                //f(buffer.size(), dwTotal);
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }

                    if (buffer.size())
                    {
                        //buffer.push_back(0);// 补充一个字符串结束符，方便查看字符串
                        result = true;
                    }
                }
            }

            ::InternetCloseHandle(hConn);
        }

        ::InternetCloseHandle(hInet);
    }

    return result;
}

bool HttpRequest(std::vector<uint8_t> &buffer, const char *url, const char *post = "", int timeout_ms = 30*1000)
{
    return HttpRequestWithCallback(buffer, url, post, timeout_ms);
}

#endif /* HTTP_H */