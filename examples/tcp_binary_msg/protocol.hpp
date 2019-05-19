#pragma once

#include <stdint.h>
//definition client <-> server protocol struct

enum {
    PROTO_MAX_BODY_SIZE = 4<<10 ,
    PROTO_MAGIC = 0x33445566,
    PROTO_ADD_CMD_REQ=1,
    PROTO_ADD_CMD_RSP=2,
};

struct proto_head_t
{
    bool is_valid_head() const {return magic == PROTO_MAGIC and pkglen <= PROTO_MAX_BODY_SIZE;}
    uint32_t get_pkg_len() const {return pkglen;}

    uint32_t magic;
    uint32_t pkglen; //head + body
    uint8_t  cmd;
    uint8_t  ver;

} __attribute__((packed));



//client -> server
//exection + operation
struct proto_add_request_t
{
    proto_head_t head;
    uint32_t pair_count;
    struct {
        int32_t num1;
        int32_t num2;
    } pair[0];

}__attribute__((packed));

//return add result
struct proto_add_response_t
{
    proto_head_t head;
    uint32_t count;
    int64_t resluts[0];
}__attribute__((packed));
