#include "storage_manager.h"
#include "FileSystem.h"
#include "File.h"
#include <utility>
#include "safe_string.h"

namespace aware
{

StorageObject::StorageObject(StorageBucket *bucket, const char *name, const char *extension)
    : _read(false), _bucket(bucket), _file(NULL), _metadata_file(NULL), _seconds(time(NULL)), _first_metadata_entry(true)
{
    safe_strcpy(_name, sizeof(_name), name);
    safe_strcpy(_extension, sizeof(_extension), extension);
    generate_id(_id, sizeof(_id), _seconds);
}

int StorageObject::add_metadata(const KeyValuePair &entry)
{
    if (_read)
    {
        return -1;
    }
    if (!_first_metadata_entry)
    {
        fprintf(_metadata_file, ",");
    }
    else
    {
        _first_metadata_entry = false;
    }
    fprintf(_metadata_file, "\n  ");
    entry.serialize_to_json_file(_metadata_file);
    return 0;
}

FILE *StorageObject::open(bool binary)
{
    char path[64];
    safe_strcpy(path, sizeof(path), "/");
    safe_strcat(path, sizeof(path), _bucket->storage_manager()->fs_mount_path());
    safe_strcat(path, sizeof(path), "/");
    safe_strcat(path, sizeof(path), _bucket->data_path());
    safe_strcat(path, sizeof(path), "/");
    safe_strcat(path, sizeof(path), _id);
    safe_strcat(path, sizeof(path), ".");
    safe_strcat(path, sizeof(path), _extension);
    FILE *file;
    if (_read)
    {
        file = fopen(path, "rb");   
    }
    else
    {
        if (binary)
        {
            file = fopen(path, "wb");
        }
        else
        {
            file = fopen(path, "w");
        }
    }

    if (file && !_metadata_file)
    {
        safe_strcpy(path, sizeof(path), "/");
        safe_strcat(path, sizeof(path), _bucket->storage_manager()->fs_mount_path());
        safe_strcat(path, sizeof(path), "/");
        safe_strcat(path, sizeof(path), _bucket->metadata_path());
        safe_strcat(path, sizeof(path), "/");
        safe_strcat(path, sizeof(path), _id);
        safe_strcat(path, sizeof(path), ".");
        safe_strcat(path, sizeof(path), "json");
        _metadata_file = fopen(path, "w");
        if (_metadata_file)
        {
            _file = file;
            char time_str[32];
            generate_iso_time_string(time_str, sizeof(time_str), _seconds);
            char filename[32];
            sprintf(filename, "%s.%s", _id, _extension);

            fprintf(_metadata_file, "{");
            add_metadata(KeyValuePair("_version", IntValue(1)));
            add_metadata(KeyValuePair("_created_at", StringValue(time_str)));
            add_metadata(KeyValuePair("_id", StringValue(_id)));
            add_metadata(KeyValuePair("_name", StringValue(_name)));
            add_metadata(KeyValuePair("_filename", StringValue(filename)));
            add_metadata(KeyValuePair("_extension", StringValue(_extension)));

            return file;
        }
        else
        {
            fclose(file);
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

int StorageObject::close()
{
    fclose(_file);
    return close_object();
}

int StorageObject::close_object()
{
    if (_metadata_file)
    {
        time_t current_time = time(NULL);
        char time_str[32];
        generate_iso_time_string(time_str, sizeof(time_str), current_time);
        add_metadata(KeyValuePair("_modified_at", StringValue(time_str)));
        fprintf(_metadata_file, "\n}\n");
        fclose(_metadata_file);

        char path[64];
        safe_strcpy(path, sizeof(path), "/");
        safe_strcat(path, sizeof(path), _bucket->storage_manager()->fs_mount_path());
        safe_strcat(path, sizeof(path), "/");
        safe_strcat(path, sizeof(path), _bucket->index_path());
        safe_strcat(path, sizeof(path), "/");
        safe_strcat(path, sizeof(path), _id);
        FILE *index_file = fopen(path, "w");
        fprintf(index_file, "%s", _extension);
        fclose(index_file);
    }
    return 0;
}

StorageObject::~StorageObject()
{
    // Add object to the index when it's closed
    close_object();
}

void StorageObject::generate_iso_time_string(char *str, size_t length, time_t time)
{
    strftime(str, length, "%Y-%m-%dT%H:%M:%SZ", gmtime(&time));
}

void StorageObject::generate_id(char *id, size_t length, time_t time)
{
    //strftime(id, length, "%Y%m%dT%H%M%S", gmtime(&time));
    RandomGenerator &rg = _bucket->storage_manager()->random_factory()->generator();
    for (int i = 0; i < length - 1; ++i)
    {
        // if (i < length) {
        id[i] = rg.sample_alphanumeric();
        // }
    }
    id[length - 1] = 0;
}

StorageBucket::StorageBucket(StorageManager *storage_manager, const char *name) : _sm(storage_manager), _name(name)
{
    if (strlen(_sm->storage_root()) > 0)
    {
        safe_strcpy(_base_path, sizeof(_base_path), _sm->storage_root());
        safe_strcat(_base_path, sizeof(_base_path), "/");
    }
    else
    {
        _base_path[0] = 0;
    }
    safe_strcat(_base_path, sizeof(_base_path), _name);
    safe_strcpy(_index_path, sizeof(_index_path), _base_path);
    safe_strcat(_index_path, sizeof(_index_path), "/index");
    safe_strcpy(_metadata_path, sizeof(_metadata_path), _base_path);
    safe_strcat(_metadata_path, sizeof(_metadata_path), "/metadata");
    safe_strcpy(_data_path, sizeof(_data_path), _base_path);
    safe_strcat(_data_path, sizeof(_data_path), "/data");
}

StorageBucket::~StorageBucket()
{
}

int StorageBucket::init()
{
    _sm->filesystem()->mkdir(_base_path, 0777);
    _sm->filesystem()->mkdir(_data_path, 0777);
    _sm->filesystem()->mkdir(_index_path, 0777);
    _sm->filesystem()->mkdir(_metadata_path, 0777);
    return 0;
}

StorageObject StorageBucket::create_object(const char *name, const char *extension)
{
    return StorageObject(this, name, extension);
}

StorageManager::StorageManager(RandomFactory *rf, FileSystem *fs, const char *fs_mount_path, const char *storage_root)
    : _rf(rf), _fs(fs), _fs_mount_path(fs_mount_path), _root(storage_root)
{
}

StorageManager::~StorageManager()
{
}

int StorageManager::init()
{
    int status = _fs->mkdir(_root, 0777);
    return status;
}

StorageBucket StorageManager::get_bucket(const char *name)
{
    StorageBucket bucket = StorageBucket(this, name);
    bucket.init();
    return bucket;
}

} // namespace aware