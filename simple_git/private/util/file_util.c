#include "../../public/util/file_util.h"
#include "../../public/util/string_util.h"

void init_file_path(FFilePath* file_path)
{
    file_path->index = 0;
    memset(file_path->paths, 0, sizeof(file_path->paths) - 1);
}

int copy_file(const char* Dest, const char* Src)
{
    //当前的缓存 缓存1MB大小，如果超过就会出问题 这个会在std C课程里面继续扩展
    char Buf[1024 * 1024] = {0};
    int FileSize = 0;
    FILE* FpSrc = NULL;
    FILE* FpDest = NULL;

    if ((FpSrc = fopen(Src, "rb")) != NULL)
    {
        if ((FpDest = fopen(Dest, "wb")) != NULL)
        {
            // 注意: fread(file) > 0 表示读出了文件，否则读文件内容失败 
            while ((FileSize = fread(Buf, 1, 512, FpSrc)) > 0)
            {
                fwrite(Buf, FileSize, sizeof(char), FpDest);
                memset(Buf, 0, sizeof(Buf));
            }

            fclose(FpSrc);
            fclose(FpDest);

            return 0;
        }
    }

    return -1;
}

void find_files(char const* folder_path, FFilePath* out_file_path, bool b_recursion)
{
    struct _finddata_t finddata;

    long hfile = 0;
    char tmp_path[8196] = {0};
    strcpy(tmp_path, folder_path);
    strcat(tmp_path, "\\*");
    if ((hfile = _findfirst(tmp_path, &finddata)) != -1)
    {
        do
        {
            if (finddata.attrib & _A_SUBDIR)
            {
                if (b_recursion)
                {
                    if (strcmp(finddata.name, ".") == 0 ||
                        strcmp(finddata.name, "..") == 0)
                    {
                        continue;
                    }

                    char new_path[8196] = {0};
                    strcpy(new_path, folder_path);
                    strcat(new_path, "\\");
                    strcat(new_path, finddata.name);

                    find_files(new_path, out_file_path, b_recursion);
                }
            }
            else
            {
                strcpy(out_file_path->paths[out_file_path->index], folder_path);
                strcat(out_file_path->paths[out_file_path->index], "\\");
                strcat(out_file_path->paths[out_file_path->index++], finddata.name);
            }
        }
        while (_findnext(hfile, &finddata) == 0);
        _findclose(hfile);
    }
}

bool create_file(char const* file_path)
{
    FILE* fp = NULL;
    if ((fp = fopen(file_path, "w+")) != NULL)
    {
        fclose(fp);

        return true;
    }

    return false;
}

bool create_folder_path(char const* folder_path)
{
    // 切分路径字符串
    FArray c_array;
    if (strstr(folder_path, "\\"))
    {
        split_string(folder_path, "\\", &c_array);
    }
    else if (strstr(folder_path, "/"))
    {
        split_string(folder_path, "/", &c_array);
    }

    // 文件路径不存在则生成相应文件夹
    char tmp_path[260] = {0};
    for (int i = 0; i < c_array.size; i++)
    {
        // 从根节点开始，逐一拼接路径
        char* folder = get_array_element(&c_array, i);
        strcat(folder, "\\");
        strcat(tmp_path, folder);
        // 无法访问
        if (_access(tmp_path, 0) == -1)
        {
            _mkdir(tmp_path); // 生成文件夹
        }
    }

    // 释放内存
    destroy_array(&c_array);

    // 最后确保能正常访问
    return _access(tmp_path, 0) == 0;
}

bool read_file(const char* file_path, char* out_content)
{
    FILE* fp = NULL;
    if ((fp = fopen(file_path, "r")) != NULL)
    {
        char buf_tmp[2048] = {0};
        int file_size = 0;
        while ((file_size = fread(buf_tmp, 1, 1024, fp)) > 0)
        {
            strcat(out_content, buf_tmp);
            memset(buf_tmp, 0, sizeof(buf_tmp));
        }

        fclose(fp);

        return out_content[0] != '\0';
    }

    return false;
}

bool add_file(const char* file_path, char* content)
{
    FILE* fp = NULL;
    if ((fp = fopen(file_path, "a+")) != NULL)
    {
        fprintf(fp, "%s", content);
        fclose(fp);

        return true;
    }

    return false;
}

bool rewrite_file(const char* file_path, char* content)
{
    FILE* fp = NULL;
    if ((fp = fopen(file_path, "w+")) != NULL)
    {
        fprintf(fp, "%s", content);
        fclose(fp);

        return true;
    }

    return false;
}

unsigned int get_file_size(const char* file_path)
{
    unsigned int file_size = 0;

    FILE* fp = NULL;
    if ((fp = fopen(file_path, "r")) != NULL)
    {
        file_size = get_file_size_by_handle(fp);

        fclose(fp);
    }

    return file_size;
}

//asdoiajoi ajs aoisjd oaisjd oiasjdoi asodao ijaosijd oaisdja index
unsigned int get_file_size_by_handle(FILE* file_handle)
{
    unsigned int file_size = 0;

    unsigned int current_read_postion = ftell(file_handle);
    fseek(file_handle, 0, SEEK_END);
    file_size = ftell(file_handle);
    fseek(file_handle, current_read_postion, SEEK_SET);

    return file_size;
}
