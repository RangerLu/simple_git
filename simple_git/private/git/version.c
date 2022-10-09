#include "../../public/array/array.h"
#include "../../public/log/log.h"
#include "../../public/util/string_util.h"
#include "../../public/util/file_util.h"
#include "../../public/git/version.h"
#include "../../public/git/path.h"
#include "../../public/git/global.h"
#include "../../public/git/service.h"

void string_to_git_commit(FGitCommit* git_commit, const char* buf)
{
    FArray c_array;
    split_string(buf, "\n", &c_array);
    {
        char* value = get_array_element(&c_array, 0);
        strcpy(git_commit->name, value);
    }
    {
        char* value = get_array_element(&c_array, 1);
        strcpy(git_commit->commit, value);
    }
    {
        char* value = get_array_element(&c_array, 2);
        strcpy(git_commit->date, value);
    }
    {
        char* value = get_array_element(&c_array, 3);
        string_to_guid(value, &git_commit->guid);
    }

    destroy_array(&c_array);
}

void string_to_git_commits(FGitCommits* git_commit, const char* buf)
{
    FArray c_array;
    split_string(buf, "\n", &c_array);
    for (int i = 0; i < c_array.size; i += 4)
    {
        FGitCommit* git_commit_element = &git_commit->commits[git_commit->size++];
        {
            char* value = get_array_element(&c_array, i);
            strcpy(git_commit_element->name, value);
        }
        {
            char* value = get_array_element(&c_array, i + 1);
            strcpy(git_commit_element->commit, value);
        }
        {
            char* value = get_array_element(&c_array, i + 2);
            strcpy(git_commit_element->date, value);
        }
        {
            char* value = get_array_element(&c_array, i + 3);
            string_to_guid(value, &git_commit_element->guid);
        }
    }

    destroy_array(&c_array);
}

void string_to_versions(const char* buf, FGitVersions* in_versions)
{
    FArray c_array;
    split_string(buf, "\n", &c_array);
    for (int i = 0; i < c_array.size; i += 4)
    {
        char* p = get_array_element(&c_array, i);
        in_versions->paths[in_versions->size].operation_type = (EVersionOperationType)atoi(p);

        p = get_array_element(&c_array, i + 1);
        strcpy(in_versions->paths[in_versions->size].file_name, p);

        p = get_array_element(&c_array, i + 2);
        string_to_guid(p, &in_versions->paths[in_versions->size].crc);

        p = get_array_element(&c_array, i + 3);
        in_versions->paths[in_versions->size++].file_size = atoi(p);
    }

    destroy_array(&c_array);
}

char* git_commit_to_string(const FGitCommit* git_commit, char* buf)
{
    char buf_guid[MAX_PATH] = {0};
    guid_to_string(buf_guid, &git_commit->guid);
    placeholder_cat_s(buf, "%s\n%s\n%s\n%s\n", git_commit->name, git_commit->commit, git_commit->date, buf_guid);

    return buf;
}

void versions_to_string(char* buf, const FGitVersions* in_versions)
{
    for (unsigned int i = 0; i < in_versions->size; i++)
    {
        char buf_tmp[MAX_PATH * 2] = {0};
        char guid_buf[MAX_PATH] = {0};
        guid_to_string(guid_buf, &in_versions->paths[i].crc);
        placeholder_cat_s(buf_tmp, "%i\n%s\n%s\n%d\n", in_versions->paths[i].operation_type,
                          in_versions->paths[i].file_name,
                          guid_buf, in_versions->paths[i].file_size);

        strcat(buf, buf_tmp);
    }
}

void init_commit(FGitCommit* git_commit)
{
    memset(git_commit->name, 0, sizeof(git_commit->name));
    memset(git_commit->commit, 0, sizeof(git_commit->commit));
    memset(git_commit->date, 0, sizeof(git_commit->date));

    init_guid(&git_commit->guid);
}

void init_git_commits(FGitCommits* git_commits)
{
    git_commits->size = 0;
    memset(git_commits->commits, 0, sizeof(git_commits->commits));
}

void init_versions(FGitVersions* versions)
{
    versions->size = 0;
    memset(versions->paths, 0, sizeof(versions->paths));
}

void init_git_path_2ds(FGitPath2Ds* path_2ds)
{
    path_2ds->size = 0;
    memset(path_2ds->paths, 0, sizeof(path_2ds->paths) - 1);
}


//////////////////////////////////////////////////////////////////////// core
char* get_git_commit(FGitCommit* git_commit)
{
    init_commit(git_commit);

    char buf_char[8196] = {0};
    if (_access(get_version_info_file(), 0) == 0)
    {
        read_file(get_version_info_file(), buf_char);

        if (buf_char[0] != '\0')
        {
            string_to_git_commit(git_commit, buf_char);
            return buf_char[0] == '\0' ? NULL : buf_char;
        }
    }

    return NULL;
}


void add_git_versions(const char* int_path, EVersionOperationType type, FGitVersions* out_versions)
{
    char new_path[MAX_PATH] = {0};
    strcpy(new_path, int_path);

    const char* p = get_project_cache();
    remove_string_start(new_path, p);

    out_versions->paths[out_versions->size].operation_type = type;
    strcpy(out_versions->paths[out_versions->size].file_name, new_path);
    create_guid(&out_versions->paths[out_versions->size].crc);
    out_versions->paths[out_versions->size].file_size = get_file_size_by_filename(int_path);

    out_versions->size++;
}
