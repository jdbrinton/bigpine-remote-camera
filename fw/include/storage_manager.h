#ifndef AWARE_STORAGE_MANAGER_H
#define AWARE_STORAGE_MANAGER_H

#include "mbed.h"
#include "File.h"
#include "key_value_pair.h"
#include "random.h"

namespace aware
{
class StorageManager;
class StorageBucket;

class StorageObject
{
public:
  StorageObject(StorageBucket *bucket, const char *name, const char *extension);
  ~StorageObject();

  FILE *open(bool binary = false);
  int close();

  int add_metadata(const KeyValuePair &entry);

private:
  void generate_id(char *id, size_t length, time_t time);
  void generate_iso_time_string(char *id, size_t length, time_t time);

  StorageBucket *_bucket;
  bool _read;
  char _name[32];
  char _id[17];
  char _extension[8];
  FILE *_file;
  FILE *_metadata_file;
  time_t _seconds;
  bool _first_metadata_entry;

  int close_object();
};

class StorageBucket
{
public:
  StorageBucket(StorageManager *storage_manager, const char *name);
  ~StorageBucket();

  StorageObject create_object(const char *name, const char *extension);

  inline StorageManager *storage_manager() { return _sm; }
  inline const char *name() { return _name; }
  inline const char *base_path() { return _base_path; }
  inline const char *data_path() { return _data_path; }
  inline const char *index_path() { return _index_path; }
  inline const char *metadata_path() { return _metadata_path; }

  int init();

private:
  StorageManager *_sm;
  const char *_name;
  char _base_path[64];
  char _index_path[64];
  char _metadata_path[64];
  char _data_path[64];
};

class StorageManager
{
public:
  StorageManager(RandomFactory *rf, FileSystem *fs, const char *fs_mount_path, const char *storage_root);
  ~StorageManager();
  int init();
  StorageBucket get_bucket(const char *name);
  inline FileSystem *filesystem() { return _fs; }
  inline const char *fs_mount_path() { return _fs_mount_path; }
  inline const char *storage_root() { return _root; }
  inline RandomFactory * random_factory() { return _rf; }


private:
  RandomFactory *_rf;
  FileSystem *_fs;
  const char *_fs_mount_path;
  const char *_root;
};
} // namespace aware

#endif