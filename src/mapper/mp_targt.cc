#include "mapper/mp_targt.h"

#include <string.h>

#include "art.h"
#include "game.h"
#include "map.h"
#include "mapper/mp_proto.h"
#include "memory.h"
#include "proto.h"
#include "window_manager_private.h"

namespace fallout {

#define TARGET_DAT "target.dat"

typedef struct TargetNode {
    TargetSubNode subnode;
    struct TargetNode* next;
} TargetNode;

typedef struct TargetList {
    TargetNode* tail;
    int count;
    int field_8;
} TargetList;

// 0x53F354
static char default_target_path_base[] = "\\fallout2\\dev\\proto\\";

// 0x559CC4
static TargetList targetlist = { NULL, 0, 0 };

// 0x559CD0
static char* target_path_base = default_target_path_base;

// 0x559DBC
static bool tgt_overriden = false;

// 0x49B2F0
void target_override_protection()
{
    char* name;

    tgt_overriden = true;

    name = getenv("MAIL_NAME");
    if (name != NULL) {
        // NOTE: Unsafe, backing buffer is 32 byte max.
        strcpy(proto_builder_name, strupr(name));
    } else {
        strcpy(proto_builder_name, "UNKNOWN");
    }
}

// 0x49B2F0
bool target_overriden()
{
    return tgt_overriden;
}

// 0x49B34C
void target_make_path(char* path, int pid)
{
    if (_cd_path_base[0] != '\0' && _cd_path_base[1] == ':') {
        strncpy(path, _cd_path_base, 2);
        strcat(path, target_path_base);
    } else {
        strcpy(path, target_path_base);
    }

    if (pid != -1) {
        strcat(path, artGetObjectTypeName(PID_TYPE(pid)));
    }
}

// 0x49B424
int target_init()
{
    // TODO: Incomplete.

    return 0;
}

// 0x49B434
int target_exit()
{
    // TODO: Incomplete.

    return 0;
}

// 0x49B454
int target_header_save()
{
    char path[COMPAT_MAX_PATH];
    FILE* stream;

    target_make_path(path, -1);
    strcat(path, TARGET_DAT);

    stream = fopen(path, "wb");
    if (stream == NULL) {
        return -1;
    }

    if (fwrite(&targetlist, sizeof(targetlist), 1, stream) != 1) {
        // FIXME: Leaking `stream`.
        return -1;
    }

    fclose(stream);
    return 0;
}

// 0x49B4E8
int target_header_load()
{
    char path[COMPAT_MAX_PATH];
    FILE* stream;

    target_make_path(path, -1);
    strcat(path, TARGET_DAT);

    stream = fopen(path, "rb");
    if (stream == NULL) {
        return -1;
    }

    if (fread(&targetlist, sizeof(targetlist), 1, stream) != 1) {
        // FIXME: Leaking `stream`.
        return -1;
    }

    targetlist.tail = NULL;
    targetlist.count = 0;

    fclose(stream);
    return 0;
}

// 0x49B58C
int target_save(int pid)
{
    char path[COMPAT_MAX_PATH];
    size_t len;
    char* extension;
    FILE* stream;
    TargetSubNode* subnode;

    if (target_ptr(pid, &subnode) == -1) {
        return -1;
    }

    target_make_path(path, pid);

    len = strlen(path);
    path[len] = '\\';
    _proto_list_str(pid, path + len + 1);

    extension = strchr(path + len + 1, '.');
    if (extension != NULL) {
        strcpy(extension + 1, "tgt");
    } else {
        strcat(path, ".tgt");
    }

    stream = fopen(path, "wb");
    if (stream == NULL) {
        return -1;
    }

    while (subnode != NULL) {
        fwrite(subnode, sizeof(TargetSubNode), 1, stream);
        subnode = subnode->next;
    }

    fclose(stream);

    return 0;
}

// 0x49B6BC
int target_load(int pid, TargetSubNode** subnode_ptr)
{
    char path[COMPAT_MAX_PATH];
    size_t len;
    char* extension;
    FILE* stream;
    TargetSubNode* subnode;

    target_make_path(path, pid);

    len = strlen(path);
    path[len] = '\\';
    _proto_list_str(pid, path + len + 1);

    extension = strchr(path + len + 1, '.');
    if (extension != NULL) {
        strcpy(extension + 1, "tgt");
    } else {
        strcat(path, ".tgt");
    }

    stream = fopen(path, "rb");
    if (stream == NULL) {
        *subnode_ptr = NULL;
        return -1;
    }

    if (target_find_free_subnode(&subnode) == -1) {
        *subnode_ptr = NULL;
        // FIXME: Leaks `stream`.
        return -1;
    }

    fread(subnode, sizeof(TargetSubNode), 1, stream);

    *subnode_ptr = subnode;

    while (subnode->next != NULL) {
        subnode->next = (TargetSubNode*)internal_malloc(sizeof(TargetSubNode));
        if (subnode->next == NULL) {
            // FIXME: Leaks `stream`.
            return -1;
        }

        subnode = subnode->next;
        fread(subnode, sizeof(TargetSubNode), 1, stream);
    }

    fclose(stream);

    return 0;
}

// 0x49B9C0
int target_find_free_subnode(TargetSubNode** subnode_ptr)
{
    TargetNode* node = (TargetNode*)internal_malloc(sizeof(TargetNode));
    if (node == NULL) {
        *subnode_ptr = NULL;
        return -1;
    }

    *subnode_ptr = &(node->subnode);

    node->subnode.field_0 = -1;
    node->subnode.next = NULL;
    node->next = targetlist.tail;

    targetlist.tail = node;
    targetlist.count++;

    return 0;
}

// 0x49BBD4
int target_remove(int pid)
{
    TargetNode* node;
    TargetSubNode* subnode;
    TargetSubNode* subnode_next;

    node = targetlist.tail;
    while (node != NULL) {
        if (node->subnode.field_0 == pid) {
            break;
        }
        node = node->next;
    }

    if (node == NULL) {
        return -1;
    }

    subnode = node->subnode.next;

    if (node == targetlist.tail) {
        targetlist.tail = targetlist.tail->next;
    }

    internal_free(node);

    while (subnode != NULL) {
        subnode_next = subnode->next;
        internal_free(subnode);
        subnode = subnode_next;
    }

    return 0;
}

// 0x49BCBC
int target_remove_all()
{
    TargetNode* node;
    TargetNode* node_next;
    TargetSubNode* subnode;
    TargetSubNode* subnode_next;

    node = targetlist.tail;
    targetlist.tail = NULL;

    while (node != NULL) {
        node_next = node->next;

        subnode = node->subnode.next;
        while (subnode != NULL) {
            subnode_next = subnode->next;
            internal_free(subnode);
            subnode = subnode_next;
        }

        internal_free(node);
        node = node_next;
    }

    return 0;
}

// 0x49BD00
int target_ptr(int pid, TargetSubNode** subnode_ptr)
{
    TargetNode* node = targetlist.tail;
    while (node != NULL) {
        if (node->subnode.field_0 == pid) {
            *subnode_ptr = &(node->subnode);
            return 0;
        }
    }

    return target_load(pid, subnode_ptr);
}

// 0x49BD38
int target_tid_ptr(int pid, int tid, TargetSubNode** subnode_ptr)
{
    TargetSubNode* subnode;
    if (target_ptr(pid, &subnode) == -1) {
        return -1;
    }

    while (subnode != NULL) {
        if (subnode->field_4 == tid) {
            *subnode_ptr = subnode;
            return 0;
        }
    }

    return -1;
}

// 0x49BD98
int pick_rot()
{
    int value;
    win_get_num_i(&value,
        -1,
        5,
        false,
        "Rotation",
        100,
        100);
    return value;
}

// 0x49BDD0
int target_pick_global_var(int* value_ptr)
{
    int value;
    int rc;

    if (gGameGlobalVarsLength == 0) {
        return -1;
    }

    rc = win_get_num_i(&value,
        0,
        gGameGlobalVarsLength - 1,
        false,
        "Global Variable Index #:",
        100,
        100);
    if (rc == -1) {
        return -1;
    }

    *value_ptr = value;
    return 0;
}

// 0x49BE20
int target_pick_map_var(int* value_ptr)
{
    int value;
    int rc;

    if (gMapGlobalVarsLength == 0) {
        return -1;
    }

    rc = win_get_num_i(&value,
        0,
        gMapGlobalVarsLength - 1,
        false,
        "Map Variable Index #:",
        100,
        100);
    if (rc == -1) {
        return -1;
    }

    *value_ptr = value;
    return 0;
}

// 0x49BE70
int target_pick_local_var(int* value_ptr)
{
    int value;
    int rc;

    if (gMapLocalVarsLength == 0) {
        return -1;
    }

    rc = win_get_num_i(&value,
        0,
        gMapLocalVarsLength - 1,
        false,
        "Local Variable Index #:",
        100,
        100);
    if (rc == -1) {
        return -1;
    }

    *value_ptr = value;
    return 0;
}

} // namespace fallout
